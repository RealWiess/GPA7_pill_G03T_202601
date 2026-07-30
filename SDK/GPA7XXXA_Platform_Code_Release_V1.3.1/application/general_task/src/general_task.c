/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2022 by Generalplus Inc.                         *
 *                                                                        *
 *  This software is copyrighted by and is the property of Generalplus    *
 *  Inc. All rights are reserved by Generalplus Inc.                      *
 *  This software may only be used in accordance with the                 *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice MUST not be removed or modified without prior   *
 *  written consent of Generalplus Technology Co., Ltd.                   *
 *                                                                        *
 *  Generalplus Inc. reserves the right to modify this software           *
 *  without notice.                                                       *
 *                                                                        *
 *  Generalplus Inc.                                                      *
 *  No.19, Industry E. Rd. IV, Hsinchu Science Park                       *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/

#include <stdio.h>
#include "project.h"
#include "cmsis_os.h"
#include "core_ca7.h"
#include "gp_stdlib.h"
#include "gplib.h"
#include "gplib_mm_gplus.h"
#include "gplib_print_string.h"
#include "general_task.h"

 //General Task Functions
static void GeneralTask(void const *taskhandle)
{
	taskinfo_t* task = (taskinfo_t*)taskhandle;
	osEvent event;
	int ret = 0;
	INT32U ack;

	if (!task)
		return;

	if (task->init_callback) {
		ret = (*(task->init_callback))(task->argument);
		osMessagePut(task->return_ack, (INT32U)&ret, 0);
		if (ret < 0) {
			DBG_PRINT("task init function fail\r\n");
			vTaskDelete(NULL);
		}
	}

	while(1)
	{
		event = osMessageGet(task->in_queue, osWaitForever);

		if (event.value.signals == GENERAL_TASK_EXIT) {
			ret = STATUS_OK;
			if (task->exit_callback)
				ret = (*(task->exit_callback))(task->argument);
			if (ret < 0)
				continue;
			osMessagePut(task->exit_ack, (INT32U)&ack, 0);
			break;
		}

		if (task->task_callback)
			ret = (*(task->task_callback))(event.value.v, task->argument);

		event = osMessageGet(task->cmd_ack, 0);
		if (event.status == osEventMessage) {
			osMessagePut(task->return_ack, (INT32U)&ret, 0);
		}
	}

	task->tid = 0;
	vTaskDelete(NULL);
}

INT32S SendToGeneralTask(taskhandle task, INT32U value)
{
	if (osOK != osMessagePut(task->in_queue, (INT32U)&value, 0))
		return -1;

	return 0;
}

INT32S SendToGeneralTaskAck(taskhandle task, INT32U value, INT32U timeout, INT32U *Ack)
{
	osEvent event;
	INT32U tmp = 0;

	osMessageGet(task->return_ack, 0);
	osMessagePut(task->cmd_ack, (INT32U)&tmp, osWaitForever);

	if (osOK != osMessagePut(task->in_queue, (INT32U)&value, 0))
		return -1;

	event = osMessageGet(task->return_ack, timeout);

	if (event.status == osEventMessage) {
		*Ack = event.value.v;
	}

	return 0;
}

taskhandle CreateGeneralTask( char* name, INT32U input_num, osPriority priority, INT32U stacksize, void* argument,
							int (*init_callback)(void*), int (*task_callback)(INT32U, void*), int (*exit_callback)(void*))
{
	int ret = 0;
	taskinfo_t* task;
	osMessageQDef(MsgQ, input_num, INT32U);
	osMessageQDef(AckQ, 1, INT32U);
	osThreadDef_t os_thread_def_GeneralTask =
	{ name, (GeneralTask), (priority), 1, (stacksize)  };

	task = (taskinfo_t*)gp_malloc(sizeof(taskinfo_t));

	if (!task)
		return NULL;

	gp_memset((void*)task, 0x0, sizeof(taskinfo_t));

	task->input_num = input_num;
	task->in_queue = osMessageCreate(osMessageQ(MsgQ), NULL);
	task->exit_ack = osMessageCreate(osMessageQ(AckQ), NULL);
	task->cmd_ack = osMessageCreate(osMessageQ(AckQ), NULL);
	task->return_ack = osMessageCreate(osMessageQ(AckQ), NULL);
	task->init_callback = init_callback;
	task->task_callback = task_callback;
	task->exit_callback = exit_callback;
	task->argument = argument;

	task->tid = osThreadCreate(osThread(GeneralTask), task);

	if (task->tid && init_callback) {
		osEvent event;

		event = osMessageGet(task->return_ack, osWaitForever);
		if (event.status == osEventMessage) {
			ret = event.value.signals;
		}
	}
	else if (!task->tid) {
		ret = -1;
	}

	if (ret < 0) {
		gp_free(task->in_queue);
		gp_free(task->exit_ack);
		gp_free(task->cmd_ack);
		gp_free(task->return_ack);
		gp_free(task);
		task = NULL;
	}

	return task;
}

INT32S GeneralTaskExit(taskhandle handle, int timeout)
{
	taskinfo_t* task = (taskinfo_t*)handle;
	osEvent event;
	int wait = 0;

	if (!task)
		return STATUS_FAIL;

	osMessageGet(task->exit_ack, 0);

	while(0 > SendToGeneralTask(handle, GENERAL_TASK_EXIT)) {
		osDelay(1);
		wait++;
		if (wait == 10)
			break;
	}

	event = osMessageGet(task->exit_ack, timeout);

	if (event.status == osEventMessage) {
		task->tid = 0;
		gp_free(task->in_queue);
		gp_free(task->exit_ack);
		gp_free(task->cmd_ack);
		gp_free(task->return_ack);
		gp_free(task);

		return STATUS_OK;
	}

	return STATUS_FAIL;
}

void DeleteGeneralTask(taskhandle handle)
{
	taskinfo_t* task = (taskinfo_t*)handle;

	if (!task)
		return;

	if (task->tid)
		osThreadTerminate(task->tid);

	if (task->in_queue)
		gp_free(task->in_queue);
	if (task->exit_ack)
		gp_free(task->exit_ack);
	if (task->cmd_ack)
		gp_free(task->cmd_ack);
	if (task->return_ack)
		gp_free(task->return_ack);

	gp_free(task);
}

