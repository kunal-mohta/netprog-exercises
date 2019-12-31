#include<stdio.h>
#include<string.h>
#include<limits.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/sem.h>
#include<sys/ipc.h>
#include<arpa/inet.h>
#include<errno.h>

#define DFL_PORT 5000

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short  *array;
	struct seminfo  *__buf;
};
													  
int server_setup () {
	int sockfd;
	struct sockaddr_in serv_addr;

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
	return sockfd;
}

int sem_setup (int N) {
	int semid;
	if ((semid = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) == -1) {
		perror("Error with semget()");
		exit(0);
	}
	union semun semctl_arg;
	semctl_arg.val = N;
	if (semctl(semid, 0, SETVAL, semctl_arg) == -1) {
		perror("Error with semctl() in sem_setup()");
		exit(0);
	}
	return semid;
}

int get_sem_val (int semid) {
	union semun semctl_arg;
	int val = semctl(semid, 0, GETVAL, semctl_arg);
	if (val == -1) {
		perror("Error with semctl() in get_sem_val()");
		exit(0);
	}
	else return val;
}

void dec_sem_val (int semid) {
	struct sembuf sops[1];

	sops[0].sem_num = 0;
	sops[0].sem_op = -1;
	sops[0].sem_flg = 0;

	while (1) {
		if (semop(semid, sops, 1) == -1) {
			if (errno == EINTR) continue;
			else {
				perror("Error with semop() in dec_sem_val()");
				exit(0);
			}
		}
		else break;
	}
	printf("\n**Semaphore decrement: Current semaphore value: %d**\n", get_sem_val(semid));
}

void inc_sem_val (int semid) {
	struct sembuf sops[1];

	sops[0].sem_num = 0;
	sops[0].sem_op = 1;
	sops[0].sem_flg = 0;

	while (1) {
		if (semop(semid, sops, 1) == -1) {
			if (errno == EINTR) continue;
			else {
				perror("Error with semop() in dec_sem_val()");
				exit(0);
			}
		}
		else break;
	}
	printf("\n**Semaphore increment: Current semaphore value: %d**\n", get_sem_val(semid));
}

void sigchldhandler () {
	// to handle zombies
	while (waitpid(-1, 0, WNOHANG) > 0); 
}

int main (int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage:\n./a.out <max number of clients(N)>\n");
		exit(0);
	}

	int N = atoi(argv[1]); // Max number of clients

	int semid = sem_setup(N);
	printf("Semaphore initialised with value: %d\n", get_sem_val(semid));
	
	int sockfd = server_setup();

	int clntfd, clnt_len; 

	signal(SIGCHLD, sigchldhandler);
	
	printf("Server ready to accept requests\n\n");
	while (1) {
		dec_sem_val(semid);
		printf("\nWaiting for a client to connect...\n");
		struct sockaddr_in clnt_addr;

		clnt_len = sizeof(clnt_addr);

		while (1) {
			if ((clntfd = accept(sockfd, (struct sockaddr *) &clnt_addr, &clnt_len)) == -1) {
				if (errno == EINTR) continue;
				perror("Error with accept()");
				exit(0);
			}
			else break;
		}
		printf("\nConnection established with a client\n");

		pid_t ch = fork();

		if (ch < 0) {
			perror("Error with fork()");
			exit(0);
		}
		else if (ch == 0) {
			// child process
			printf("\nHandling client\n");
			// server-client communication goes here
			sleep(5);

			printf("\nClosing connection with client\n");
			close(clntfd);
			inc_sem_val(semid);
			exit(1);
		}
		else {
			// parent process
			close(clntfd);
		}
	}
}
