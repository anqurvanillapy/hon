#ifndef _HON_H
#define _HON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#define DEFAULT_SLOTS       16
#define MAX_SLOTS           256

#define MSGBOX_BUFSIZ       256

#define MAX_DELIVERY_COUNT  8
#define MAX_DELIVERY_DELAY  3000000

/**
 *  Utilities.
 */

#define HON_API

// Takes ownership from the caller.
#define HON_OWNER(x) x

#define UNLIKELY(cond) __builtin_expect((cond), 0)

#define ERRNO_ASSERT(x)                                                     \
	do {                                                                    \
		if (UNLIKELY(!(x))) {                                               \
			const char* errmsg = strerror(errno);                           \
			fprintf(stderr, "%s (%s:%d)\n", errmsg, __FILE__, __LINE__);    \
			fflush(stderr);                                                 \
			abort();                                                        \
		}                                                                   \
	} while (0)

#define CTX_SHALL_BE_RUNNING        \
	do {                            \
		if (UNLIKELY(!_started)) {  \
			errno = ENETDOWN;       \
			ERRNO_ASSERT(0);        \
		}                           \
	} while (0)

#define FREEN(x) do { free(x); x = NULL; } while (0)

#define CACHE_ALIGNED __attribute__((__aligned__(sizeof(size_t) * 2)))

/**
 *  Messaging.
 */

typedef struct hon_msg_t {
	atomic_size_t   seq;
	int             from;
	int             to;
	size_t          size;
	void*           data;
} hon_msg_t;

HON_API hon_msg_t* hon_msg_create(size_t size, HON_OWNER(void*) data);

typedef struct hon_mailbox_t {
	atomic_size_t   inpos   CACHE_ALIGNED;
	atomic_size_t   outpos  CACHE_ALIGNED;
	atomic_size_t   size    CACHE_ALIGNED;
	hon_msg_t*      buf;
	size_t          mask;
	uint64_t        last_tsc;
} hon_mailbox_t;

hon_mailbox_t*  hon_mailbox_create(void);
void            hon_mailbox_destroy(hon_mailbox_t* self);

int             hon_mailbox_push(hon_mailbox_t* self,
								 HON_OWNER(hon_msg_t*) msg);
int             hon_mailbox_pop(hon_mailbox_t* self, hon_msg_t* msg);

size_t          hon_mailbox_size(hon_mailbox_t* self);
int             hon_mailbox_is_full(hon_mailbox_t* self);

/**
 *  Context.
 */

typedef struct hon_slot_t {
	int             nin;
	hon_mailbox_t*  inbox;
	int             nout;
	hon_mailbox_t*  outbox;
	int             terminating;
} hon_slot_t;

typedef struct hon_ctx_t {
	int             nslot;
	hon_slot_t*     slots;
	pthread_mutex_t mtx;
} hon_ctx_t;

void            hon_ctx_init(void);
void            hon_ctx_shutdown(void);

int             hon_ctx_attach(void);
hon_slot_t*     hon_ctx_get_slot(int id);
void            hon_ctx_deliver_messages(hon_mailbox_t* self);

/**
 *  Actor.
 */

typedef void (hon_actor_be_t) (void* args);

typedef struct hon_actor_t {
	int             id;
	pthread_t       tid;
	hon_actor_be_t* fn;
	void*           args;
} hon_actor_t;

HON_API hon_actor_t*    hon_actor_create(void);
HON_API void            hon_actor_behavior(hon_actor_t* self,
										   hon_actor_be_t* fn,
										   void* args);
HON_API void            hon_actor_start(hon_actor_t* self);

HON_API int             hon_msg_send(hon_actor_t* self,
									 hon_actor_t* to,
									 HON_OWNER(hon_msg_t*) msg);
HON_API int             hon_msg_recv(hon_actor_t* self,
									 hon_msg_t* msg);

#endif /* !_HON_H */
