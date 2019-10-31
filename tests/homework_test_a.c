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

/**
 * If no integer has ever been written to the active slot in the device since the
 * device was activated, the read() call will block until an integer 
 * is written into the slot.
 * NOTE: BE SURE TO REINITIALIZE THE DRIVER BEFORE RUNNING THIS TEST (./down && ./up)
 */
int main()
{
	int fd, ret;
	int slot_index, read_index, read_value;
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

	/* Ensure slot is set correctly */
	read_index = -1;
	if ((ret = ioctl(fd, HIOCGETSLOT, &read_index)) < 0) {
		printf("HIOCGETSLOT error.\n");
		close_dev(fd);
		return ERROR;
	}
	printf("HIOCGETSLOT current slot index: %d.\n", read_index);
	assert(ret == slot_index);

	/* Read slot without writing first -- this should block */
	/* use ./write to unblock */
	/* ./homework_test_a & to start multiple readers that will all unblock on ./write */
	printf("About to call read -- this should block!\n");
	if ((size = read(fd, &read_value, sizeof(read_value))) < 0) {
		printf("Error reading: %d\n", errno);
		close_dev(fd);
		return ERROR;
	}
	printf("Read %d: %d bytes.\n", read_value, size);

	ret = close_dev(fd);

	return ret;
}