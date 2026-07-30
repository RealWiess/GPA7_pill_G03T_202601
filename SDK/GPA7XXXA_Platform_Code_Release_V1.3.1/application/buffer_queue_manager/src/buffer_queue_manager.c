#include "core_ca7.h"
#include "buffer_queue_manager.h"
#include "gplib_mm_gplus.h"

//Buffer Queue Functions
buffer_queue_t* BufferQueueInit(INT32U Buf_Addr, INT32U Frame_Size, INT32U Buf_Num)
{
	INT32S i;
	INT32U buf_ptr;
	buffer_queue_t* bi = (buffer_queue_t*)gp_malloc(sizeof(buffer_queue_t));
	osMessageQDef(MsgQ, Buf_Num, INT32U);

	if (!bi)
		return 0;

	memset(bi, 0, sizeof(buffer_queue_t));

	if (Buf_Addr) {
		bi->ext_ptr = (void*)Buf_Addr;
		buf_ptr = Buf_Addr;
	}
	else
	{
		bi->ptr = (void*)gp_malloc(Frame_Size*Buf_Num);
		if (!bi->ptr)
		{
			gp_free(bi);
			return 0;
		}
		buf_ptr = (INT32U)bi->ptr;
	}

	bi->size = Frame_Size;
	bi->buf_num = Buf_Num;

	bi->bufq = osMessageCreate(osMessageQ(MsgQ), NULL);

	for (i = 0; i < Buf_Num; i++) {
		osMessagePut(bi->bufq, (int)&buf_ptr, 0);
		buf_ptr += Frame_Size;
	}
	bi->free_num = Buf_Num;

	return bi;
}

INT32U BufferQueueGetBuf(buffer_queue_t* bi)
{
	INT32U ret = 0;
	osEvent event;

	event = osMessageGet(bi->bufq, 0);

	if (event.status == osEventMessage) {
		ret = event.value.v;
		bi->free_num --;
	}

	return ret;
}

int BufferQueueFreeBuf(buffer_queue_t* bi, INT32U addr)
{
	INT32S ret = -1;
	if (bi->ptr) {
		if ((addr < (unsigned int)bi->ptr) || addr >= ((unsigned int)bi->ptr + bi->size*bi->buf_num))
			return -1;
	}

	ret = osMessagePut(bi->bufq, (int)&addr, 0);

	if (ret == osOK)
		bi->free_num ++;

	return ret;
}

void BufferQueueDestroy(buffer_queue_t* bi)
{
	if (bi->ptr)
		gp_free(bi->ptr);

	if (bi->bufq)
		gp_free(bi->bufq);

	gp_free(bi);
}

int BufferQueueQuery(buffer_queue_t* bi)
{
	INT32S ret;

	ret = bi->free_num;

	return ret;
}

int BufferQueueWaitFinish(buffer_queue_t* bi, int timeout)
{
	int start = 0;

	if (timeout)
		start = xTaskGetTickCount();
	while(BufferQueueQuery(bi) < bi->buf_num) {
		osDelay(1);
		if (start && ((xTaskGetTickCount() - start) >= timeout))
			return STATUS_FAIL;
	}

	return STATUS_OK;
}

int BufferQueueReset(buffer_queue_t* bi)
{
	int i;
	INT32U buf_ptr;
	osEvent event;

	if (!bi)
		return STATUS_FAIL;

	if (bi->ptr)
		buf_ptr = (unsigned int)bi->ptr;
	else if (bi->ext_ptr)
		buf_ptr = (unsigned int)bi->ext_ptr;
	else
		return STATUS_FAIL;

	while(1) {
		event = osMessageGet(bi->bufq, 0);
		if (event.status != osEventMessage)
			break;
	}
	for (i = 0; i < bi->buf_num; i++) {
		osMessagePut(bi->bufq, (int)&buf_ptr, 0);
		buf_ptr += bi->size;
	}
	bi->free_num = bi->buf_num;

	return STATUS_OK;
}
