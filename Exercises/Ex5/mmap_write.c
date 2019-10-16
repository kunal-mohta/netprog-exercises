#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<netdb.h>
#include<stdlib.h>

struct addrinfo * lookup_host (char *host, char *service) {
	struct addrinfo hints, *res;

	res = (struct addrinfo *) malloc(sizeof(struct addrinfo));

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = 0;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

	if (getaddrinfo(host, service, &hints, &res) != 0) {
		perror("Error in getaddrinfo()");
		exit(0);
	}

	return res;
}

void FREE_ADDRINFO (struct addrinfo * x) {
	free(x);
}

void send_request (struct addrinfo *res, char *host) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
		perror("Error with connect");
		exit(0);
	}

	char *get_msg = malloc(1024 * sizeof(char));
	sprintf(get_msg, "GET / HTTP/1.0\r\nHost: %s\r\n\r\n", host);
	write(sockfd, get_msg, strlen(get_msg));
	char out[1024*1024];
	int n = read(sockfd, out, 1024*1024);
	out[n] = 0;

	printf("size: %d\n\n\nout: %s\n", n, out);
}

int main (int argc, char *argv[]) {
	if (argc != 3) {
		printf("Format to run is: ./a.out <url> <port>\n");
		exit(0);
	}

	char *host = argv[1], *service = argv[2];
	printf("Host: %s, port: %s\n", host, service);

	struct addrinfo *res = lookup_host(host, service);
	
	send_request(res, host);

	FREE_ADDRINFO(res);
}
