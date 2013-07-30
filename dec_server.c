#include  <stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<errno.h>
#include	<netdb.h>
#include	"dec_server.h"

int main(int argc , char *argv[]){
	int i;
	for (i=1; i < argc; i++){
		if (strcmp(argv[i],"-h") == 0){
			showHelp();
		}else
		if (strcmp(argv[i],"-p") == 0){
			portNumber = atoi(argv[++i]);
		}else
		if (strcmp(argv[i],"-l") == 0){
			strcpy(fileName,argv[++i]);
			logEnabled = 1;
			FILE *fp;
			if ((fp = fopen(fileName,"r")) != NULL){
				printf("All the logs will be logged in %s.\n",fileName);
				fclose(fp);
			}else{
				printf("No such file exist. Please enter the correct file name.\n");
				return 0;
			}
		}else
			printf("%s is a wrong option. Please enter the correct option",argv[i]);
	}
	startDecServer();
	return 0;
}

/************************************************************
*This function starts the server based on the options given.
*If no options are given then it takes the default values.
*************************************************************/
void startDecServer(){
	int fd;
	fd = socket(AF_INET,SOCK_STREAM,0);
	if (fd == -1){
		printf("Unable to create socket!!%d\n",errno);
		exit(0);
	}
	struct sockaddr_in serv_addr;
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(portNumber);

	if (bind(fd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1){
		printf("Unable to bind socket!!\n");
		exit(0);
	}
	if (listen(fd, 5) == -1){
		printf("%d",errno);
	}
	struct sockaddr_in cli_addr;
	unsigned clilen;
	int connfd;
	clilen = sizeof(cli_addr);
	while(1)
	{
		connfd = accept(fd, (struct sockaddr *) &cli_addr, &clilen);
		if(connfd < 0)
			fprintf(stderr,"server: accept error\n"), exit(0);
		clinIp = inet_ntoa(cli_addr.sin_addr);

		struct hostent *he;
		struct in_addr ipv4addr;

		inet_pton(AF_INET, clinIp, &ipv4addr);
		he = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);

		char commandLine[500],req[100];
		read(connfd,commandLine,sizeof(commandLine));
		sprintf(req,"Request received from %s:\n%s",he->h_name,commandLine);
		parseRequest(commandLine);
		strcat(finalMessage,"-----------------------------------\n");
		write(connfd,finalMessage,sizeof(finalMessage));
		logger(req);
		logger(finalMessage);
		finalMessage[0] = '\0';
		close(connfd);
	}
}

/************************************************************
*This function parses the incoming requests and calls a
*particular function for a particular action.
*************************************************************/
void parseRequest(char inputCommandLine[]){
	char *tokenpch,*restpch,*bl,*tokenbl,*restbl;
	while (tokenpch = strtok_r(inputCommandLine, ";", &restpch)){
		bl = tokenpch;
		while (tokenbl = strtok_r(bl, " ", &restbl)){
			if (strcmp(tokenbl,"reset") == 0){
				reset();
			}
			if (strcmp(tokenbl,"insert") == 0){
				nbOfEventsInVListTmp = nbOfEventsInVList;
				makeCopy(verticalList,verticalListTmp);
				isRequestCorrupted = 0;
				insert(restbl);
			}
			if (strcmp(tokenbl,"query") == 0){
				query(restbl);
			}
			bl = restbl;
		}
		inputCommandLine = restpch;
	}
}

void makeCopy(struct node *from[], struct node *to[]){
	int i,j;
	for (i=0;i<nbOfEventsInVListTmp;i++){
		to[i] = malloc(sizeof(struct node));;
		to[i]->event = from[i]->event;
		to[i]->nbOfAfterEvents = from[i]->nbOfAfterEvents;
		for (j=0;j<from[i]->nbOfAfterEvents;j++){
			to[i]->horizontalList[j] = malloc(sizeof(struct node));
			to[i]->horizontalList[j]->event = from[i]->horizontalList[j]->event;
		}
	}
}

void rollBack(){
	int i,j;
	for (i=0;i<nbOfEventsInVListTmp;i++){
		verticalList[i]->event = verticalListTmp[i]->event;
		verticalList[i]->nbOfAfterEvents = verticalListTmp[i]->nbOfAfterEvents;
		for (j=0;j<verticalListTmp[i]->nbOfAfterEvents;j++){
			verticalList[i]->horizontalList[j]->event = verticalListTmp[i]->horizontalList[j]->event;
		}
	}
	nbOfEventsInVList = nbOfEventsInVListTmp;
	for (i=0; i<nbOfEventsInVListTmp; i++){
		for (j=0; j<verticalListTmp[i]->nbOfAfterEvents; j++){
			verticalListTmp[i]->horizontalList[j] = NULL;
		}
		verticalListTmp[i] = NULL;
	}
}

void insert(char *param){
	char *tokenbl,*restParam;
	while (tokenbl = strtok_r(param, " ", &restParam)){
		if (strcmp(tokenbl,"insert") != 0 && isRequestCorrupted == 0){
			insertThisRelationship(tokenbl);
		}
		param = restParam;
	}
}

void query(char *param){
	char *tokenbl,*restParam;
	char events[2];int i=0;
	while (tokenbl = strtok_r(param, " ", &restParam)){
		if (strcmp(tokenbl,"query") != 0){
			events[i++] = tokenbl[0];
		}
		param = restParam;
	}
	getRelationship(events[0],events[1]);
}


void insertThisRelationship(char *relationShip){
	char *tokenbl,*restParam;
	char events[2];int i=0;
	while (tokenbl = strtok_r(relationShip, "->", &restParam)){
		events[i++] = tokenbl[0];
		relationShip = restParam;
	}
	if (events[0] != events[1])
		insertRelationship(events[0],events[1]);
	else{
		strcpy(reponseMessage,"Both events are same.\n");
		strcat(finalMessage,reponseMessage);
	}
}

/************************************************************
*This function inserts the two events based on the request.
*************************************************************/
void insertRelationship(char before, char after){
	int isBeforeNodePresent, isAfterNodePresent;
	isBeforeNodePresent = isNodePresent(before);
	isAfterNodePresent = isNodePresent(after);
	if (isBeforeNodePresent == 0 && isAfterNodePresent == 0){
		//add both the nodes
		struct node *nBefore = malloc(sizeof(struct node));
		struct node *nAfter = malloc(sizeof(struct node));
		nBefore->event = before;
		verticalList[nbOfEventsInVList++] = nBefore;
		nAfter->event = after;
		verticalList[nbOfEventsInVList++] = nAfter;
		nBefore->horizontalList[nBefore->nbOfAfterEvents++] = nAfter;
		strcpy(reponseMessage,"Response from server: INSERT DONE.\n");
		strcat(finalMessage,reponseMessage);
	}else if (isBeforeNodePresent == 1 && isAfterNodePresent == 0){
		//add after node in horizontalList of before node
		struct node *nAfter = malloc(sizeof(struct node));
		nAfter->event = after;
		struct node *nBefore = getNodePointer(before);
		nBefore->horizontalList[nBefore->nbOfAfterEvents++] = nAfter;
		//add after node in verticalList
		verticalList[nbOfEventsInVList++] = nAfter;
		strcpy(reponseMessage,"Response from server: INSERT DONE.\n");
		strcat(finalMessage,reponseMessage);
	}else if (isBeforeNodePresent == 0 && isAfterNodePresent == 1){
		//add before node in verticalList
		struct node *nBefore = malloc(sizeof(struct node));
		nBefore->event = before;
		verticalList[nbOfEventsInVList++] = nBefore;
		//add after node in horizontalList of before node
		struct node *nAfter = getNodePointer(after);
		nBefore->horizontalList[nBefore->nbOfAfterEvents++] = nAfter;
		strcpy(reponseMessage,"Response from server: INSERT DONE.\n");
		strcat(finalMessage,reponseMessage);
	}else if (isBeforeNodePresent == 1 && isAfterNodePresent == 1){
		//find the relation between the nodes
		struct node *nBefore = getNodePointer(before);
		struct node *nAfter = getNodePointer(after);
		int relation;
		relation = findRelationship(nBefore,nAfter);
		if (relation == 0){
			relation = findRelationship(nAfter,nBefore);
			if (relation == 0){
				nBefore->horizontalList[nBefore->nbOfAfterEvents++] = nAfter;
				sprintf(reponseMessage,"Response from server: INSERT DONE.\n");
				strcat(finalMessage,reponseMessage);
			}else{
				sprintf(reponseMessage,"Response from server: CONFLICT DETECTED. INSERT FAILED.\n");
				strcat(finalMessage,reponseMessage);
				sprintf(reponseMessage,"\t%c->%c and %c->%c cannot be true at the same time!\n",before,after,after,before);
				strcat(finalMessage,reponseMessage);
				sprintf(reponseMessage,"\tYour entire insert request will not be processed.\n");
				strcat(finalMessage,reponseMessage);
				rollBack();
				isRequestCorrupted = 1;
			}
		}else{
			sprintf(reponseMessage,"Already is desired relationship.\n");
			strcat(finalMessage,reponseMessage);
		}
	}
}

/************************************************************
*This function prints the global list.
*************************************************************/
void printNodes(){
	int i,j;
	for (i=0; i<nbOfEventsInVList; i++){
		printf("%c = ",verticalList[i]->event);
		for (j=0; j<verticalList[i]->nbOfAfterEvents; j++)
			printf("%c ",verticalList[i]->horizontalList[j]->event);
		printf("\n");
	}
}

/************************************************************
*This function resets the global list.
*************************************************************/
void reset(){
	int i,j;
	for (i=0; i<nbOfEventsInVList; i++){
		for (j=0; j<verticalList[i]->nbOfAfterEvents; j++){
			verticalList[i]->horizontalList[j] = NULL;
		}
		verticalList[i] = NULL;
	}
	nbOfEventsInVList = 0;
	sprintf(reponseMessage,"Response from server: RESET DONE.\n");
	strcat(finalMessage,reponseMessage);
}

/************************************************************
*This function tells whether a particular event is present
*in the global list of not.
*************************************************************/
int isNodePresent(char event){
	int i;
	for (i=0; i<nbOfEventsInVList; i++){
		if (verticalList[i]->event == event)
			return 1;
	}
	return 0;
}

/************************************************************
*This function finds the relationship between two events.
*************************************************************/
int findRelationship(struct node *before,struct node *after){//printf("findRelationship %c and %c\n",before,after);
	int i;
	found = 0;
	for (i=0; i<before->nbOfAfterEvents; i++){
		if (before->horizontalList[i]->event == after->event){
			found = 1;
		}else{
			findRelationship(getNodePointer(before->horizontalList[i]->event),after);
		}
	}
	return found;
}

/************************************************************
*This function returns the relationship between two events.
*************************************************************/
void getRelationship(char before,char after){//printf("Query %c and %c\n",before,after);
	int isBeforeNodePresent, isAfterNodePresent;
	isBeforeNodePresent = isNodePresent(before);
	isAfterNodePresent = isNodePresent(after);
	if (isBeforeNodePresent == 1 && isAfterNodePresent == 1){
		struct node *nBefore = getNodePointer(before);
		struct node *nAfter = getNodePointer(after);
		int relation;
		relation = findRelationship(nBefore,nAfter);
		if (relation == 0){
			relation = findRelationship(nAfter,nBefore);
			if (relation == 0){
				sprintf(reponseMessage,"Response from server: %c and %c are concurrent.\n",before,after);
				strcat(finalMessage,reponseMessage);
			}else{
				sprintf(reponseMessage,"Response from server: %c happened before %c.\n",after,before);
				strcat(finalMessage,reponseMessage);
			}
		}else{
			sprintf(reponseMessage,"Response from server: %c happened before %c.\n",before,after);
			strcat(finalMessage,reponseMessage);
		}
	}else if (isBeforeNodePresent == 0 && isAfterNodePresent == 0){
		sprintf(reponseMessage,"Response from server: Events not found: %c, %c.\n",before,after);
		strcat(finalMessage,reponseMessage);
	}else if (isBeforeNodePresent == 1 && isAfterNodePresent == 0){
		sprintf(reponseMessage,"Response from server: Event not found: %c.\n",after);
		strcat(finalMessage,reponseMessage);
	}else if (isBeforeNodePresent == 0 && isAfterNodePresent == 1){
		sprintf(reponseMessage,"Response from server: Event not found: %c.\n",before);
		strcat(finalMessage,reponseMessage);
	}
}

/************************************************************
*This function gets the pointer of the node where the event
*resides.
*************************************************************/
struct node *getNodePointer(char event){
	int i;
	struct node *pointer;
	for (i=0; i<nbOfEventsInVList; i++){
		if (verticalList[i]->event == event)
			pointer = verticalList[i];
	}
	return pointer;
}

/************************************************************
*This function logs the activity of the server either in a 
*file or on the stdout based on the option provided.
*************************************************************/
void logger(char message[]){
	if (logEnabled ==1){
		FILE *logfile;
		logfile = fopen (fileName, "a");
		if (logfile == NULL)
		{
			fprintf(stderr, "Can't open output file %s!\n",fileName);
			exit(1);
		}
		fputs (message,logfile);
		fclose(logfile);
		}
	else{
		printf("%s",message);
	}
}


/************************************************************
*This function will display the help options.
*************************************************************/
void showHelp(){
	printf("\n---------------------------------------------------------------------\n");
	printf("-h\t:Print the usage summary with all the options.");
	printf("\n-p\t:Sets the port number for the server. By default it is 9090.");
	printf("\n-l\t:Sets the file where logs will be logged.");
	printf("\n---------------------------------------------------------------------\n\n");
	exit(0);
}
