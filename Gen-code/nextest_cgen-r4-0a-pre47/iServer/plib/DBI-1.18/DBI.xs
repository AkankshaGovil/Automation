/* $Id: DBI.xs,v 1.1.2.1 2002/12/18 19:35:09 santosh Exp $
 *
 * Copyright (c) 1994, 1995, 1996, 1997  Tim Bunce  England.
 *
 * See COPYRIGHT section in DBI.pm for usage and distribution rights.
 */

#define IN_DBI_XS 1	/* see DBIXS.h */

#include "DBIXS.h"	/* DBI public interface for DBD's written in C	*/

#define MY_VERSION "DBI(" XS_VERSION ")"

#if (defined USE_THREADS || defined PERL_CAPI || defined PERL_OBJECT)
static int xsbypass = 0;	/* disable XSUB->XSUB shortcut		*/
#else
static int xsbypass = 1;	/* enable XSUB->XSUB shortcut		*/
#endif

#define DBI_MAGIC '~'

/* Retrieve imp_??h_t struct from handle magic.	*/
/* Cast increases required alignment of target type	*/
/* not a problem since we created the pointers anyway.	*/
#define DBIh_FROM_MG(mg) ((imp_xxh_t*)(void*)SvPVX((mg)->mg_obj))

/* If the tests fail with errors about 'setlinebuf' then try	*/
/* deleting the lines in the block below except the setvbuf one	*/
#ifndef PerlIO_setlinebuf
#ifdef HAS_SETLINEBUF
#define PerlIO_setlinebuf(f)        setlinebuf(f)
#else
#ifndef USE_PERLIO
#define PerlIO_setlinebuf(f)        setvbuf(f, Nullch, _IOLBF, 0)
#endif
#endif
#endif

#ifndef PerlIO
#  define PerlIO FILE
#  define PerlIO_printf fprintf
#  define PerlIO_stderr() stderr
#  define PerlIO_close(f) fclose(f)
#  define PerlIO_open(f,m) fopen(f,m)
#  define PerlIO_flush(f) Fflush(f)
#  define PerlIO_puts(f,s) fputs(s,f)
#endif

#ifndef aTHX_
#  define aTHX_
#  define aTHXo_
#  define CopFILEGV(cop) cop->cop_filegv
#  define CopLINE(cop)   cop->cop_line
#  define get_sv perl_get_sv
#endif


static imp_xxh_t *dbih_getcom	   _((SV *h));
static void       dbih_clearcom	   _((imp_xxh_t *imp_xxh));
static int	  dbih_logmsg	   _((SV *h, imp_xxh_t *imp_xxh, char *fmt, ...));
static SV	 *dbih_make_com	   _((SV *parent_h, char *imp_class, STRLEN imp_size, STRLEN extra));
static SV	 *dbih_make_fdsv   _((SV *sth, char *imp_class, STRLEN imp_size, char *col_name));
static AV        *dbih_get_fbav	   _((imp_sth_t *imp_sth));
static SV	 *dbih_event	   _((SV *h, char *name, SV*, SV*));
static int        dbih_set_attr_k  _((SV *h, SV *keysv, int dbikey, SV *valuesv));
static SV        *dbih_get_attr_k  _((SV *h, SV *keysv, int dbikey));

static int	quote_type _((int sql_type, int p, int s, int *base_type, void *v));
static int	dbi_hash _((char *string, long i));
char *neatsvpv _((SV *sv, STRLEN maxlen));

DBISTATE_DECLARE;

struct imp_drh_st { dbih_drc_t com; };
struct imp_dbh_st { dbih_dbc_t com; };
struct imp_sth_st { dbih_stc_t com; };
struct imp_fdh_st { dbih_fdc_t com; };


/* Internal Method Attributes (attached to dispatch methods when installed) */

typedef struct dbi_ima_st {
    short minargs;
    short maxargs;
    short hidearg;
    char *usage_msg;
    U16   flags;
    U16   trace_level;
} dbi_ima_t;

#define IMA_HAS_USAGE		0x0001	/* check parameter usage	*/
#define IMA_FUNC_REDIRECT	0x0002	/* is $h->func(..., "method")	*/
#define IMA_KEEP_ERR		0x0004	/* don't reset err & errstr	*/
#define IMA_spare	 	0x0008	/* */
#define IMA_NO_TAINT_IN   	0x0010	/* don't check for tainted args	*/
#define IMA_NO_TAINT_OUT   	0x0020	/* don't taint results		*/

#define DBIc_STATE_adjust(imp_xxh, state)				 \
    (SvOK(state)	/* SQLSTATE is implemented by driver   */	 \
	? (strEQ(SvPV(state,lna),"00000") ? &sv_no : sv_mortalcopy(state))\
	: (SvTRUE(DBIc_ERR(imp_xxh))					 \
	    ? sv_2mortal(newSVpv("S1000",5)) /* General error	*/	 \
	    : &sv_no)			/* Success ("00000")	*/	 \
    )

#define DBI_LAST_HANDLE		g_dbi_last_h /* special fake inner handle	*/
#define DBI_LAST_HANDLE_PARENT	((SV*)DBIc_PARENT_H(DBIh_COM(DBI_LAST_HANDLE)))
#define DBI_IS_LAST_HANDLE(h)	(SvRVx(DBI_LAST_HANDLE) == SvRV(h))
#define DBI_SET_LAST_HANDLE(h)	(SvRVx(DBI_LAST_HANDLE) =  SvRV(h))
#define DBI_UNSET_LAST_HANDLE	(SvRVx(DBI_LAST_HANDLE) =  &sv_undef)
#define DBI_LAST_HANDLE_OK	(SvRVx(DBI_LAST_HANDLE) != &sv_undef )

#ifdef PERL_LONG_MAX
#define MAX_LongReadLen PERL_LONG_MAX
#else
#define MAX_LongReadLen 2147483647L
#endif

#ifdef DBI_USE_THREADS
static char *dbi_build_opt = "-thread";
#else
static char *dbi_build_opt = "-nothread";
#endif


/* --- make DBI safe for multiple perl interpreters --- */
/*     Contributed by Murray Nesbitt of ActiveState     */
typedef struct {
    SV   *dbi_last_h;
    dbistate_t* dbi_state;
} PERINTERP_t;

#if defined(MULTIPLICITY) || defined(PERL_OBJECT) || defined(PERL_CAPI)

#   if (PATCHLEVEL == 4) && (SUBVERSION < 68)
#     define dPERINTERP_SV                                     \
        SV *perinterp_sv = get_sv(MY_VERSION, FALSE)
#   else
#     define dPERINTERP_SV                                     \
        SV *perinterp_sv = *hv_fetch(PL_modglobal, MY_VERSION, \
                                 sizeof(MY_VERSION)-1, TRUE)
#   endif

#   define dPERINTERP_PTR(T,name)                            \
	T name = (T)(perinterp_sv && SvIOK(perinterp_sv)     \
                 ? (T)SvIVX(perinterp_sv) : NULL)
#   define dPERINTERP                                        \
	dPERINTERP_SV; dPERINTERP_PTR(PERINTERP_t *, PERINTERP)
#   define INIT_PERINTERP \
	dPERINTERP;                                          \
	Newz(0,PERINTERP,1,PERINTERP_t);                     \
	sv_setiv(perinterp_sv, (IV)PERINTERP)

#   undef DBIS
#   define DBIS			(PERINTERP->dbi_state)

#else
    static PERINTERP_t Interp;
#   define dPERINTERP typedef int _interp_DBI_dummy
#   define PERINTERP (&Interp)
#   define INIT_PERINTERP 
#endif

#define g_dbi_last_h            (PERINTERP->dbi_last_h)

/* --- */

static void
#ifdef CAN_PROTOTYPE
check_version(char *name, int dbis_cv, int dbis_cs, int need_dbixs_cv, int drc_s, 
	int dbc_s, int stc_s, int fdc_s)
#else
check_version(name, dbis_cv, dbis_cs, need_dbixs_cv, drc_s, dbc_s, stc_s, fdc_s)
    char *name;
    int dbis_cv, dbis_cs, need_dbixs_cv;
    int drc_s, dbc_s, stc_s, fdc_s;
#endif
{
    dPERINTERP;
    char *msg = "you probably need to rebuild the DBD driver (or possibly the DBI)";
    if (dbis_cv != DBISTATE_VERSION || dbis_cs != sizeof(*DBIS))
	croak("DBI/DBD internal version mismatch (DBI is v%d/s%d, DBD %s expected v%d/s%d) %s.\n",
	    DBISTATE_VERSION, sizeof(*DBIS), name, dbis_cv, dbis_cs, msg);
    /* Catch structure size changes - We should probably force a recompile if the DBI	*/
    /* runtime version is different from the build time. That would be harsh but safe.	*/
    if (drc_s != sizeof(dbih_drc_t) || dbc_s != sizeof(dbih_dbc_t) ||
	stc_s != sizeof(dbih_stc_t) || fdc_s != sizeof(dbih_fdc_t) )
	    croak("%s (dr:%d/%d, db:%d/%d, st:%d/%d, fd:%d/%d), %s.\n",
		"DBI/DBD internal structure mismatch",
		drc_s, sizeof(dbih_drc_t), dbc_s, sizeof(dbih_dbc_t),
		stc_s, sizeof(dbih_stc_t), fdc_s, sizeof(dbih_fdc_t), msg);
}

static void
dbi_bootinit()
{
INIT_PERINTERP;

    Newz(dummy, DBIS, 1, dbistate_t);

#ifdef DBI_USE_THREADS
    Newz(1, DBIS->mutex, 1, dbi_mutex);
    MUTEX_INIT(DBIS->mutex);
#endif

    /* store version and size so we can spot DBI/DBD version mismatch	*/
    DBIS->check_version = check_version;
    DBIS->version = DBISTATE_VERSION;
    DBIS->size    = sizeof(*DBIS);
    DBIS->xs_version = DBIXS_VERSION;

	/* store some other critical values */
    DBIS->debug	 = 0;
    DBIS->logmsg = dbih_logmsg;
    DBIS->logfp	 = PerlIO_stderr();
    DBIS->neatsvpvlen = get_sv("DBI::neat_maxlen", GV_ADDMULTI);
    sv_setiv(DBIS->neatsvpvlen, 400);

    /* publish address of dbistate so dynaloaded DBD's can find it	*/
    sv_setiv(get_sv(DBISTATE_PERLNAME,1), (IV)DBIS);

    DBISTATE_INIT; /* check DBD code to set DBIS from DBISTATE_PERLNAME	*/

    /* store some function pointers so DBD's can call our functions	*/
    DBIS->getcom      = dbih_getcom;
    DBIS->clearcom    = dbih_clearcom;
    DBIS->event       = dbih_event;
    DBIS->set_attr_k  = dbih_set_attr_k;
    DBIS->get_attr_k  = dbih_get_attr_k;
    DBIS->get_fbav    = dbih_get_fbav;
    DBIS->make_fdsv   = dbih_make_fdsv;
    DBIS->neat_svpv   = neatsvpv;
    DBIS->bind_as_num = quote_type;
    DBIS->hash        = dbi_hash;

    /* Remember the last handle used. BEWARE! Sneaky stuff here!	*/
    /* We want a handle reference but we don't want to increment	*/
    /* the handle's reference count and we don't want perl to try	*/
    /* to destroy it during global destruction. Take care!		*/
    g_dbi_last_h  = newRV(&sv_undef);
    SvROK_off(g_dbi_last_h);	/* so sv_clean_objs() won't destroy it	*/
    DBI_UNSET_LAST_HANDLE;	/* ensure setup the correct way		*/

    /* trick to avoid 'possible typo' warnings	*/
    gv_fetchpv("DBI::state",  GV_ADDMULTI, SVt_PV);
    gv_fetchpv("DBI::err",    GV_ADDMULTI, SVt_PV);
    gv_fetchpv("DBI::errstr", GV_ADDMULTI, SVt_PV);
    gv_fetchpv("DBI::lasth",  GV_ADDMULTI, SVt_PV);
    gv_fetchpv("DBI::rows",   GV_ADDMULTI, SVt_PV);
}


/* ----------------------------------------------------------------- */
/* Utility functions                                                 */


char *
neatsvpv(sv, maxlen) /* return a tidy ascii value, for debugging only */
    SV * sv;
    STRLEN maxlen;
{
    dPERINTERP;
    STRLEN len;
    SV *nsv = Nullsv;
    SV *infosv = Nullsv;
    char *v;

    /* We take care not to alter the supplied sv in any way at all.	*/

    if (!sv)
	return "Null!";				/* should never happen	*/

    /* try to do the right thing with magical values			*/
    if (SvMAGICAL(sv)) {
	if (DBIS->debug >= 3) {	/* add magic details to help debugging	*/
	    MAGIC* mg;
	    infosv = sv_2mortal(newSVpv(" (magic-",0));
	    if (SvSMAGICAL(sv)) sv_catpvn(infosv,"s",1);
	    if (SvGMAGICAL(sv)) sv_catpvn(infosv,"g",1);
	    if (SvRMAGICAL(sv)) sv_catpvn(infosv,"r",1);
	    sv_catpvn(infosv,":",1);
	    for (mg = SvMAGIC(sv); mg; mg = mg->mg_moremagic)
		sv_catpvn(infosv, &mg->mg_type, 1);
	    sv_catpvn(infosv, ")", 1);
	}
	if (SvGMAGICAL(sv))
	    mg_get(sv);		/* trigger magic to FETCH the value	*/
    }

    if (!SvOK(sv)) {
	if (SvTYPE(sv) >= SVt_PVAV)
	    return sv_reftype(sv,0);	/* raw AV/HV etc, not via a ref	*/
	if (!infosv)
	    return "undef";
	sv_insert(infosv, 0,0, "undef",5);
	return SvPVX(infosv);
    }

    if (SvNIOK(sv)) {	  /* is a numeric value - so no surrounding quotes	*/
	char buf[48];
	if (SvPOK(sv)) {  /* already has string version of the value, so use it	*/
	    v = SvPV(sv,len);
	    if (len == 0) { v="''"; len=2; } /* catch &sv_no style special case	*/
	    if (!infosv)
		return v;
	    sv_insert(infosv, 0,0, v, len);
	    return SvPVX(infosv);
	}
	/* we don't use SvPV here since we don't want to alter sv in _any_ way	*/
	if (SvIOK(sv))
	     sprintf(buf, "%ld", (long)SvIVX(sv));
	else sprintf(buf, "%g",  (double)SvNVX(sv));
	nsv = sv_2mortal(newSVpv(buf, 0));
	if (infosv)
	    sv_catsv(nsv, infosv);
	return SvPVX(nsv);
    }

    nsv = sv_newmortal();
    sv_upgrade(nsv, SVt_PV);

    if (SvROK(sv)) {
	if (!SvAMAGIC(sv))	/* (un-amagic'd) refs get no special treatment	*/
	    v = SvPV(sv,len);
	else {
	    /* handle Overload magic refs */
	    SvAMAGIC_off(sv);	/* should really be done via local scoping */
	    v = SvPV(sv,len);	/* XXX how does this relate to SvGMAGIC?   */
	    SvAMAGIC_on(sv);
	}
	sv_setpvn(nsv, v, len);
	if (infosv)
	    sv_catsv(nsv, infosv);
	return SvPV(nsv, len);
    }

    if (SvPOK(sv))		/* usual simple string case		   */
	v = SvPV(sv,len);
    else			/* handles all else via sv_2pv()	   */
	v = SvPV(sv,len);	/* XXX how does this relate to SvGMAGIC?   */

    /* for strings we limit the length and translate codes	*/
    if (maxlen == 0)
	maxlen = SvIV(DBIS->neatsvpvlen);
    if (maxlen < 6)			/* handle daft values	*/
	maxlen = 6;
    maxlen -= 2;			/* account for quotes	*/
    if (len > maxlen) {
	SvGROW(nsv, (int)(1+maxlen+4+1));
	sv_setpvn(nsv, "'", 1);
	sv_catpvn(nsv, v, maxlen-3);	/* account for three dots */
	sv_catpvn(nsv, "...'", 4);
    } else {
	SvGROW(nsv, (int)(1+len+1+1));
	sv_setpvn(nsv, "'", 1);
	sv_catpvn(nsv, v, len);
	sv_catpvn(nsv, "'", 1);
    }
    if (infosv)
	sv_catsv(nsv, infosv);
    v = SvPV(nsv, len);
    while(len-- > 0) { /* cleanup string (map control chars to ascii etc) */
	char c = v[len] & 0x7F;	/* ignore top bit for multinational chars */
	if (!isPRINT(c) && !isSPACE(c))
	    v[len] = '.';
    }
    return v;
}


static char *
mkvname(stash, item, uplevel)	/* construct a variable name	*/
    HV *stash;
    char *item;
    int uplevel;
{
    STRLEN lna;
    SV *sv = sv_newmortal();
    sv_setpv(sv, HvNAME(stash));
    if(uplevel) {
	while(SvCUR(sv) && *SvEND(sv)!=':')
	    --SvCUR(sv);
	if (SvCUR(sv))
	    --SvCUR(sv);
    }
    sv_catpv(sv, "::");
    sv_catpv(sv, item);
    return SvPV(sv, lna);
}


static int
dbi_hash(key, i)
    char *key;
    long i; /* spare */
{
    STRLEN klen = strlen(key);
    U32 hash = 0;
    while (klen--)
        hash = hash * 33 + *key++;
    hash &= 0x7FFFFFFF;	/* limit to 31 bits		*/
    hash |= 0x40000000;	/* set bit 31			*/
    return -(int)hash;	/* return negative int	*/
}


static int
#ifdef I_STDARG
dbih_logmsg(SV *h, imp_xxh_t *imp_xxh, char *fmt, ...)
#else
dbih_logmsg(h, imp_xxh, fmt, va_alist)
SV *h;
imp_xxh_t *imp_xxh;
char *fmt;
va_dcl
#endif
{
    dPERINTERP;
    va_list args;
#ifdef I_STDARG
    va_start(args, fmt);
#else
    va_start(args);
#endif
    (void) PerlIO_vprintf(DBIS->logfp, fmt, args);
    va_end(args);
    return 1;
}


static int
set_trace_file(file)
    SV *file;
{
    dPERINTERP;
    STRLEN lna;
    char *filename;
    PerlIO *fp;
    if (!file)			/* no arg == no change			*/
	return 0;
    if (!SvOK(file)) {		/* undef arg == reset back to stderr	*/
	if (DBILOGFP != PerlIO_stderr())
	    PerlIO_close(DBILOGFP);
	DBILOGFP = PerlIO_stderr();
	return 1;
    }
    filename = SvPV(file, lna);
    fp = PerlIO_open(filename, "a+");
    if (fp == Nullfp) {
	warn("Can't open trace file %s: %s", filename, Strerror(errno));
	return 0;
    }
    else {
	if (DBILOGFP != PerlIO_stderr())
	    PerlIO_close(DBILOGFP);
	DBILOGFP = fp;
	/* if this line causes your compiler or linker to choke	*/
	/* then just comment it out, it's not essential.	*/
	PerlIO_setlinebuf(fp);	/* force line buffered output */
	return 1;
    }
}


static SV *
dbih_inner(orv, what)	/* convert outer to inner handle else croak */
    SV *orv;         	/* ref of outer hash */
    char *what;		/* error msg, NULL=no croak and return NULL */
{
    dPERINTERP;
    MAGIC *mg;
    SV *ohv;		/* outer HV after derefing the RV	*/
    SV *hrv;		/* dni inner handle RV-to-HV		*/

    /* enable a raw HV (not ref-to-HV) to be passed in, eg DBIc_MY_H */
    ohv = SvROK(orv) ? SvRV(orv) : orv;

    if (!ohv || SvTYPE(ohv) != SVt_PVHV) {
	if (!what)
	    return NULL;
	if (!SvOK(orv))
	    croak("%s given an undefined handle %s",
		what, "(perhaps returned from a previous call which failed)");
	croak("%s handle %s is not a DBI handle", what, neatsvpv(orv,0));
    }
    if (!SvMAGICAL(ohv)) {
	sv_dump(orv);
	croak("%s handle %s is not a DBI handle (has no magic)",
		what, neatsvpv(orv,0));
    }

    if ( (mg=mg_find(ohv,'P')) == NULL) {	/* hash tie magic	*/
	/* maybe it's already an inner handle... */
	if (mg_find(ohv, DBI_MAGIC) == NULL) {
	    if (!what)
		return NULL;
	    croak("%s handle %s is not a valid DBI handle",
		what, neatsvpv(orv,0));
	}
	hrv = orv; /* was already a DBI handle inner hash */
    }
    else {
	hrv = mg->mg_obj;  /* inner hash of tie */
    }

    /* extra checks if being paranoid */
    if (DBIS->debug && (!SvROK(hrv) || SvTYPE(SvRV(hrv)) != SVt_PVHV)) {
	if (!what)
	    return NULL;
	croak("panic: %s inner handle %s is not a hash ref",
		what, neatsvpv(hrv,0));
    }
    return hrv;
}


#ifdef DBI_USE_THREADS
static void
dbi_cond_signal(imp_xxh)
    imp_xxh_t *imp_xxh;
{
    if (!imp_xxh || !DBIc_THR_COND(imp_xxh))
	return;
    DBI_LOCK;
    if (PL_threadnum && DBIS->debug >= 4)
	PerlIO_printf(DBILOGFP,"    .. thread %lu leaving %s\n",
		DBIc_THR_USER(imp_xxh), HvNAME(DBIc_IMP_STASH(imp_xxh)));
    DBIc_THR_USER(imp_xxh) = DBIc_THR_USER_NONE; /* free for other thread to enter */
    COND_SIGNAL(DBIc_THR_COND(imp_xxh));
    DBI_UNLOCK;
}
#endif




/* --------------------------------------------------------------------	*/
/* Functions to manage a DBI handle (magic and attributes etc).     	*/

static imp_xxh_t *
dbih_getcom(hrv)	/* Get com struct for handle. Must be fast.	*/
    SV *hrv;
{
    dPERINTERP;
    MAGIC *mg;
    SV *sv;

    /* important and quick sanity check (esp non-'safe' Oraperl)	*/
    if (!SvROK(hrv)			/* must at least be a ref */
	&& hrv != DBI_LAST_HANDLE	/* special for var::FETCH */) {
	sv_dump(hrv);
	croak("Invalid DBI handle %s", neatsvpv(hrv,0));
    }

    sv = SvRV(hrv);

    /* Short cut for common case. We assume that a magic var always	*/
    /* has magic and that DBI_MAGIC, if present, will be the first.	*/
    if (SvRMAGICAL(sv) && (mg=SvMAGIC(sv))->mg_type == DBI_MAGIC) {
	/* ignore 'cast increases required alignment' warning	*/
	return DBIh_FROM_MG(mg);
    }

    /* Validate handle (convert outer to inner if required)	*/
    hrv = dbih_inner(hrv, "dbih_getcom");
    mg  = mg_find(SvRV(hrv), DBI_MAGIC);

    /* ignore 'cast increases required alignment' warning	*/
    return DBIh_FROM_MG(mg);
}


static SV *
dbih_setup_attrib(h, attrib, parent, read_only)
    SV *h;
    char *attrib;
    SV *parent;
    int read_only;
{
    dPERINTERP;
    STRLEN len = strlen(attrib);

    SV *asv = *hv_fetch((HV*)SvRV(h), attrib, len, 1);
    /* we assume that we won't have any existing 'undef' attribures here */
    /* (or, alternately, we take undef to mean 'copy from parent')	 */
    if (!SvOK(asv)) {	/* attribute doesn't already exists (the common case) */
	SV **psv;
	if (!parent || !SvROK(parent))
	    croak("dbih_setup_attrib(%s): %s not set and no parent supplied",
		    neatsvpv(h,0), attrib);
	psv = hv_fetch((HV*)SvRV(parent), attrib, len, 0);
	if (!psv)
	    croak("dbih_setup_attrib(%s): %s not set and not in parent",
		    neatsvpv(h,0), attrib);
	sv_setsv(asv, *psv); /* copy attribute from parent to handle */
    }
    if (DBIS->debug >= 5) {
	PerlIO_printf(DBILOGFP,"    dbih_setup_attrib(%s, %s, %s)",
	    neatsvpv(h,0), attrib, neatsvpv(parent,0));
	if (SvOK(asv))
	     PerlIO_printf(DBILOGFP," %s (already defined)\n", neatsvpv(asv,0));
	else PerlIO_printf(DBILOGFP," %s (copied from parent)\n", neatsvpv(asv,0));
    }
    if (read_only)
	SvREADONLY_on(asv);
    return asv;
}


static SV *
dbih_make_fdsv(sth, imp_class, imp_size, col_name)
    SV *sth;
    char *imp_class;		/* eg "DBD::Driver::fd" */
    STRLEN imp_size;
    char *col_name;
{
    dPERINTERP;
    STRLEN cn_len = strlen(col_name);
    imp_fdh_t *imp_fdh;
    SV *fdsv;
    if (imp_size < sizeof(imp_fdh_t) || cn_len<10 || strNE("::fd",&col_name[cn_len-4]))
	croak("panic: dbih_makefdsv %s '%s' imp_size %d invalid",
		imp_class, col_name, imp_size);
    if (DBIS->debug >= 3)
	PerlIO_printf(DBILOGFP,"    dbih_make_fdsv(%s, %s, %d, '%s')\n",
		neatsvpv(sth,0), imp_class, imp_size, col_name);
    fdsv = dbih_make_com(sth, imp_class, imp_size, cn_len+2);
    imp_fdh = (imp_fdh_t*)(void*)SvPVX(fdsv);
    imp_fdh->com.col_name = ((char*)imp_fdh) + imp_size;
    strcpy(imp_fdh->com.col_name, col_name);
    return fdsv;
}


static SV *
dbih_make_com(parent_h, imp_class, imp_size, extra)
    SV *parent_h;
    char *imp_class;		/* eg "DBD::Driver::db" */
    STRLEN imp_size;
    STRLEN extra;
{
    dPERINTERP;
    char *errmsg = "Can't make DBI com handle for %s: %s";
    HV *imp_stash;
    SV *dbih_imp_sv;
    imp_xxh_t *imp;

    if ( (imp_stash = gv_stashpv(imp_class, FALSE)) == NULL)
        croak(errmsg, imp_class, "unknown package");

    if (imp_size == 0) {
	/* get size of structure to allocate for common and imp specific data   */
	char *imp_size_name = mkvname(imp_stash, "imp_data_size", 0);
	imp_size = SvIV(get_sv(imp_size_name, 0x05));
	if (imp_size == 0) {
	    imp_size = sizeof(imp_sth_t);
	    if (sizeof(imp_dbh_t) > imp_size)
		imp_size = sizeof(imp_dbh_t);
	    if (sizeof(imp_drh_t) > imp_size)
		imp_size = sizeof(imp_drh_t);
	    imp_size += 64;
	}
    }

    if (DBIS->debug >= 3)
	PerlIO_printf(DBILOGFP,"    dbih_make_com(%s, %s, %d)\n",
		neatsvpv(parent_h,0), imp_class, imp_size);

    dbih_imp_sv = newSV(imp_size);

    imp = (imp_xxh_t*)(void*)SvPVX(dbih_imp_sv);
    memzero((char*)imp, imp_size);

    DBIc_IMP_STASH(imp) = imp_stash;

    if (!parent_h) {		/* only a driver (drh) has no parent	*/
	DBIc_PARENT_H(imp)    = (HV*)&sv_undef;
	DBIc_PARENT_COM(imp)  = NULL;
	DBIc_TYPE(imp)	      = DBIt_DR;
	DBIc_on(imp,DBIcf_WARN		/* set only here, children inherit	*/
		   |DBIcf_ACTIVE	/* drivers are 'Active' by default	*/
		   |DBIcf_AutoCommit	/* advisory, driver must manage this	*/
	);
#ifdef DBI_USE_THREADS
	Newz(1, DBIc_THR_COND(imp), 1, dbi_cond);
	COND_INIT(DBIc_THR_COND(imp));
#endif
    } else {		
	imp_xxh_t *parent_com = DBIh_COM(parent_h);
	DBIc_PARENT_H(imp)    = (HV*)SvREFCNT_inc(parent_h); /* ensure it lives	*/
	DBIc_PARENT_COM(imp)  = parent_com;	 	/* shortcut for speed	*/
	DBIc_TYPE(imp)	      = DBIc_TYPE(parent_com) + 1;
	DBIc_FLAGS(imp)       = DBIc_FLAGS(parent_com) & ~DBIcf_INHERITMASK;
	DBIc_THR_COND(imp)    = DBIc_THR_COND(parent_com);
	++DBIc_KIDS(parent_com);
    }
    /* handles come into life with DBIc_THR_USER set to DBIc_THR_USER_NONE	*/
    /* because DBIc_THR_USER indicates which thread has 'entered the DBI'	*/
    /* on this handle and, at this point, none has.				*/
    DBIc_THR_USER(imp) = DBIc_THR_USER_NONE;

    if (DBIc_TYPE(imp) == DBIt_ST) {
	imp_sth_t *imp_sth = (imp_sth_t*)imp;
	DBIc_ROW_COUNT(imp_sth)  = -1;
    }

    DBIc_COMSET_on(imp);	/* common data now set up		*/

    /* The implementor should DBIc_IMPSET_on(imp) when setting up	*/
    /* any private data which will need clearing/freeing later.		*/

    return dbih_imp_sv;
}


static void
dbih_setup_handle(orv, imp_class, parent, imp_datasv)
    SV *orv;         /* ref of outer hash */
    char *imp_class;
    SV *parent;
    SV *imp_datasv;
{
    dPERINTERP;
    SV *h;
    char *errmsg = "Can't setup DBI handle of %s to %s: %s";
    SV *dbih_imp_sv;
    SV *dbih_imp_rv;
    char imp_mem_name[300];
    HV  *imp_mem_stash;
    imp_xxh_t *imp;
    imp_xxh_t *parent_imp;

    h      = dbih_inner(orv, "dbih_setup_handle");
    parent = dbih_inner(parent, NULL);	/* check parent valid (& inner)	*/

    if (DBIS->debug >= 3)
	PerlIO_printf(DBILOGFP,"    dbih_setup_handle(%s=>%s, %s, %lx, %s)\n",
	    neatsvpv(orv,0), neatsvpv(h,0), imp_class, (long)parent, neatsvpv(imp_datasv,0));

    if (mg_find(SvRV(h), DBI_MAGIC) != NULL)
	croak(errmsg, neatsvpv(orv,0), imp_class, "already a DBI (or ~magic) handle");

    strcpy(imp_mem_name, imp_class);
    strcat(imp_mem_name, "_mem");
    if ( (imp_mem_stash = gv_stashpv(imp_mem_name, FALSE)) == NULL) 
        croak(errmsg, neatsvpv(orv,0), imp_mem_name, "unknown _mem package");

    DBI_LOCK;

    dbih_imp_sv = dbih_make_com(parent, imp_class, 0, 0);
    imp = (imp_xxh_t*)(void*)SvPVX(dbih_imp_sv);

    dbih_imp_rv = newRV(dbih_imp_sv);	/* just needed for sv_bless */
    sv_bless(dbih_imp_rv, imp_mem_stash);
    sv_free(dbih_imp_rv);

    DBIc_MY_H(imp) = (HV*)SvRV(orv);	/* take _copy_ of pointer, not new ref	*/
    DBIc_IMP_DATA(imp) = (imp_datasv) ? newSVsv(imp_datasv) : &sv_undef;

    if (DBIc_TYPE(imp) <= DBIt_ST) {
	SV **tmp_svp;
	parent_imp = (parent) ? DBIh_COM(parent) : NULL;
	/* Copy some attributes from parent if not defined locally and	*/
	/* also take address of attributes for speed of direct access.	*/
	/* parent is null for drh, in which case h must hold the values	*/
#define COPY_PARENT(name,ro) SvREFCNT_inc(dbih_setup_attrib(h, (name), parent, ro))
#define DBIc_ATTR(imp, f) _imp2com(imp, attr.f)
	/* XXX we should validate that these are the right type (refs etc)	*/
	DBIc_ATTR(imp, Err)      = COPY_PARENT("Err",1);	/* scalar ref	*/
	DBIc_ATTR(imp, State)    = COPY_PARENT("State",1);	/* scalar ref	*/
	DBIc_ATTR(imp, Errstr)   = COPY_PARENT("Errstr",1);	/* scalar ref	*/
	DBIc_ATTR(imp, Handlers) = COPY_PARENT("Handlers",1);	/* array ref	*/
	DBIc_ATTR(imp, Debug)    = COPY_PARENT("Debug",0);	/* scalar (int)	*/
	if (parent)
	     DBIc_LongReadLen(imp) = DBIc_LongReadLen(parent_imp);
	else DBIc_LongReadLen(imp) = DBIc_LongReadLen_init;

	switch (DBIc_TYPE(imp)) {
	case DBIt_DB:
	    /* pre-load Driver attribute */
	    hv_store((HV*)SvRV(h), "Driver", 6, newRV((SV*)DBIc_MY_H(parent_imp)), 0);
	    break;
	case DBIt_ST:
	    /* pre-load Database attribute */
	    hv_store((HV*)SvRV(h), "Database", 8, newRV((SV*)DBIc_MY_H(parent_imp)), 0);
	    /* copy (alias) Statement from the sth up into the dbh	*/
	    tmp_svp = hv_fetch((HV*)SvRV(h), "Statement", 9, 0);
	    if (tmp_svp && SvOK(*tmp_svp)) /* can be null eg Oracle ref cursors	*/
		hv_store((HV*)SvRV(parent), "Statement", 9, SvREFCNT_inc(*tmp_svp), 0);
	    break;
	}
    }

    /* Use DBI magic on inner handle to carry handle attributes 	*/
    sv_magic(SvRV(h), dbih_imp_sv, DBI_MAGIC, Nullch, 0);
    SvREFCNT_dec(dbih_imp_sv);	/* since sv_magic() incremented it	*/
    SvRMAGICAL_on(SvRV(h));	/* so magic gets sv_clear'd ok		*/

    DBI_SET_LAST_HANDLE(h);

	DBI_UNLOCK;
}


static void
dbih_dumpcom(imp_xxh, msg, level)
    imp_xxh_t *imp_xxh;
    char *msg;
    int level;
{
    dPERINTERP;
    SV *flags = sv_2mortal(newSVpv("",0));
    STRLEN lna;
    char *pad = "      ";
    if (!msg)
	msg = "dbih_dumpcom";
    PerlIO_printf(DBILOGFP,"    %s (h 0x%lx, com 0x%lx):\n", msg,
	    (IV)DBIc_MY_H(imp_xxh), (IV)imp_xxh);
    if (DBIc_COMSET(imp_xxh))			sv_catpv(flags,"COMSET ");
    if (DBIc_IMPSET(imp_xxh))			sv_catpv(flags,"IMPSET ");
    if (DBIc_ACTIVE(imp_xxh))			sv_catpv(flags,"Active ");
    if (DBIc_WARN(imp_xxh))			sv_catpv(flags,"Warn ");
    if (DBIc_COMPAT(imp_xxh))			sv_catpv(flags,"CompatMode ");
    if (DBIc_is(imp_xxh, DBIcf_ChopBlanks))	sv_catpv(flags,"ChopBlanks ");
    if (DBIc_is(imp_xxh, DBIcf_RaiseError))	sv_catpv(flags,"RaiseError ");
    if (DBIc_is(imp_xxh, DBIcf_PrintError))	sv_catpv(flags,"PrintError ");
    if (DBIc_is(imp_xxh, DBIcf_ShowErrorStatement))	sv_catpv(flags,"ShowErrorStatement ");
    if (DBIc_is(imp_xxh, DBIcf_AutoCommit))	sv_catpv(flags,"AutoCommit ");
    if (DBIc_is(imp_xxh, DBIcf_LongTruncOk))	sv_catpv(flags,"LongTruncOk ");
    if (DBIc_is(imp_xxh, DBIcf_MultiThread))	sv_catpv(flags,"MultiThread ");
    if (DBIc_is(imp_xxh, DBIcf_Taint))		sv_catpv(flags,"Taint ");
    PerlIO_printf(DBILOGFP,"%s FLAGS 0x%lx: %s\n", pad, (long)DBIc_FLAGS(imp_xxh), SvPV(flags,lna));
    PerlIO_printf(DBILOGFP,"%s TYPE %d\n",	pad, DBIc_TYPE(imp_xxh));
    PerlIO_printf(DBILOGFP,"%s PARENT %s\n",	pad, neatsvpv((SV*)DBIc_PARENT_H(imp_xxh),0));
    PerlIO_printf(DBILOGFP,"%s KIDS %ld (%ld active)\n", pad,
		    (long)DBIc_KIDS(imp_xxh), (long)DBIc_ACTIVE_KIDS(imp_xxh));
    PerlIO_printf(DBILOGFP,"%s IMP_DATA %s in '%s'\n", pad,
	    neatsvpv(DBIc_IMP_DATA(imp_xxh),0), HvNAME(DBIc_IMP_STASH(imp_xxh)));
    if (DBIc_LongReadLen(imp_xxh) != DBIc_LongReadLen_init)
	PerlIO_printf(DBILOGFP,"%s LongReadLen %ld\n", pad, (long)DBIc_LongReadLen(imp_xxh));
#ifdef DBI_USE_THREADS
    if (DBIc_THR_COND(imp_xxh)) {
	PerlIO_printf(DBILOGFP,"%s thread cond var 0x%lx", pad, (long)DBIc_THR_COND(imp_xxh));
	PerlIO_printf(DBILOGFP,", tid %lu\n", (unsigned long)DBIc_THR_USER(imp_xxh));
    }
#endif

    if (DBIc_TYPE(imp_xxh) <= DBIt_DB) {
	imp_dbh_t *imp_dbh = (imp_dbh_t*)imp_xxh;
	if (DBIc_CACHED_KIDS(imp_dbh))
	    PerlIO_printf(DBILOGFP,"%s CachedKids %d\n", pad, (int)HvKEYS(DBIc_CACHED_KIDS(imp_dbh)));
    }
    if (DBIc_TYPE(imp_xxh) == DBIt_ST) {
	imp_sth_t *imp_sth = (imp_sth_t*)imp_xxh;
	PerlIO_printf(DBILOGFP,"%s NUM_OF_FIELDS %d\n", pad, DBIc_NUM_FIELDS(imp_sth));
	PerlIO_printf(DBILOGFP,"%s NUM_OF_PARAMS %d\n", pad, DBIc_NUM_PARAMS(imp_sth));
    }
    if (level > 0) {
        SV* value;
	char *key;
	I32   keylen;
	SV *inner = dbih_inner(DBIc_MY_H(imp_xxh), msg);
	PerlIO_printf(DBILOGFP,"%s cached attributes:\n", pad);
	while ( (value = hv_iternextsv((HV*)SvRV(inner), &key, &keylen)) ) {
	    PerlIO_printf(DBILOGFP,"%s   '%s' => %s\n", pad, key, neatsvpv(value,0));
	}
    }
}


static void
dbih_clearcom(imp_xxh)
    imp_xxh_t *imp_xxh;
{
    dPERINTERP;
    dTHR;
    int dump = FALSE;
    int auto_dump = (DBIS->debug >= 6);

    /* Note that we're very much on our own here. DBIc_MY_H(imp_xxh) almost	*/
    /* certainly points to memory which has been freed. Don't use it!		*/

    /* --- pre-clearing sanity checks --- */

    if (!DBIc_COMSET(imp_xxh)) {	/* should never happen	*/
	dbih_dumpcom(imp_xxh, "dbih_clearcom: DBI handle already cleared", 0);
	return;
    }

    if (auto_dump)
	dbih_dumpcom(imp_xxh,"dbih_clearcom", 0);

    if (!dirty) {
	if (DBIc_TYPE(imp_xxh) <= DBIt_DB) {
	    imp_dbh_t *imp_dbh = (imp_dbh_t*)imp_xxh; /* works for DRH also */
	    if (DBIc_CACHED_KIDS(imp_dbh)) {
		warn("DBI handle cleared whilst still holding %d cached kids",
			HvKEYS(DBIc_CACHED_KIDS(imp_dbh)) );
		SvREFCNT_dec(DBIc_CACHED_KIDS(imp_dbh)); /* may recurse */
		DBIc_CACHED_KIDS(imp_dbh) = Nullhv;
	    }
	}

	if (DBIc_ACTIVE(imp_xxh)) {	/* bad news		*/
	    warn("DBI handle cleared whilst still active");
	    DBIc_ACTIVE_off(imp_xxh);
	    dump = TRUE;
	}

	/* check that the implementor has done its own housekeeping	*/
	if (DBIc_IMPSET(imp_xxh)) {
	    warn("DBI Handle has uncleared implementors data");
	    dump = TRUE;
	}

	if (DBIc_KIDS(imp_xxh)) {
	    warn("DBI Handle has %d uncleared child handles",
		    (int)DBIc_KIDS(imp_xxh));
	    dump = TRUE;
	}
    }

    if (dump && !auto_dump) /* else was already dumped above */
	dbih_dumpcom(imp_xxh, "dbih_clearcom", 0);

    /* --- pre-clearing adjustments --- */

    if (DBIc_PARENT_COM(imp_xxh) && !dirty) {
	--DBIc_KIDS(DBIc_PARENT_COM(imp_xxh));
    }

    /* --- clear fields (may invoke object destructors) ---	*/

    if (DBIc_TYPE(imp_xxh) == DBIt_ST) {
	imp_sth_t *imp_sth = (imp_sth_t*)imp_xxh;
	if (DBIc_FIELDS_AV(imp_sth));
	    sv_free((SV*)DBIc_FIELDS_AV(imp_sth));
    }

    sv_free(DBIc_IMP_DATA(imp_xxh));	/* do this first	*/
    if (DBIc_TYPE(imp_xxh) <= DBIt_ST) {	/* DBIt_FD doesn't have attr */
	sv_free(_imp2com(imp_xxh, attr.Handlers));
	sv_free(_imp2com(imp_xxh, attr.Debug));
	sv_free(_imp2com(imp_xxh, attr.State));
	sv_free(_imp2com(imp_xxh, attr.Err));
	sv_free(_imp2com(imp_xxh, attr.Errstr));
    }

#ifdef DBI_USE_THREADS
    if (DBIc_TYPE(imp_xxh) == DBIt_DR && DBIc_THR_COND(imp_xxh)) {
	COND_DESTROY(DBIc_THR_COND(imp_xxh));
	Safefree(DBIc_THR_COND(imp_xxh));
    }
#endif

    sv_free((SV*)DBIc_PARENT_H(imp_xxh));	/* do this last		*/

    DBIc_COMSET_off(imp_xxh);

    if (DBIS->debug >= 4)
	PerlIO_printf(DBILOGFP,"    dbih_clearcom 0x%lx (com 0x%lx, type %d) done.\n\n",
		(IV)DBIc_MY_H(imp_xxh), (IV)imp_xxh, DBIc_TYPE(imp_xxh));
}


/* --- Functions for handling field buffer arrays ---		*/

static AV *
dbih_setup_fbav(imp_sth)
    imp_sth_t *imp_sth;
{
    dPERINTERP;
    int i;
    AV *av;

   if (DBIc_FIELDS_AV(imp_sth))
	return DBIc_FIELDS_AV(imp_sth);

    i = DBIc_NUM_FIELDS(imp_sth);
    if (i <= 0 || i > 32000)	/* trap obvious mistakes */
	croak("dbih_setup_fbav: invalid number of fields: %d%s",
		i, ", NUM_OF_FIELDS attribute probably not set right");
    av = newAV();
    if (DBIS->debug >= 3)
	PerlIO_printf(DBILOGFP,"    dbih_setup_fbav for %d fields => 0x%lx\n",
		    i, (long)av);
    /* load array with writeable SV's. Do this backwards so	*/
    /* the array only gets extended once.			*/
    while(i--)			/* field 1 stored at index 0	*/
	av_store(av, i, newSV(0));
    SvREADONLY_on(av);		/* protect against shift @$row etc */
    /* row_count will need to be manually reset by the driver if the	*/
    /* sth is re-executed (since this code won't get rerun)		*/
    DBIc_ROW_COUNT(imp_sth) = 0;
    DBIc_FIELDS_AV(imp_sth) = av;
    return av;
}


static AV *
dbih_get_fbav(imp_sth)	/* Called once per-fetch: must be fast	*/
    imp_sth_t *imp_sth;
{
    AV *av;

    if (DBIc_TYPE(imp_sth) != DBIt_ST)
	croak("dbih_get_fbav: bad handle type: %d", DBIc_TYPE(imp_sth));

    if ( (av = DBIc_FIELDS_AV(imp_sth)) == Nullav)
	av = dbih_setup_fbav(imp_sth);

    if (DBIc_is(imp_sth, DBIcf_Taint)) {
	dTHR;
	TAINT;	/* affects sv_setsv()'s called within same perl statement */
    }

    /* XXX fancy stuff to happen here later (re scrolling etc)	*/
    ++DBIc_ROW_COUNT(imp_sth);
    return av;
}


static int
dbih_sth_bind_col(sth, col, ref, attribs)
    SV *sth;
    SV *col;
    SV *ref;
    SV *attribs;
{
    dPERINTERP;
    D_imp_sth(sth);
    AV *av;
    int idx = SvIV(col);
    int fields = DBIc_NUM_FIELDS(imp_sth);

    if (fields <= 0) {
	attribs = attribs;	/* avoid 'unused variable' warning	*/
	croak("Statement has no result columns to bind%s",
	    DBIc_ACTIVE(imp_sth)
		? "" : " (perhaps you need to call execute first)");
    }

    if (!SvROK(ref) || SvTYPE(SvRV(ref)) >= SVt_PVBM)	/* XXX LV */
	croak("Can't %s->bind_col(%s, %s,...), need a reference to a scalar",
		neatsvpv(sth,0), neatsvpv(col,0), neatsvpv(ref,0));

    if ( (av = DBIc_FIELDS_AV(imp_sth)) == Nullav)
	av = dbih_setup_fbav(imp_sth);

    if (DBIS->debug >= 3)
	PerlIO_printf(DBILOGFP,"    dbih_sth_bind_col %s => %s\n",
		neatsvpv(col,0), neatsvpv(ref,0));

    if (idx < 1 || idx > fields)
	croak("bind_col: column %d is not a valid column (1..%d)",
			idx, fields);

    /* use supplied scalar as storage for this column */
    SvREADONLY_off(av);
    av_store(av, idx-1, SvREFCNT_inc(SvRV(ref)) );
    SvREADONLY_on(av);
    return 1;
}


static int
quote_type(sql_type, p, s, t, v)	/* don't use - in a state of flux	*/
    int sql_type;
    int p, s;		/* not used (yet?), pass as zero */
	int *t;
    void *v;
{
    /* Returns true if type should be bound as a number else	*/
    /* false implying that binding as a string should be okay.	*/
    /* The true value is either SQL_INTEGER or SQL_DOUBLE which	*/
    /* can be used as a hint if desired.			*/
    switch(sql_type) {
    case SQL_INTEGER:
    case SQL_SMALLINT:
    case SQL_TINYINT:
    case SQL_BIGINT:
	return 0;
    case SQL_FLOAT:
    case SQL_REAL:
    case SQL_DOUBLE:
	return 0;
    case SQL_NUMERIC:
    case SQL_DECIMAL:
	return 0;	/* bind as string to attempt to retain precision */
    }
    return 1;
}


/* --- Generic Handle Attributes (for all handle types) ---	*/

static int
dbih_set_attr_k(h, keysv, dbikey, valuesv)	/* XXX split into dr/db/st funcs */
    SV *h;
    SV *keysv;
    int dbikey;
    SV *valuesv;
{
    dPERINTERP;
    dTHR;
    D_imp_xxh(h);
    STRLEN keylen;
    char  *key = SvPV(keysv, keylen);
    int    htype = DBIc_TYPE(imp_xxh);
    int    on = (SvTRUE(valuesv));
    int    internal = 1; /* DBIh_IN_PERL_DBD(imp_xxh); -- for DBD's in perl */
    int    cacheit = 0;

    if (DBIS->debug >= 3)
	PerlIO_printf(DBILOGFP,"    STORE %s %s => %s\n",
		neatsvpv(h,0), neatsvpv(keysv,0), neatsvpv(valuesv,0));

    if (strEQ(key, "CompatMode")) {
	(on) ? DBIc_COMPAT_on(imp_xxh) : DBIc_COMPAT_off(imp_xxh);
    }
    else if (strEQ(key, "Warn")) {
	(on) ? DBIc_WARN_on(imp_xxh) : DBIc_WARN_off(imp_xxh);
    }
    else if (internal && strEQ(key, "Active")) {
	if (on) {
	    D_imp_sth(h);
	    DBIc_ACTIVE_on(imp_xxh);
	    /* for pure-perl drivers on second and subsequent	*/
	    /* execute()'s, else row count keeps rising.	*/
	    if (htype==DBIt_ST && DBIc_FIELDS_AV(imp_sth))
		DBIc_ROW_COUNT(imp_sth) = 0;
	}
	else {
	    DBIc_ACTIVE_off(imp_xxh);
	}
    }
    else if (strEQ(key, "InactiveDestroy")) {
	(on) ? DBIc_IADESTROY_on(imp_xxh) : DBIc_IADESTROY_off(imp_xxh);
    }
    else if (strEQ(key, "RootClass")) {
	cacheit = 1;	/* just save it */
    }
    else if (strEQ(key, "RowCacheSize")) {
	cacheit = 0;	/* ignore it */
    }
    else if (strEQ(key, "ChopBlanks")) {
	DBIc_set(imp_xxh,DBIcf_ChopBlanks, on);
    }
    else if (strEQ(key, "LongReadLen")) {
	if (SvNV(valuesv) < 0 || SvNV(valuesv) > MAX_LongReadLen)
	    croak("Can't set LongReadLen < 0 or > %ld",MAX_LongReadLen);
	DBIc_LongReadLen(imp_xxh) = SvIV(valuesv);
    }
    else if (strEQ(key, "LongTruncOk")) {
	DBIc_set(imp_xxh,DBIcf_LongTruncOk, on);
    }
    else if (strEQ(key, "RaiseError")) {
	DBIc_set(imp_xxh,DBIcf_RaiseError, on);
    }
    else if (strEQ(key, "PrintError")) {
	DBIc_set(imp_xxh,DBIcf_PrintError, on);
    }
    else if (strEQ(key, "ShowErrorStatement")) {
	DBIc_set(imp_xxh,DBIcf_ShowErrorStatement, on);
    }
    else if (strEQ(key, "MultiThread") && internal) {
	/* here to allow pure-perl drivers to set MultiThread */
	DBIc_set(imp_xxh,DBIcf_MultiThread, on);
	if (on && DBIc_WARN(imp_xxh)) {
	    warn("MultiThread support not yet implemented in DBI");
	}
    }
    else if (strEQ(key, "Taint")) {
	DBIc_set(imp_xxh,DBIcf_Taint, on);
    }
    else if (htype<=DBIt_DB && keylen==10 && strEQ(key, "CachedKids")) {
	D_imp_dbh(h);	/* XXX also for drh */
	if (DBIc_CACHED_KIDS(imp_dbh)) {
	    SvREFCNT_dec(DBIc_CACHED_KIDS(imp_dbh));
	    DBIc_CACHED_KIDS(imp_dbh) = Nullhv;
	}
	if (SvROK(valuesv)) {
	    DBIc_CACHED_KIDS(imp_dbh) = (HV*)SvREFCNT_inc(SvRV(valuesv));
	}
    }
    else if (htype<=DBIt_DB && strEQ(key, "AutoCommit")) {
	/* The driver should have intercepted this and handled it.	*/
	croak("DBD driver has not implemented the AutoCommit attribute");
	/* DBIc_set(imp_xxh,DBIcf_AutoCommit, on); */
    }
    else if (htype==DBIt_ST && strEQ(key, "NUM_OF_FIELDS")) {
	D_imp_sth(h);
	if (DBIc_NUM_FIELDS(imp_sth) > 0)	/* don't change NUM_FIELDS! */
	    croak("NUM_OF_FIELDS already set to %d", DBIc_NUM_FIELDS(imp_sth));
	DBIc_NUM_FIELDS(imp_sth) = SvIV(valuesv);
	cacheit = 1;
    }
    else if (htype==DBIt_ST && strEQ(key, "NUM_OF_PARAMS")) {
	D_imp_sth(h);
	DBIc_NUM_PARAMS(imp_sth) = SvIV(valuesv);
	cacheit = 1;
    }
    else {	/* XXX should really be an event ? */
	if (isUPPER(*key)) {
	    char *hint = "";
	    if (strEQ(key, "NUM_FIELDS"))
		hint = " (perhaps you meant NUM_OF_FIELDS)";
	    croak("Can't set %s->{%s}: unrecognised attribute%s",
		    neatsvpv(h,0), key, hint);
	}
	/* Allow private_* attributes to be stored in the cache.	*/
	/* This is designed to make life easier for people subclassing	*/
	/* the DBI classes and may be of use to simple perl DBD's.	*/
	if (strnNE(key,"private_",8) && strnNE(key,"dbd_",4))
	    return FALSE;
	cacheit = 1;
    }
    if (cacheit) {
	/* old code
	SV **svp = hv_fetch((HV*)SvRV(h), key, keylen, 1);
	SvREFCNT_dec(*svp);
	*svp = SvREFCNT_inc(valuesv);
	*/
	hv_store((HV*)SvRV(h), key, keylen, newSVsv(valuesv), 0);
    }
    return TRUE;
}


static SV *
dbih_get_attr_k(h, keysv, dbikey)			/* XXX split into dr/db/st funcs */
    SV *h;
    SV *keysv;
    int dbikey;
{
    dPERINTERP;
    /* dTHR; not currently needed */
    D_imp_xxh(h);
    STRLEN lna;
    STRLEN keylen;
    char  *key = SvPV(keysv, keylen);
    int    htype = DBIc_TYPE(imp_xxh);
    SV	*valuesv = &sv_undef;
    int    cacheit = FALSE;
    char *p;
    int i;
    SV	*sv;
    SV	**svp;

    /* DBI quick_FETCH will service some requests (e.g., cached values)	*/

    /* XXX needs to be split into separate dr/db/st funcs	*/
    /* XXX probably needs some form of hashing->switch lookup	*/

    if (htype==DBIt_ST && keylen==7 && strnEQ(key, "NAME_", 5)
	&& (key[5]=='u' || key[5]=='l') && key[6] == 'c'
    ) {
	D_imp_sth(h);
	AV *name = NULL;

	/* fetch from tied outer handle to trigger FETCH magic	*/
	svp = hv_fetch((HV*)DBIc_MY_H(imp_sth), "NAME",4, FALSE);
	sv = (svp) ? *svp : &sv_undef;
	if (SvGMAGICAL(sv))	/* resolve the magic		*/
	    mg_get(sv);		/* can core dump in 5.004	*/
	name = (AV*)SvRV(sv);

	if (sv && name) {
	    int up = (key[5] == 'u');
	    AV *av = newAV();
	    i = DBIc_NUM_FIELDS(imp_sth);
	    assert(i == AvFILL(name)+1);
	    while(--i >= 0) {
		sv = newSVsv(AvARRAY(name)[i]);
		for (p = SvPV(sv,lna); p && *p; ++p) {
#ifdef toUPPER_LC
		    *p = (up) ? toUPPER_LC(*p) : toLOWER_LC(*p);
#else
		    *p = (up) ? toUPPER(*p) : toLOWER(*p);
#endif
		}
		av_store(av, i, sv);
	    }
	    valuesv = newRV(sv_2mortal((SV*)av));
	    cacheit = TRUE;	/* can't change */
	}
    }
    else if (htype==DBIt_ST && keylen==13 && strEQ(key, "NUM_OF_FIELDS")) {
	D_imp_sth(h);
	valuesv = newSViv(DBIc_NUM_FIELDS(imp_sth));
	if (DBIc_NUM_FIELDS(imp_sth) > 0)
	    cacheit = TRUE;	/* can't change once set */
    }
    else if (htype==DBIt_ST && keylen==13 && strEQ(key, "NUM_OF_PARAMS")) {
	D_imp_sth(h);
	valuesv = newSViv(DBIc_NUM_PARAMS(imp_sth));
	cacheit = TRUE;	/* can't change */
    }
    else if (keylen==12 && strEQ(key, "RowCacheSize")) {
	valuesv = &sv_undef;
    }
    else if (keylen==4 && strEQ(key, "Type")) {
	char *type;
	switch(htype) {
	case DBIt_DR: type = "dr"; break;
	case DBIt_DB: type = "db"; break;
	case DBIt_ST: type = "st"; break;
	case DBIt_FD: type = "fd"; break;
	default:      type = "??";
	}
	valuesv = newSVpv(type,0);
	cacheit = TRUE;	/* can't change */
    }
    /* these are here but are, sadly, not called because not-preloading	*/
    /* them into the handle attrib cache caused wierdness in t/proxy.t	*/
    /* that I never got to the bottom of. One day maybe.		*/
    else if (  (htype==DBIt_ST && keylen==8 && strEQ(key, "Database"))
	    || (htype==DBIt_DB && keylen==6 && strEQ(key, "Driver"))
    ) {
	D_imp_from_child(imp_dbh, imp_dbh_t, imp_xxh);
	valuesv = newRV((SV*)DBIc_MY_H(imp_dbh));
	cacheit = TRUE;	/* XXX may change? */
    }
    else if (htype==DBIt_ST && keylen==11 && strEQ(key, "RowsInCache")) {
	valuesv = &sv_undef;
    }
    else if (keylen==6  && strEQ(key, "Active")) {
	valuesv = boolSV(DBIc_ACTIVE(imp_xxh));
    }
    else if (keylen==4  && strEQ(key, "Kids")) {
	valuesv = newSViv(DBIc_KIDS(imp_xxh));
    }
    else if (keylen==10  && strEQ(key, "ActiveKids")) {
	valuesv = newSViv(DBIc_ACTIVE_KIDS(imp_xxh));
    }
    else if (keylen==4  && strEQ(key, "Warn")) {
	valuesv = boolSV(DBIc_WARN(imp_xxh));
    }
    else if (htype<=DBIt_DB && keylen==10  && strEQ(key, "CachedKids")) {
	D_imp_dbh(h);
	HV *hv = DBIc_CACHED_KIDS(imp_dbh);
	valuesv = (hv) ? newRV((SV*)hv) : &sv_undef;
    }
    else if (keylen==10 && strEQ(key, "CompatMode")) {
	valuesv = boolSV(DBIc_COMPAT(imp_xxh));
    }
    else if (keylen==15 && strEQ(key, "InactiveDestroy")) {
	valuesv = boolSV(DBIc_IADESTROY(imp_xxh));
    }
    else if (keylen==10 && strEQ(key, "ChopBlanks")) {
	valuesv = boolSV(DBIc_has(imp_xxh,DBIcf_ChopBlanks));
    }
    else if (keylen==11 && strEQ(key, "LongReadLen")) {
	valuesv = newSVnv((double)DBIc_LongReadLen(imp_xxh));
    }
    else if (keylen==11 && strEQ(key, "LongTruncOk")) {
	valuesv = boolSV(DBIc_has(imp_xxh,DBIcf_LongTruncOk));
    }
    else if (keylen==10 && strEQ(key, "RaiseError")) {
	valuesv = boolSV(DBIc_has(imp_xxh,DBIcf_RaiseError));
    }
    else if (keylen==10 && strEQ(key, "PrintError")) {
	valuesv = boolSV(DBIc_has(imp_xxh,DBIcf_PrintError));
    }
    else if (keylen==18 && strEQ(key, "ShowErrorStatement")) {
	valuesv = boolSV(DBIc_has(imp_xxh,DBIcf_ShowErrorStatement));
    }
    else if (keylen==10 && strEQ(key, "MultiThread")) {
	valuesv = boolSV(DBIc_has(imp_xxh,DBIcf_MultiThread));
    }
    else if (keylen==5  && strEQ(key, "Taint")) {
	valuesv = boolSV(DBIc_has(imp_xxh,DBIcf_Taint));
    }
    else if (htype<=DBIt_DB && keylen==10 && strEQ(key, "AutoCommit")) {
	/* The driver should have intercepted this and handled it.	*/
	croak("DBD driver has not implemented the AutoCommit attribute");
	/* valuesv = boolSV(DBIc_has(imp_xxh,DBIcf_AutoCommit)); */
    }
    else {	/* finally check the actual hash just in case	*/
	svp = hv_fetch((HV*)SvRV(h), key, keylen, FALSE);
	if (svp)
	    valuesv = *svp;
	else if (isUPPER(*key))
	    croak("Can't get %s->{%s}: unrecognised attribute",neatsvpv(h,0),key);
	else
	    valuesv = &sv_undef;	/* dbd_*, private_* etc	*/
    }
    if (cacheit) {
	svp = hv_fetch((HV*)SvRV(h), key, keylen, TRUE);
	sv = *svp;
if (DBIS->debug >= 9) {
warn("cacheit %d %s: old: sv%p='%s', new: '%s'", getpid(), key, sv, neatsvpv(sv,0),neatsvpv(valuesv,0));
}
	*svp = SvREFCNT_inc(valuesv);
	sv_free(sv);
    }
    if (DBIS->debug >= 3)
	PerlIO_printf(DBILOGFP,"    .. FETCH %s %s = %s%s\n", neatsvpv(h,0),
	    neatsvpv(keysv,0), neatsvpv(valuesv,0), cacheit?" (cached)":"");
    if (valuesv == &sv_yes || valuesv == &sv_no || valuesv == &sv_undef)
	return valuesv;	/* no need to mortalize yes or no */
    return sv_2mortal(valuesv);
}


static SV *			/* find attrib in handle or its parents	*/
dbih_find_attr(h, keysv, copydown, spare)
    SV *h;
    SV *keysv;
    int copydown;		/* copydown attribute down from parent	*/
    int spare;
{
    D_imp_xxh(h);
    SV *ph;
    STRLEN keylen;
    char  *key = SvPV(keysv, keylen);
    SV *valuesv;
    SV **svp = hv_fetch((HV*)SvRV(h), key, keylen, FALSE);
    if (svp)
	valuesv = *svp;
    else
    if (!SvOK(ph=(SV*)DBIc_PARENT_H(imp_xxh)))
	valuesv = Nullsv;
    else /* recurse up */
	valuesv = dbih_find_attr(ph, keysv, copydown, spare);
    if (valuesv && copydown)
	hv_store((HV*)SvRV(h), key, keylen, newSVsv(valuesv), 0);
    return valuesv;	/* return actual sv, not a mortalised copy	*/
}


/* --------------------------------------------------------------------	*/
/* Functions implementing Error and Event Handling.                   	*/


static SV *
dbih_event(hrv, evtype, a1, a2)
    SV *hrv;    /* ref to inner hash */
    char *evtype;
    SV *a1, *a2;
{
    dPERINTERP;
    dSP;
    D_imp_xxh(hrv);
    /* We arrive here via DBIh_EVENT* macros (see DBIXS.h) called from	*/
    /* DBD driver C code OR $h->event() method (in DBD::_::common)	*/
    /* If an array of handlers is defined then call them in reverse	*/
    /* order until one returns true */

    AV *handlers_av = (AV*)DBIc_HANDLERS(imp_xxh);
    SV *status = &sv_undef;
    SV *evtype_sv = Nullsv;
    int i;

    if (DBIS->debug >= 3)
	PerlIO_printf(DBILOGFP,"    %s EVENT %s %s on %s\n",
	    evtype, neatsvpv(a1,0), neatsvpv(a2,0), neatsvpv(hrv,0));

    if (SvTYPE(handlers_av) != SVt_PVAV) {	/* must be \@ or undef	*/
	if (SvOK(handlers_av))
	    warn("%s->{Handlers} (%s) is not an array reference or undef",
		neatsvpv(hrv,0), neatsvpv((SV*)handlers_av,0));
	return &sv_undef;
    }

    i = av_len(handlers_av) + 1;
    while(--i >= 0) {	/* Call each handler in turn	*/
	SV *sv = *av_fetch(handlers_av, i, 1);
	if (!evtype_sv)
	    evtype_sv = sv_2mortal(newSVpv(evtype,0));
	/* call handler */
/* XXX probably need a better way. Note that DBD::ExampleP uses this! */
	PUSHMARK(sp);
	EXTEND(sp, 4);
	PUSHs(hrv);
	PUSHs(evtype_sv);
	if (SvOK(a2) || SvOK(a1)) { PUSHs(a1); }
	if (SvOK(a2))             { PUSHs(a2); }
	PUTBACK;
	perl_call_sv(sv, G_SCALAR);	/* NOTE: May longjmp (die)	*/
	SPAGAIN;
	status = POPs;
	PUTBACK;
	if (DBIS->debug >= 3)
	    PerlIO_printf(DBILOGFP, "    %s handler%d %s returned %s\n",
		    neatsvpv(hrv,0), i,
		    neatsvpv(sv,0), neatsvpv(status,0));
	if (SvTRUE(status))	/* event was handled so		*/
	    break;		/* don't call any more handlers	*/
    }

    return status;	/* true if event was 'handled' (not defined)	*/
}


/* ----------------------------------------------------------------- */
/* Functions implementing DBI dispatcher shortcuts.                  */

/* This function implements the DBI FETCH shortcut mechanism.
Any handle attribute FETCH will come to this function (see dispatch).
This function returns either an SV for the fetched value or NULL.
If NULL is returned the dispatcher will call the full FETCH method.
 - If key =~ /^_/ then return NULL (so driver can hide private attribs)
 - If the key does not exist return NULL (may be a virtual attribute).
 - If value is not a ref then return value (the main shortcut).
 - If it's a CODE ref then run CODE and return it's result value!
     (actually it sets a flag so dispatch will run code for us).
 - If it's a ref to a CODE ref then return the CODE ref
     (an escape mechanism to allow real code refs to be stored).
 - Else return the ref.
*/

static SV * 
quick_FETCH(hrv, keysv, imp_msv)
    SV *hrv;	/* ref to inner hash */
    SV *keysv;
    SV **imp_msv;	/* Code GV or CV */
{
    void *tmp;
    SV *sv;
    STRLEN lp;
    char *key = SvPV(keysv,lp);
    int type;
    if (*key == '_')
	return NULL;	/* never quick_FETCH a '_' prefixed attribute */
    if ( (tmp = hv_fetch((HV*)SvRV(hrv), key, lp, 0)) == NULL)
	return NULL;	/* does not exist */
    sv = *(SV**)tmp;
    if (!SvROK(sv))	/* return value of all non-refs directly	*/
	return sv;	/* this is the main shortcut	*/
    if ( (type=SvTYPE(SvRV(sv))) == SVt_RV
	&& SvTYPE(SvRV(SvRV(sv))) == SVt_PVCV)
	return SvRV(sv); /* return deref if ref to CODE ref */
    if (type != SVt_PVCV)
	return sv;	 /* return non-code refs */
    *imp_msv = (SV*)SvRV(sv); /* tell dispatch() to execute this code instead */
    return NULL;
}


static void
log_where(logfp, trace_level)
    PerlIO *logfp;
    int trace_level;
{
    dTHR;
    char *file, *sep;
    STRLEN len;
    if (CopLINE(curcop)) {
	file = SvPV(GvSV(CopFILEGV(curcop)), len);
	if (trace_level<=4) {
	    if ( (sep=strrchr(file,'/')) || (sep=strrchr(file,'\\')))
		file = sep+1;
	}
        PerlIO_printf(logfp," at %s line %ld", file, (long)CopLINE(curcop));
    }
    PerlIO_printf(logfp,"%s.", dirty ? " during global destruction" : "");
}


static void
clear_cached_kids(h, imp_xxh, meth_name, trace_level)
    SV *h;
    imp_xxh_t *imp_xxh;
    char *meth_name;
    int trace_level;
{
    dPERINTERP;
    if (DBIc_TYPE(imp_xxh) <= DBIt_DB && DBIc_CACHED_KIDS((imp_drh_t*)imp_xxh)) {
	if (trace_level >= 2) {
	    PerlIO_printf(DBILOGFP,"    >> %s %s clearing %d CachedKids\n",
		meth_name, neatsvpv(h,0), (int)HvKEYS(DBIc_CACHED_KIDS((imp_drh_t*)imp_xxh)));
	    PerlIO_flush(DBILOGFP);
	}
	/* This will probably recurse through dispatch to DESTROY the kids */
	/* For drh we should probably explicitly do dbh disconnects */
	SvREFCNT_dec(DBIc_CACHED_KIDS((imp_drh_t*)imp_xxh));
	DBIc_CACHED_KIDS((imp_drh_t*)imp_xxh) = Nullhv;
    }
}


/* ----------------------------------------------------------------- */
/* ---   The DBI dispatcher. The heart of the perl DBI.          --- */

XS(XS_DBI_dispatch)         /* prototype must match XS produced code */
{
    dXSARGS;
    dPERINTERP;

    SV *h   = ST(0);		/* the DBI handle we are working with	*/
    SV *st1 = ST(1);		/* used in debugging */
    SV *st2 = ST(2);		/* used in debugging */
    SV *orig_h = h;
    MAGIC *mg;
    STRLEN lna;
    int gimme = GIMME;
    int debug = DBIS->debug;	/* local, may change during dispatch	*/
    int is_DESTROY = FALSE;
    int keep_error = FALSE;
    int i, outitems;
    int call_depth;

    char	*meth_name = GvNAME(CvGV(cv));
    dbi_ima_t	*ima       = (dbi_ima_t*)CvXSUBANY(cv).any_ptr;
    imp_xxh_t	*imp_xxh   = NULL;
    SV		*imp_msv   = NULL; /* handle implementors method (GV or CV) */
    SV		*qsv       = NULL; /* quick result from a shortcut method   */


    if (!SvROK(h) || SvTYPE(SvRV(h)) != SVt_PVHV) {
        croak("%s: handle %s is not a hash reference",meth_name,neatsvpv(h,0));
    }

    if (debug >= 9) {
	PerlIO *logfp = DBILOGFP;
        PerlIO_printf(logfp,"    >> %-11s DISPATCH (%s rc%ld/%ld @%ld g%x a%lx)",
	    meth_name, neatsvpv(h,0),
	    (long)SvREFCNT(h), (SvROK(h) ? (long)SvREFCNT(SvRV(h)) : (long)-1),
	    (long)items, (int)gimme, (long)ima);
	log_where(logfp, debug);
	PerlIO_puts(logfp, "\n");	/* end of the line */
	PerlIO_flush(logfp);
    }

    if (*meth_name=='D' && strEQ(meth_name,"DESTROY")) {
	/* note that croak()'s won't propagate, only append to $@ */
	is_DESTROY = TRUE;
	keep_error = TRUE;
    }

    /* Check method call against Internal Method Attributes */
    if (ima) {

	if (ima->flags & IMA_FUNC_REDIRECT) {
	    SV *meth_name_sv = POPs;
	    PUTBACK;
	    --items;
	    if (!SvPOK(meth_name_sv) || SvNIOK(meth_name_sv))
		croak("%s->%s() invalid redirect method name %s",
			neatsvpv(h,0), meth_name, neatsvpv(meth_name_sv,0));
	    meth_name = SvPV(meth_name_sv, lna);
	}
    	if (ima->flags & IMA_KEEP_ERR)
	    keep_error = TRUE;

	if (ima->flags & IMA_HAS_USAGE) {
	    char *err = NULL;
	    char msg[200];

	    if (ima->minargs && (items < ima->minargs
				|| (ima->maxargs>0 && items > ima->maxargs))) {
		/* the error reporting is a little tacky here */
		sprintf(msg,
		    "DBI %s: invalid number of parameters: handle + %ld\n",
		    meth_name, (long)items-1);
		err = msg;
	    }
	    /* arg type checking could be added here later */
	    if (err) {
		croak("%sUsage: %s->%s(%s)", err, "$h", meth_name,
		    (ima->usage_msg) ? ima->usage_msg : "...?");
	    }
	}

    }

    /* If h is a tied hash ref, switch to the inner ref 'behind' the tie.
       This means *all* DBI methods work with the inner (non-tied) ref.
       This makes it much easier for methods to access the real hash
       data (without having to go through FETCH and STORE methods) and
       for tie and non-tie methods to call each other.
    */
    if (SvRMAGICAL(SvRV(h)) && (mg=mg_find(SvRV(h),'P'))!=NULL) {

        if (SvPVX(mg->mg_obj)==NULL) {  /* maybe global destruction */
            if (debug >= 3)
                PerlIO_printf(DBILOGFP,
		    "    <> %s for %s ignored (inner handle gone)\n",
		    meth_name, neatsvpv(h,0));
	    XSRETURN(0);
        }
	/* Distinguish DESTROY of tie (outer) from DESTROY of inner ref	*/
	/* This may one day be used to manually destroy extra internal	*/
	/* refs if the application ceases to use the handle.		*/
	if (is_DESTROY) {
	    imp_xxh = DBIh_COM(mg->mg_obj);
	    if (DBIc_TYPE(imp_xxh) <= DBIt_DB && DBIc_CACHED_KIDS((imp_drh_t*)imp_xxh))
		clear_cached_kids(mg->mg_obj, imp_xxh, meth_name, debug);
	    if (debug >= 3)
                PerlIO_printf(DBILOGFP,"    <> DESTROY ignored for outer handle %s (inner %s)\n",
		    neatsvpv(h,0), neatsvpv(mg->mg_obj,0));
	    /* for now we ignore it since it'll be followed soon by	*/
	    /* a destroy of the inner hash and that'll do the real work	*/
	    XSRETURN(0);
	}
        h = mg->mg_obj; /* switch h to inner ref			*/
        ST(0) = h;      /* switch handle on stack to inner ref		*/
    }

    imp_xxh = DBIh_COM(h); /* get common Internal Handle Attributes	*/

    if (tainting && items > 1		      /* method call has args	*/
	&& DBIc_is(imp_xxh, DBIcf_Taint)      /* taint checks requested	*/
	&& !(ima && ima->flags & IMA_NO_TAINT_IN)
    ) {
	for(i=1; i < items; ++i) {
	    if (SvTAINTED(ST(i))) {
		char buf[100];
		sprintf(buf,"parameter %d of %s->%s method call",
			i, SvPV(h,lna), meth_name);
		tainted = 1;	/* needed for TAINT_PROPER to work	*/
		TAINT_PROPER(buf);	/* die's */
	    }
	}
    }

    if ( (i = DBIc_DEBUGIV(imp_xxh)) > debug) {
	/* bump up debugging if handle wants it	*/
	debug = i;
    }

    /* record this inner handle for use by DBI::var::FETCH	*/
    if (is_DESTROY) {

	if (DBIc_TYPE(imp_xxh) <= DBIt_DB && DBIc_CACHED_KIDS((imp_drh_t*)imp_xxh))
	    clear_cached_kids(h, imp_xxh, meth_name, debug);

	if (DBI_IS_LAST_HANDLE(h)) {	/* if destroying _this_ handle */
	    SV *lhp = DBI_LAST_HANDLE_PARENT;
	    (SvROK(lhp)) ? DBI_SET_LAST_HANDLE(lhp) : DBI_UNSET_LAST_HANDLE;
	} /* otherwise don't alter last handle */

	if (DBIc_IADESTROY(imp_xxh)) { /* want's ineffective destroy	*/
	    DBIc_ACTIVE_off(imp_xxh);
	}
	call_depth = 0;
    }
    else {
	DBI_SET_LAST_HANDLE(h);
	SAVEINT(DBIc_CALL_DEPTH(imp_xxh));
	call_depth = ++DBIc_CALL_DEPTH(imp_xxh);
    }

    /* --- dispatch --- */

    if (!keep_error)
	DBIh_CLEAR_ERROR(imp_xxh);

    /* Now check if we can provide a shortcut implementation here. */
    /* At the moment we only offer a quick fetch mechanism.        */
    if (*meth_name=='F' && strEQ(meth_name,"FETCH")) {
	qsv = quick_FETCH(h, ST(1), &imp_msv);
    }

    if (qsv) { /* skip real method call if we already have a 'quick' value */

	ST(0) = sv_mortalcopy(qsv);
	outitems = 1;

    }
    else {

	if (debug) {
	    SAVEI32(DBIS->debug);	/* fall back to orig value later */
	    DBIS->debug = debug;	/* make value global (for now)	 */
	    if (ima && debug < ima->trace_level) {
		debug = 0;		/* silence dispatch log for this method	*/
	    }
	}

#ifdef DBI_USE_THREADS		/* only pay the cost with threaded perl	*/
	if (PL_threadnum) {	/* and only if > one thread exists	*/
	    DBI_LOCK;
	    while(DBIc_THR_USER(imp_xxh) != DBIc_THR_USER_NONE && DBIc_THR_USER(imp_xxh) != thr->tid) {
		if (debug >= 4) {
		    PerlIO_printf(DBILOGFP,"    .. %s: thread %lu waiting for thread %lu to leave %s\n",
			    meth_name, thr->tid, DBIc_THR_USER(imp_xxh), HvNAME(DBIc_IMP_STASH(imp_xxh)));
		    PerlIO_flush(DBILOGFP);
		}
		COND_WAIT(DBIc_THR_COND(imp_xxh), DBIS->mutex);
	    }
	    DBIc_THR_USER(imp_xxh) = thr->tid;
	    if (debug >= 4) {
		PerlIO_printf(DBILOGFP,"    .. %s: thread %lu entering %s\n",
			meth_name, thr->tid, HvNAME(DBIc_IMP_STASH(imp_xxh)));
		PerlIO_flush(DBILOGFP);
	    }
	    SAVEDESTRUCTOR(dbi_cond_signal, imp_xxh);  /* arrange later cond signal	*/
	    DBI_UNLOCK;
	}
#endif

	if (!imp_msv) {
	    imp_msv = (SV*)gv_fetchmethod(DBIc_IMP_STASH(imp_xxh), meth_name);
	    if (!imp_msv) {
		if (dirty && is_DESTROY) {
		    XSRETURN(0);
		}
		croak("Can't locate DBI object method \"%s\" via package \"%s\"",
		    meth_name, HvNAME(DBIc_IMP_STASH(imp_xxh)));
	    }
	}

	if (debug >= 2) {
	    PerlIO *logfp = DBILOGFP;
	    /* Full pkg method name (or just meth_name for ANON CODE)	*/
	    char *imp_meth_name = (isGV(imp_msv)) ? GvNAME(imp_msv) : meth_name;
	    HV *imp_stash = DBIc_IMP_STASH(imp_xxh);
	    PerlIO_printf(logfp, "%c   -> %s ",
			call_depth>1 ? '0'+call_depth : ' ', imp_meth_name);
	    if (imp_meth_name[0] == 'A' && strEQ(imp_meth_name,"AUTOLOAD"))
		    PerlIO_printf(logfp, "\"%s\" ", meth_name);
	    if (isGV(imp_msv) && GvSTASH(imp_msv) != imp_stash)
		PerlIO_printf(logfp, "in %s ", HvNAME(GvSTASH(imp_msv)));
	    PerlIO_printf(logfp, "for %s (%s", HvNAME(imp_stash),
			SvPV(orig_h,lna));
	    if (h != orig_h)	/* show inner handle to aid tracing */
		 PerlIO_printf(logfp, "~0x%lx", (long)SvRV(h));
	    else PerlIO_printf(logfp, "~INNER");
	    for(i=1; i<items; ++i) {
		PerlIO_printf(logfp," %s",
		    (ima && i==ima->hidearg) ? "****" : neatsvpv(ST(i),0));
	    }
#ifdef DBI_USE_THREADS
	    PerlIO_printf(logfp, ") thr%lu\n", thr->tid);
#else
	    PerlIO_printf(logfp, ")\n");
#endif
	    PerlIO_flush(logfp);
	}

	PUSHMARK(mark);  /* mark arguments again so we can pass them on	*/

	/* Note: the handle on the stack is still an object blessed into a
	 * DBI::* class and *not* the DBD::*::* class whose method is being
	 * invoked. This *is* correct and should be largely transparent.
	 */

	/* SHORT-CUT ALERT! */
	if (xsbypass && isGV(imp_msv) && CvXSUB(GvCV(imp_msv))) {

	    /* If we are calling an XSUB we jump directly to its C code and
	     * bypass perl_call_sv(), pp_entersub() etc. This is fast.
	     * This code is copied from a small section of pp_entersub().
	     */
	    I32 markix = TOPMARK;
	    CV *xscv   = GvCV(imp_msv);
	    (void)(*CvXSUB(xscv))(aTHXo_ xscv);	/* Call the C code directly */

	    if (gimme == G_SCALAR) {    /* Enforce sanity in scalar context */
		if (++markix != stack_sp - stack_base ) {
		    if (markix > stack_sp - stack_base)
			 *(stack_base + markix) = &sv_undef;
		    else *(stack_base + markix) = *stack_sp;
		    stack_sp = stack_base + markix;
		}
		outitems = 1;
	    }
	    else {
		outitems = stack_sp - (stack_base + markix);
	    }

	}
	else {
	    outitems = perl_call_sv(isGV(imp_msv) ? (SV*)GvCV(imp_msv) : imp_msv, gimme);
	}

	if (debug) { /* XXX restore local vars so ST(n) works below	*/
	    SPAGAIN; sp -= outitems; ax = (sp - stack_base) + 1; 
	}

	/* We might perform some fancy error handling here one day	*/
    }

    if (debug >= 1) {
	PerlIO *logfp = DBILOGFP;
	if (!keep_error && SvTRUE(DBIc_ERR(imp_xxh)))
	    PerlIO_printf(logfp,"    !! ERROR: %s %s\n",
		neatsvpv(DBIc_ERR(imp_xxh),0), neatsvpv(DBIc_ERRSTR(imp_xxh),0));
	PerlIO_printf(logfp,"%c%c  <- %s",
		    (call_depth > 1)                ? '0'+call_depth : ' ',
		    (DBIc_is(imp_xxh, DBIcf_Taint)) ? 'T'            : ' ',
		    meth_name);
	if (debug==1) { /* make debug level 1 for some methods more useful */
	    if (	(*meth_name=='S' && strEQ(meth_name,"STORE"))
	    	||	(*meth_name=='p' && strEQ(meth_name,"prepare"))
	    	||	(*meth_name=='e' && strEQ(meth_name,"execute"))
	    	||	(*meth_name=='d' && strEQ(meth_name,"do"))
	    ) {
		/* we only have the first two parameters available here */
		PerlIO_printf(logfp,"(");
		if (items >= 1)
		    PerlIO_printf(logfp,"%s", neatsvpv(st1,0));
		if (items >= 2)
		    PerlIO_printf(logfp," %s", neatsvpv(st2,0));
		PerlIO_printf(logfp,"%s)", (items > 2) ? " ..." : "");
	    }
	}

	if (gimme & G_ARRAY)
	     PerlIO_printf(logfp,"= (");
	else PerlIO_printf(logfp,"=");
	for(i=0; i < outitems; ++i) {
	    SV *s = ST(i);
	    if ( SvROK(s) && SvTYPE(SvRV(s))==SVt_PVAV) {
		AV *av = (AV*)SvRV(s);
		int avi;
		PerlIO_printf(logfp, " [");
		for(avi=0; avi <= AvFILL(av); ++avi)
		    PerlIO_printf(logfp, " %s",  neatsvpv(AvARRAY(av)[avi],0));
		PerlIO_printf(logfp, " ]");
	    }
	    else {
		PerlIO_printf(logfp, " %s",  neatsvpv(s,0));
		if ( SvROK(s) && SvTYPE(SvRV(s))==SVt_PVHV && !SvOBJECT(SvRV(s)) )
		    PerlIO_printf(logfp, "%ldkeys", HvKEYS(SvRV(s)));
	    }
	}
	if (gimme & G_ARRAY) {
	    PerlIO_printf(logfp," ) [%d items]", outitems);
	}
	if (qsv) /* flag as quick and peek at the first arg (still on the stack) */
	    PerlIO_printf(logfp," (%s from cache)", neatsvpv(st1,0));
	/* XXX add flag to show pid here? */
	log_where(logfp, debug); /* add file and line number information */
	PerlIO_puts(logfp, "\n");	/* end of the line */
	PerlIO_flush(logfp);
    }

    if (   !keep_error				/* so would be a new error	*/
	&& SvTRUE(DBIc_ERR(imp_xxh))		/* and an error exists		*/
	&& call_depth <= 1			/* skip nested (internal) calls	*/
	&& DBIc_has(imp_xxh, DBIcf_RaiseError|DBIcf_PrintError)
	/* check that we're not nested inside a call to our parent */
	&& (!DBIc_PARENT_COM(imp_xxh) || DBIc_CALL_DEPTH(DBIc_PARENT_COM(imp_xxh)) < 1)
    ) {
	SV *msg;
	SV **statement;
	char intro[200];
	sprintf(intro,"%s %s failed: ", HvNAME(DBIc_IMP_STASH(imp_xxh)), meth_name);
	msg = sv_2mortal(newSVpv(intro,0));
	sv_catsv(msg, DBIc_ERRSTR(imp_xxh));

	if (    DBIc_has(imp_xxh, DBIcf_ShowErrorStatement)
	    && (DBIc_TYPE(imp_xxh) == DBIt_ST
		|| strEQ(meth_name,"prepare")
		|| strEQ(meth_name,"do")
		)
	    && (statement = hv_fetch((HV*)SvRV(h), "Statement", 9, 0))
	    &&  SvOK(*statement)
	) {
	    sv_catpv(msg, " [for statement ``");
	    sv_catsv(msg, *statement);
	    sv_catpv(msg, "''])");
	}

	if (DBIc_has(imp_xxh, DBIcf_PrintError))
	    warn("%s", SvPV(msg,lna));
	if (DBIc_has(imp_xxh, DBIcf_RaiseError))
	    croak("%s", SvPV(msg,lna));
    }

    if (tainting
	&& DBIc_is(imp_xxh, DBIcf_Taint)      /* taint checks requested	*/
	&& 0/*disabled*/ /* !(ima && ima->flags & IMA_NO_TAINT_OUT) */
    ) {
	dTHR;
	TAINT; /* affects sv_setsv()'s within same perl statement */
	for(i=0; i < outitems; ++i) {
	    I32 avi;
	    char *p;
	    SV *s;
	    SV *agg = ST(i);
	    if ( !SvROK(agg) )
		continue;
	    agg = SvRV(agg);
#define DBI_OUT_TAINTABLE(s) (!SvREADONLY(s) && !SvTAINTED(s))
	    switch (SvTYPE(agg)) {
	    case SVt_PVAV:
		for(avi=0; avi <= AvFILL((AV*)agg); ++avi) {
		    s = AvARRAY((AV*)agg)[avi];
		    if (DBI_OUT_TAINTABLE(s))
			SvTAINTED_on(s);
		}
		break;
	    case SVt_PVHV:
		hv_iterinit((HV*)agg);
		while( (s = hv_iternextsv((HV*)agg, &p, &avi)) ) {
		    if (DBI_OUT_TAINTABLE(s))
			SvTAINTED_on(s);
		}
		break;
	    default:
		if (DBIc_WARN(imp_xxh)) {
		    PerlIO_printf(DBILOGFP,"Don't know how to taint contents of returned %s (type %ld)",
			neatsvpv(agg,0), SvTYPE(agg));
		}
	    }
	}
    }

    XSRETURN(outitems);
}


/* --------------------------------------------------------------------	*/

/* comment and placeholder styles to accept and return */
#define DBIpp_cm_cs 0x0001   /* C style	*/
#define DBIpp_cm_hs 0x0002   /* #	*/
#define DBIpp_cm_dd 0x0004   /* --	*/
#define DBIpp_cm_br 0x0008   /* {}	*/

#define DBIpp_ph_qm 0x0100   /* ?	*/
#define DBIpp_ph_cn 0x0200   /* :1	*/
#define DBIpp_ph_cs 0x0400   /* :name	*/
#define DBIpp_ph_sp 0x0800   /* %s	*/

#define DBIpp_L_BRACE '{'
#define DBIpp_R_BRACE '}'
#define PS_accept(flag)  DBIbf_has(ps_accept,(flag))
#define PS_return(flag)  DBIbf_has(ps_return,(flag))

SV *
preparse(SV *h, char *statement, U32 ps_return, U32 ps_accept)
{
/*
	The idea hear is that ps_accept defines which constructs to
	recognize (accept) as valid in the source string (other
	constructs are ignored), and ps_return defines which
	constructs are valid to return in the result string.

	If a construct that is valid in the input is also valid in the
	output then it's simply copied. If it's not valid in the output
	then it's editied into one of the valid forms (ideally the most
	'standard' and/or information preserving one).

	For example, if ps_accept includes '--' style comments but
	ps_return doesn't, but ps_return does include '#' style
	comments then any '--' style comments would be rewritten as '#'
	style comments.

	Similarly for placeholders. DBD::Oracle, for example, would say
	'?', ':1' and ':name' are all acceptable input, but only
	':name' should be returned.

*/
    int ph_type = 0;
    int idx = 1;

    char in_quote = '\0';
    char in_comment = '\0';
    char *src, *start, *dst, *stmnt;
    char *style = "", *laststyle = '\0';

    SV *new_stmt_sv = newSV(strlen(statement) * 3);
    sv_setpv(new_stmt_sv,"");

    src  = statement;
    dst = SvPVX(new_stmt_sv);

/* XXX select cm_fallback and ph_fallback from ps_return
	for use in the rewrite code to be added below
*/

    while ( *src ) {
	if (in_comment) {
	    /* are we at the end of the comment we're in? */
	    if (	(in_comment == '-' && *src == '\n') 
		||	(in_comment == '#' && *src == '\n')
		||	(in_comment == DBIpp_L_BRACE && *src == DBIpp_R_BRACE)
		||	(in_comment == '/' && *src == '*' && *(src+1) == '/')
	    ) {
		if (in_comment == '/')
		    *dst++ = *src++; /* avoids asterisk-slash-asterisk issues */
		in_comment = '\0';
	    }
	    *dst++ = *src++;
	    continue;
	}

	if (in_quote) {
	    if (*src == '%' && ph_type == 4)
		*dst++ = '%';
	    if (*src == in_quote) {
		in_quote = 0;
	    }
	    *dst++ = *src++;
	    continue;
	}

	/* Look for start of comments */
	if (   (*src == '-' && *(src+1) == '-' && PS_accept(DBIpp_cm_dd))
	    || (*src == '/' && *(src+1) == '*' && PS_accept(DBIpp_cm_cs))
	    || (*src == '#'                    && PS_accept(DBIpp_cm_hs))
	    || (*src == DBIpp_L_BRACE          && PS_accept(DBIpp_cm_br))
	) {
	    in_comment = *src;
	    /* We know *src & the next char are to be copied, so do */
	    /*  it.  In the case of C-style comments, it happens to */
	    /*  help us avoid slash-asterisk-slash oddities.        */
	    *dst++ = *src++;
	    if (*src=='-' || *src=='/')
		*dst++ = *src++; /* skip past 2nd char of double char delimiters */
	    continue;
	}

	if (	!(*src==':' && (PS_accept(DBIpp_ph_cn) || PS_accept(DBIpp_ph_cs)))
	   &&	!(*src=='?' &&  PS_accept(DBIpp_ph_qm))
	) {
	    if (*src == '\'' || *src == '"')
		in_quote = *src;
	    if (*src == '%' && ph_type == 4)
		*dst++ = '%';
	    *dst++ = *src++;
	    continue;
	}

	/* only here for : or ? outside of a comment or literal	*/

	start = dst;			/* save name inc colon	*/ 
	*dst++ = *src++;

	if (*start == '?')			/* X/Open Standard */
	{
	    style = "?";
	    switch (ph_type)
	    {
	    case 1: /* '?' -> ':p1' (etc) */
		sprintf(start,":p%d", idx++);
		dst = start+strlen(start);
		break;
	    case 2: 
		break;
	    case 4: /* '?' -> '%s' */
		*start  = '%';
		*dst++ = 's';
		break;
	    }
	} 
	else if (isDIGIT(*src))   /* :1 */ 
	{
	    style = ":1";
	    switch (ph_type)
	    {
	    case 1: /* ':1'->':p1'  */
	       idx = atoi(src);
	       *dst++ = 'p';
	       if (idx <= 0) {
		   warn("\tPlaceholder :%d invalid, placeholders must be >= 1\n", idx);
		   return(0);
	       }
	       while(isDIGIT(*src))
		   *dst++ = *src++;
	       break;
	    case 2: 
	       *start = '?';
	       *dst = *start;
	       if (atoi(src) != idx) {
		   warn("\tPlaceholder :%d out of sequence, must be in sequential order (:a1, :a2 ..)\n", idx);
		   return(0);
	       }
	       while(isDIGIT(*src)) *src++;
	       idx++;
	       break;
	    case 4: /* ':1' -> '%s' */
	       sprintf(start,"%%s");
	       dst = start+strlen(start);
	       if (atoi(src) != idx) {
		   warn("\tPlaceholder :%d out of sequence, must be in sequential order (:a1, :a2 ..)\n", idx);
		   return(0);
	       }
	       while(isDIGIT(*src)) *src++; 
	       idx++;
	       break;
	    }
	} 
	else if (isALNUM(*src))         /* :foo */ 
	{
	    style = ":foo";
	    switch (ph_type)
	    {
	    case 1: 
		while (isALNUM(*src))	/* includes '_'	*/
		       *dst++ = *src++;
		break;
	    case 2: 
	       *start = '?';
	       *dst = *start;
	       while (isALNUM(*src)) *src++; /* includes '_'	*/
	       break;
	    case 4: /* ':foo' -> '%s' */
	       sprintf(start,"%%s");
	       dst = start+strlen(start);
	       while (isALNUM(*src)) *src++; /* includes '_'	*/
	       break;
	    }
	}
	else {			/* perhaps ':=' PL/SQL construct */
	    continue;
	}

	/*dst = '\0';*/			/* handy for debugging	*/

	if (laststyle && style != laststyle) {
	    warn("\tCan't mix placeholder styles (%s / %s)\n", style, laststyle);
	    return(0);
	}
	laststyle = style;
    }
    *dst = '\0';

    /* warn about probable parsing errors, but continue anyway */
    switch (in_quote)
    {
    case '\'':
	warn("\tIncomplete single-quoted string\n");
	break;
    case '\"':
	warn("\tIncomplete double-quoted string (delimited identifier)\n");
	break;
    }
    switch (in_comment)
    {
    case DBIpp_L_BRACE:
	warn("\tIncomplete bracketed {...} comment\n");
	break;
    case '/':
	warn("\tIncomplete bracketed C-Style comment\n");
	break;
    case '\n':
	warn("\tIncomplete double-dash or shell comment\n");
	break;
    }
    SvCUR_set(new_stmt_sv, strlen(SvPVX(new_stmt_sv)));
    *SvEND(new_stmt_sv) = '\0';
    return new_stmt_sv;
}


/* --------------------------------------------------------------------	*/
/* The DBI Perl interface (via XS) starts here. Currently these are 	*/
/* all internal support functions. Note install_method and see DBI.pm	*/

MODULE = DBI   PACKAGE = DBI

REQUIRE:    1.929
PROTOTYPES: DISABLE


BOOT:
    items = items;		/* avoid 'unused variable' warning	*/
    dbi_bootinit();


I32
constant()
	PROTOTYPE:
    ALIAS:
	SQL_ALL_TYPES	= SQL_ALL_TYPES
	SQL_CHAR	= SQL_CHAR
	SQL_NUMERIC	= SQL_NUMERIC
	SQL_DECIMAL	= SQL_DECIMAL
	SQL_INTEGER	= SQL_INTEGER
	SQL_SMALLINT	= SQL_SMALLINT
	SQL_FLOAT	= SQL_FLOAT
	SQL_REAL	= SQL_REAL
	SQL_DOUBLE	= SQL_DOUBLE
	SQL_DATE	= SQL_DATE
	SQL_TIME	= SQL_TIME
	SQL_TIMESTAMP	= SQL_TIMESTAMP
	SQL_VARCHAR	= SQL_VARCHAR
	SQL_LONGVARCHAR = SQL_LONGVARCHAR
	SQL_BINARY	= SQL_BINARY
	SQL_VARBINARY	= SQL_VARBINARY
	SQL_LONGVARBINARY = SQL_LONGVARBINARY
	SQL_TINYINT	= SQL_TINYINT
	SQL_BIGINT	= SQL_BIGINT
	SQL_WCHAR       = SQL_WCHAR
	SQL_WVARCHAR    = SQL_WVARCHAR
	SQL_WLONGVARCHAR = SQL_WLONGVARCHAR
	SQL_BIT         = SQL_BIT
	DBIpp_cm_cs	= DBIpp_cm_cs
	DBIpp_cm_hs	= DBIpp_cm_hs
	DBIpp_cm_dd	= DBIpp_cm_dd
	DBIpp_cm_br	= DBIpp_cm_br
	DBIpp_ph_qm	= DBIpp_ph_qm
	DBIpp_ph_cn	= DBIpp_ph_cn
	DBIpp_ph_cs	= DBIpp_ph_cs
	DBIpp_ph_sp	= DBIpp_ph_sp
    CODE:
    RETVAL = ix;
    OUTPUT:
    RETVAL


SV *
preparse(h, statement, ps_accept, ps_return)
    SV *	h
    char *	statement
    IV		ps_accept
    IV		ps_return
	


void
_setup_handle(sv, imp_class, parent, imp_datasv)
    SV *	sv
    char *	imp_class
    SV *	parent
    SV *	imp_datasv
    CODE:
    dbih_setup_handle(sv, imp_class, parent, SvOK(imp_datasv) ? imp_datasv : Nullsv);
    ST(0) = &sv_undef;


void
_get_imp_data(sv)
    SV *	sv
    CODE:
    D_imp_xxh(sv);
    ST(0) = sv_mortalcopy(DBIc_IMP_DATA(imp_xxh)); /* okay if NULL	*/


void
_handles(sv)
    SV *	sv
    PPCODE:
    /* return the outer and inner handle for any given handle */
    D_imp_xxh(sv);
    SV *ih = sv_mortalcopy( dbih_inner(sv, "_handles") );
    SV *oh = sv_2mortal(newRV((SV*)DBIc_MY_H(imp_xxh))); /* XXX dangerous */
    EXTEND(SP, 2);
    PUSHs(oh);	/* returns outer handle then inner */
    PUSHs(ih);


void
set_err(sv, errval, errstr=&sv_no, state=&sv_undef)
    SV *	sv
    SV *	errval
    SV *	errstr
    SV *	state
    CODE:
    {
    dPERINTERP;
    D_imp_xxh(sv);
    sv_setsv(DBIc_ERR(imp_xxh),    errval);
    if (errstr==&sv_no || !SvOK(errstr))
	errstr = errval;
    sv_setsv(DBIc_ERRSTR(imp_xxh), errstr);
    if (SvOK(state)) {
	STRLEN len;
	if (SvPV(state, len) && len != 5)
	    croak("set_err: state must be 5 character string");
	sv_setsv(DBIc_STATE(imp_xxh), state);
    }
    else {
	(void)SvOK_off(DBIc_STATE(imp_xxh));
    }
    sv = dbih_inner(sv,"set_err");
    DBI_SET_LAST_HANDLE(sv);
    /* We don't check RaiseError and call die here because that must be	*/
    /* done by returning theough dispatch and letting the DBI handle it	*/
    ST(0) = &sv_undef;
    }


void
neat(sv, maxlen=0)
    SV *	sv
    U32	maxlen
    CODE:
    ST(0) = sv_2mortal(newSVpv(neatsvpv(sv, maxlen), 0));


int
hash(key, i=0)
    char *key
    int i
    CODE:
    RETVAL = dbi_hash(key, i);
    OUTPUT:
    RETVAL

void
looks_like_number(...)
    PPCODE:
    int i;
    EXTEND(SP, items);
    for(i=0; i < items ; ++i) {
	SV *sv = ST(i);
	if (!SvOK(sv) || (SvPOK(sv) && SvCUR(sv)==0))
	    PUSHs(&sv_undef);
	else if ( looks_like_number(sv) )
	    PUSHs(&sv_yes);
	else
	    PUSHs(&sv_no);
    }
	

void
_install_method(class, meth_name, file, attribs=Nullsv)
    char *	class
    char *	meth_name
    char *	file
    SV *	attribs
    CODE:
    {
    dPERINTERP;
    /* install another method name/interface for the DBI dispatcher	*/
    int debug = (DBIS->debug >= 7);
    CV *cv;
    SV **svp;
    dbi_ima_t *ima = NULL;
    class = class;		/* avoid 'unused variable' warning	*/

    if (debug)
	PerlIO_printf(DBILOGFP,"install_method %s\t", meth_name);

    if (strnNE(meth_name, "DBI::", 5))	/* XXX m/^DBI::\w+::\w+$/	*/
	croak("install_method: invalid name '%s'", meth_name);

    if (attribs && SvROK(attribs)) {
	SV *sv;
	/* convert and store method attributes in a fast access form	*/
	if (SvTYPE(SvRV(attribs)) != SVt_PVHV)
	    croak("install_method %s: bad attribs", meth_name);

	sv = newSV(sizeof(*ima));
	ima = (dbi_ima_t*)(void*)SvPVX(sv);
	memzero((char*)ima, sizeof(*ima));
	DBD_ATTRIB_GET_IV(attribs, "O",1, svp, ima->flags);
	DBD_ATTRIB_GET_IV(attribs, "T",1, svp, ima->trace_level);
	DBD_ATTRIB_GET_IV(attribs, "H",1, svp, ima->hidearg);

	if ( (svp=DBD_ATTRIB_GET_SVP(attribs, "U",1)) != NULL) {
	    STRLEN lna;
	    AV *av = (AV*)SvRV(*svp);
	    ima->minargs    = SvIV(*av_fetch(av, 0, 1));
	    ima->maxargs    = SvIV(*av_fetch(av, 1, 1));
			      svp = av_fetch(av, 2, 0);
	    ima->usage_msg  = savepv( (svp) ? SvPV(*svp,lna) : "");
	    ima->flags |= IMA_HAS_USAGE;
	    if (debug)
		PerlIO_printf(DBILOGFP,"    usage: min %d, max %d, '%s', tl %d\n",
			ima->minargs, ima->maxargs, ima->usage_msg, ima->trace_level);
	}
	if (debug)
	    PerlIO_printf(DBILOGFP,", flags 0x%x", ima->flags);

    } else if (attribs && SvOK(attribs)) {
	croak("install_method %s: attributes not a ref", meth_name);
    }
    cv = newXS(meth_name, XS_DBI_dispatch, file);
    CvXSUBANY(cv).any_ptr = ima;
    if (debug)
	PerlIO_printf(DBILOGFP,"\n");
    ST(0) = &sv_yes;
    }


int
trace(sv, level=-1, file=Nullsv)
    SV *	sv
    int	level
    SV *	file
    ALIAS:
    _debug_dispatch = 1
    CODE:
    {
    dPERINTERP;
    if (!DBIS) {
	sv=sv; ix=ix;		/* avoid 'unused variable' warnings	*/
	croak("DBI not initialised");
    }
    if (level == -1) level = DBIS->debug;
    /* Return old/current value. No change if new value not given.	*/
    RETVAL = DBIS->debug;
    set_trace_file(file);	/* always call this regardless of level */
    if (level != DBIS->debug) {
	if (level > 0) {
	    PerlIO_printf(DBILOGFP,"    DBI %s%s dispatch trace level set to %d\n",
		XS_VERSION, dbi_build_opt, level);
	    if (!dowarn)
		PerlIO_printf(DBILOGFP,"    Note: perl is running without the recommended perl -w option\n");
	    PerlIO_flush(DBILOGFP);
	}
	DBIS->debug = level;
	sv_setiv(get_sv("DBI::dbi_debug",0x5), level);
    }
    }
    OUTPUT:
    RETVAL



void
dump_handle(sv, msg="DBI::dump_handle", level=0)
    SV *	sv
    char *	msg
    int 	level
    CODE:
    {
    D_imp_xxh(sv);
    dbih_dumpcom(imp_xxh, msg, level);
    }


void
_svdump(sv)
    SV *	sv
    CODE:
    {
    dPERINTERP;
    PerlIO_printf(DBILOGFP, "DBI::_svdump(%s)", neatsvpv(sv,0));
#ifdef DEBUGGING
    sv_dump(sv);
#endif
    }


MODULE = DBI   PACKAGE = DBI::var

void
FETCH(sv)
    SV *	sv
    CODE:
    dPERINTERP;
    /* Note that we do not come through the dispatcher to get here.	*/
    STRLEN lna;
    char *meth = SvPV(SvRV(sv),lna);	/* what should this tie do ?	*/
    char type = *meth++;		/* is this a $ or & style	*/
    HV *imp_stash;
    GV *imp_gv;
    int ok  = DBI_LAST_HANDLE_OK;
    imp_xxh_t *imp_xxh = (ok) ? DBIh_COM(DBI_LAST_HANDLE) : NULL;
    int trace = 0;

    if (DBIS->debug >= 2 || (ok && DBIc_DEBUGIV(imp_xxh) >= 2)) {
	trace = 2;
	PerlIO_printf(DBILOGFP,"    -> $DBI::%s (%c) FETCH from lasth=", meth, type);
	if (ok) {
	    SvROK_on(DBI_LAST_HANDLE);
	    PerlIO_printf(DBILOGFP,"%s\n", neatsvpv(DBI_LAST_HANDLE,0));
	    SvROK_off(DBI_LAST_HANDLE);
	} else {
	    PerlIO_printf(DBILOGFP,"none\n");
	}
    }

    if (type == '!') {	/* special case for $DBI::lasth */
	if (!ok) {
	    XSRETURN_UNDEF;
	}
	/* Currently we can only return the INNER handle.	*/
	/* This handle should only be used for true/false tests	*/
	SvROK_on(DBI_LAST_HANDLE);
	ST(0) = sv_mortalcopy(DBI_LAST_HANDLE);
	if (trace)
	    PerlIO_printf(DBILOGFP,"    <- $DBI::%s= %s (inner)\n",
				meth, neatsvpv(DBI_LAST_HANDLE,0));
	SvROK_off(DBI_LAST_HANDLE);
	XSRETURN(1);
    }
    if ( !ok ) {		/* warn() may be changed to a debug later */
	warn("Can't read $DBI::%s, last handle unknown or destroyed", meth);
	XSRETURN_UNDEF;
    }

    if (type == '*') {	/* special case for $DBI::err, see also err method	*/
	SV *errsv = DBIc_ERR(imp_xxh);
	if (trace)
	    PerlIO_printf(DBILOGFP,"    <- err= %s\n", neatsvpv(errsv,0));
	ST(0) = sv_mortalcopy(errsv);
	XSRETURN(1);
    }
    if (type == '"') {	/* special case for $DBI::state	*/
	SV *state = DBIc_STATE(imp_xxh);
	ST(0) = DBIc_STATE_adjust(imp_xxh, state);
	if (trace)
	    PerlIO_printf(DBILOGFP,"    <- state= %s\n", neatsvpv(ST(0),0));
	XSRETURN(1);
    }
    if (type == '$') { /* lookup scalar variable in implementors stash */
	char *vname = mkvname(DBIc_IMP_STASH(imp_xxh), meth, 0);
	SV *vsv = get_sv(vname, 1);
	if (trace)
	    PerlIO_printf(DBILOGFP,"    <- %s = %s\n", vname, neatsvpv(vsv,0));
	ST(0) = sv_mortalcopy(vsv);
	XSRETURN(1);
    }
    /* default to method call via stash of implementor of DBI_LAST_HANDLE */
    imp_stash = DBIc_IMP_STASH(imp_xxh);
    if (DBIS->debug >= 2)
        fprintf(DBILOGFP,"    >> %s::%s\n", HvNAME(imp_stash), meth);
    ST(0) = DBI_LAST_HANDLE;
    if ((imp_gv = gv_fetchmethod(imp_stash,meth)) == NULL) {
        croak("Can't locate $DBI::%s object method \"%s\" via package \"%s\"",
            meth, meth, HvNAME(imp_stash));
    }
    PUSHMARK(mark);  /* reset mark (implies one arg as we were called with one arg?) */
    perl_call_sv((SV*)GvCV(imp_gv), GIMME);



MODULE = DBI   PACKAGE = DBD::_::st

void
_get_fbav(sth)
    SV *	sth
    CODE:
    D_imp_sth(sth);
    AV *av = dbih_get_fbav(imp_sth);
    ST(0) = sv_2mortal(newRV((SV*)av));

void
_set_fbav(sth, src_rv)
    SV *	sth
    SV *	src_rv
    CODE:
    D_imp_sth(sth);
    int i;
    AV *src_av;
    AV *dst_av = dbih_get_fbav(imp_sth);
    int num_fields = AvFILL(dst_av)+1;
    if (!SvROK(src_rv) || SvTYPE(SvRV(src_rv)) != SVt_PVAV)
	croak("_set_fbav(%s): not an array ref", neatsvpv(src_rv,0));
    src_av = (AV*)SvRV(src_rv);
    if (AvFILL(src_av)+1 != num_fields)
	croak("_set_fbav(%s): array has %d fields, should have %d%s",
		neatsvpv(src_rv,0), AvFILL(src_av)+1, num_fields);
    for(i=0; i < num_fields; ++i) {	/* copy over the row	*/
	sv_setsv(AvARRAY(dst_av)[i], AvARRAY(src_av)[i]);
    }
    ST(0) = sv_2mortal(newRV((SV*)dst_av));


void
bind_col(sth, col, ref, attribs=Nullsv)
    SV *	sth
    SV *	col
    SV *	ref
    SV *	attribs
    CODE:
    DBD_ATTRIBS_CHECK("bind_col", sth, attribs);
    ST(0) = boolSV(dbih_sth_bind_col(sth, col, ref, attribs));

void
bind_columns(sth, ...)
    SV *	sth
    CODE:
    D_imp_sth(sth);
    SV *colsv;
    SV *attribs = &sv_undef;
    int fields = DBIc_NUM_FIELDS(imp_sth);
    int skip = 0;
    int i;
    if (fields <= 0 && !DBIc_ACTIVE(imp_sth))
	croak("Statement has no result columns to bind %s",
		"(perhaps you need to successfully call execute first)");
    ST(0) = &sv_yes;
    /* Backwards compatibility for old-style call with attribute hash	*/
    /* ref as first arg. Skip arg if undef or a hash ref.		*/
    if (!SvOK(ST(1)) || (SvROK(ST(1)) && SvTYPE(SvRV(ST(1)))==SVt_PVHV)) {
	attribs = ST(1);
	DBD_ATTRIBS_CHECK("bind_columns", sth, attribs);
	skip = 1;
    }
    if (items-(1+skip) != fields)
	croak("bind_columns called with %ld refs when %d needed.", items-(1+skip), fields);
    colsv = sv_2mortal(newSViv(0));
    for(i=1; i < items-skip; ++i) {
	sv_setiv(colsv, i);
	if (!dbih_sth_bind_col(sth, colsv, ST(skip+i), attribs)) {
	    ST(0) = &sv_no;
	    break;
	}
    }


void
fetchrow_array(sth)
    SV *	sth
    ALIAS:
    fetchrow = 1
    PPCODE:
    dPERINTERP;
    SV *retsv;
    if (CvDEPTH(cv) == 99) {
	ix = ix;	/* avoid 'unused variable' warning'		*/
        croak("Deep recursion. Probably fetchrow-fetch-fetchrow loop.");
    }
    PUSHMARK(sp);
    XPUSHs(sth);
    PUTBACK;
    if (perl_call_method("fetch", G_SCALAR) != 1)
	croak("panic: DBI fetch");	/* should never happen */
    SPAGAIN;
    retsv = POPs;
    PUTBACK;
    if (SvROK(retsv) && SvTYPE(SvRV(retsv)) == SVt_PVAV) {
	D_imp_sth(sth);
	int num_fields, i;
	AV *bound_av;
	AV *av = (AV*)SvRV(retsv);
	num_fields = AvFILL(av)+1;
	EXTEND(sp, num_fields+1);

	/* We now check for bind_col() having been called but fetch	*/
	/* not returning the fields_svav array. Probably because the	*/
	/* driver is implemented in perl. XXX This logic may change later.	*/
	bound_av = DBIc_FIELDS_AV(imp_sth); /* bind_col() called ?	*/
	if (bound_av && av != bound_av) {
	    /* let dbih_get_fbav know what's going on	*/
	    bound_av = dbih_get_fbav(imp_sth);
	    if (DBIS->debug >= 3) {
		PerlIO_printf(DBILOGFP,
		    "fetchrow: updating fbav 0x%lx from 0x%lx\n",
		    (long)bound_av, (long)av);
	    }
	    for(i=0; i < num_fields; ++i) {	/* copy over the row	*/
		sv_setsv(AvARRAY(bound_av)[i], AvARRAY(av)[i]);
	    }
	}
	for(i=0; i < num_fields; ++i) {
	    PUSHs(AvARRAY(av)[i]);
	}
    }


SV *
fetchrow_hashref(sth, keyattrib="NAME")
    SV *	sth
    char *	keyattrib
    PREINIT:
    dPERINTERP;
    SV *rowavr;
    CODE:
    PUSHMARK(sp);
    XPUSHs(sth);
    PUTBACK;
    if (perl_call_method("fetch", G_SCALAR) != 1)
	croak("panic: DBI fetch");	/* should never happen */
    SPAGAIN;
    rowavr = POPs;
    PUTBACK;
    /* have we got an array ref in rowavr */
    if (SvROK(rowavr) && SvTYPE(SvRV(rowavr)) == SVt_PVAV) {
	D_imp_sth(sth);
	/* get field names. copy to invoke magic FETCH and for safety */
        SV *ka_rv = newSVsv(*hv_fetch((HV*)DBIc_MY_H(imp_sth), keyattrib,strlen(keyattrib), TRUE));
	AV *ka_av = (AV*)SvRV(ka_rv);
	AV *rowav = (AV*)SvRV(rowavr);
	int num_fields = AvFILL(rowav)+1;
	HV *hv = newHV();

	while (--num_fields >= 0) {
	    STRLEN len;
	    SV  **field_name_svp = av_fetch(ka_av, num_fields, 1);
	    char *field_name     = SvPV(*field_name_svp, len);
	    hv_store(hv, field_name, len, newSVsv((SV*)(AvARRAY(rowav)[num_fields])), 0);
	}
	RETVAL = newRV((SV*)hv);
	SvREFCNT_dec(hv);  	/* since newRV incremented it	*/
	SvREFCNT_dec(ka_rv);	/* since we created it		*/
    }
    else {
	RETVAL = &sv_undef;
#if (PERL_VERSION < 4) || ((PERL_VERSION == 4) && (PERL_SUBVERSION <= 4))
	RETVAL = newSV(0); /* mutable undef for 5.004_04 */
#endif
    }
    OUTPUT:
    RETVAL


void
fetch(sth)
    SV *	sth
    ALIAS:
    fetchrow_arrayref = 1
    CODE:
    int num_fields;
    if (CvDEPTH(cv) == 99) {
	ix = ix;	/* avoid 'unused variable' warning'		*/
        croak("Deep recursion. Probably fetch-fetchrow-fetch loop.");
    }
    PUSHMARK(sp);
    XPUSHs(sth);
    PUTBACK;
    num_fields = perl_call_method("fetchrow", G_ARRAY);	/* XXX change the name later */
    if (num_fields == 0) {
	ST(0) = &sv_undef;
    } else {
	D_imp_sth(sth);
	AV *av = dbih_get_fbav(imp_sth);
	if (num_fields != AvFILL(av)+1)
	    croak("fetchrow returned %d fields, expected %d",
		    num_fields, AvFILL(av)+1);
	SPAGAIN;
	while(--num_fields >= 0)
	    sv_setsv(AvARRAY(av)[num_fields], POPs);
	PUTBACK;
	ST(0) = sv_2mortal(newRV((SV*)av));
    }


void
rows(sth)
    SV *        sth
    CODE:
    D_imp_sth(sth);
    IV rows = DBIc_ROW_COUNT(imp_sth);
    ST(0) = sv_2mortal(newSViv(rows));


void
finish(sth)
    SV *	sth
    CODE:
    D_imp_sth(sth);
    DBIc_ACTIVE_off(imp_sth);
    ST(0) = &sv_yes;


MODULE = DBI   PACKAGE = DBD::_::common


void
DESTROY(...)
    CODE:
    /* the interesting stuff happens in DBD::_mem::common::DESTROY */
	items = items;	/* avoid 'unused variable' warning	*/
    ST(0) = &sv_undef;


void
STORE(h, keysv, valuesv)
    SV *	h
    SV *	keysv
    SV *	valuesv
    CODE:
    ST(0) = &sv_yes;
    if (!dbih_set_attr_k(h, keysv, 0, valuesv))
	    ST(0) = &sv_no;
 

void
FETCH(h, keysv)
    SV *	h
    SV *	keysv
    CODE:
    ST(0) = dbih_get_attr_k(h, keysv, 0);


void
event(h, type, a1=&sv_undef, a2=&sv_undef)
    SV *	h
    char *	type
    SV *	a1
    SV *	a2
    CODE:
    {
    dPERINTERP;
    ST(0) = sv_mortalcopy(DBIh_EVENT2(h, type, a1, a2));
    }


void
private_data(h)
    SV *	h
    CODE:
    D_imp_xxh(h);
    ST(0) = sv_mortalcopy(DBIc_IMP_DATA(imp_xxh));


void
err(h)
    SV * h
    CODE:
    D_imp_xxh(h);
    SV *errsv = DBIc_ERR(imp_xxh);
    ST(0) = sv_mortalcopy(errsv);

void
state(h)
    SV * h
    CODE:
    D_imp_xxh(h);
    STRLEN lna;
    SV *state = DBIc_STATE(imp_xxh);
    ST(0) = DBIc_STATE_adjust(imp_xxh, state);

void
errstr(h)
    SV *    h
    CODE:
    D_imp_xxh(h);
    SV *errstr = DBIc_ERRSTR(imp_xxh);
    SV *err;
    /* If there's no errstr but there is an err then use err */
    if (!SvTRUE(errstr) && (err=DBIc_ERR(imp_xxh)) && SvTRUE(err))
	    errstr = err;
    ST(0) = sv_mortalcopy(errstr);


int
trace(sv, level=0, file=Nullsv)
    SV *	sv
    int	level
    SV *file
    ALIAS:
    debug = 1
    CODE:
    {
    dPERINTERP;
    D_imp_xxh(sv);
    SV *dsv = DBIc_DEBUG(imp_xxh);
    /* Return trace level in effect now. No change if new value not given */
    RETVAL = (DBIS->debug > SvIV(dsv)) ? DBIS->debug : SvIV(dsv);
    set_trace_file(file);
    if (level != RETVAL) {	 /* set value */
	if (level > 0) {
	    PerlIO_printf(DBILOGFP,"    %s trace level set to %d in DBI %s%s\n",
		neatsvpv(sv,0), level, XS_VERSION, dbi_build_opt);
	    if (!dowarn && level>0)
		PerlIO_printf(DBILOGFP,"    Note: perl is running without the recommended perl -w option\n");
	    PerlIO_flush(DBILOGFP);
	    ix = ix;		/* avoid 'unused variable' warning	*/
	}
	sv_setiv(dsv, level);
    }
    }
    OUTPUT:
    RETVAL


void
trace_msg(sv, msg, min_level=1)
    SV *sv
    char *msg
    int min_level
    PREINIT:
    int debug = 0;
    CODE:
    {
    dPERINTERP;
    if (SvROK(sv)) {
	D_imp_xxh(sv);
	debug = DBIc_DEBUGIV(imp_xxh);
    }
    if (DBIS->debug >= min_level || debug >= min_level) {
	PerlIO_puts(DBILOGFP, msg);
        ST(0) = &sv_yes;
    }
    else {
        ST(0) = &sv_no;
    }
    }


void
rows(h)
    SV *        h
    CODE:
    /* fallback esp for $DBI::rows after $drh was last used */
	if (0) h = h;	/* avoid unused variable warning */
    ST(0) = sv_2mortal(newSViv(-1));


MODULE = DBI   PACKAGE = DBD::_mem::common

void
DESTROY(imp_xxh_rv)
    SV *	imp_xxh_rv
    CODE:
    dPERINTERP;
    /* ignore 'cast increases required alignment' warning	*/
    imp_xxh_t *imp_xxh = (imp_xxh_t*)SvPVX(SvRV(imp_xxh_rv));
    DBIS->clearcom(imp_xxh);

# end
