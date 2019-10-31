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
	int slot_index, read_index, slot_value;
	ssize_t size;
	int *read_buffer = malloc(sizeof(int));

	/* Open */
	if ((fd = open(HW_DEV, O_RDWR)) < 0) {
		printf("OPEN error: %d.\n", fd);
		return ERROR;
	}
	printf("OPEN %s succeeded.\n", HW_DEV);

	/* IOCTL HIOCSLOT Set Current Slot Index */
	slot_index = 3;
	if (ioctl(fd, HIOCSLOT, &slot_index) <0) {
		printf("HIOCSLOT error.\n");
		close_dev(fd);
		return ERROR;
	}

	/* IOCTL HIOCGETSLOT Get Current Slot Index */
	read_index = -1;
	if (ioctl(fd, HIOCGETSLOT, &read_index) < 0) {
		printf("HIOCGETSLOT error.\n");
		close_dev(fd);
		return ERROR;
	}
	printf("HIOCGETSLOT current slot index: %d.\n", read_index);

	/* We expect the currentSlot index to be the one we set. */
	assert(read_index == slot_index);

	/* IOCTL HIOCLEARSLOT Clear Current Slot Index */
	if (ioctl(fd, HIOCCLEARSLOT) < 0) {
		printf("HIOCLEARSLOT error.\n");
		close_dev(fd);
		return ERROR;
	}
	
	/* We expect HIOCGETSLOT to return an error since the slot number has been cleared */
	if (ioctl(fd, HIOCGETSLOT, &read_index) < 0) {
		printf("HIOCCLEARSLOT succeeded.\n");
	} else {
		printf("HIOCGETSLOT error.\n");
		close_dev(fd);
		return ERROR;
	}

	/* Close */
	ret = close_dev(fd);
	return ret;
}
