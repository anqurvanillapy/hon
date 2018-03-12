#include "hon.h"

static int _started = 0;
static hon_ctx_t* _ctx;

static int
_hon_ctx_new_msgbox(int i)
{
	_ctx->slots[i].nin      = 0;
	_ctx->slots[i].inbox    = (hon_lfq_t*)malloc(sizeof(hon_lfq_t));

	if (UNLIKELY(!_ctx->slots[i].inbox)) {
		return 0;
	}

	_ctx->slots[i].nout     = 0;
	_ctx->slots[i].outbox   = (hon_lfq_t*)malloc(sizeof(hon_lfq_t));

	if (UNLIKELY(!_ctx->slots[i].outbox)) {
		return 0;
	}

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

	_started = 1;
}

int
hon_ctx_attach()
{
	hon_ctx_init();

	int n = _ctx->nslot;

	if (UNLIKELY(!_hon_ctx_new_msgbox(n))) {
		return -1;
	}

	++_ctx->nslot;
	return n;
}
