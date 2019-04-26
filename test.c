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


char buffer[1024];
pthread_t srv_tids[100];
pthread_t cli_tids[100];

pthread_t servThread;
int srv_thds=0,cli_thds=0;
int pid;
static void * handle(void *);
char *addrs[] = {"220.149.244.211", "220.149.244.212", "220.149.244.213","220.149.244.214"};

int srv_sock, cli_sock;
	int port_num, ret;
	struct sockaddr_in addr;
	int len;

//functions
void srv_listen(void *arg);
static void * handle(void * arg);
int client(void * arg);

int main()
{
	char input[1];
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
		printf("Type the number of the computer you want to connect");
		getline(&input,1,stdin);
		pthread_create(&cli_thds[cli_tids], NULL, client, (void *)addrs[atoi(input)-1]);
		cli_thds++;
	}

	return 0;
}

void srv_listen(void *arg)
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

	while (1) {
	int len = 0;

	printf("from client ----\n");
	memset(recv_buffer, 0, sizeof(recv_buffer));
	len = recv(cli_sockfd, recv_buffer, sizeof(recv_buffer), 0);
	if (len == 0) continue;
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

int client(void * arg)
{
	char ipaddress[] = (char *)arg;
	int fd_sock;
	struct sockaddr_in addr_cli;
	int len;
	size_t getline_len;

	// socket creation
	fd_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_sock == -1) {
		perror("socket");
		return 0;
	}

	// addr binding, and connect
	memset(&addr_cli, 0, sizeof(addr_cli));
	addr_cli.sin_family = AF_INET;
	addr_cli.sin_port = htons (port_num);
	inet_pton(AF_INET, ipaddress, &addr_cli.sin_addr);

	ret = connect(fd_sock, (struct sockaddr_cli *)&addr_cli, sizeof(addr_cli));
	if (ret == -1) {
		perror("connect");
		close(fd_sock);
		return 0;
	}

	while (1) {
		send(fd_sock, "Comp1Con", 8, 0);
		break;
		/*
		buffer = NULL;
		printf("send$ ");
		ret = getline(&buffer, &getline_len, stdin);
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
		send(fd_sock, buffer, len, 0);
		free(buffer);

		memset(r_buffer, 0, sizeof(r_buffer));
		len = recv(fd_sock, r_buffer, sizeof(r_buffer), 0);
		if (len < 0) break;
		printf("server says $ %s\n", r_buffer);
		fflush(NULL); */
	}
	close(fd_sock);
	return 0;
}
