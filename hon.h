#ifndef _HON_H
#define _HON_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdatomic.h>

#define DEFAULT_SLOTS   16
#define MAX_SLOTS       256

#define MSGBOX_BUFSIZ   256

/**
 *  Utilities.
 */

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

#define FREEN(x) do { free(x); x = NULL; } while (0)

#define CACHE_ALIGNED __attribute__((__aligned__(sizeof(size_t) * 2)))

/**
 *  Messaging.
 */

typedef struct hon_msg_t {
	atomic_size_t   seq;
	size_t          size;
	void*           data;
} hon_msg_t;

typedef struct hon_msgbox_t {
	atomic_size_t   inpos   CACHE_ALIGNED;
	atomic_size_t   outpos  CACHE_ALIGNED;
	hon_msg_t*      buf     CACHE_ALIGNED;
	size_t          mask;
} hon_msgbox_t;

hon_msgbox_t* hon_msgbox_create(void);

/**
 *  Context.
 */

typedef struct hon_slot_t {
	int             nin;
	hon_msgbox_t*   inbox;
	int             nout;
	hon_msgbox_t*   outbox;
} hon_slot_t;

typedef struct hon_ctx_t {
	int         nslot;
	hon_slot_t* slots;
} hon_ctx_t;

void    hon_ctx_init(void);
int     hon_ctx_attach(void);

/**
 *  Actor.
 */

typedef void (hon_actor_be_t) (void* args);

typedef struct hon_actor_t {
	int             sid;
	pthread_t       tid;
	hon_actor_be_t* fn;
	void*           args;
} hon_actor_t;

hon_actor_t*    hon_actor_create(void);
void            hon_actor_behavior(hon_actor_t* self,
								   hon_actor_be_t* fn,
								   void* args);
void            hon_actor_start(hon_actor_t* self);

#endif /* !_HON_H */
