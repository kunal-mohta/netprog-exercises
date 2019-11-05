#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

#define __MAX_PENDING__ 1024
#define __MAX_MSG_COUNT__ 5 
#define __MAX_MSG_SIZE__ 1024

int server_setup (int port) {
	// Basic TCP server setup
	int serv_sock;

	struct sockaddr_in serv_addr;

	if ((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error creating socket\n");
		exit(0);
	}

	if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
		printf("Error sock opt\n");
		exit(0);
	}

	bzero(&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	if (bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)	{
		printf("Error binding socket: %d\n", errno);
		exit(0);
	}

	if (listen(serv_sock, __MAX_PENDING__) < 0) {
		printf("Error listening socket: %d\n", errno);
		exit(0);
	}

	return serv_sock;
}

int main (int argc, char *argv[]) {

	if (argc != 2) {
		printf("Usage:\n./a.out <port>\n");
		exit(0);
	}
	int port = atoi(argv[1]);

	// Server setup
	int clnt_sock, serv_sock = server_setup(port);
	struct sockaddr_in clnt_addr;

	int maxfd = serv_sock; // for select argument
	int max_even_index = -1, max_odd_index = -1; // for even_clients[] & odd_clients[]

	int even_clients[FD_SETSIZE], odd_clients[FD_SETSIZE]; // storing fds of connected clients

	fd_set rset, wset, allrset, allwset;

	for (int i = 0; i < FD_SETSIZE; i++) {
		even_clients[i] = -1;
		odd_clients[i] = -1;
	}

	FD_ZERO(&allrset);
	FD_ZERO(&allwset);
	FD_SET(serv_sock, &allrset);
	FD_SET(serv_sock, &allwset);

	// queues for storing data that is to be sent to even/odd fds
	char *even_buf_queue[__MAX_MSG_COUNT__], *odd_buf_queue[__MAX_MSG_COUNT__]; 

	// pointers for the current position in the queue
	int even_queue_ptr = -1, odd_queue_ptr = -1;

	// arrays of queue pointers till where clients have been sent data
	int even_clients_ptr[FD_SETSIZE], odd_clients_ptr[FD_SETSIZE];

	while (true) {
		rset = allrset;
		wset = allwset;

		int nready = select(maxfd + 1, &rset, &wset, NULL, NULL);
	
		if (FD_ISSET(serv_sock, &rset)) {
			// new connection
			int clnt_len = sizeof(clnt_addr);
			clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_addr, &clnt_len);

			printf("Connection established at fd: %d\n", clnt_sock);

			int j;
			if (clnt_sock % 2 == 0) {
				// even fd
				for (j = 0; j < FD_SETSIZE; j++) {
					if (even_clients[j] == -1) {
						even_clients[j] = clnt_sock;
						even_clients_ptr[j] = even_queue_ptr + 1;
						break;
					}
				}
				if (j > max_even_index) max_even_index = j;
			}
			else {
				// odd fd
				for (j = 0; j < FD_SETSIZE; j++) {
					if (odd_clients[j] == -1) {
						odd_clients[j] = clnt_sock;
						odd_clients_ptr[j] = odd_queue_ptr + 1;
						break;
					}
				}
				if (j > max_odd_index) max_odd_index = j;
			}

			if (j == FD_SETSIZE) {
				printf("Too many clients. Exiting server...\n");
				exit(0);
			}

			FD_SET(clnt_sock, &allrset);

			if (clnt_sock > maxfd) maxfd = clnt_sock;

			nready--;
			if (nready <= 0) continue;
		}

		for (int x = 0; x <= max_odd_index; x++) {
			/*printf("odd loop\n");*/
			// check odd clients
			int csock = odd_clients[x];

			if (csock == -1) continue;

			if (FD_ISSET(csock, &rset)) {
				// Readability check
				char even_buf[__MAX_MSG_SIZE__];
				int nb = read(csock, even_buf, __MAX_MSG_SIZE__);

				if (nb < 0) {
					printf("Error reading from fd %d\n", csock);
					perror("");
					break;
				}
				else if (nb == 0) {
					close(csock);
					FD_CLR(csock, &allrset);
					odd_clients[x] = -1;
					printf("Connection closed for fd %d\n", csock);
					break;
				}
				else {
					even_queue_ptr = (even_queue_ptr + 1) % __MAX_MSG_COUNT__;
					even_buf[nb] = 0;
					even_buf_queue[even_queue_ptr] = even_buf;
					printf("Received data at fd %d:\n%s\n", csock, even_buf);

					for (int m = 0; m <= max_even_index; m++) {
						if (even_clients[m] != -1) FD_SET(even_clients[m], &allwset);
					}
				}
			}
			
			if (FD_ISSET(csock, &wset)) {
				if (odd_clients_ptr[x] != ((odd_queue_ptr + 1) % __MAX_MSG_COUNT__)) {
					// writability check
					// and check if more message(s) present in the queue
					write(csock, odd_buf_queue[odd_clients_ptr[x]], strlen(odd_buf_queue[odd_clients_ptr[x]]));
					printf("Written data to fd %d:\n%s\n", csock, odd_buf_queue[odd_clients_ptr[x]]);
					odd_clients_ptr[x] = (odd_clients_ptr[x] + 1) % __MAX_MSG_COUNT__;
				}
				else {
					FD_CLR(csock, &allwset);
				}
			}
		}

		
		for (int x = 0; x <= max_even_index; x++) {
			/*printf("even loop\n");*/
			// check even clients
			int csock = even_clients[x];

			if (csock == -1) continue;

			if (FD_ISSET(csock, &rset)) {
				// Readability check
				char odd_buf[__MAX_MSG_SIZE__];
				int nb = read(csock, odd_buf, __MAX_MSG_SIZE__);

				if (nb < 0) {
					printf("Error reading from fd %d\n", csock);
					perror("");
					break;
				}
				else if (nb == 0) {
					close(csock);
					FD_CLR(csock, &allrset);
					even_clients[x] = -1;
					printf("Connection closed for fd %d\n", csock);
					break;
				}
				else {
					odd_queue_ptr = (odd_queue_ptr + 1) % __MAX_MSG_COUNT__;
					odd_buf[nb] = 0;
					odd_buf_queue[odd_queue_ptr] = odd_buf;
					printf("Received at fd %d:\n%s\n", csock, odd_buf);
					
					for (int m = 0; m <= max_odd_index; m++) {
						if (odd_clients[m] != -1) FD_SET(odd_clients[m], &allwset);
					}
				}
			}
			
			if (FD_ISSET(csock, &wset)) {
				if (even_clients_ptr[x] != ((even_queue_ptr + 1) % __MAX_MSG_COUNT__)) {
					// writability check
					// and check if more message(s) present in the queue
					write(csock, even_buf_queue[even_clients_ptr[x]], strlen(even_buf_queue[even_clients_ptr[x]]));
					printf("Written data to fd %d:\n%s\n", csock, even_buf_queue[even_clients_ptr[x]]);
					even_clients_ptr[x] = (even_clients_ptr[x] + 1) % __MAX_MSG_COUNT__;
				}
				else {
					FD_CLR(csock, &allwset);
				}
			}
		}
	}
}

