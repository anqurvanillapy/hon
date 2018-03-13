#include "hon.h"

hon_mailbox_t*
hon_mailbox_create()
{
	hon_mailbox_t* self = (hon_mailbox_t*)malloc(sizeof(hon_mailbox_t));

	if (UNLIKELY(!self)) {
		return NULL;
	}

	self->buf = (hon_msg_t*)malloc(sizeof(hon_msg_t) * MSGBOX_BUFSIZ);

	if (UNLIKELY(!self->buf)) {
		FREEN(self);
		return NULL;
	}

	memset(self->buf, 0, sizeof(hon_msg_t) * MSGBOX_BUFSIZ);
	self->inpos = self->outpos = 0;
	self->mask = MSGBOX_BUFSIZ - 1;
	return self;
}

void
hon_mailbox_destroy(hon_mailbox_t* self)
{
	FREEN(self->buf);
	FREEN(self);
}

int
hon_mailbox_push(hon_mailbox_t* self, HON_OWNER(hon_msg_t*) msg)
{
	hon_msg_t* tmp;
	size_t pos, seq;
	intptr_t diff;

	pos = atomic_load_explicit(&self->inpos, memory_order_relaxed);

	for (;;) {
		tmp     = &self->buf[pos & self->mask];
		seq     = atomic_load_explicit(&tmp->seq, memory_order_acquire);
		diff    = (intptr_t)seq - (intptr_t)pos;

		if (diff == 0) {
			if (atomic_compare_exchange_weak_explicit(&self->inpos,
													  &pos,
													  pos + 1,
													  memory_order_release,
													  memory_order_relaxed)) {
				break;
			}
		} else if (diff > 0) {
			pos = atomic_load_explicit(&self->inpos, memory_order_relaxed);
		} else {
			return 0;
		}
	}

	tmp->from = msg->from;
	tmp->to= msg->to;
	tmp->size = msg->size;
	tmp->data = msg->data;
	atomic_fetch_add_explicit(&self->size, 1, memory_order_relaxed);
	atomic_store_explicit(&tmp->seq, pos + 1, memory_order_release);
	FREEN(msg);
	return 1;
}

int
hon_mailbox_pop(hon_mailbox_t* self, hon_msg_t* msg)
{
	hon_msg_t* tmp;
	size_t pos, seq;
	intptr_t diff;

	pos = atomic_load_explicit(&self->outpos, memory_order_relaxed);

	for (;;) {
		tmp     = &self->buf[pos % self->mask];
		seq     = atomic_load_explicit(&tmp->seq, memory_order_acquire);
		diff    = (intptr_t)seq - (intptr_t)(pos + 1);

		if (diff == 0) {
			if (atomic_compare_exchange_weak_explicit(&self->outpos,
													  &pos,
													  pos + 1,
													  memory_order_release,
													  memory_order_relaxed)) {
				break;
			}
		} else if (diff > 0) {
			pos = atomic_load_explicit(&self->outpos, memory_order_relaxed);
		} else {
			return 0;
		}
	}

	msg->size = tmp->size;
	msg->data = tmp->data;
	atomic_fetch_add_explicit(&self->size, 1, memory_order_relaxed);
	atomic_store_explicit(&tmp->seq, pos + MSGBOX_BUFSIZ, memory_order_release);
	return 1;
}

size_t
hon_mailbox_size(hon_mailbox_t* self)
{
	size_t size = atomic_load_explicit(&self->size, memory_order_relaxed);
	return size;
}

HON_API hon_msg_t*
hon_msg_create(size_t size, HON_OWNER(void*) data)
{
	hon_msg_t* self = (hon_msg_t*)malloc(sizeof(hon_msg_t));

	if (UNLIKELY(!self)) {
		return NULL;
	}

	memset(self, 0, sizeof(hon_msg_t));
	self->size = size;
	self->data = data;
	data = NULL;
	return self;
}

HON_API int
hon_msg_send(hon_actor_t* self, hon_actor_t* to, HON_OWNER(hon_msg_t*) msg)
{
	if (UNLIKELY(!self || !msg)) {
		errno = EINVAL;
		return 0;
	}

	hon_slot_t* slot = hon_ctx_get_slot(self->id);

	if (UNLIKELY(!slot)) {
		errno = EBADSLT;
		return 0;
	}

	msg->from = self->id;
	msg->to = to ? to->id : self->id;
	return hon_mailbox_push(slot->outbox, msg);
}

HON_API int
hon_msg_recv(hon_actor_t* self, hon_msg_t* msg)
{
	if (UNLIKELY(!self || !msg)) {
		return 0;
	}

	hon_slot_t* slot = hon_ctx_get_slot(self->id);

	if (UNLIKELY(!slot)) {
		return 0;
	}

	return hon_mailbox_pop(slot->inbox, msg);
}
