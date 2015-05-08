/* include iowraph */
/* Our own header.  Tabs are set for 4 spaces, not 8 */

#ifndef	__iowrap_h
#define	__iowrap_h

		/* prototypes for our stdio wrapper functions: see {Sec errors} */
void	 Fclose(FILE *);
FILE	*Fdopen(int, const char *);
char	*Fgets(char *, int, FILE *);
FILE	*Fopen(const char *, const char *);
void	 Fputs(const char *, FILE *);

#endif	/* __iowrap_h */
