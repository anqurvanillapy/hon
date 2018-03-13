#include "hon.h"

static int _started = 0;
static hon_ctx_t* _ctx = NULL;

static int
_hon_ctx_new_mailbox(int id)
{
	_ctx->slots[id].inbox = hon_mailbox_create();
	if (UNLIKELY(!_ctx->slots[id].inbox)) {
		return 0;
	}

	_ctx->slots[id].outbox = hon_mailbox_create();
	if (UNLIKELY(!_ctx->slots[id].outbox)) {
		FREEN(_ctx->slots[id].inbox);
		return 0;
	}

	_ctx->slots[id].terminating = 0;
	_ctx->slots[id].nin = _ctx->slots[id].nout = 0;
	return 1;
}

void
hon_ctx_init()
{
	if (_started) {
		return;
	}

	_ctx = (hon_ctx_t*)malloc(sizeof(hon_ctx_t));
	ERRNO_ASSERT(_ctx);

	_ctx->nslot = 0;
	_ctx->slots = (hon_slot_t*)malloc(sizeof(hon_slot_t) * DEFAULT_SLOTS);
	ERRNO_ASSERT(_ctx->slots);

	int rc = pthread_mutex_init(&_ctx->mtx, NULL);
	ERRNO_ASSERT(rc == 0);

	_started = 1;
}

int
hon_ctx_attach()
{
	hon_ctx_init();

	int n = _ctx->nslot;

	if (UNLIKELY(!_hon_ctx_new_mailbox(n))) {
		return -1;
	}

	++_ctx->nslot;
	return n;
}

void
hon_deliver_messages(int id)
{
	pthread_mutex_lock(&_ctx->mtx);
	// TODO
	pthread_mutex_unlock(&_ctx->mtx);
}

void
hon_ctx_shutdown()
{
}
