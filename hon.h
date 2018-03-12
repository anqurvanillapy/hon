#ifndef _HON_H
#define _HON_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#define DEFAULT_SLOTS   16
#define MAX_SLOTS       256

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

#define FREEN(x)    do { free(x); x = NULL; } while (0)

typedef struct hon_lfq_t {
	// TODO
} hon_lfq_t;

/**
 *  Context.
 */

typedef struct hon_slot_t {
	int nin;
	hon_lfq_t* inbox;
	int nout;
	hon_lfq_t* outbox;
} hon_slot_t;

typedef struct hon_ctx_t {
	int nslot;
	hon_slot_t* slots;
} hon_ctx_t;

void    hon_ctx_init(void);
int     hon_ctx_attach(void);

/**
 *  Actor.
 */

typedef void* (hon_actorfn_t) (void* args);

typedef struct hon_actor_t {
	int sid;        // slot id
	pthread_t tid;  // pthread id
	hon_actorfn_t* fn;
	void* args;
} hon_actor_t;

hon_actor_t*    hon_actor_create(void);
int             hon_actor_start(hon_actor_t* self,
								hon_actorfn_t* fn,
								void* args);

#endif /* !_HON_H */
