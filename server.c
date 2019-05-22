//==========================================================================================
// 	server
//		This program manages agents (IDed by ip addresses).  It prevents unauthorized
//		agents from accessing data, and provides authorized agents with: a list of active
//		agents, and a log of agent activities.
//==========================================================================================
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <time.h> 
#define MAX_AGS 5

// Each agent is IDed by their ip address.  Start monitors seconds they are active.
struct Agent{
	char ipaddr[20];
	time_t start;
};

void timeToFile(FILE *fpw);
void addMember(struct Agent *agents, char *ipaddr);
int isMember(struct Agent *agents, char *ipaddr);
void removeMember(struct Agent *agents, char *ipaddr);
void getList(struct Agent *agents, char *agentList);

int main(int argc, char *argv[]) 
{ 
	
	if (argc < 2)
	{
		printf("Please include port number on command line.\n");
		return 0;
	}
	int portNum = atoi(argv[1]);

	struct Agent agents[MAX_AGS];
	for (int j = 0; j < MAX_AGS; j++){
		memset(agents[j].ipaddr, 0, sizeof(agents[j].ipaddr));		// Ensure agent is empty (eg inactive);
	}

	// Server messages.
	char *actError = "$ ERROR: Unknown action. Choose from: JOIN, LEAVE, LIST, LOG\0";
	char *ok = "$ OK\0";
	char *already = "$ ALREADY MEMBER\0";
	char *notMemb = "$ NOT MEMBER\0";
	// Reading and writing to text file.
	FILE *fpw;
	char writeBuff[1024];
	char readBuff[1024];
	char *agentList[1024];
	fpw = fopen("log.txt", "w+");
	int isMem;

	// Not mine.
	int server_fd, client_socket, streamread; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	char buffer[1024] = {0}; 
	
	// Create socket.
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("ERROR: failure to create socket."); 
		exit(EXIT_FAILURE); 
	} 
	
	// Attach socket to port.
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{ 
		perror("ERROR: failure to attach socket to port."); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( portNum ); 
	
	// Bind socket to server ip address.
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("ERROR: failure to bind."); 
		exit(EXIT_FAILURE); 
	} 

	while (1)
	{
		memset(buffer, 0, sizeof(buffer));
		//printf("\nBuffer: %s", buffer);
		fflush(stdout);		

		if (listen(server_fd, 3) < 0){ 
			perror("listen"); 
			exit(EXIT_FAILURE); 
		} 
		if ((client_socket = accept(server_fd, (struct sockaddr *)&address, 
						(socklen_t*)&addrlen))<0){ 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		} 
		
		streamread = read( client_socket , buffer, 1024); 

		// Get client's ip address.
		struct sockaddr_in* pv4addr = (struct sockaddr_in*)&address;
		struct in_addr ipAddr = pv4addr->sin_addr;  

		// Convert to string.
		char cAddr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &ipAddr, cAddr, INET_ADDRSTRLEN);
		
		// Grab and validate agent's action.
		int actNum = getAction(buffer);
		//printf("\nYour actNum: %d", actNum);

		// Correct bad actions.
		if (actNum == 0){
			printf("\n%s", actError);
			send(client_socket , actError , strlen(actError) , 0); 
			close(client_socket);
		} 

		// Process JOIN request.
		if (actNum == 1){
			printf("\n# %s", buffer);
			timeToFile(fpw);
			fprintf(fpw, "Receive -- Agent %s -- Request to join.", cAddr);	
			isMem = isMember(&agents, cAddr);
			//printf("\nisMem: %d", isMem);
			
			if (isMem == 0)
			{
				addMember(&agents, cAddr);
				printf("\n$ Agent %s added.", cAddr);
				timeToFile(fpw);
				fprintf(fpw, "Respond -- Agent %s -- Join granted.", cAddr);
				send(client_socket , ok , strlen(ok) , 0); 
				close(client_socket);
			}
			else
			{
				printf("\n$ Agent %s already member.", cAddr);
				timeToFile(fpw);
				fprintf(fpw, "Respond -- Agent %s -- No change.", cAddr);
				send(client_socket, already, strlen(already), 0);
				close(client_socket);
			}
			fflush(fpw);
		}
		if (actNum == 2){
			printf("\n# %s", buffer);
			timeToFile(fpw);
			fprintf(fpw, "Receive -- Agent %s -- Request to leave.", cAddr);	
			isMem = isMember(&agents, cAddr);
			//printf("\nisMem %d", isMem);
			if (isMem == 0){
				printf("\n$ Agent %s not member.", cAddr);
				timeToFile(fpw);
				fprintf(fpw, "Respond -- Agent %s -- Not member. No change.", cAddr);
				send(client_socket , notMemb , strlen(notMemb) , 0); 
				close(client_socket);
			}
			else{
				removeMember(agents, cAddr);
				printf("\n$ Agent %s has left.", cAddr);
				timeToFile(fpw);
				fprintf(fpw, "Respond -- Agent %s -- Leave granted.", cAddr);
				send(client_socket, ok, strlen(ok), 0);
				close(client_socket);
			}
			fflush(fpw);
		}
		if (actNum == 3){
			printf("\n# %s", buffer);
			timeToFile(fpw);
			fprintf(fpw, "Receive -- Agent %s -- Request to access agent list.", cAddr);	
			isMem = isMember(&agents, cAddr);
			if (isMem == 0){
				printf("\n$ Agent %s not member.", cAddr);
				timeToFile(fpw);
				fprintf(fpw, "Respond -- Agent %s -- Not member. Agent list access denied.", cAddr);
				send(client_socket , notMemb , strlen(notMemb) , 0); 
				close(client_socket);
			}
			else{
				printf("\n$ List sent.");
				timeToFile(fpw);
				fprintf(fpw, "Respond -- Agent %s -- Agent list access granted.", cAddr);
				getList(&agents, agentList);
				//printf("\n%s", agentList);
				send(client_socket, agentList, strlen(agentList), 0);
				close(client_socket);
			}	
			fflush(fpw);		
		}
		if (actNum == 4){
			printf("\n# %s", buffer);
			timeToFile(fpw);	
			fprintf(fpw, "Receive -- Agent %s -- Request to access agent log.", cAddr);	
			isMem = isMember(&agents, cAddr);
			if (isMem == 0){
				printf("\n$ Agent %s not member.", cAddr);
				timeToFile(fpw);
				fprintf(fpw, "Respond -- Agent %s -- Not member. Access to agent log denied.", cAddr);
				send(client_socket , notMemb , strlen(notMemb) , 0); 
				close(client_socket);
			}
			else{
				printf("\n$ Log sent.");
				// Read from file.
				FILE *fpr;
				fpr = fopen("log.txt", "r");
				while (fgets(readBuff, 1024, fpr) != NULL)
					send(client_socket, readBuff, strlen(readBuff), 0);
				close(fpr);

				timeToFile(fpw);
				fprintf(fpw, "Respond -- Agent %s -- Access to agent log granted.", cAddr);
				close(client_socket);
			}
			fflush(fpw);	
		}
	}
	close(fpw);
	return 0; 
} 

//===============================================================================
//  getAction:		Maps action string to action integer.
//		Takes:  	String; action command.
//		Returns:  	Integer; 0 - invalid; 1 - join; 2 - leave; 3 - list; 4 - log.
//===============================================================================
int getAction(char* action)
{
	//printf("\nHere is your action: %s", action);

	// Accepted actions.
	char sJoin[] = {'J','O','I','N','\0'};
	char sLeave[] = {'L','E','A','V','E','\0'};
	char sList[] = {'L','I','S','T','\0'};
	char sLog[] = {'L','O','G','\0'};
	int isdiff = 0;

	// Test JOIN
	isdiff = strcmp(sJoin, action);
	if (isdiff == 0) return 1;
	// Test LEAVE
	isdiff = strcmp(sLeave, action);
	if (isdiff == 0) return 2;
	// Test LIST
	isdiff = strcmp(sList, action);
	if (isdiff == 0) return 3;
	// Test LOG
	isdiff = strcmp(sLog, action);
	if (isdiff == 0) return 4;

	return 0;
}
//===============================================================================
//	timeToFile:		writes a string timestamp to a text file.
//		takes:		FILE pointer.
//===============================================================================
void timeToFile(FILE *fpw){
	time_t t = time(NULL);
	struct tm time = *localtime(&t);
	fprintf(fpw, "\n%02d:%02d:%02d -- ", time.tm_hour, time.tm_min, time.tm_sec);
}

//===============================================================================
//	addMember:		Adds member to agent list.  Prints error if list full.
//		takes:		Array of Agents.
//					String of new agent's ip address.
//===============================================================================
void addMember(struct Agent *agents, char *ipaddr){

	for (int j = 0; j < MAX_AGS; j++){
		if (agents[j].ipaddr[0] == '\0'){
			//printf("\nAgent %d empty.  Adding", j);
			strcpy(agents[j].ipaddr, ipaddr);
			agents[j].start = time(0);
			return;
		}
	}
	printf("\nERROR: agent list full. Max is %d.", MAX_AGS);
}
//===============================================================================
//	isMember:		Tests if a provided ip address exists in agent list.
//		takes:		Array of Agents.
//					String of tester ip address.
//		returns:	Int; 1 if member, 0 if not member.	
//===============================================================================
int isMember(struct Agent *agents, char *ipaddr){
	
	int isdiff;
	for (int j = 0; j < MAX_AGS; j++){
		isdiff = strcmp(agents[j].ipaddr, ipaddr);
		if (isdiff == 0){
			return 1;
		}
	}
	return 0;
}
//===============================================================================
//	removeMember:	Removes an agent from the list via ip address.
//		takes:		Array of agents.
//					String of ip address.
//===============================================================================
void removeMember(struct Agent *agents, char *ipaddr){
	int isdiff;
	for (int j = 0; j < MAX_AGS; j++){
		isdiff = strcmp(agents[j].ipaddr, ipaddr);
		if (isdiff == 0){
			//printf("\nagent %d: ( %s ) removed.", j, agents[j].ipaddr);
			memset(agents[j].ipaddr, 0, sizeof(agents[j].ipaddr));
			//printf("\nagent %d: ( %s )", j, agents[j].ipaddr);
		}
	}
}
//===============================================================================
//	getList:		Creates a string of agents, and their seconds active.
//		takes:		Array of Agents.
//					String agentList.
//===============================================================================
void getList(struct Agent *agents, char *agentList){

	int secs;
	int *timeStr[128];
	memset(agentList, 0, sizeof(agentList));

	strcpy(agentList, "\n");
	for (int j = 0; j < MAX_AGS; j++){
		if (agents[j].ipaddr[0] == '\0') continue;		// Skip empty agent entries.
		strcat(agentList, agents[j].ipaddr);			// Add ip addr to front.
		// Get seconds active.
		memset(timeStr, 0, sizeof(timeStr));			// Clear seconds string.
		secs = difftime(time(0), agents[j].start);		// Get seconds active.
		sprintf(timeStr, "%d", secs);					// Convert to string.
		strcat(agentList, ",  ");						// Write to list.
		strcat(agentList, timeStr);
		strcat(agentList, "\n");
	}
}