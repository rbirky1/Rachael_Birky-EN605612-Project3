#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <minix/homework_ioctl.h>

#define HW_DEV "/dev/homework"
#define ERROR -1

int main()
{
	int fd, ret;

	/* Open */
	if ((fd = open(HW_DEV, O_RDWR)) < 0) {
		printf("OPEN error: %d.\n", fd);
		ret = ERROR;
	} else {
		printf("OPEN %s succeeded.\n", HW_DEV);
		if ((ret = close(fd)) <0)
			printf("CLOSE error: %d.\n", ret);
		else
			printf("CLOSE %d succeeded.\n", ret);
	}

	return ret;
}