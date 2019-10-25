#include<stdio.h>
#include<string.h>
#include<limits.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define DFL_PORT 5000

int main (int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage:\n./a.out <max number of clients(N)>\n");
		exit(0);
	}

	int N = atoi(argv[1]); // Max number of clients

	int sockfd, clntfd, clnt_len; 
	struct sockaddr_in serv_addr, clnt_addr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error with socket()");
		exit(0);
	}

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(DFL_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		perror("Error with bind()");
		exit(0);
	}

	// using INT_MAX here because
	// limit on clients is to be established using
	// System V semaphores
	if (listen(sockfd, INT_MAX) != 0) {
		perror("Error with listen()");
		exit(0);
	}

	clnt_len = sizeof(clnt_addr);

	if ((clntfd = accept(sockfd, (struct sockaddr *) &clnt_addr, &clnt_len)) == -1) {
		perror("Error with accept()");
		exit(0);
	}

	pid_t ch = fork();

	if (ch < 0) {
		perror("Error with fork()");
		exit(0);
	}
	else if (ch == 0) {
		// child process
		printf("Handling client\n");
	}
	else {
		// parent process
	}
}
