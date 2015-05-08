#include	"unp.h"
#include	"rs.h"


Cmd *
htonCmd(Cmd *cmdp)
{
	if (cmdp != NULL) {
		cmdp->cmdlen = htonl(cmdp->cmdlen);
		cmdp->cmdseq = htonl(cmdp->cmdseq);
		cmdp->cmdtyp = htonl(cmdp->cmdtyp);
		cmdp->cmdrval = htonl(cmdp->cmdrval);
		cmdp->cmdact = htonl(cmdp->cmdact);
	}

	return(cmdp);
}

int
BufToEnts(char *bufp, int buflen, CliEntry **clipp)
{
	Cmd *cmdp;
	CliEntry **s;

	s = clipp;
	while (buflen > 0) {
		cmdp = (Cmd *)bufp;
		CmdToEnt(cmdp, cmdp->cmdlen, *s);
		s++;
		buflen -= cmdp->cmdlen;
		bufp += cmdp->cmdlen;
	}

	*s = NULL;

	if (buflen != 0) {
		NETERROR(MRSD, ("BufToEnts: Corrupt Packet Received \n"));
		FreeCliEntries(clipp);
		return(-1);
	}
	else
		return(0);
}

int
EntsToBuf(CliEntry ***clippp, char *buf, int buflen)
{
	int 	datalen = 0;
	int		cmdlen;

	while ((**clippp != NULL) && ((cmdlen = (**clippp)->cmdlen) + datalen < buflen) ) {
		memcpy(buf+datalen, (char *) htonCmd((Cmd *)((**clippp)->clicmd)), cmdlen);
		datalen += cmdlen;
		FreeCliEntry(**clippp);     /* Free the CliEntry after copying it */
		(*clippp)++;
	}

	return(datalen);
}

int
EntToCmd(CliEntry *clip,  char *buf, int *buflen)
{
	if ((clip != NULL) && (*buflen > clip->cmdlen))  {
		memcpy(buf, clip->clicmd, clip->cmdlen);
		Free(clip);     /* Free the CliEntry after copying it */
		return(0);
	}
	
	return(-1);
}

int
CmdToEnt(Cmd *cmdp, int cmdlen, CliEntry *clip)
{
	clip = Malloc(sizeof(CliEntry));
	clip->cmdlen = cmdp->cmdlen;
	clip->seqno = cmdp->cmdseq;
	clip->clicmd = (char *)cmdp;
	return(0);
}

/* Skip All the digits and whitespaces and return the pointer */
char *
skip( char *line)
{
	char *s = line;

	while (isdigit((int)*s) || isspace((int)*s))
		s++;	

	return(s);
}

/*
 * This function skips the cli command in the pkt
 * pointed to by cmdp. The return value points to the next
 * If it skips over the last command, then it returns NULLCMD
 */
Cmd *
SkipCmd(RSPkt *pktp, Cmd *cmdp)
{
	char *cmdpos;

	cmdpos = (char *)cmdp + cmdp->cmdlen;

	if ( ((char *)pktp + PktLen(pktp)) > cmdpos )
		return( (Cmd *)cmdpos );
	else 
		return( NULLCMD );
}

/*
 * PktToStr takes a null terminated buffer , buf, of length
 * buflen. This buffer may contain multiple '\n' terminated
 * lines. StrArr returns an array of lines . Each line is 
 * represented by a null terminated string. No memory is allocated
 * for StrArr. The calling function should allocate memory 
 * for maximum possible nonempty lines present in the buffer.
 */
int
PktToStr(char *buf, int buflen, char **StrArr)
{
	char	**line;
	char	*toksep = "\n";

    line = StrArr;

    /* Check for a null at the end of the data string */
	if (buf[buflen-1] != '\0') {
		NETERROR(MRSD, ("PktToStr: Rcvd Pkt corrupt. Null character not found at the end \n"));
		return(-1);
	}

	*line = strtok(buf, toksep);
    while( *line++ != NULL )
        *line = strtok(NULL, toksep);

	return(0);
}

/*
 * StrToPkt takes a pointer to an array of lines and fills
 * a buffer pointed to by buf. All the lines are newline terminated
 * to form one string. It returns a value of 0 if successful. 
 * Frees all the Lines. 
 */ 
int
StrToPkt(char **StrArr, char *buf, int buflen)
{
	char	**line;

	line = StrArr;
	*buf = '\0';     /* Create an empty string */

	while ((*line != NULL) && ((buflen -= (strlen(*line)+1)) > 0)) {
		strcat(buf, *line);
		strcat(buf, "\n");
		Free(*line);		/* Free the line after copying it */
		line++;	
	}

	Free(StrArr);   		/* Free the pointer to lines */

	if (buflen <= 0) {
		NETERROR(MRSD, ("StrToPkt: Pkt space overflow\n"));
		return(-1);
	}

	return(0);
}
