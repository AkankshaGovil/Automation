

/* Written by genchars.pl version 1.97 */



#ifdef CC_TERMIOS
# define TermStructure struct termios
# ifdef NCCS
#  define LEGALMAXCC NCCS
# else
#  ifdef NCC
#   define LEGALMAXCC NCC
#  endif
# endif
#else
# ifdef CC_TERMIO
#  define TermStructure struct termio
#  ifdef NCC
#   define LEGALMAXCC NCC
#  else
#   ifdef NCCS
#    define LEGALMAXCC NCCS
#   endif
#  endif
# endif
#endif

#if !defined(LEGALMAXCC)
# define LEGALMAXCC 126
#endif

#if defined(CC_TERMIO) || defined(CC_TERMIOS)

char	* cc_names[] = {	
#if defined(VDISCARD) && (VDISCARD < LEGALMAXCC)
	"DISCARD",	
#else				
	"",			
#endif				
#if defined(VFLUSH) && (VFLUSH < LEGALMAXCC)
	"DISCARD",	
#else				
	"",			
#endif				
#if defined(VDSUSP) && (VDSUSP < LEGALMAXCC)
	"DSUSPEND",	
#else				
	"",			
#endif				
#if defined(VEOF) && (VEOF < LEGALMAXCC)
	"EOF",	
#else				
	"",			
#endif				
#if defined(VEOL) && (VEOL < LEGALMAXCC)
	"EOL",	
#else				
	"",			
#endif				
#if defined(VEOL2) && (VEOL2 < LEGALMAXCC)
	"EOL2",	
#else				
	"",			
#endif				
#if defined(VERASE) && (VERASE < LEGALMAXCC)
	"ERASE",	
#else				
	"",			
#endif				
#if defined(VWERASE) && (VWERASE < LEGALMAXCC)
	"ERASEWORD",	
#else				
	"",			
#endif				
#if defined(VINTR) && (VINTR < LEGALMAXCC)
	"INTERRUPT",	
#else				
	"",			
#endif				
#if defined(VKILL) && (VKILL < LEGALMAXCC)
	"KILL",	
#else				
	"",			
#endif				
#if defined(VMIN) && (VMIN < LEGALMAXCC)
	"MIN",	
#else				
	"",			
#endif				
#if defined(VQUIT) && (VQUIT < LEGALMAXCC)
	"QUIT",	
#else				
	"",			
#endif				
#if defined(VLNEXT) && (VLNEXT < LEGALMAXCC)
	"QUOTENEXT",	
#else				
	"",			
#endif				
#if defined(VQUOTE) && (VQUOTE < LEGALMAXCC)
	"QUOTENEXT",	
#else				
	"",			
#endif				
#if defined(VREPRINT) && (VREPRINT < LEGALMAXCC)
	"REPRINT",	
#else				
	"",			
#endif				
#if defined(VSTART) && (VSTART < LEGALMAXCC)
	"START",	
#else				
	"",			
#endif				
#if defined(VSTATUS) && (VSTATUS < LEGALMAXCC)
	"STATUS",	
#else				
	"",			
#endif				
#if defined(VSTOP) && (VSTOP < LEGALMAXCC)
	"STOP",	
#else				
	"",			
#endif				
#if defined(VSUSP) && (VSUSP < LEGALMAXCC)
	"SUSPEND",	
#else				
	"",			
#endif				
#if defined(VSWTCH) && (VSWTCH < LEGALMAXCC)
	"SWITCH",	
#else				
	"",			
#endif				
#if defined(VSWTC) && (VSWTC < LEGALMAXCC)
	"SWITCH",	
#else				
	"",			
#endif				
#if defined(VTIME) && (VTIME < LEGALMAXCC)
	"TIME",	
#else				
	"",			
#endif				
};

const int MAXCC = 0	
#if defined(VDISCARD)  && (VDISCARD < LEGALMAXCC)
	+1		/* DISCARD */
#endif			
#if defined(VFLUSH)  && (VFLUSH < LEGALMAXCC)
	+1		/* DISCARD */
#endif			
#if defined(VDSUSP)  && (VDSUSP < LEGALMAXCC)
	+1		/* DSUSPEND */
#endif			
#if defined(VEOF)  && (VEOF < LEGALMAXCC)
	+1		/* EOF */
#endif			
#if defined(VEOL)  && (VEOL < LEGALMAXCC)
	+1		/* EOL */
#endif			
#if defined(VEOL2)  && (VEOL2 < LEGALMAXCC)
	+1		/* EOL2 */
#endif			
#if defined(VERASE)  && (VERASE < LEGALMAXCC)
	+1		/* ERASE */
#endif			
#if defined(VWERASE)  && (VWERASE < LEGALMAXCC)
	+1		/* ERASEWORD */
#endif			
#if defined(VINTR)  && (VINTR < LEGALMAXCC)
	+1		/* INTERRUPT */
#endif			
#if defined(VKILL)  && (VKILL < LEGALMAXCC)
	+1		/* KILL */
#endif			
#if defined(VMIN)  && (VMIN < LEGALMAXCC)
	+1		/* MIN */
#endif			
#if defined(VQUIT)  && (VQUIT < LEGALMAXCC)
	+1		/* QUIT */
#endif			
#if defined(VLNEXT)  && (VLNEXT < LEGALMAXCC)
	+1		/* QUOTENEXT */
#endif			
#if defined(VQUOTE)  && (VQUOTE < LEGALMAXCC)
	+1		/* QUOTENEXT */
#endif			
#if defined(VREPRINT)  && (VREPRINT < LEGALMAXCC)
	+1		/* REPRINT */
#endif			
#if defined(VSTART)  && (VSTART < LEGALMAXCC)
	+1		/* START */
#endif			
#if defined(VSTATUS)  && (VSTATUS < LEGALMAXCC)
	+1		/* STATUS */
#endif			
#if defined(VSTOP)  && (VSTOP < LEGALMAXCC)
	+1		/* STOP */
#endif			
#if defined(VSUSP)  && (VSUSP < LEGALMAXCC)
	+1		/* SUSPEND */
#endif			
#if defined(VSWTCH)  && (VSWTCH < LEGALMAXCC)
	+1		/* SWITCH */
#endif			
#if defined(VSWTC)  && (VSWTC < LEGALMAXCC)
	+1		/* SWITCH */
#endif			
#if defined(VTIME)  && (VTIME < LEGALMAXCC)
	+1		/* TIME */
#endif			
	;

XS(XS_Term__ReadKey_GetControlChars)
{
	dXSARGS;
	if (items < 0 || items > 1) {
		croak("Usage: Term::ReadKey::GetControlChars()");
	}
	SP -= items;
	{
                PerlIO * file;
		TermStructure s;
	        if (items < 1)
	            file = STDIN;
	        else {
	            file = IoIFP(sv_2io(ST(0)));
	        }

#ifdef CC_TERMIOS 
		if(tcgetattr(PerlIO_fileno(file),&s))
#else
# ifdef CC_TERMIO
		if(ioctl(fileno(PerlIO_file),TCGETA,&s))
# endif
#endif
			croak("Unable to read terminal settings in GetControlChars");
		else {
			int i;
			EXTEND(sp,MAXCC*2);		
#if defined(VDISCARD) && (VDISCARD < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[0],strlen(cc_names[0])))); /* DISCARD */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VDISCARD],1))); 	
#endif			
#if defined(VFLUSH) && (VFLUSH < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[1],strlen(cc_names[1])))); /* DISCARD */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VFLUSH],1))); 	
#endif			
#if defined(VDSUSP) && (VDSUSP < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[2],strlen(cc_names[2])))); /* DSUSPEND */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VDSUSP],1))); 	
#endif			
#if defined(VEOF) && (VEOF < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[3],strlen(cc_names[3])))); /* EOF */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VEOF],1))); 	
#endif			
#if defined(VEOL) && (VEOL < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[4],strlen(cc_names[4])))); /* EOL */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VEOL],1))); 	
#endif			
#if defined(VEOL2) && (VEOL2 < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[5],strlen(cc_names[5])))); /* EOL2 */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VEOL2],1))); 	
#endif			
#if defined(VERASE) && (VERASE < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[6],strlen(cc_names[6])))); /* ERASE */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VERASE],1))); 	
#endif			
#if defined(VWERASE) && (VWERASE < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[7],strlen(cc_names[7])))); /* ERASEWORD */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VWERASE],1))); 	
#endif			
#if defined(VINTR) && (VINTR < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[8],strlen(cc_names[8])))); /* INTERRUPT */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VINTR],1))); 	
#endif			
#if defined(VKILL) && (VKILL < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[9],strlen(cc_names[9])))); /* KILL */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VKILL],1))); 	
#endif			
#if defined(VMIN) && (VMIN < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[10],strlen(cc_names[10])))); /* MIN */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VMIN],1))); 	
#endif			
#if defined(VQUIT) && (VQUIT < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[11],strlen(cc_names[11])))); /* QUIT */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VQUIT],1))); 	
#endif			
#if defined(VLNEXT) && (VLNEXT < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[12],strlen(cc_names[12])))); /* QUOTENEXT */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VLNEXT],1))); 	
#endif			
#if defined(VQUOTE) && (VQUOTE < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[13],strlen(cc_names[13])))); /* QUOTENEXT */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VQUOTE],1))); 	
#endif			
#if defined(VREPRINT) && (VREPRINT < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[14],strlen(cc_names[14])))); /* REPRINT */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VREPRINT],1))); 	
#endif			
#if defined(VSTART) && (VSTART < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[15],strlen(cc_names[15])))); /* START */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VSTART],1))); 	
#endif			
#if defined(VSTATUS) && (VSTATUS < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[16],strlen(cc_names[16])))); /* STATUS */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VSTATUS],1))); 	
#endif			
#if defined(VSTOP) && (VSTOP < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[17],strlen(cc_names[17])))); /* STOP */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VSTOP],1))); 	
#endif			
#if defined(VSUSP) && (VSUSP < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[18],strlen(cc_names[18])))); /* SUSPEND */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VSUSP],1))); 	
#endif			
#if defined(VSWTCH) && (VSWTCH < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[19],strlen(cc_names[19])))); /* SWITCH */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VSWTCH],1))); 	
#endif			
#if defined(VSWTC) && (VSWTC < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[20],strlen(cc_names[20])))); /* SWITCH */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VSWTC],1))); 	
#endif			
#if defined(VTIME) && (VTIME < LEGALMAXCC)	
PUSHs(sv_2mortal(newSVpv(cc_names[21],strlen(cc_names[21])))); /* TIME */
PUSHs(sv_2mortal(newSVpv((char*)&s.c_cc[VTIME],1))); 	
#endif			
			
		}
		PUTBACK;
		return;
	}
}

XS(XS_Term__ReadKey_SetControlChars)
{
	dXSARGS;
	/*if ((items % 2) != 0) {
		croak("Usage: Term::ReadKey::SetControlChars(%charpairs,file=STDIN)");
	}*/
	SP -= items;
	{
		TermStructure s;
		PerlIO * file;
	        if ((items % 2) == 1)
	            file = IoIFP(sv_2io(ST(items-1)));
	        else {
	            file = STDIN;
	        }

#ifdef CC_TERMIOS
		if(tcgetattr(PerlIO_fileno(file),&s))
#else
# ifdef CC_TERMIO
		if(ioctl(fileno(PerlIO_file),TCGETA,&s))
# endif
#endif
			croak("Unable to read terminal settings in SetControlChars");
		else {
			int i;
			char * name, value;
			for(i=0;i+1<items;i+=2) {
				name = SvPV(ST(i),PL_na);
				if( SvIOKp(ST(i+1)) || SvNOKp(ST(i+1)) )/* If Int or Float */
					value = (char)SvIV(ST(i+1));         /* Store int value */
				else                                    /* Otherwise */
					value = SvPV(ST(i+1),PL_na)[0];          /* Use first char of PV */

	if (0) ;					
#if defined(VDISCARD) && (VDISCARD < LEGALMAXCC)	
	else if(strcmp(name,cc_names[0])==0) /* DISCARD */ 
		s.c_cc[VDISCARD] = value;		
#endif							
#if defined(VFLUSH) && (VFLUSH < LEGALMAXCC)	
	else if(strcmp(name,cc_names[1])==0) /* DISCARD */ 
		s.c_cc[VFLUSH] = value;		
#endif							
#if defined(VDSUSP) && (VDSUSP < LEGALMAXCC)	
	else if(strcmp(name,cc_names[2])==0) /* DSUSPEND */ 
		s.c_cc[VDSUSP] = value;		
#endif							
#if defined(VEOF) && (VEOF < LEGALMAXCC)	
	else if(strcmp(name,cc_names[3])==0) /* EOF */ 
		s.c_cc[VEOF] = value;		
#endif							
#if defined(VEOL) && (VEOL < LEGALMAXCC)	
	else if(strcmp(name,cc_names[4])==0) /* EOL */ 
		s.c_cc[VEOL] = value;		
#endif							
#if defined(VEOL2) && (VEOL2 < LEGALMAXCC)	
	else if(strcmp(name,cc_names[5])==0) /* EOL2 */ 
		s.c_cc[VEOL2] = value;		
#endif							
#if defined(VERASE) && (VERASE < LEGALMAXCC)	
	else if(strcmp(name,cc_names[6])==0) /* ERASE */ 
		s.c_cc[VERASE] = value;		
#endif							
#if defined(VWERASE) && (VWERASE < LEGALMAXCC)	
	else if(strcmp(name,cc_names[7])==0) /* ERASEWORD */ 
		s.c_cc[VWERASE] = value;		
#endif							
#if defined(VINTR) && (VINTR < LEGALMAXCC)	
	else if(strcmp(name,cc_names[8])==0) /* INTERRUPT */ 
		s.c_cc[VINTR] = value;		
#endif							
#if defined(VKILL) && (VKILL < LEGALMAXCC)	
	else if(strcmp(name,cc_names[9])==0) /* KILL */ 
		s.c_cc[VKILL] = value;		
#endif							
#if defined(VMIN) && (VMIN < LEGALMAXCC)	
	else if(strcmp(name,cc_names[10])==0) /* MIN */ 
		s.c_cc[VMIN] = value;		
#endif							
#if defined(VQUIT) && (VQUIT < LEGALMAXCC)	
	else if(strcmp(name,cc_names[11])==0) /* QUIT */ 
		s.c_cc[VQUIT] = value;		
#endif							
#if defined(VLNEXT) && (VLNEXT < LEGALMAXCC)	
	else if(strcmp(name,cc_names[12])==0) /* QUOTENEXT */ 
		s.c_cc[VLNEXT] = value;		
#endif							
#if defined(VQUOTE) && (VQUOTE < LEGALMAXCC)	
	else if(strcmp(name,cc_names[13])==0) /* QUOTENEXT */ 
		s.c_cc[VQUOTE] = value;		
#endif							
#if defined(VREPRINT) && (VREPRINT < LEGALMAXCC)	
	else if(strcmp(name,cc_names[14])==0) /* REPRINT */ 
		s.c_cc[VREPRINT] = value;		
#endif							
#if defined(VSTART) && (VSTART < LEGALMAXCC)	
	else if(strcmp(name,cc_names[15])==0) /* START */ 
		s.c_cc[VSTART] = value;		
#endif							
#if defined(VSTATUS) && (VSTATUS < LEGALMAXCC)	
	else if(strcmp(name,cc_names[16])==0) /* STATUS */ 
		s.c_cc[VSTATUS] = value;		
#endif							
#if defined(VSTOP) && (VSTOP < LEGALMAXCC)	
	else if(strcmp(name,cc_names[17])==0) /* STOP */ 
		s.c_cc[VSTOP] = value;		
#endif							
#if defined(VSUSP) && (VSUSP < LEGALMAXCC)	
	else if(strcmp(name,cc_names[18])==0) /* SUSPEND */ 
		s.c_cc[VSUSP] = value;		
#endif							
#if defined(VSWTCH) && (VSWTCH < LEGALMAXCC)	
	else if(strcmp(name,cc_names[19])==0) /* SWITCH */ 
		s.c_cc[VSWTCH] = value;		
#endif							
#if defined(VSWTC) && (VSWTC < LEGALMAXCC)	
	else if(strcmp(name,cc_names[20])==0) /* SWITCH */ 
		s.c_cc[VSWTC] = value;		
#endif							
#if defined(VTIME) && (VTIME < LEGALMAXCC)	
	else if(strcmp(name,cc_names[21])==0) /* TIME */ 
		s.c_cc[VTIME] = value;		
#endif							
	else
		croak("Invalid control character passed to SetControlChars");
				
			}
#ifdef CC_TERMIOS
		if(tcsetattr(PerlIO_fileno(file),TCSANOW,&s))
#else
# ifdef CC_TERMIO
		if(ioctl(fileno(PerlIO_file),TCSETA,&s))
# endif
#endif
			croak("Unable to write terminal settings in SetControlChars");
		}
	}
	XSRETURN(1);
}


#endif



#ifdef CC_SGTTY

struct termstruct {
	struct sgttyb s1;

};
#define TermStructure struct termstruct

char	* cc_names[] = {	
	"ERASE",			
	"KILL",			
};

#define MAXCC	2

XS(XS_Term__ReadKey_GetControlChars)
{
	dXSARGS;
	if (items < 0 || items > 1) {
		croak("Usage: Term::ReadKey::GetControlChars()");
	}
	SP -= items;
	{
		PerlIO * file;
		TermStructure s;
	        if (items < 1)
	            file = STDIN;
	        else {
	            file = IoIFP(sv_2io(ST(0)));
	        }
        if(ioctl(fileno(PerlIO_file),TIOCGETP,&s.s1) 
			)
			croak("Unable to read terminal settings in GetControlChars");
		else {
			int i;
			EXTEND(sp,MAXCC*2);		
PUSHs(sv_2mortal(newSVpv(cc_names[0],strlen(cc_names[0])))); /* ERASE */
PUSHs(sv_2mortal(newSVpv(&s.s1.sg_erase,1))); 	
PUSHs(sv_2mortal(newSVpv(cc_names[1],strlen(cc_names[1])))); /* KILL */
PUSHs(sv_2mortal(newSVpv(&s.s1.sg_kill,1))); 	
			
		}
		PUTBACK;
		return;
	}
}

XS(XS_Term__ReadKey_SetControlChars)
{
	dXSARGS;
	/*if ((items % 2) != 0) {
		croak("Usage: Term::ReadKey::SetControlChars(%charpairs,file=STDIN)");
	}*/
	SP -= items;
	{
		PerlIO * file;
		TermStructure s;
	        if ((items%2)==0)
	            file = STDIN;
	        else {
	            file = IoIFP(sv_2io(ST(items-1)));
	        }

	        if(ioctl(PerlIO_fileno(file),TIOCGETP,&s.s1) 
			)
			croak("Unable to read terminal settings in SetControlChars");
		else {
			int i;
			char * name, value;
			for(i=0;i+1<items;i+=2) {
				name = SvPV(ST(i),PL_na);
				if( SvIOKp(ST(i+1)) || SvNOKp(ST(i+1)) )/* If Int or Float */
					value = (char)SvIV(ST(i+1));         /* Store int value */
				else                                    /* Otherwise */
					value = SvPV(ST(i+1),PL_na)[0];          /* Use first char of PV */

	if (0) ;					
	else if(strcmp(name,cc_names[0])==0) /* ERASE */ 
		s.s1.sg_erase = value;		
	else if(strcmp(name,cc_names[1])==0) /* KILL */ 
		s.s1.sg_kill = value;		
	else
		croak("Invalid control character passed to SetControlChars");
				
			}
	        if(ioctl(fileno(PerlIO_file),TIOCSETN,&s.s1) 
			) croak("Unable to write terminal settings in SetControlChars");
		}
	}
	XSRETURN(1);
}

#endif

#if !defined(CC_TERMIO) && !defined(CC_TERMIOS) && !defined(CC_SGTTY)
#define TermStructure int
XS(XS_Term__ReadKey_GetControlChars)
{
	dXSARGS;
	if (items <0 || items>1) {
		croak("Usage: Term::ReadKey::GetControlChars([FileHandle])");
	}
	SP -= items;
	{
		ST(0) = sv_newmortal();
		PUTBACK;
		return;
	}
}

XS(XS_Term__ReadKey_SetControlChars)
{
	dXSARGS;
	if (items < 0 || items > 1) {
		croak("Invalid control character passed to SetControlChars");
	}
	SP -= items;
	XSRETURN(1);
}

#endif

