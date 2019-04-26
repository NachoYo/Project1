#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>


char r_buffer[1024];
pthread_t tids[100];
pid_t pid;
int thds;/*, pid*/
char dneig[]="0 1 3 1 1";
char adress[]="220.149.244.211";
int status;

//int pid;


void Server();
static void * handle(void *);
void Client();

int main(int argc, char *argv[])
{
	while(1)
	{
		//int aux;
		//printf (" If you want to send something press 1. \n"
 		//+"Type the destination computer followed by the message \n ");
		//scanf("%d",&aux);
		
		/*if(aux==1)
		{
			pid=fork();
		}*/
		
		pid=fork();

		if ( pid == -1) 
		{
			printf (" Error creating the process of the client \n ");
			return -1;
		} else if (! pid ) 
		{
			Client();
		} else 
		{
			//waitpid(pid, &status, 0);			
			Server();
			//
		}
	}
	

}

void Client()
{
	int fd_sock, cli_sock;
	int port_num, ret;
	struct sockaddr_in addr;
	int len;
	size_t getline_len;

	// arg parsing
	/*if (argc != 3) {
		printf("usage: cli srv_ip_addr port\n");
		return 0;
	}*/
	
	port_num = 65000; //atoi(argv[2]);

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

	while (1) {
		//buffer = NULL;
		//printf("send$ ");
		//ret = getline(&buffer, &getline_len, stdin);
		/*if (ret == -1) { // EOF
			perror("getline");
			close(fd_sock);
			break;
		}
		len = strlen(buffer);
		if (len == 0) {
			free(buffer);
			continue;
		}*/
		send(fd_sock, adress, strlen(adress), 0);
		//free(buffer);

		memset(r_buffer, 0, sizeof(r_buffer));
		len = recv(fd_sock, r_buffer, sizeof(r_buffer), 0);
		if (len < 0) break;
		printf("server says $ %s\n", r_buffer);
		fflush(NULL);
	}
	// bye-bye
	close(fd_sock);
	

}

void Server()
{
	
	int srv_sock, cli_sock;
	int port_num, ret;
	struct sockaddr_in addr;
	int len;

	
	port_num = 65000; //atoi(argv[1]);

	// socket creation
	srv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (srv_sock == -1) 
	{
		perror("Server socket CREATE fail!!");
		return;
	}

	// addr binding
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htons (INADDR_ANY); // 32bit IPV4 addr that not use static IP addr
	addr.sin_port = htons (port_num); // using port num
	
	ret = bind (srv_sock, (struct sockaddr *)&addr, sizeof(addr));
	
	if (ret == -1) 
	{
		perror("BIND error!!");
		close(srv_sock);
		return;
	}

	for (;;) 
	{
		// Listen part
		ret = listen(srv_sock, 0);

		if (ret == -1) {
			perror("LISTEN stanby mode fail");
			close(srv_sock);
			break;
		}

		// Accept part ( create new client socket for communicate to client ! )
		cli_sock = accept(srv_sock, (struct sockaddr *)NULL, NULL); // client socket
		if (cli_sock == -1) {
			perror("cli_sock connect ACCEPT fail");
			close(srv_sock);
			break;//Modif 1
		}
		thds++;
		// cli handler
		pthread_create(&tids[thds], NULL, handle, &cli_sock); //Modif 2: agregar & a "handle"
	} // end for
	
	
}


static void * handle(void * arg)
{
	int cli_sockfd = *(int *)arg;
	int ret = -1;
	char *recv_buffer = (char *)malloc(1000 *sizeof(char));
	char *send_buffer = (char *)malloc(1000 *sizeof(char));
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
           
	/* get peer addr */
	struct sockaddr peer_addr;
	socklen_t peer_addr_len;
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr_len = sizeof(peer_addr);
	ret = getpeername(cli_sockfd, &peer_addr, &peer_addr_len);
	ret = getnameinfo(&peer_addr, peer_addr_len, 
		hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), 
		NI_NUMERICHOST | NI_NUMERICSERV); 

	if (ret != 0) 
	{
		ret = -1;
		pthread_exit(&ret);
	}
	/* read from client host:port */

	while (1) 
	{
		int len = 0;

		printf("from client [%s:%s] ----\n",hbuf, sbuf);
		memset(recv_buffer, 0, sizeof(recv_buffer));
		len = recv(cli_sockfd, recv_buffer, sizeof(recv_buffer), 0);
		if (len < -1) 
		{
		
			break;
		}
		//else if(len==-1)continue;
		printf("%s\n len:%d\n", recv_buffer, len);
		memset(send_buffer, 0, sizeof(send_buffer));
		sprintf(send_buffer, "[%s:%s]%s len:%d\n", 
					hbuf, sbuf, recv_buffer, len);
		len = strlen(send_buffer);

		ret = send(cli_sockfd, send_buffer, len, 0);
		if (ret == -1) break;
		printf("----\n");
		fflush(NULL);

	}
	close(cli_sockfd);
	ret = 0;
	pthread_exit(&ret);
}

