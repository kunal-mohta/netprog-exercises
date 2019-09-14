#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include<setjmp.h>

#define MAX_LINE_LENGTH 100

jmp_buf place;

void parentHandler (int signo, siginfo_t *siginfo, void *context) {
//	printf("hereee %d\n", siginfo->si_pid);
	longjmp(place, siginfo->si_pid);
}

struct childp {
	pid_t pid;
	int isReady;
};

int main (int argc, char *argv[]) {
	if (argc < 3) {
		printf("Incorrect number of arguments\n");
		printf("File name and Child process count needed\n");
		exit(0);
	}


	int N = atoi(argv[2]);

	pid_t procId;
//	pid_t pidArr[N];
//	int isReady[N];
	struct childp childArr[N];

	struct sigaction sig;
	sig.sa_handler = parentHandler;
	sig.sa_flags = SA_SIGINFO;
	if (sigaction(SIGUSR2, &sig, NULL) < 0) {
		perror("sigaction");
	}

	for (int i = 0; i < N; i++) {
		procId = fork();

		if (procId == 0) {
			printf("Child created, pid = %d\n", getpid());

			FILE *f = fopen(argv[1], "r");
			sigset_t allSigs;
			sigfillset(&allSigs);
			sigdelset(&allSigs, SIGINT);
			if (sigprocmask(SIG_SETMASK, &allSigs, NULL) == -1)
				printf("sigprocmask");

			for(;;) {
				siginfo_t si;
				int sig;
				sig = sigwaitinfo(&allSigs, &si);
				if (sig == -1) printf("sigwaitinfo");
				if (sig == SIGUSR1) {
					printf("Child %d got signal from Load balancer\n",
						getpid());

					char s[MAX_LINE_LENGTH];
					int x;
//					while (x != EOF) {
					x = fscanf(f, "%[^\n]\n", s);
					if (x != EOF) {
						printf("Reading line in child %d: %s\n",
							 getpid(), s);
						sleep(1);
					}
					else printf("EOF %d\n", getpid());
//					}
					kill(getppid(), SIGUSR2);
				}
			}
		}
		else {
			sleep(1);
//			pidArr[i] = procId;
			childArr[i].pid = procId;
			childArr[i].isReady = 1;

		}
	}

	int ret;
	if ((ret = setjmp(place)) != 0) {
		for (int i = 0; i < N; i++) {
			if (childArr[i].pid == ret) childArr[i].isReady = 1;
		}
	}

	while (1) {
		for (int i = 0; i < N; i++) {
			if (childArr[i].isReady == 1) {
				childArr[i].isReady = 0;
				printf("Load balancer sending signal to child %d\n",
								childArr[i].pid);
				kill(childArr[i].pid, SIGUSR1);
			}
		}

	}

}

