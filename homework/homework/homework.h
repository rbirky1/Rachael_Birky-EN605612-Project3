#ifndef __HOMEWORK_H
#define __HOMEWORK_H

/* Number of slots */
#define NUM_SLOTS 5

/* Uninitialized value */
#define UNINITIALIZED -999

/* Error Return Code */
#define INVALID -1

#define NUM_HW_T 100

/* Derived from /usr/src/minix/drivers/tty/tty/tty.h */
struct hw;
typedef struct hw {
	endpoint_t hw_inendpt;		/* process that made the call, or NONE */
	cdev_id_t hw_inid;			/* ID of suspended read request */
	cp_grant_id_t hw_ingrant;	/* grant where data is to go */
	size_t hw_insize;			/* size parameter of suspended read request */
} hw_t;

#endif /* __HOMEWORK_H */
