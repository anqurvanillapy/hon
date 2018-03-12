#include "hon.h"

hon_msgbox_t*
hon_msgbox_create()
{
	hon_msgbox_t* msgbox = (hon_msgbox_t*)malloc(sizeof(hon_msgbox_t));

	if (UNLIKELY(!msgbox)) {
		return NULL;
	}

	msgbox->buf = (hon_msg_t*)malloc(sizeof(hon_msg_t) * MSGBOX_BUFSIZ);

	if (UNLIKELY(!msgbox->buf)) {
		FREEN(msgbox);
		return NULL;
	}

	msgbox->inpos = msgbox->outpos = 0;
	msgbox->mask = MSGBOX_BUFSIZ - 1;
	return msgbox;
}
