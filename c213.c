#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define INFINITY 9999

//char route[4];
char *buffer;
char r_buffer[1024];
char message[1024];
char auxiliar[1024];
char *costs="# 2 2 9 0 0 9";
char *identifier="3";
int table[5][5]={{0,0,0,0,0},
 {1,0,9,8,0},
 {0,0,0,0,0},
{0,0,0,0,0}};


void dijkstra(int G[5][5],int startnode);

int main()
{
	int begin=0;
	int fd_sock, cli_sock;
	int port_num=8888, ret;
	struct sockaddr_in addr;
	int len;
	size_t getline_len;
	
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
	while (1) {
		if(begin){
		memset(r_buffer, 0, sizeof(r_buffer));
		len = recv(fd_sock, r_buffer, sizeof(r_buffer), 0);
		if (len < 0) break;
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
			dijkstra(table,5);
		}
		else if(r_buffer[0]!='1'&&r_buffer[1]=='2')
		{
			printf("(Forwarded from computer no.1)\nComputer no.%c says: ",r_buffer[0]);
			for(int i=2;i<sizeof(r_buffer);i++){
			printf("%c",r_buffer[i]);
			}
		}
		else if(r_buffer[0]=='1'&&r_buffer[1]=='2')
		{
			printf("(Sent directly)\nComputer no.%c says: ",r_buffer[0]);
			for(int i=2;i<sizeof(r_buffer);i++){
			printf("%c",r_buffer[i]);
			}
		}
		fflush(NULL);
		buffer = NULL;
		printf("Which machine do you want to send a message?\n");
		ret = getline(&buffer, &getline_len, stdin);
		strcat(message,identifier);
		strcat(message,buffer);
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
			sleep(1);
			len = strlen(costs);
			send(fd_sock, costs, len, 0);
			begin=1;	
			printf("Sending the cost\n");
			sleep(1);
			memset(r_buffer, 0, sizeof(r_buffer));
			len = recv(fd_sock, r_buffer, sizeof(r_buffer), 0);
			if (len < 0) break;
			printf("Computer 1 says: %s\n", r_buffer);
			fflush(NULL);
			buffer = NULL;
			
		}
	}
	// bye-bye
	close(fd_sock);
	return 0;
}
void dijkstra(int G[5][5],int startnode)
{
 
	int cost[5][5],distance[5],pred[5];
	int visited[5],count,mindistance,nextnode,i,j;
	int cnt=1;
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
	
	while(count<5-1)
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
		//char aux[]="";
		if(i!=startnode)
		{
			printf("\nDistance of node%d=%d",i,distance[i]);
			printf("\nPath=%d",i);
			
			j=i;
			do
			{
				j=pred[j];
				printf("<-%d",j);
				
				//sprintf(aux,"%s%d",aux,j);
				cnt++;
				
			}while(j!=startnode);
			printf("Destination Computer: %d No. of hops: %d Total distnce: %d\n",i, cnt-1, distance[i]);
			
	}
}
