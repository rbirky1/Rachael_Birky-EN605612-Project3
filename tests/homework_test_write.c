#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <minix/homework_ioctl.h>
#include <assert.h>

#define HW_DEV "/dev/homework"
#define ERROR -1

int close_dev(int fd);

int close_dev(int fd) {
	int ret = 0;
	if ((ret = close(fd)) <0) {
		printf("CLOSE error: %d.\n", ret);
	}
	printf("CLOSE %s succeeded.\n", HW_DEV);
	return ret;
}

int main()
{
	int fd, ret;
	int slot_index, slot_value, read_value;
	ssize_t size;

	/* Open */
	if ((fd = open(HW_DEV, O_RDWR)) < 0) {
		printf("OPEN error: %d.\n", fd);
		return ERROR;
	}
	printf("OPEN %s succeeded.\n", HW_DEV);

	/* Set slot to read */
	slot_index = 3;
	if (ioctl(fd, HIOCSLOT, &slot_index) <0) {
		printf("HIOCSLOT error.\n");
		close_dev(fd);
		return ERROR;
	}

	/* Write */
	slot_value = 10;
	if ((size = write(fd, &slot_value, sizeof(slot_value))) < 0) {
		printf("WRITE error.\n");
		close_dev(fd);
		return ERROR;
	}
	printf("WRITE %d: (%d bytes)\n", slot_value, size);

	 /* Read slot */
	if ((size = read(fd, &read_value, sizeof(read_value))) < 0) {
		printf("Error reading: %d\n", errno);
		close_dev(fd);
		return ERROR;
	}
	printf("READ %d: %d bytes.\n", read_value, size);

	/* We expect the value to be the value we wrote */
	assert(read_value == slot_value);

	ret = close_dev(fd);

	return ret;
}