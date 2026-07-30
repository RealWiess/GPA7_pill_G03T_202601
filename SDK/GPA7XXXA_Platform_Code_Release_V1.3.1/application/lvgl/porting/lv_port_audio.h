#ifndef LV_PORT_AUDIO_H_INCLUDED
#define LV_PORT_AUDIO_H_INCLUDED

#include "lvgl.h"
#include "application.h"

typedef enum {
    LV_GP_AUDIO_CMD_START,
    LV_GP_AUDIO_CMD_STOP,
    LV_GP_AUDIO_CMD_PAUSE,
    LV_GP_AUDIO_CMD_RESUME,
    _LV_GP_AUDIO_CMD_LAST
} lv_gp_audio_cmd_t;

lv_res_t lv_gp_audio_init(void);
void lv_gp_audio_uninit(void);
lv_res_t lv_gp_audio_set_src(const char * file_path);
lv_res_t lv_gp_audio_set_cmd(lv_gp_audio_cmd_t cmd);
lv_res_t lv_gp_audio_set_volume(INT8U volume);
AUDIO_CODEC_STATUS lv_gp_audio_get_status();

#endif // LV_PORT_AUDIO_H_INCLUDED
