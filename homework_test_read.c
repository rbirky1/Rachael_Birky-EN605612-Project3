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
	int slot_index, read_value;
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

	 /* Read slot */
	if ((size = read(fd, &read_value, sizeof(read_value))) < 0) {
		printf("Error reading: %d\n", errno);
		close_dev(fd);
		return ERROR;
	}
	printf("Read %d: %d bytes.\n", read_value, size);

	/* Since we have not written a value, we expect the value to be 0 */
	assert(read_value == 0);

	ret = close_dev(fd);

	return ret;
}