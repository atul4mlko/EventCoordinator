int portNumber = 9090;
char fileName[100];
char *clinIp;
int logEnabled = 0;
int isRequestCorrupted = 0;

struct node{
  char event;
	int nbOfAfterEvents;
	struct node *horizontalList[100];
};

struct node *verticalList[100];
struct node *verticalListTmp[100];
int nbOfEventsInVList = 0;
int nbOfEventsInVListTmp = 0;
int found = 0;
char reponseMessage[500];
char finalMessage[1000];

struct node *stack[100];

//functions

void showHelp();
void startDecServer();
void parseRequest(char []);
void logger(char []);

void reset();
void insert(char *);
void query();
void insertThisRelationship(char *);
void insertRelationship(char, char);
void printNodes();
struct node *getNodePointer(char);
int isNodePresent(char);
int findRelationship(struct node *,struct node *);
void getRelationship(char, char);
void reset();
void makeCopy(struct node *[], struct node *[]);
void rollBack();
