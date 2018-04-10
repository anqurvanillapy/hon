#include "hon.h"

static int _started = 0;
static hon_ctx_t* _ctx = NULL;

static uint64_t
_rdtsc()
{
	uint32_t low, high;
	__asm__ __volatile__ ("rdtsc" : "=a"(low), "=d"(high));
	return (uint64_t)high << 32 | low;
}

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

void
hon_ctx_shutdown()
{
	CTX_SHALL_BE_RUNNING;
	// TODO
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

hon_slot_t*
hon_ctx_get_slot(int id)
{
	CTX_SHALL_BE_RUNNING;

	if (UNLIKELY(id < 0 && id >= _ctx->nslot)) {
		errno = EBADSLT;
		return NULL;
	}

	hon_slot_t* slot = &_ctx->slots[id];

	if (UNLIKELY(!slot->inbox || !slot->outbox)) {
		errno = EBADSLT;
		return NULL;
	}

	if (UNLIKELY(slot->terminating)) {
		errno = ESHUTDOWN;
		return NULL;
	}

	return slot;
}

void
hon_ctx_deliver_messages(hon_mailbox_t* self)
{
	CTX_SHALL_BE_RUNNING;

	if ((self->last_tsc && _rdtsc() - self->last_tsc < MAX_DELIVERY_DELAY)
	|| hon_mailbox_size(self) < MAX_DELIVERY_COUNT) {
		return;
	}

	pthread_mutex_lock(&_ctx->mtx);

	size_t i;
	size_t nmsg = hon_mailbox_size(self);

	for (i = 0; i < nmsg; ++i) {
		hon_msg_t* msg = NULL;
		ERRNO_ASSERT(hon_mailbox_pop(self, msg));

		if (UNLIKELY(msg->cmd != HON_CMD_NIL)) {
			_hon_ctx_process_cmd(/* TODO */);
		}

		hon_slot_t* dst = hon_ctx_get_slot(msg->to);
		ERRNO_ASSERT(dst);

		if (UNLIKELY(hon_mailbox_is_full(dst->inbox))) {
			ERRNO_ASSERT(hon_mailbox_push(dst->outbox, msg));
			continue;
		}

		ERRNO_ASSERT(hon_mailbox_push(dst->inbox, msg));
	}

	pthread_mutex_unlock(&_ctx->mtx);
}
