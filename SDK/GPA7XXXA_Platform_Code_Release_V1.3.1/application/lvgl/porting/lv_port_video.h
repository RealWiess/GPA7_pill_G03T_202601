#ifndef LV_PORT_VIDEO_H_INCLUDED
#define LV_PORT_VIDEO_H_INCLUDED

#include "application.h"
#include "lvgl.h"

extern const lv_obj_class_t lv_gp_player_class;

typedef enum {
    LV_GP_PLAYER_CMD_START,
    LV_GP_PLAYER_CMD_STOP,
    LV_GP_PLAYER_CMD_PAUSE,
    LV_GP_PLAYER_CMD_RESUME,
    _LV_GP_PLAYER_CMD_LAST
} lv_gp_player_cmd_t;

lv_obj_t * lv_gp_player_create(lv_obj_t * parent);
lv_res_t lv_gp_player_set_src(lv_obj_t * obj, const char * path);
lv_res_t lv_gp_player_set_volume(lv_obj_t * obj, INT8S vol);
void lv_gp_player_set_cmd(lv_obj_t * obj, lv_gp_player_cmd_t cmd);
INT8U lv_gp_player_get_status(lv_obj_t * obj);

#endif // LV_PORT_VIDEO_H_INCLUDED
