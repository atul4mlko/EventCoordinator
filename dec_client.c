#include  <stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<errno.h>
#include	<inttypes.h>
#include	<netdb.h>
#include	"dec_client.h"

int main(int argc , char *argv[]){
	int i;
	for (i=1; i < argc; i++){
		if (strcmp(argv[i],"-h") == 0){
			showHelp();
		}else
		if (strcmp(argv[i],"-p") == 0){
			portNumber = atoi(argv[++i]);
		}else
		if (strcmp(argv[i],"-s") == 0){
			strcpy(serverHost,argv[++i]);
		}else
			printf("%s is a wrong option. Please enter the correct option",argv[i]);
	}
	clientSetup();
	return 0;
}

/************************************************************
*This function sets up and connect the client to the server.
*************************************************************/
void clientSetup(){
	struct hostent *hp, *gethostbyname();
	int s;
	if ((hp = gethostbyname(serverHost)) == NULL) {
		fprintf(stderr, "%s: unknown host\n",serverHost);
		exit(1);
	}
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	memcpy(&serv_addr.sin_addr, hp->h_addr, hp->h_length);
	serv_addr.sin_port = htons(portNumber);
		if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			exit(1);
		}
		if (connect(s, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
			perror("connect");
			exit(1);
		} else
			fprintf(stderr, "Connected...\n");
		char inputSeq[500],response[500];
		fgets(inputSeq,sizeof inputSeq, stdin);
		write(s,inputSeq,sizeof(inputSeq));
		read(s,response,sizeof(response));
		printf("%s\n",response);
		close(s);
}

/************************************************************
*This function will display the help options.
*************************************************************/
void showHelp(){
	printf("\n---------------------------------------------------------------------\n");
	printf("-h\t:Print the usage summary with all the options.");
	printf("\n-s\t:Connects to this server-host.");
	printf("\n-p\t:Connects to this port number of the server. By default it is 9090.");
	printf("\n---------------------------------------------------------------------\n\n");
	exit(0);
}
