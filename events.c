#include<stdio.h>
#include<stdlib.h>

struct node{
  char event;
	int nbOfAfterEvents;
	int visited;
	struct node *horizontalList[100];
};

struct node *verticalList[100];
int nbOfEventsInVList = 0;
int found = 0;

struct node *stack[100];

void insertRelationship(char, char);
void printNodes();
struct node *getNodePointer(char);
int isNodePresent(char);
int findRelationship(struct node *,struct node *);
void performDFSOnNode(struct node *);
int query(char, char);
void reset();

void main(){
	insertRelationship('A','B');
	insertRelationship('B','C');
	insertRelationship('D','E');
	//insertRelationship('A','D');
	//insertRelationship('E','F');
	//insertRelationship('D','M');
	//performDFSOnNode(getNodePointer('A'));
	//printNodes();
	//printf("Insert A->F\n");
	//insertRelationship('A','F');
	query('A','D');
	//reset();
	//printNodes();
}


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
	}else if (isBeforeNodePresent == 1 && isAfterNodePresent == 0){
		//add after node in horizontalList of before node
		struct node *nAfter = malloc(sizeof(struct node));
		nAfter->event = after;
		struct node *nBefore = getNodePointer(before);
		nBefore->horizontalList[nBefore->nbOfAfterEvents++] = nAfter;
		//add after node in verticalList
		verticalList[nbOfEventsInVList++] = nAfter;
	}else if (isBeforeNodePresent == 0 && isAfterNodePresent == 1){
		//add before node in verticalList
		struct node *nBefore = malloc(sizeof(struct node));
		nBefore->event = before;
		verticalList[nbOfEventsInVList++] = nBefore;
		//add after node in horizontalList of before node
		struct node *nAfter = getNodePointer(after);
		nBefore->horizontalList[nBefore->nbOfAfterEvents++] = nAfter;
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
				printf("Concurrent.\n");
			}else{
				printf("Conflict.\n");
			}
		}else{
			printf("Desired.\n");
		}
			//if already in desired relation then send the same msg
			//if in the opposite relation then send a CONFLICT msg
			//if in concurrent relation then relate them as per the request
	}
}

void printNodes(){
	int i,j;
	for (i=0; i<nbOfEventsInVList; i++){
		printf("%c = ",verticalList[i]->event);
		for (j=0; j<verticalList[i]->nbOfAfterEvents; j++)
			printf("%c ",verticalList[i]->horizontalList[j]->event);
		printf("\n");
	}
}

void reset(){
	int i,j;
	for (i=0; i<nbOfEventsInVList; i++){
		for (j=0; j<verticalList[i]->nbOfAfterEvents; j++){
			verticalList[i]->horizontalList[j] = NULL;
		}
		verticalList[i] = NULL;
	}
	nbOfEventsInVList = 0;
}

int isNodePresent(char event){
	int i;
	for (i=0; i<nbOfEventsInVList; i++){
		if (verticalList[i]->event == event)
			return 1;
	}
	return 0;
}

int findRelationship(struct node *before,struct node *after){//printf("findRelationship %c and %c\n",before,after);
	int i,value;
	before->visited = 1;
	for (i=0; i<before->nbOfAfterEvents; i++){
		if (before->horizontalList[i]->event == after->event){
			found = 1;
		}else{
			findRelationship(getNodePointer(before->horizontalList[i]->event),after);
		}
	}
	return found;
}

int query(char before,char after){//printf("query %c and %c\n",before,after);
	if (isNodePresent(before) == 0 || isNodePresent(after) == 0){
		printf("Node doest exists!!");exit(0);
	}
	struct node *nBefore = getNodePointer(before);
	struct node *nAfter = getNodePointer(after);
	int relation;
	relation = findRelationship(nBefore,nAfter);
	if (relation == 0){
		relation = findRelationship(nAfter,nBefore);
		if (relation == 0){
			printf("Concurrent.\n");
		}else{
			printf("Conflict.\n");
		}
	}else{
		printf("Desired.\n");
	}
}
struct node *getNodePointer(char event){
	int i;
	struct node *pointer;
	for (i=0; i<nbOfEventsInVList; i++){
		if (verticalList[i]->event == event)
			pointer = verticalList[i];
	}
	return pointer;
}
