#ifndef __HOMEWORK_H
#define __HOMEWORK_H

/* Number of slots */
#define NUM_SLOTS 5

/* Uninitialized value */
#define UNINITIALIZED -999

/* Error Return Code */
#define INVALID -1

struct hw_t {
	endpoint_t hw_inendpt;		/* process that made the call, or NONE */
	cdev_id_t hw_inid;			/* ID of suspended read request */
	cp_grant_id_t hw_ingrant;	/* grant where data is to go */
	size_t hw_insize;			/* size parameter of suspended read request */
	u64_t hw_inposition;
	int hw_inflags;
	devminor_t hw_inminor;
	struct hw_t* next;			/* next reader in the queue */
	struct hw_t* previous;		/* previous reader in the queue */
};

#endif /* __HOMEWORK_H */
