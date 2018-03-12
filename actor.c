#include "hon.h"

hon_actor_t*
hon_actor_create()
{
	hon_actor_t* self = (hon_actor_t*)malloc(sizeof(hon_actor_t));

	if (UNLIKELY(!self)) {
		return NULL;
	}

	self->sid = hon_ctx_attach();

	if (UNLIKELY(self->sid < 0)) {
		FREEN(self);
		return NULL;
	}

	self->tid = 0;
	return self;
}

int
hon_actor_start(hon_actor_t* self, hon_actorfn_t* fn, void* args)
{
	self->fn    = fn;
	self->args  = args;
	pthread_create(&self->tid, NULL, self->fn, self->args);
	pthread_detach(self->tid);
	return 1;
}
