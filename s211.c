#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <pthread.h>
#define INFINITY 9999
  
#define TRUE   1  
#define FALSE  0  
#define PORT 8888  

int state1=0, state2=0;
 int cnt=0;
int table[5][5]={{0,1,2,3,1},
 {0,0,0,0,0},
 {0,0,0,0,0},
{0,0,0,0,0}};
char *addrs[] = {"220.149.244.211", "220.149.244.212", "220.149.244.213","220.149.244.214","220.149.244.215"};
char *identifier="1";
char mess_buff[1024];
size_t getline_len;
#define INFINITY 9999
void dijkstra(int G[5][5],int n,int startnode);


pthread_t sendthd;
char number[20];
int opt = TRUE;   
int master_socket , addrlen , new_socket , client_socket[30] ,  
max_clients = 30 , activity, i , valread , sd;   
int max_sd;   
struct sockaddr_in address;
    
char buffer[1025];  //data buffer of 1K  
char sendbuffer[1025];
//set of socket descriptors  
fd_set readfds;   
         
//a message  
char *message = "Welcome to the server\r\n";  
char *message2 = "The cost matrix is:\n";
char *sendingmsg;

void dijkstra(int G[5][5],int n,int startnode);
static void * sentmsg(void * arg);

int main(int argc , char *argv[])   
{   

    //initialise all client_socket[] to 0 so not checked  
    for (i = 0; i < max_clients; i++)   
    {   
        client_socket[i] = 0;   
    }   
         
    //create a master socket  
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }   
     
    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
     
    //type of socket created  
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons( PORT );   
         
    //bind the socket to localhost port 8888  
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   
    printf("Listener on port %d \n", PORT);   
         
    //try to specify maximum of 3 pending connections for the master socket  
    if (listen(master_socket, 3) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
         
    //accept the incoming connection  
    addrlen = sizeof(address);   
    puts("Waiting for connections ...");   
         
    while(TRUE)   
    {   
        //clear the socket set  
        FD_ZERO(&readfds);   
     
        //add master socket to set  
        FD_SET(master_socket, &readfds);   
        max_sd = master_socket;   
             
        //add child sockets to set  
        for ( i = 0 ; i < max_clients ; i++)   
        {   
            //socket descriptor  
            sd = client_socket[i];   
                 
            //if valid socket descriptor then add to read list  
            if(sd > 0)   
                FD_SET( sd , &readfds);   
                 
            //highest file descriptor number, need it for the select function  
            if(sd > max_sd)   
                max_sd = sd;   
        }   
     
        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   
       
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }   
             
        //If something happened on the master socket ,  
        //then its an incoming connection  
        if (FD_ISSET(master_socket, &readfds))   
        {   
            if ((new_socket = accept(master_socket,  
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)   
            {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }   
             
            //inform user of socket number - used in send and receive commands  
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n", new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
           
            //send new connection greeting message  
            if( send(new_socket, message, strlen(message), 0) != strlen(message) )   
            {   
                perror("send");
            }   
                 
            puts("Welcome message sent successfully");   
                 
            //add new socket to array of sockets  
            for (i = 0; i < max_clients; i++)   
            {   
                //if position is empty  
                if( client_socket[i] == 0 )   
                {   
                    client_socket[i] = new_socket;   
                    printf("Adding to list of sockets as %d\n" , i);   
                         
                    break;   
                }   
            }   
        }   
             //pthread_create(&sendthd, NULL, sentmsg, (void*)&opt);
	    
        //else its some IO operation on some other socket 
        for (i = 0; i < max_clients; i++)   
        {   
            sd = client_socket[i];   
            if (FD_ISSET( sd , &readfds))   
            {   
                //Check if it was for closing , and also read the  
                //incoming message  
                if ((valread = read( sd , buffer, 1024)) == 0)   
                {   
                    //Somebody disconnected , get his details and print  
                    getpeername(sd , (struct sockaddr*)&address , \ 
                        (socklen_t*)&addrlen);   
                    printf("Host disconnected , ip %s , port %d \n" ,  
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
                         
                    //Close the socket and mark as 0 in list for reuse  
                    close( sd );   
                    client_socket[i] = 0;   
                }   
                     
                //Echo back the message that came in  
                else{  
                     	if(buffer[0]=='#')
                     	{
                          	cnt++;
			     	for(int i=0;i<5;i++)
		               	{
					table[atoi(&buffer[1])][i]=atoi(&buffer[(i+2)*2]);
					printf("%d\n",atoi(&buffer[i+2]));
				}
			 printf("TABLE[1] %d %d %d %d %d \n",table[0][0],table[0][1],table[0][2],table[0][3],table[0][4]);
			 printf("TABLE[2] %d %d %d %d %d \n",table[1][0],table[1][1],table[1][2],table[1][3],table[1][4]);
			 printf("TABLE[3] %d %d %d %d %d \n",table[2][0],table[2][1],table[2][2],table[2][3],table[2][4]);
			 printf("TABLE[4] %d %d %d %d %d \n",table[3][0],table[3][1],table[3][2],table[3][3],table[3][4]);
                         printf("TABLE[5] %d %d %d %d %d \n",table[4][0],table[4][1],table[4][2],table[4][3],table[4][4]);
			 printf("\n-----------------------\n\n");
			 if(cnt==4)
			 {
				 
				 printf("The cost table is complete!\n");
				 printf("\n-----------------------\n\n");
				 dijkstra(table,5,0);
				 for(int i=0;i<5;i++){
				 	for(int j=0;j<5;j++){
						 mess_buff[0] ='+';
					 	sprintf(mess_buff,"%s%d",mess_buff,table[i][j]);
				 		}
				 }
				 sleep(1);
				 for(int i=0;i<sizeof(client_socket);i++)
				 {
					//send(client_socket[i],message2,strlen(message2),0); 
					 message2=(char *)mess_buff;
					 //message2="+0123110980290093800710970";
					send(client_socket[i],message2,strlen(message2),0); 
				  }
				 dijkstra(table,5,1);
				 cnt++;
			 }
			     memset(buffer, 0, sizeof(buffer));
                     }
                     else{
				if(buffer[1]=='2'||buffer[1]=='3'||buffer[1]=='4'||buffer[1]=='5')
				{
					printf("RECIEVED A MESSAGE\n");
					printf("Forwarding message from computer:%c to computer %c...\n",buffer[0],buffer[1]);
					for(int i=0;i<sizeof(client_socket);i++)
					{
						send(client_socket[i],(char *)buffer,strlen(buffer),0); 
				  	}
					printf("Forwarded message sent");
				}
			 	else if(buffer[1]=='1'){
					printf("RECIEVED A MESSAGE\n");
                     			printf("Computer no.%c Says: ",buffer[0]);
		     			for(int i=2;i<sizeof(buffer);i++)
			   		{
						printf("%c",buffer[i]);
			   	 	}
			    	printf("\n");
                    		//set the string terminating NULL byte on the end  
                    		//of the data read  
                   		buffer[valread] = '\0';  
				}
		    	}
			memset(buffer, 0, sizeof(buffer));
                }   
            } 
        }   
    }   
    return 0;   
} 

static void * sentmsg(void * arg)
{
	while(1){
		state1=1;
		state2=0;
		printf("Which machine do you want to send a message?\n");
		getline(&sendingmsg, &getline_len, stdin);
		strcat(message,identifier);
		strcat(message,sendingmsg);
		state1=0;
		state2=1;
		printf("Type your message:\n");
		getline(&sendingmsg, &getline_len, stdin);
		strcat(message,sendingmsg);
		for(int i=0;i<sizeof(client_socket);i++)
			{
				send(client_socket[i],(char *)message2,strlen(message2),0); 
			}
	}
}
void dijkstra(int G[5][5],int n,int startnode)
{
 
	int cost[5][5],distance[5],pred[5];
	int visited[5],count,mindistance,nextnode,i,j;
	int cnt=1;
	
	//pred[] stores the predecessor of each node
	//count gives the number of nodes seen so far
	//create the cost matrix
	for(i=0;i<n;i++)
		for(j=0;j<n;j++)
			if(G[i][j]==0)
				cost[i][j]=INFINITY;
			else
				cost[i][j]=G[i][j];
	
	//initialize pred[],distance[] and visited[]
	for(i=0;i<n;i++)
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
		for(i=0;i<n;i++)
			if(distance[i]<mindistance&&!visited[i])
			{
				mindistance=distance[i];
				nextnode=i;
			}
			
			//check if a better path exists through nextnode			
			visited[nextnode]=1;
			for(i=0;i<n;i++)
				if(!visited[i])
					if(mindistance+cost[nextnode][i]<distance[i])
					{
						distance[i]=mindistance+cost[nextnode][i];
						pred[i]=nextnode;
					}
		count++;
	}
 	printf("Routing Table - Computer %d (%s)\n",startnode+1,addrs[startnode]);
	//print the path and distance of each node
	for(i=0;i<n;i++)
		if(i!=startnode)
		{
			
			//printf("\nPath=%d",i);
			
			j=i;
			do
			{
				cnt++;
				j=pred[j];
				//printf("<-%d",j);
			}while(j!=startnode);
			printf("Destination Computer: %d (%s) No. of hops: %d Total distnce: %d\n",i+1,addrs[i], cnt-1, distance[i]);
			cnt=1;
	}
}
