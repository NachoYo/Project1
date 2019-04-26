#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>

int table[5][5]={{0,1,3,1,1},
 {0,0,0,0,0},
 {0,0,0,0,0},
 {0,0,0,0,0}};
bool start =false;
bool begin =false;
char buffer[1024];
char *input;
char r_buffer[1024];
char *message;

pthread_t srv_tids[100];
pthread_t cli_tids[100];

pthread_t servThread;
int srv_thds=0,cli_thds=0;
int pid;
//static void * handle(void *);
char *addrs[] = {"220.149.244.211", "220.149.244.212", "220.149.244.213","220.149.244.214","220.149.244.215"};

int srv_sock, cli_sock;
int port_num, ret;
struct sockaddr_in addr;
int len;
size_t getline_len;

//functions
void * srv_listen(void *arg);
static void * handle(void * arg);
void * client(void * arg);
void Send(int table[5][5]);

//information about costs
char costs[]="#2 15019";

int main()
{
	// arg parsing

	port_num = 56000;

	// socket creation
	srv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (srv_sock == -1) {
		perror("Server socket CREATE fail!!");
		return 0;
	}

	// addr binding
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htons (INADDR_ANY); // 32bit IPV4 addr that not use static IP addr
	addr.sin_port = htons (port_num); // using port num
	
	ret = bind (srv_sock, (struct sockaddr *)&addr, sizeof(addr));
	if (ret == -1) {
		perror("BIND error!!");
		close(srv_sock);
		return 0;
	}
	pthread_create(&servThread, NULL, srv_listen, NULL);

	while(1){
		if(begin)
		{
			for(int i=0;i<5;i++)
			{
				if(i!=2)
				{
				strcpy(message, costs);
				pthread_create(&cli_thds[cli_tids], NULL, client, (void *)addrs[i]);
					cli_thds++;
				}
			}
			begin=false;
			start=true;
		}
		if(start)
		{
		input = NULL;
		printf("Type the number of the computer you want to connect");
		printf("\n(press 6 to exit)");
		getline(&input,&getline_len,stdin);
		if(input=="6")
		{
			return 0;
		}
		printf("\nWrite the message you want to send");
		getline(&message,&getline_len,stdin);
		
		pthread_create(&cli_thds[cli_tids], NULL, client, (void *)addrs[atoi(input)-1]);
		cli_thds++;	
		}
	}

	return 0;
}

void * srv_listen(void *arg)
{
	for (;;) {
	// Listen part
	ret = listen(srv_sock, 0);

	if (ret == -1) {
		perror("LISTEN stanby mode fail");
		close(srv_sock);
		return 0;
	}

	// Accept part ( create new client socket for communicate to client ! )
	cli_sock = accept(srv_sock, (struct sockaddr *)NULL, NULL); // client socket
	if (cli_sock == -1) {
		perror("cli_sock connect ACCEPT fail");
		close(srv_sock);
	}
	srv_thds++;
	// cli handler
	pthread_create(&srv_tids[srv_thds], NULL, handle, &cli_sock);
	} // end for
}

static void * handle(void * arg)
{
	int cli_sockfd = *(int *)arg;
	int ret = -1;
	char *recv_buffer = (char *)malloc(1024);
	char *send_buffer = (char *)malloc(1024);
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

	if (ret != 0) {
		ret = -1;
		pthread_exit(&ret);
	}
	/* read from client host:port */

	int len = 0;
	printf("A computer has beed connected");
	memset(recv_buffer, 0, sizeof(recv_buffer));
	len = recv(cli_sockfd, recv_buffer, sizeof(recv_buffer), 0);
	printf("%s\n len:%d\n", recv_buffer, len);
	memset(send_buffer, 0, sizeof(send_buffer));
	sprintf(send_buffer, "[%s:%s]%s len:%d\n", 
				hbuf, sbuf, recv_buffer, len);
	len = strlen(send_buffer);
	ret = send(cli_sockfd, send_buffer, len, 0);
	printf("----\n");
	fflush(NULL);
	if(recv_buffer[0]=='#')
	{
		for(int i=0;i<5;i++)
		{
		table[atoi((char *)recv_buffer[1])][i]=atoi((char *)recv_buffer[i+3]);
		if(recv_buffer[1]=='1'&&i==4)
			begin=true;
		}
		
	}
	close(cli_sockfd);
	ret = 0;
	pthread_exit(&ret);
}

void * client(void * arg)
{
	char *ipaddress = (char *)(arg);
	int fd_sock;
	struct sockaddr_in addr_cli;
	int len;

	// socket creation
	fd_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_sock == -1) {
		perror("socket");
		return;
	}

	// addr binding, and connect
	memset(&addr_cli, 0, sizeof(addr_cli));
	addr_cli.sin_family = AF_INET;
	addr_cli.sin_port = htons (port_num);
	inet_pton(AF_INET, ipaddress, &addr_cli.sin_addr);

	ret = connect(fd_sock, (struct sockaddr *)&addr_cli, sizeof(addr_cli));
	if (ret == -1) {
		perror("connect");
		close(fd_sock);
		return;
	}
	send(fd_sock, (void *)message, strlen(message), 0);
	close(fd_sock);
	pthread_exit(&ret);
	return;
}
