#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/time.h>
#include<string.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<sys/mman.h>

#define RANDOM_CHAR 'r'
#define ONE_GB 1024*1024*1024

double timeDiff (struct timeval start, struct timeval end);

int main () {
	char *one_gb_data = (char *) malloc(ONE_GB * sizeof(char));
	memset(one_gb_data, RANDOM_CHAR, ONE_GB);

	struct timeval writeStartTime, writeEndTime, mmapStartTime, mmapEndTime;

	/** writing 1Gb data to a file using write() **/
	int fd = open("bigfile.txt", O_CREAT | O_WRONLY, 0664);
	if (fd < 0) {
		perror("Error creating file\n");
		exit(0);
	}
	printf("Starting to write 1GB data to bigfile.txt using write()\n");
	// start measuring time
	if (gettimeofday(&writeStartTime, NULL) == -1) {
		perror("Error getting start time\n");
		exit(0);
	}

	int n = write(fd, one_gb_data, ONE_GB);
	if (n < 0) {
		perror("Error writing to the file\n");
		exit(0);
	}
	
	//stop measuring time
	if (gettimeofday(&writeEndTime, NULL) == -1) {
		perror("Error getting end time for write\n");
		exit(0);
	}

	printf("Finished!\n\n");

	close(fd);
	
	/** writing 1Gb data to a file using memory maps **/
	int fd2 = open("bigfile2.txt", O_CREAT | O_RDWR, 0664);
	if (fd2 < 0) {
		perror("Error creating file\n");
		exit(0);
	}
	ftruncate(fd2, ONE_GB);
	//write(fd2, "", 1);
	char *addr = mmap(NULL, ONE_GB, PROT_WRITE, MAP_SHARED, fd2, 0);
	if (addr == MAP_FAILED) {
		perror("Error creating memory map\n");
		exit(0);
	}
	printf("Starting to write 1GB data to bigfile2.txt using mmap()\n");
	
	// start measuring time
	if (gettimeofday(&mmapStartTime, NULL) == -1) {
		perror("Error getting start time\n");
		exit(0);
	}

	//memset(addr, RANDOM_CHAR, ONE_GB); 
	strcpy(addr, one_gb_data);
	//strcpy(addr, one_gb_data);
	if (msync(addr, ONE_GB, MS_SYNC) == -1) {
		perror("Error in msync\n");	
	}

	// stop measuring time
	if (gettimeofday(&mmapEndTime, NULL) == -1) {
		perror("Error getting end time for write\n");
		exit(0);
	}
	
	printf("Finished!\n\n");

	munmap(addr, ONE_GB); 
	close(fd2);

	// printing time taken
	printf("Time taken for writing using write() : %lf ms\n", timeDiff(writeStartTime, writeEndTime));
	printf("Time taken for writing using mmap() : %lf ms\n", timeDiff(mmapStartTime, mmapEndTime));
}

double timeDiff (struct timeval start, struct timeval end) {
	double startSec = start.tv_sec;
	double startUSec = start.tv_usec;
	double endSec = end.tv_sec;
	double endUSec = end.tv_usec;

	double diff = (endSec * 1000) + (endUSec / 1000) - (startSec * 1000) - (startUSec / 1000);
	return diff;
}
