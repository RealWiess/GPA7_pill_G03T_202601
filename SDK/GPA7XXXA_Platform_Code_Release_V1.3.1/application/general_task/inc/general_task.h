#ifndef GENERAL_TASK_H
#define GENERAL_TASK_H

#include "cmsis_os.h"

typedef struct taskinfo_s
{
	osMessageQId in_queue;
	unsigned int input_num;
	osThreadId 	tid;
	void*		argument;
	osMessageQId exit_ack;
	osMessageQId cmd_ack;
	osMessageQId return_ack;
	int (*init_callback)(void* argument);
	int (*task_callback)(unsigned int value, void* argument);
	int (*exit_callback)(void* argument);
} taskinfo_t;

typedef taskinfo_t* taskhandle;

extern INT32S SendToGeneralTask(taskhandle task, INT32U value);
extern INT32S SendToGeneralTaskAck(taskhandle task, INT32U value, INT32U timeout, INT32U *Ack);
extern taskhandle CreateGeneralTask( char* name, INT32U input_num, osPriority priority, INT32U stacksize, void* argument,
									int (*init_callback)(void*), int (*task_callback)(INT32U, void*), int (*exit_callback)(void*));
extern INT32S GeneralTaskExit(taskhandle handle, int timeout);
extern void DeleteGeneralTask(taskhandle handle);

#define GENERAL_TASK_EXIT 0xFFFF0000
#endif
