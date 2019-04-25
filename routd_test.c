#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <arpa/inet.h>
#define MAX 5

#define MAXBUF 256

//char *adds[] = {"220.149.244.211", "220.149.244.212", "220.149.244.213","220.149.244.214"};
char *adds[] = {"192.168.0.8","192.168.0.8", "192.168.0.8", "192.168.0.8","192.168.0.8"};//array of string containing IP addresses
int table[MAX][MAX]={{0,1,3,1,1},
 {0,0,0,0,0},
 {0,0,0,0,0},
 {0,0,0,0,0}}; //at the beginning it only has values from neighbor



int n=MAX;



void Client(char message,char adress) //this function sends the info to neighbors in the format of "<index of 
//the row of the table> <index of the columns of table> <value of distance>"
{
	char r_buffer[1024];
	int fd_sock, cli_sock;
	int port_num, ret;
	struct sockaddr_in addr;
	int len;
	size_t getline_len;
	
	port_num = 56000; 

	// socket creation
	fd_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_sock == -1) 
	{
		perror("socket");
		//return 0;
	}

	// addr binding, and connect
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port_num);
	inet_pton(AF_INET, adress, &addr.sin_addr);

	ret = connect(fd_sock, (struct sockaddr *)&addr, sizeof(addr));
	if (ret == -1) 
	{
		perror("connect");
		close(fd_sock);
		return;
	}

	while (1) 
	{
		
		send(fd_sock, message, strlen(msge), 0);
		free(buffer);

		memset(r_buffer, 0, sizeof(r_buffer));
		len = recv(fd_sock, r_buffer, sizeof(r_buffer), 0);
		if (len < 0) break;
		//printf("server says $ %s\n", r_buffer); //echo
		fflush(NULL);
	}

}

void Send(int tab[MAX][MAX])
{

	for(int i=0;i<n;i++)
  		{
			for (int j=1;j<n;j++)
			{  			
				if(tab[i][j]!=0)
				{
					int cli_fork=fork();
					char msg[];
					char adress;
					strcpy(adress,adds[j]);
					sprintf(msg, "%d %d %d", i,j,tab[i][j]);
					if(cli_fork == 0)
		  			{
		  				Client(msg,adress);
		  				//exit(0);
		  			}
					
				}
				
			}
  		}
}
void Write(char [] msge)//function that reads from the clients and write the value in the correct place of matrix
{
	//not implemented
}

int main()
{
	
	while(1)
	{
		

   
  		
		Send(table[MAX][MAX]);

  		Server();
  		

	}

  
	return 0;
}

void Server(void)
{

	char buffer[MAXBUF];
	//int fds[5];
	struct sockaddr_in addr;
	struct sockaddr_in client;
	int addrlen, n,max=0;;
	int srv_sock, ret;
	//fd_set rset;
	struct pollfd pollfds[5];  
	char *recv_buffer = (char *)malloc(1000 *sizeof(char));
	char *send_buffer = (char *)malloc(1000 *sizeof(char));
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	struct sockaddr peer_addr;
	socklen_t peer_addr_len;

	srv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (srv_sock == -1) 
	{
		perror("Server socket CREATE fail!!");
		//return;
	}
	memset(&addr, 0, sizeof (addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(56001);
	addr.sin_addr.s_addr = INADDR_ANY;
	
	ret =bind(srv_sock,(struct sockaddr*)&addr ,sizeof(addr)); 
	//bind (srv_sock, (struct sockaddr *)&addr, sizeof(addr));

	if (ret == -1) 
	{
		perror("BIND error!!");
		close(srv_sock);
		return;
	}
	
	//listen (srv_sock, 5); 
	ret = listen(srv_sock, 5);

	if (ret == -1) {
		perror("LISTEN stanby mode fail");
		close(srv_sock);
		break;
	}

	
	for (i=0;i<5;i++) 
	{  
		memset(&client, 0, sizeof (client));
		addrlen = sizeof(client);
		pollfds[i].fd = accept(srv_sock,(struct sockaddr*)&client, &addrlen);
		if(pollfds[i].fd== -1)
		{
		
			perror("cli_sock connect ACCEPT fail");
			//close(srv_sock);
			continue;//Modif 1
		}else 
			pollfds[i].events = POLLIN;
		
		
		
		
	}
	sleep(1);
	
	//puts("round again");
	poll(pollfds, 5, 50000);

	for(i=0;i<5;i++) 
	{
		if (pollfds[i].revents & POLLIN)
		{
			pollfds[i].revents = 0;
			memset(buffer,0,MAXBUF);
			memset(&peer_addr, 0, sizeof(peer_addr));
			peer_addr_len = sizeof(peer_addr);			
			ret = getpeername(pollfds[i], &peer_addr, &peer_addr_len);
			ret = getnameinfo(&peer_addr, peer_addr_len, 
			hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), 
			NI_NUMERICHOST | NI_NUMERICSERV); 
			
			if (ret != 0) 
			{
				ret = -1;
				pthread_exit(&ret);
			}
							
			
			//read(pollfds[i].fd, buffer, MAXBUF);
			len = recv(pollfds[i], buffer, sizeof(buffer), 0);
			if (len < -1) 
			{
		
				break;
			}				
			printf("From client [%s:%s] ----\n",hbuf, sbuf);
			//puts(buffer);
			printf("%s\n len:%d\n", recv_buffer, len);
			memset(send_buffer, 0, sizeof(send_buffer));
			sprintf(send_buffer, "[%s:%s]%s len:%d\n", 
						hbuf, sbuf, recv_buffer, len);
			len = strlen(send_buffer);
			//printf(recv_buffer);
			Read(buffer);
			ret = send(cli_sockfd, send_buffer, len, 0);
			if (ret == -1) break;
			printf("----\n");
			fflush(NULL);
		}
		
	}	


}



