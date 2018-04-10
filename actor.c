#include "hon.h"

static void*
_hon_actor_threadfn(void* args)
{
	hon_actor_t* self = (hon_actor_t*)args;
	self->fn(self->args);
	hon_msg_t* cmd = hon_cmd_create(HON_CMD_TERM);
	hon_msg_send(self, self, cmd);
	return NULL;
}

hon_actor_t*
hon_actor_create()
{
	hon_actor_t* self = (hon_actor_t*)malloc(sizeof(hon_actor_t));

	if (UNLIKELY(!self)) {
		return NULL;
	}

	self->id = hon_ctx_attach();

	if (UNLIKELY(self->id < 0)) {
		FREEN(self);
		return NULL;
	}

	self->tid = 0;
	return self;
}

void
hon_actor_behavior(hon_actor_t* self, hon_actor_be_t* fn, void* args)
{
	self->fn    = fn;
	self->args  = args;
}

void
hon_actor_start(hon_actor_t* self)
{
	assert(self);
	assert(self->fn);

	pthread_create(&self->tid, NULL, _hon_actor_threadfn, self);
	pthread_detach(self->tid);
}
