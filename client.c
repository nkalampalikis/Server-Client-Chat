
/* Developed by Nikolaos Kalampalikis
   
   Chat Server for CS3516 Project */

#include"stdio.h"  
#include"stdlib.h"  
#include"sys/types.h"  
#include"sys/socket.h"  
#include"string.h"  
#include"netinet/in.h"  
#include"netdb.h"
  
 
#define BUF_SIZE 1024 
  
int main(int argc, char**argv) { 
	
	//Client Configurations
	struct sockaddr_in addr, cl_addr;  
	int sockfd, ret;  
	char buffer[BUF_SIZE];  
	struct hostent * server;
	char * serverAddr;
	int time =0;

	if (argc < 2) {
		printf("usage: client < ip address >\n");
		exit(1);  
	}

	serverAddr = argv[1]; 
 
 	sockfd = socket(AF_INET, SOCK_STREAM, 0);  
	
	// Creating a Socket
	if (sockfd < 0) { 
 
		printf("Error creating socket!\n");  
		exit(1);  
	}  
	
	printf("Socket created...\n");
   
	
	memset(&addr, 0, sizeof(addr));  
	addr.sin_family = AF_INET;  
	addr.sin_addr.s_addr = inet_addr(serverAddr);
	addr.sin_port = htons(2500);
     
	//Connecting to the Server 
	ret = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));  
	if (ret < 0) {  
	printf("Error connecting to the server!\n");  
	exit(1);  
	}

	memset(buffer, 0, BUF_SIZE);
  
	printf("Connected to the server...\n");  
	printf("\n");
	printf("\n");
	printf("Special Features");
	printf("\n");
	printf("\n");
	printf("Whisper to someone by using /whisper name message \n");
	printf("Change your name by using /name newname \n");
	printf("See who is active by using /active \n");
	printf("Quit the chat by using /quit \n");
	printf("\n");
	printf("\n");
	printf("[YOU]:");

	// Processing the messages
	while (fgets(buffer, BUF_SIZE, stdin) != NULL) {


		ret = sendto(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *) &addr, sizeof(addr));  
	
		if (ret < 0) {  
			printf("Error sending data!\n\t-%s", buffer);  
		}
	
		ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
  
		if (ret < 0) {  
			printf("Error receiving data!\n");    
		} else {
		
		fputs(buffer, stdout);
		printf("[YOU]:");
		} 
		 
	}

	 
	return 0;    
} 
