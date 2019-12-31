#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<sys/time.h>
#include<string.h>
#include<sys/socket.h>
#include<netdb.h>
#include<stdlib.h>

#define DEFAULT_HTTP_PORT "http"
#define BASIC_SEPARATE printf("\n-------------------------------------------------------------\n");
#define MAJOR_SEPARATE printf("\n\n================================================================================================\n\n");
#define MTU 1500
#define IP_SIZE 15 
#define MAX_CONTENT_LENGTH 1024
#define POST_BODY_FILE "post_body.txt"

struct addrinfo * lookup_host (char *host, char *service) {
	struct addrinfo hints, *res;

	res = (struct addrinfo *) malloc(sizeof(struct addrinfo));

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = 0;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

	int err = getaddrinfo(host, service, &hints, &res);
	if (err != 0) {
		if (err == EAI_SYSTEM) {
			perror("Error in getaddrinfo()");
			exit(0);
		}
		else {
			fprintf(stderr, "Error in getaddrinfo(): %s\n", gai_strerror(err));
			exit(0);
		}
	}

	return res;
}

void FREE_ADDRINFO (struct addrinfo * x) {
	free(x);
}

double time_diff (struct timeval start, struct timeval end) {
	double startSec = start.tv_sec;
	double startUSec = start.tv_usec;
	double endSec = end.tv_sec;
	double endUSec = end.tv_usec;

	double diff = (endSec * 1000) + (endUSec / 1000) - (startSec * 1000) - (startUSec / 1000);
	return diff;
}

char * get_path (char *url) {
	char *path = strchr(url, '/');
	if (path == NULL) return "/";
	else return path;
}

char * get_ip (struct addrinfo *res) {
	char *ip = malloc(IP_SIZE * sizeof(char));
	inet_ntop(res->ai_family, &(((struct sockaddr_in *) res->ai_addr)->sin_addr), ip, res->ai_addrlen);
	return ip;
}

void send_get_request (struct addrinfo *res, char *host, char *path) {
	printf("\n\n***************************************************HTTP GET***************************************************\n");
	int counter = 1;
	while (res != NULL) {
		int sockfd = socket(res->ai_family, res->ai_socktype, 0);
		if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
			perror("Error with connect");
			exit(0);
		}

		MAJOR_SEPARATE
		printf("IP address %d: %s\n", counter, get_ip(res));
		char *get_msg = malloc(MTU * sizeof(char));
		bzero(get_msg, MTU * sizeof(char));
		sprintf(get_msg, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);
		
		BASIC_SEPARATE
		printf("Sending data:\n\n");
		printf("%s\n", get_msg);
		BASIC_SEPARATE

		struct timeval start, end;
		if (write(sockfd, get_msg, strlen(get_msg)) == (strlen(get_msg))) {
			BASIC_SEPARATE
			printf("Response from server:\n\n");
			
			if (gettimeofday(&start, NULL) == -1) {
				perror("Error getting start time");
				exit(0);
			}

			while (1) {
				char out[MTU+1];
				int nb = read(sockfd, out, MTU);
				if (nb <= 0) break;
				out[nb] = 0;
				printf("%s", out);
			}

			if (gettimeofday(&end, NULL) == -1) {
				perror("Error getting end time");
				exit(0);
			}

			BASIC_SEPARATE
		}
		else perror("Error writing to server");
		printf("Time taken to get response: %lf ms\n", time_diff(start, end));
		free(get_msg);
		res = res->ai_next;
		counter++;
	}
}

void send_post_request (struct addrinfo *res, char *host, char *path) {
	printf("\n\n***************************************************HTTP POST**************************************************\n");
	printf("HTTP Post request body read from file %s\n", POST_BODY_FILE);
	int counter = 1;
	while (res != NULL) {
		int sockfd = socket(res->ai_family, res->ai_socktype, 0);
		if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
			perror("Error with connect");
			exit(0);
		}

		MAJOR_SEPARATE
		printf("IP address %d: %s\n", counter, get_ip(res));

		char post_body[MAX_CONTENT_LENGTH];
		int fd = open(POST_BODY_FILE, O_RDONLY);
		int post_body_size = read(fd, post_body, MAX_CONTENT_LENGTH);
		if (post_body_size < 0) {
			printf("Error reading post data from %s file\nUsing empty post body instead...\n", POST_BODY_FILE);
			post_body[0] = 0;
			post_body_size = 0;
		}
		else {
			post_body[post_body_size] = 0;
		}
			
		char *get_msg = malloc(MTU * sizeof(char));
		bzero(get_msg, MTU * sizeof(char));
		sprintf(get_msg, "POST %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\nContent-length: %d\r\n\r\n%s", path, host, post_body_size, post_body);
		
		BASIC_SEPARATE
		printf("Sending data:\n\n");
		printf("%s\n", get_msg);
		BASIC_SEPARATE

		struct timeval start, end;
		if (write(sockfd, get_msg, strlen(get_msg)) == (strlen(get_msg))) {
			BASIC_SEPARATE
			printf("Response from server:\n\n");
			
			if (gettimeofday(&start, NULL) == -1) {
				perror("Error getting start time");
				exit(0);
			}

			while (1) {
				char out[MTU+1];
				int nb = read(sockfd, out, MTU);
				if (nb <= 0) break;
				out[nb] = 0;
				printf("%s", out);
			}

			if (gettimeofday(&end, NULL) == -1) {
				perror("Error getting end time");
				exit(0);
			}

			BASIC_SEPARATE
		}
		else perror("Error writing to server");
		printf("Time taken to get response: %lf ms\n", time_diff(start, end));
		free(get_msg);
		res = res->ai_next;
		counter++;
	}
}

int main (int argc, char *argv[]) {
	char *url, *service;
	if (argc < 2 || argc > 3) {
		printf("Usage format:\n./a.out <url> <port>\n./a.out <url>\n");
		exit(0);
	}
	else if (argc == 2) {
		printf("Selecting default HTTP port (see /etc/services)\n");
		service = DEFAULT_HTTP_PORT;
	}
	else if (argc == 3) {
		printf("Selecting custom port: %s\n", argv[2]);
		service = argv[2];
	}
	url = argv[1];

	char *path;
	path = strchr(url, '/');
	path = (path == NULL) ? "/" : path;

	char *url_dup = strdup(url);
	char *host = strtok(url_dup, "/");
	
	printf("Input Host: %s\n", host);
	printf("Input Path: %s\n", path);

	struct addrinfo *res = lookup_host(host, service);
	
	send_get_request(res, host, path);
	send_post_request(res, host, path);
	FREE_ADDRINFO(res);
	free(url_dup);
}
