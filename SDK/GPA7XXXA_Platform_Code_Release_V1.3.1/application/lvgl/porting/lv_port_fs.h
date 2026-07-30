/**
 * @file lv_port_fs_templ.h
 *
 */

/*Copy this file as "lv_port_fs.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_FS_TEMPL_H
#define LV_PORT_FS_TEMPL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"
#include "fs.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
#if defined(RUN_ON_HMI_BOARD) && (RUN_ON_HMI_BOARD == 1)
    #define HMI_DEMO_CARD_SEL   FS_SD
    #define FS_ROOT_PATH        "C:\\"
    #define FS_LETTER           'C'
#else
    #define HMI_DEMO_CARD_SEL   FS_SD1
    #define FS_ROOT_PATH        "F:\\"
    #define FS_LETTER           'F'
#endif

#define GP_FS_FATFS_CACHE_SIZE (63U * 1024U)
/**********************
 * GLOBAL PROTOTYPES
 **********************/
INT16S lv_fs_gp_init(void);
void fs_dummy_open(void);
char *get_fs_root_path(INT8U drv);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_FS_TEMPL_H*/

#endif /*Disable/Enable content*/
