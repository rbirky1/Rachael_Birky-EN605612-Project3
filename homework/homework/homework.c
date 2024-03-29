#include <minix/drivers.h>
#include <minix/chardriver.h>
#include <stdio.h>
#include <stdlib.h>
#include <minix/ds.h>
#include <minix/homework_ioctl.h>
#include "homework.h"

#define DEBUG 1

/* Function prototypes for the homework driver. */
static int homework_open(devminor_t minor, int access, endpoint_t user_endpt);
static int homework_close(devminor_t minor);
static ssize_t homework_read(devminor_t minor, u64_t position, endpoint_t endpt,
	cp_grant_id_t grant, size_t size, int flags, cdev_id_t id);
static ssize_t homework_write(devminor_t minor, u64_t position, endpoint_t endpt,
	cp_grant_id_t grant, size_t size, int flags, cdev_id_t id);
static int homework_ioctl(devminor_t minor, unsigned long request, endpoint_t endpt,
	cp_grant_id_t grant, int flags, endpoint_t user_endpt, cdev_id_t id);

/* SEF functions and variables. */
static void sef_local_startup(void);
static int sef_cb_init(int type, sef_init_info_t *info);
static int sef_cb_lu_state_save(int);
static int lu_state_restore(void);
static void homework_init(void);

/* Entry points to the homework driver. */
static struct chardriver homework_tab =
{
	.cdr_open	= homework_open,
	.cdr_close	= homework_close,
	.cdr_read	= homework_read,
	.cdr_write	= homework_write,
	.cdr_ioctl	= homework_ioctl
};

static int open_counter;

/* Current slot */
int currentSlot;

struct hw_t* head = NULL;
struct hw_t* tail = NULL;

/**
 * Open homework driver
 * - initializes all the slots to 0
 * - initializes the currentSlot to UNINITIALIZED (-1)
 *
 * @param minor the minor number of the device
 * @param access the access of the caller
 */
static int homework_open(devminor_t UNUSED(minor), int UNUSED(access),
	endpoint_t user_endpt)
{
	if (DEBUG)
		printf("homework_open(). Called %d time(s)\n", ++open_counter);

	return OK;
}

/**
 * Close homework driver
 * - resets all the slots to UNINITIALIZED (-1)
 * - resets the currentSlot to UNINITIALIZED (-1)
 *
 * @param minor the minor number of the device
 */
static int homework_close(devminor_t UNUSED(minor))
{
	if (DEBUG)
		printf("homework_close()\n");

	return OK;
}

/**
 * Read homework driver
 * - validates currentSlot index
 * - validates requested size
 * - copies value from currentSlot to caller (sys_safecopyto)
 *
 * @param minor the minor number of the device
 * @param position current location of read (unused)
 * @param endpt memory of caller
 * @param grant of caller to determine access permission
 * @param size number of bytes to deliver to caller
 * @param flags unused
 * @param id unused
 */
static ssize_t homework_read(devminor_t minor, u64_t position,
	endpoint_t endpt, cp_grant_id_t grant, size_t size, int flags,
	cdev_id_t id)
{
	int ret;
	struct hw_t *hwp;

	if (DEBUG){
		printf("homework_read()\n");
		printf("reader id: %d.\n", id);
	}

	/* Suspend caller if currentSlot of value at currentSlot is uninitialized */
	if (currentSlot == UNINITIALIZED || slots[currentSlot] == UNINITIALIZED)
	{
		struct hw_t *hwp;
		hwp = (struct hw_t*)malloc(sizeof(struct hw_t));
		hwp->hw_inendpt = endpt;
		hwp->hw_inid = id;
		hwp->hw_ingrant = grant;
		hwp->hw_insize = size;
		hwp->hw_inposition = position;
		hwp->hw_inflags = flags;
		hwp->hw_inminor = minor;
		hwp->next = NULL;

		if (!head)
		{
			head = hwp;
			tail = hwp;
			head->previous = NULL;
			tail->next = NULL;
		}
		else
		{
			hwp->previous = tail;
			tail->next = hwp;
			tail = hwp;
		}

		if (DEBUG && currentSlot == UNINITIALIZED) {
			printf("Current slot uninitialized; blocking caller.\n");
		}
		if (DEBUG && slots[currentSlot] == UNINITIALIZED)
		{
			printf("Value at current slot uninitialized; blocking caller.\n");
		}
		return EDONTREPLY;
	}

	/* Validate request slot number */
	if (currentSlot < 0 || currentSlot > (NUM_SLOTS - 1)) {
		printf("Invalid slot number: %d\n", currentSlot);
		return INVALID;
	}

	/* Validate requested read size */
	if (size > sizeof(slots[currentSlot])) {
		printf("Invalid size specified: %d\n", size);
		return INVALID;
	}

	/* Copy the requested value to the caller using sys_safecopyto. */
	if ((ret = sys_safecopyto(endpt, grant, 0, (vir_bytes) &slots[currentSlot], size)) != OK)
	{
		if (DEBUG)
			printf("sys_safecopyto error: %d.\n", ret);
		return ret;
	}

	/* Return the number of bytes read. */
	return size;
}

/**
 * Write homework driver
 * - validates currentSlot index
 * - validates requested size
 * - copies given value from caller to currentSlot (sys_safecopyfrom)
 *
 * @param minor the minor number of the device
 * @param minor the minor number of the device
 * @param position current location of write (unused)
 * @param endpt memory of caller
 * @param grant of caller to determine access permission
 * @param size number of bytes to receive from caller
 * @param flags unused
 * @param id unused
 */
static ssize_t homework_write(devminor_t minor, u64_t position,
	endpoint_t endpt, cp_grant_id_t grant, size_t size, int flags,
	cdev_id_t id)
{
	int ret, i;
	struct hw_t *hwp;

	if (DEBUG)
		printf("homework_write()\n");

	/* Validate request slot number */
	if (currentSlot < 0 || currentSlot > (NUM_SLOTS - 1)) {
		printf("Invalid slot number: %d\n", currentSlot);
		return INVALID;
	}

	/* Validate requested write size */
	if (size > sizeof(slots[currentSlot])) {
		printf("Invalid size specified: %d\n", size);
		return INVALID;
	}

	/* Copy the received value to the slot using sys_safecopyfrom. */
	if ((ret = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) &slots[currentSlot], sizeof(int))) != OK) {
		if (DEBUG)
			printf("sys_safecopyfrom error: %d.\n", ret);
		return ret;
	}

	if (head)
	{
		hwp = head;
		while (hwp)
		{
			if (hwp->hw_inendpt != NONE)
			{
				size_t read_size = homework_read(hwp->hw_inminor, hwp->hw_inposition, hwp->hw_inendpt, hwp->hw_ingrant, hwp->hw_insize, hwp->hw_inflags, hwp->hw_inid);
				if (DEBUG) {
					printf("unblocking reader: %d.\n", hwp->hw_inid);
				}
				chardriver_reply_task(hwp->hw_inendpt, hwp->hw_inid, read_size);
				hwp->hw_inendpt = NONE;

				// Center
				if (hwp->previous && hwp->next)
				{
					hwp->previous->next = hwp->next;
					hwp->next->previous = hwp->previous;
				}
				// Head
				else if (hwp->next && !hwp->previous)
				{
					head = hwp->next;
					hwp->next->previous = NULL;
				}
				// Tail
				else if (hwp->previous && !hwp->next)
				{
					tail = hwp->previous;
					hwp->previous->next = NULL;
				}

				free(hwp);
			}
			hwp = hwp->next;
		}
	}

	return size;
}

/**
 * I/O Control for homework driver supporting:
 * - HIOCSLOT: set currentSlot to index given by caller if a valid index
 * - HIOCCLEARSLOT: sets value at currentSlot to 0 and currentSlot to UNINITIALIZED
 * - HIOCGETSLOT: sends value of slot at currentSlot to caller
 *
 * @param minor the minor number of the device
 * @param request macro indicated which I/O control mode to execute
 * @param endpt memory of caller
 * @param grant of caller to determine access permission
 * @param flags unused
 * @param user_endpt out memory
 * @param id unused
 */
static int homework_ioctl(devminor_t UNUSED(minor), unsigned long request, endpoint_t endpt,
	cp_grant_id_t grant, int UNUSED(flags), endpoint_t user_endpt, cdev_id_t UNUSED(id))
{
	int ret;
	int slot;

	if (DEBUG)
		printf("homework_ioctl()\n");

	switch(request)
	{
		case HIOCSLOT:
			if (DEBUG)
				printf("Case HIOCSLOT.\n");
			if ((ret = sys_safecopyfrom(endpt, grant, 0, (vir_bytes) &slot, sizeof(slot))) == OK) {
				if (slot < 0 || slot > (NUM_SLOTS - 1)) {
					printf("Invalid slot specified: %d\n", slot);
					ret = INVALID;
				} else {
					currentSlot = slot;
				}
			}
			break;

		case HIOCCLEARSLOT:
			if (DEBUG)
				printf("Case HIOCCLEARSLOT.\n");
			if (currentSlot < 0 || currentSlot > (NUM_SLOTS - 1)) {
				printf("Invalid currentSlot: %d\n", currentSlot);
				ret = INVALID;
			} else {
				slots[currentSlot] = 0;
				printf("Slot %d cleared.\n", currentSlot);
				currentSlot = UNINITIALIZED;
				if (DEBUG)
					printf("Current slot set to %d\n", currentSlot);
				ret = OK;
			}
			break;

		case HIOCGETSLOT:
			if (DEBUG)
				printf("Case HIOCGETSLOT.\n");
			if (currentSlot < 0 || currentSlot > (NUM_SLOTS - 1)) {
				printf("Invalid currentSlot: %d\n", currentSlot);
				ret = INVALID;
			} else {
				ret = currentSlot; //sys_safecopyto(endpt, grant, 0, (vir_bytes) &currentSlot, sizeof(currentSlot));
			}
			break;

		default:
			printf("Invalid HIOC request: %lu.\n", request);
			ret = INVALID;
			break;
	}

	return ret;
}

static int sef_cb_lu_state_save(int UNUSED(state)) {
	/* Save the state. */
	ds_publish_u32("open_counter", open_counter, DSF_OVERWRITE);

	return OK;
}

static int lu_state_restore() {
	/* Restore the state. */
	u32_t value;

	ds_retrieve_u32("open_counter", &value);
	ds_delete_u32("open_counter");
	open_counter = (int) value;

	return OK;
}

static void sef_local_startup()
{
	/*
	 * Register init callbacks. Use the same function for all event types
	 */
	sef_setcb_init_fresh(sef_cb_init);
	sef_setcb_init_lu(sef_cb_init);
	sef_setcb_init_restart(sef_cb_init);

	/*
	 * Register live update callbacks.
	 */
	/* - Agree to update immediately when LU is requested in a valid state. */
	sef_setcb_lu_prepare(sef_cb_lu_prepare_always_ready);
	/* - Support live update starting from any standard state. */
	sef_setcb_lu_state_isvalid(sef_cb_lu_state_isvalid_standard);
	/* - Register a custom routine to save the state. */
	sef_setcb_lu_state_save(sef_cb_lu_state_save);

	/* Let SEF perform startup. */
	sef_startup();
}

static int sef_cb_init(int type, sef_init_info_t *UNUSED(info))
{
	/* Initialize the homework driver. */
	int do_announce_driver = TRUE;

	switch(type) {
		case SEF_INIT_FRESH:
		if (DEBUG)
			printf("Hey, I've started fresh!\n");
		break;
		case SEF_INIT_LU:
			/* Restore the state. */
		lu_state_restore();
		do_announce_driver = FALSE;
		if (DEBUG)
			printf("Hey, I'm a new version!\n");
		break;
		case SEF_INIT_RESTART:
		if (DEBUG)
			printf("Hey, I've just been restarted!\n");
		break;
	}

	homework_init();

	/* Announce we are up when necessary. */
	if (do_announce_driver) {
		chardriver_announce();
	}

	/* Initialization completed successfully. */
	return OK;
}

static void homework_init()
{
	/* Initialize all slots to zero */
	int i;
	for (i=0; i<NUM_SLOTS; i++) {
		slots[i] = UNINITIALIZED;
		if (DEBUG)
			printf("Slot %d initialized to %d.\n", i, slots[i]);
	}

	/* Initialize currentSlot to uninitialized */
	currentSlot = UNINITIALIZED;
	if (DEBUG)
		printf("currentSlot initialized to %d.\n", currentSlot);
}

int main(void)
{
	/* Perform initialization. */
	sef_local_startup();

	/* Run the main loop. */
	chardriver_task(&homework_tab);
	return OK;
}

