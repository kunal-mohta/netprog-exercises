#include<sys/socket.h>
#include<arpa/inet.h>

#define DFL_PORT 5000

int main (int argc, char *argv[]) {
	int i;
	if (argc == 1) i = 1;
	else i = atoi(argv[1]);

	while (i) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(DFL_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
	i--;
	}
}
