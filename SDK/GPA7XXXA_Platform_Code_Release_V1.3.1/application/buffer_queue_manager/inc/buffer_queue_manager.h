#ifndef BUFFER_QUEUE_MANAGER_H
#define BUFFER_QUEUE_MANAGER_H

#include "project.h"
#include "cmsis_os.h"

typedef struct buffer_queue_s
{
	int buf_num;
	int size;
	int free_num;
	osMessageQId bufq;
	void* ptr;
	void* ext_ptr;
} buffer_queue_t;

typedef buffer_queue_t* bufferhandle;

extern buffer_queue_t* BufferQueueInit(INT32U Buf_Addr, INT32U Frame_Size, INT32U Buf_Num);
extern INT32U BufferQueueGetBuf(buffer_queue_t* bi);
extern int BufferQueueFreeBuf(buffer_queue_t* bi, INT32U addr);
extern void BufferQueueDestroy(buffer_queue_t* bi);
extern int BufferQueueQuery(buffer_queue_t* bi);
extern int BufferQueueWaitFinish(buffer_queue_t* bi, int timeout);
extern int BufferQueueReset(buffer_queue_t* bi);

#endif
