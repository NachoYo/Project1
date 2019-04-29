#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

int state1=0, state2=0;
char *buffer;
char r_buffer[1024];
char message[1024];
char *costs="# 1 1 0 9 8 0";
char *identifier="2";
char auxiliar[1024];
int table[5][5]={{0,0,0,0,0},
 {1,0,9,8,0},
 {0,0,0,0,0},
{0,0,0,0,0}};
#define INFINITY 9999
void dijkstra(int G[5][5],int startnode);
int begin=0;
int fd_sock, cli_sock;
int port_num=8888, ret;
struct sockaddr_in addr;
int len;
size_t getline_len;
pthread_t listenthd;

static void * listenmsg(void * arg);

int main()
{
	//Socket Creation
	fd_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_sock == -1) {
		perror("socket");
		return 0;
	}

	// addr binding, and connect
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port_num);
	inet_pton(AF_INET, "220.149.244.211", &addr.sin_addr);

	ret = connect(fd_sock, (struct sockaddr *)&addr, sizeof(addr));
	if (ret == -1) {
		perror("connect");
		close(fd_sock);
		return 0;
	}
	pthread_create(&listenthd, NULL, listenmsg, (void*)&len);
	while (1) {
		len = recv(fd_sock, r_buffer, sizeof(r_buffer), 0);
		if(r_buffer[0]=='+'){
			for(int e=0;e<5;e++){
				for(int j=0;j<5;j++){
				sprintf(auxiliar,"%c",r_buffer[e*5+j+1]);
				table[e][j]=atoi(auxiliar);
				printf("Elemento q guarda:%c\n elemento guardado:%d\n",r_buffer[e*5+j+1],table[e][j]);
					memset(auxiliar, 0, sizeof(auxiliar));
				}
				printf("Cambio de linea %d\n",e);
			}
			printf("TABLE[1] %d %d %d %d %d \n",table[0][0],table[0][1],table[0][2],table[0][3],table[0][4]);
			 printf("TABLE[2] %d %d %d %d %d \n",table[1][0],table[1][1],table[1][2],table[1][3],table[1][4]);
			 printf("TABLE[3] %d %d %d %d %d \n",table[2][0],table[2][1],table[2][2],table[2][3],table[2][4]);
			 printf("TABLE[4] %d %d %d %d %d \n",table[3][0],table[3][1],table[3][2],table[3][3],table[3][4]);
			printf("TABLE[5] %d %d %d %d %d \n",table[4][0],table[4][1],table[4][2],table[4][3],table[4][4]);
			memset(r_buffer, 0, sizeof(r_buffer));
			if(state1==1)
				printf("Which machine do you want to send a message?\n");
			else if(state2==1)
				printf("Type your message (for the machine you typed):\n");
		}
		else if(r_buffer[0]!='1'&&r_buffer[1]=='2')
		{
			printf("(Forwarded from computer no.1)\nComputer no.%c says: ",r_buffer[0]);
			for(int i=2;i<sizeof(r_buffer);i++){
			printf("%c",r_buffer[i]);
			}
			if(state1==1)
				printf("Which machine do you want to send a message?\n");
			else if(state2==1)
				printf("Type your message (for the machine you typed):\n");
				
		}
		else if(r_buffer[0]=='1'&&r_buffer[1]=='2')
		{
			printf("(Sent directly)\nComputer no.%c says: ",r_buffer[0]);
			for(int i=2;i<sizeof(r_buffer);i++){
			printf("%c",r_buffer[i]);
			}
			if(state1==1)
				printf("Which machine do you want to send a message?\n");
			else if(state2==1)
				printf("Type your message (for the machine you typed):\n");
		}
		printf("server says: %s\n", r_buffer);
		fflush(NULL);
		buffer = NULL;
		memset(r_buffer, 0, sizeof(r_buffer));
	}
	// bye-bye
	close(fd_sock);
	return 0;
}

static void * listenmsg(void * arg)
{
	while(1){
		
		if(begin){
		//sending
		state1=1;
		state2=0;
		printf("Which machine do you want to send a message?\n");
		ret = getline(&buffer, &getline_len, stdin);
		strcat(message,identifier);
		strcat(message,buffer);
		state1=0;
		state2=1;
		printf("Type your message:\n");
		ret = getline(&buffer, &getline_len, stdin);
		strcat(message,buffer);
		if (ret == -1) { // EOF
			perror("getline");
			close(fd_sock);
			break;
		}
		len = strlen(buffer);
		if (len == 0) {
			free(buffer);
			continue;
		}
		buffer=(char *)message;
		printf("Lo que va a mandar: %s\n",buffer);
		send(fd_sock, buffer, len, 0);
		memset(message, 0, sizeof(message));
		buffer=NULL;
		}
		else{
			len = strlen(costs);
			send(fd_sock, costs, len, 0);
			begin=1;	
			printf("Sending the cost\n");
			sleep(1);
			memset(r_buffer, 0, sizeof(r_buffer));
			len = recv(fd_sock, r_buffer, sizeof(r_buffer), 0);
			if (len < 0) break;
			printf("server says: %s\n", r_buffer);
			fflush(NULL);
			buffer = NULL;
		}
	}
}

/*
void dijkstra(int G[5][5],int startnode)
{
 
	int cost[5][5],distance[5],pred[5];
	int visited[5],count,mindistance,nextnode,i,j;
	
	//pred[] stores the predecessor of each node
	//count gives the number of nodes seen so far
	//create the cost matrix
	for(i=0;i<5;i++)
		for(j=0;j<5;j++)
			if(G[i][j]==0)
				cost[i][j]=INFINITY;
			else
				cost[i][j]=G[i][j];
	
	//initialize pred[],distance[] and visited[]
	for(i=0;i<5;i++)
	{
		distance[i]=cost[startnode][i];
		pred[i]=startnode;
		visited[i]=0;
	}
	
	distance[startnode]=0;
	visited[startnode]=1;
	count=1;
	
	while(count<n-1)
	{
		mindistance=INFINITY;
		
		//nextnode gives the node at minimum distance
		for(i=0;i<5;i++)
			if(distance[i]<mindistance&&!visited[i])
			{
				mindistance=distance[i];
				nextnode=i;
			}
			
			//check if a better path exists through nextnode			
			visited[nextnode]=1;
			for(i=0;i<5;i++)
				if(!visited[i])
					if(mindistance+cost[nextnode][i]<distance[i])
					{
						distance[i]=mindistance+cost[nextnode][i];
						pred[i]=nextnode;
					}
		count++;
	}
 
	//print the path and distance of each node
	for(i=0;i<5;i++)
		if(i!=startnode)
		{
			printf("\nDistance of node%d=%d",i,distance[i]);
			printf("\nPath=%d",i);
			
			j=i;
			do
			{
				j=pred[j];
				printf("<-%d",j);
			}while(j!=startnode);
	}
}
*/
