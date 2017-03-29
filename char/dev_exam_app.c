
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>


#define DEV_NAME "/dev/dev_exam"
#define DEV_MAJOR 254
#define DEV_MINOR 5

#define MY_IOC_NUM 100
#define MY_IOC_READ _IOR(MY_IOC_NUM, 0, int)
#define MY_IOC_WRITE _IOW(MY_IOC_NUM, 1, int)
#define MY_IOC_STATUS _IO(MY_IOC_NUM, 2)
#define MY_IOC_READ_WRITE _IOWR(MY_IOC_NUM, 3, int)
#define MY_IOC_NR 4



int main(int argc, char *argv[])
{
    int data = 0;
	int dev = -1;

	if ((dev = open(DEV_NAME, O_RDWR)) < 0) {
		printf("[App Message] : open error\n");
		return -1;
	}

	ioctl(dev, MY_IOC_READ, &data);
	ioctl(dev, MY_IOC_WRITE, &data);
	ioctl(dev, MY_IOC_STATUS);
	ioctl(dev, MY_IOC_READ, &data);
	ioctl(dev, MY_IOC_READ_WRITE, &data);

	close(dev);	
    return 0;
}
