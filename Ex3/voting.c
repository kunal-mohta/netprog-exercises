#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<sys/msg.h>
#include<unistd.h>

#define MAX_BUF 100
#define MSGQ_PATH "./voting.c"
#define PROJ_ID 'X'

int coinToss();

struct my_msg {
	long mtype;
	int content;
};

int main (int argc, char *argv[]) {
	int childCount;
	if (argc < 2) {
		childCount = 5;
		printf("Default number of children (5) being created.\n");
	}
	else {
		childCount = atoi(argv[1]);
		printf("Number of children being created is %d.\n", childCount);
	}


	// key generation
	key_t key;

	if ((key = ftok(MSGQ_PATH, PROJ_ID)) == -1) {
		printf("ftok error\n");
		exit(1);
	}


	// message queue creation
	int msqid;

	if ((msqid = msgget(key, IPC_CREAT | 0644)) == -1) {
		printf("msgget error\n");
		exit(1);
	}

	if (childCount == 0 && msqid >= 0) {
		// removing the queue
		if (msgctl(msqid, IPC_RMID, NULL) == -1) {
			printf("msgctl remove error\n");
		}
		exit(0);
	}


	// children creation
	for (int i = 0; i < childCount; i++) {
		pid_t pid;

		if ((pid = fork()) == 0) {
			for (;;) {
				struct my_msg parentNotif;
				// accept only messages meant for you
				if (msgrcv(msqid, &parentNotif, sizeof(parentNotif), i+1, 0) == -1) {
					printf("msgrcv error\n");
					exit(1);
				}

				printf("Child %d received parent's signal.\n", i+1);

				int tossVal = coinToss();
				printf("Child number %d chooses %d.\n", i+1, tossVal);

				struct my_msg childChoice;
				childChoice.mtype = childCount + 1; // intended for the parent
				childChoice.content = tossVal;
				if (msgsnd(msqid, &childChoice, sizeof(childChoice), 0) == -1) {
					printf("msgsnd error\n");
					exit(1);
				}
				printf("Child %d sent its choice %d to parent.\n", i+1, childChoice.content);

			}

			exit(0);
		}
	}


	// in parent again
	for (;;) {
		printf("\n\nVOTING\n\n");

		// parent sending message to all the children
		for (int i = 0; i < childCount; i++) {
			struct my_msg notifyChild;
			notifyChild.mtype = i + 1; // mtype is child number
			notifyChild.content = 1; // random content
			if (msgsnd(msqid, &notifyChild, sizeof(notifyChild), 0) == -1) {
				printf("msgsnd error\n");
				exit(1);
			}
			printf("Parent sending message to Child %d.\n", i+1);
		}

		int vRes = 0;
		// parent waiting for children's response
		for (int i = 0; i < childCount; i++) {
			struct my_msg childVote;
			if (msgrcv(msqid, &childVote, sizeof(childVote), childCount + 1, 0) == -1) {
				printf("msgrcv error\n");
				exit(1);
			}
			printf("Parent received child vote: %d.\n", childVote.content);

			if (childVote.content == 1) vRes++;
			else if (childVote.content == 0) vRes--;
		}

		printf("\nVERDICT: ");
		if (vRes == 0) printf("It's a tie between 0 and 1!!\n");
		else if (vRes > 0) printf("Majority votes to 1 => Accepted!!\n");
		else printf("Majority votes to 0 => Rejected!!\n");

		sleep(3);
	}


}

int coinToss () {
	srand(time(0) ^ getpid());
	return rand() % 2;
}
