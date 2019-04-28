#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

char *buffer;
char message[1024];
char r_buffer[1024];
char *costs="# 3 3 8 0 0 7";
char *identifier="4";
int table[5][5]={{0,0,0,0,0},
 {1,0,9,8,0},
 {0,0,0,0,0},
{0,0,0,0,0}};

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
		printf("server says $ %s\n", r_buffer);
		fflush(NULL);
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
		
		
		strcat(message,identifier);
		strcat(message,buffer);
		buffer=(char *)message;
			
		printf("Lo que va a mandar: %s\n",message);
		send(fd_sock, buffer, len, 0);
		memset(message, 0, sizeof(message));
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
			printf("server says: %s\n", r_buffer);
			fflush(NULL);
			buffer = NULL;
		}
	}
	// bye-bye
	close(fd_sock);
	return 0;
}

