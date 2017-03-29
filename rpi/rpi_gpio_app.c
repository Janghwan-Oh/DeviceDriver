#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define DEV_NAME "/dev/rpi_gpio"
#define BUF_SIZE 100

int main(int argc, char *argv[])
{
	int fd;
	int buf[BUF_SIZE];

	fd = open(DEV_NAME, O_RDWR, 777);

	printf("fd = %d\n", fd);
	if (fd < 0) {
		printf("Failed to open file\n");
		return -1;
	}
	printf("Success to open a file\n");

	sleep(2);
	//read(fd, buf, BUF_SIZE - 1);
	
	close(fd);
    return 0;
}


