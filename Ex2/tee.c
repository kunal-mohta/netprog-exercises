#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>

#define MAX_READ 1000

int main (int argc, char *argv[]) {
	int fileCount = argc - 1;
	int fdArr[fileCount];

	// storing the fds of all the files in args
	if (fileCount > 0) {
		for (int i = 1; i < argc; i++) {
			int flags = O_CREAT | O_WRONLY | O_TRUNC; // new files created and existing files overriden
			int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // rw-rw-rw
			int fd = open(argv[i], flags, perms);

			if (fd == -1) {
				printf("Error opening file: %s\n", argv[i]);
			}

			fdArr[i - 1] = fd;
		}
	}

    ssize_t numread = 0;

    while (1) {
        char buffer[MAX_READ + 1];
		// reading from stdin
        numread = read(STDIN_FILENO, buffer, MAX_READ);

        if (numread <= 0) break;

        buffer[numread] = '\0';

		// writing into all the required files
		for (int i = 0; i < fileCount; i++) {
			if (fdArr[i] != -1) {
				int w = write(fdArr[i], buffer, numread);
				if (w == -1) {
					printf("Error writing to file: %s\n", argv[i+1]);
				}
			}
		}

		// writing to the stdout
        write(STDOUT_FILENO, buffer, numread);
    }
}

