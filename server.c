

/* Developed by Nikolaos Kalampalikis
   
   Chat Server for CS3516 Project */


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>

#define LimitOfClients  200

static unsigned int NumOfClients = 0;
static int ID = 1;

// Client struct
typedef struct {
	struct sockaddr_in addr;	// Client Address 
	int connfd;			// Connection file descriptor 
	int ID;				// Client ID 
	char Name[32];			// Client Name
} Client;

// List of Clients
Client *Clients[LimitOfClients];





/*



 ALL TYPES OF MESSAGES




*/

// Send message to all Clients except the sender 
void ToOthers(char *s, int ID){
	for(int i=0;i<LimitOfClients;i++){
		if(Clients[i]){
			if(Clients[i]->ID != ID){
				write(Clients[i]->connfd, s, strlen(s));
			}
		}
	}
}

// Sends message to all  
void ToAll(char *s){
	for(int i=0;i<LimitOfClients;i++){
		if(Clients[i]){
			write(Clients[i]->connfd, s, strlen(s));
		}
	}
}

// Sends message to the sender him/her self 
void ToSelf(const char *s, int connfd){
	write(connfd, s, strlen(s));
}

// Sends message to a certain client 
void ToSomeone(char *s, int ID){
	for(int i=0;i<LimitOfClients;i++){
		if(Clients[i]){
			if(Clients[i]->ID == ID){
				write(Clients[i]->connfd, s, strlen(s));
			}
		}
	}
}

// Strip Input 
void Strip(char *s){
	while(*s != '\0'){
		if(*s == '\r' || *s == '\n'){
			*s = '\0';
		}
		s++;
	}
}



/* 




ADDING AND REMOVING FROM THE QUEUE




*/

// Adds a client to the queue 
void AddToTheQ(Client *cl){
	for(int i=0;i<LimitOfClients;i++){
		if(!Clients[i]){
			Clients[i] = cl;
			return;
		}
	}
}

// Delete client from queue 
void DeleteFromTheQ(int ID){
	for(int i=0;i<LimitOfClients;i++){
		if(Clients[i]){
			if(Clients[i]->ID == ID){
				Clients[i] = NULL;
				return;
			}
		}
	}
}



/*



 CHAT FEATURES




*/


// Helper function that processes the buffer command and sends the messages 
void *BufferProcess(void *arg){
	char Output[1024];
	char Input[1024];
	int rlen;

	NumOfClients++;
	Client *cli = (Client *)arg;

	// Notification when a client joins the server
	printf(" CLIENT WITH ID %d, JUST JOINED THE SERVER.\n", cli->ID);
	//sprintf(Output, " CLIENT WITH ID %d, JUST JOINED THE SERVER.\r\n", cli->ID);
	//ToAll(Output);

	// Receive input from client 
	while((rlen = read(cli->connfd, Input, sizeof(Input)-1)) > 0){
	        Input[rlen] = '\0';
	        Output[0] = '\0';
		Strip(Input);
		
		// Ignore empty buffer 
		if(!strlen(Input)){
			continue;
		}
	
		// Special options 
		if(Input[0] == '/'){
			char *command, *key;
			command = strtok(Input," ");
			if(!strcmp(command, "/quit")){
				break;
			}else if(!strcmp(command, "/name")){
				key = strtok(NULL, " ");
				if(key){
					char *old_Name = strdup(cli->Name);
					strcpy(cli->Name, key);
					sprintf(Output, "THE USER %s IS NOW CALLED %s\r\n", old_Name, cli->Name);
					free(old_Name);
					ToAll(Output);
				}else{
					ToSelf("YOUR NEW  NAME CANNOT BE EMPTY!\r\n", cli->connfd);
				}
			// Whispering feature of the chat
			}else if(!strcmp(command, "/whisper")){
				key = strtok(NULL, " ");
				if(key){
					int ID = atoi(key);
					key = strtok(NULL, " ");
					if(key){
						sprintf(Output, "[PM][%s]", cli->Name);
						while(key != NULL){
							strcat(Output, " ");
							strcat(Output, key);
							key = strtok(NULL, " ");
						}
						strcat(Output, "\r\n");
						ToSomeone(Output, ID);
					}else{
						ToSelf("MESSAGE CANNOT BE EMPTY!\r\n", cli->connfd);
					}
				}else{
					ToSelf("REFERENCE CANNOT BE EMPTY!\r\n", cli->connfd);
				}

			// Sends a list of all the active Clients to the client that made the request
			}else if(!strcmp(command, "/active")){
				sprintf(Output, ">Clients %d\r\n", NumOfClients);
				ToSelf(Output, cli->connfd);
				
				char active[128];

				for(int i=0;i<LimitOfClients;i++){
					if(Clients[i]){
						sprintf(active, "ACTIVE: %s \r\n", Clients[i]->Name);
						ToSelf(active, cli->connfd);
					}
				}
			// Exception for unknown command
			}else{
				ToSelf("UNKOWN COMMAND!\r\n", cli->connfd);
			}
		}else{
			// Send message 
			sprintf(Output, "[%s] %s\r\n", cli->Name, Input);
			ToOthers(Output, cli->ID);
		}
	}

	// Close connection and delete client
	close(cli->connfd);
	
	// Notification when a client joins the server
	printf(" CLIENT WITH ID %d, JUST LEFT THE SERVER.\r\n", cli->ID);
	/*sprintf(Output, " CLIENT WITH ID %d, JUST LEFT THE SERVER.\r\n", cli->ID);
	ToAll(Output);
*/
	DeleteFromTheQ(cli->ID);
	free(cli);
	NumOfClients--;
	pthread_detach(pthread_self());
	
	return NULL;
}

int main(int argc, char *argv[]){
	int listenfd = 0, connfd = 0;
	struct sockaddr_in ServerAddr;
	struct sockaddr_in ClientAddr;
	pthread_t ThreadID;

	// Socket configurations 
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	ServerAddr.sin_port = htons(2500); 

	// Bind function
	if(bind(listenfd, (struct sockaddr*)&ServerAddr, sizeof(ServerAddr)) < 0){
		perror("FAILED BINDING!");
		return 1;
	}

	// Listen function
	if(listen(listenfd, 10) < 0){
		perror("FAILED LISTENING!");
		return 1;
	}

	printf("THE SERVER IS RUNNING!\n");

	// Handing client requests 
	while(1){

		socklen_t clilen = sizeof(ClientAddr);
		connfd = accept(listenfd, (struct sockaddr*)&ClientAddr, &clilen);

		// Check if the limit of Clients is reached 
		if((NumOfClients+1) == LimitOfClients){
			printf("NO MORE CLIENTS CAN JOIN THE SERVER!\n");
			close(connfd);
			continue;
		}

		// Client configurations 
		Client *cli = (Client *)malloc(sizeof(Client));
		cli->addr = ClientAddr;
		cli->connfd = connfd;
		cli->ID = ID++;
		sprintf(cli->Name, "%d", cli->ID);

		// Add client to the queue and handle the requests to a thread 
		AddToTheQ(cli);
		pthread_create(&ThreadID, NULL, &BufferProcess, (void*)cli);

	}
}
