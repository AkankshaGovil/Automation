
/* This program is used to send SIP Messages from a file to a particular SIP Client 

   USAGE:

   ./sendmessage <dest-ip> <dest-port> <sipmessage file>
*/
#include	<stdio.h>
#include	<sys/types.h>
#include	<stdlib.h>

#include 	<sys/time.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include 	<unistd.h>
#include 	<strings.h>
#define	MAXLINE	512

/*************************** DECLARATIONS ***********************/
void sendfile(FILE *fp, int sockfd, struct sockaddr*  pserv_addr, int servlen);

/**************************** DECLARATIONS **********************/


void sendfile(FILE *fp, int sockfd, struct sockaddr*  pserv_addr, int servlen)
{
	char * message;
	char ch;
	int length;

	message = (char *) malloc(5000*sizeof(char));
	if (message == NULL) { printf ("\nOut of memory\n"); exit(0);}
	length = 0;

	/*while ((ch=fgetc(fp))!=EOF)*/
	while ( fread(&ch,1,1,fp) !=0 )
		message[length++]=ch;
	message[length]='\0';

	printf ("######### Message Being Sent ################\n%s\n",message);

			if (sendto(sockfd, message, length, 0, pserv_addr, servlen) != length)
			{
				printf("Client : send to failed.\n");
				exit(0);
			}
			else printf("Message sent successfully.\n");
	if (ferror(fp))
	{
		printf("Client : Error in reading file.\n");
		exit(0);
	}
}

int main(int argc, char *argv[])
{
	int			sockfd;
	struct sockaddr_in	cli_addr, serv_addr;
	FILE *file=NULL;
	char temp[2];
	if (argc != 4)
	{
		printf ("\nUsage:\n%s <dest-ip> <dest-port> <file>\n",argv[0]);
		exit(0);
	}

	if( (file = fopen(argv[3],"rb")) ==NULL)
	{
		printf("error in opening file\n");
		exit(0);
	}
	fgets(temp,2,file);
	if(temp[0]=='\0')
	{
		printf("error in opening file\n");
		exit(0);
	}	
	rewind(file);
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port        = htons(atoi(argv[2]));

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("Client : Could not open datagram socket.\n");
		exit(0);
	}

	bzero((char *) &cli_addr, sizeof(cli_addr));
	cli_addr.sin_family      = AF_INET;
	cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	cli_addr.sin_port        = htons(0);
	if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0)
	{
		printf("Client : Could not bind to local address.\n");
		exit(0);
	}

	printf("Client : bound to port.\nNow sending data.\n");
	sendfile(file, sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

	close(sockfd);
	fclose(file);
	return 0;
}
