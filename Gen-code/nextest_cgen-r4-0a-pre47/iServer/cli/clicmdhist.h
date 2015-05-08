#ifndef _clicmdhist_h_
#define _clicmdhist_h_

void CliCmdHistoryPrint(FILE *out);

char * CliCmdHistoryGet(int i);

int CliCmdHistoryAdd(char *cmd);

void CliCmdHistoryInit(void);

int
CliHandleInteractive(char *prompt, char *buffer, int buflen, 
	int *argc, char **argv, int maxargv);

int PrintCommandPrompt(FILE *out, char *prompt);

int ProcessCommandLine(FILE *in, char *commandbuffer, int buflen);

#endif /* _clicmdhist_h_ */
