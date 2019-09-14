#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>

#define MAX_READ 100

int main () {

// reading from a file using fd
/*	int fd = open("./xyz.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

	if (fd == -1) {
		printf("Error opening file\n");
		exit(0);
	}
	else {
		printf("here\n");
	}

	close(fd);
*/


	// reading from STDIN fd

	ssize_t numread = 0;

	while (1) {
		char buffer[MAX_READ + 1];
		numread = read(STDIN_FILENO, buffer, MAX_READ);

		if (numread == -1) break;

		buffer[numread] = '\0';
		printf("%s", buffer);
	}
}
