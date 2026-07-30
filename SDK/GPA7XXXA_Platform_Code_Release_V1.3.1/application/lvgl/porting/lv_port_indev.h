
/**
 * @file lv_port_indev_templ.h
 *
 */

/*Copy this file as "lv_port_indev.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_INDEV_H
#define LV_PORT_INDEV_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define INPUT_TOUCH_PAD     1
#if defined(RUN_ON_HMI_BOARD) && (RUN_ON_HMI_BOARD == 1)
    #define INPUT_ADC_KEY       0
    #define INPUT_IO_KEY        1
#else
    #define INPUT_ADC_KEY       1
    #define INPUT_IO_KEY        0
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_port_indev_init(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_INDEV_H*/

#endif /*Disable/Enable content*/
