#ifndef __HOMEWORK_IOCTL_H
#define __HOMEWORK_IOCTL_H

#include <minix/ioctl.h>

/* IOCTL Commands for Homework Driver */
#define HIOCSLOT _IOW('h', 1, int)
#define HIOCCLEARSLOT _IO('h', 2)
#define HIOCGETSLOT _IOR('h', 3, int)

#endif /* __HOMEWORK_IOCTL_H */
