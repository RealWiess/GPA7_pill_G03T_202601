#ifndef LV_PORT_TOUCH_H
#define LV_PORT_TOUCH_H

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
//touch point structure
typedef struct
{
	INT8U TouchSta;     //touch state, bit7~5: reserved; bit4~0: touch flag, corresponding touch point 5~1.
	INT16U x[5];		//the maximum touch number supported is 5
	INT16U y[5];

}TouchPointRef_t;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
extern INT32S touch_init(void);
extern INT32S touch_scan(TouchPointRef_t *TPR_Structure);

#endif /* LV_PORT_TOUCH_H */
