#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#define DFL_PORT 5000

int main (int argc, char *argv[]) {
	int i;
	if (argc == 1) i = 1;
	else i = atoi(argv[1]);

	/*while (i) {*/
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(DFL_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));

	if (fork() == 0) {
		while (1) {
			char buf[1024];
			int n = read(sockfd, buf, 1024);
			buf[n] = 0;
			printf("Received: %s\n", buf);
		}
	}
	while (1) {
		printf("\n> ");
		size_t cmd_size = 1024+1;
		char *cmd_buf = malloc(sizeof(char) * cmd_size);
		ssize_t cmd_size_act = getline(&cmd_buf, &cmd_size, stdin);
		write(sockfd, cmd_buf, cmd_size_act);
	}

	/*i--;*/
	/*}*/
}
