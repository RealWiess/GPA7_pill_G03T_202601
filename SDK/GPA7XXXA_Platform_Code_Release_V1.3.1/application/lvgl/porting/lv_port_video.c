/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2023 by Generalplus Inc.                         *
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
#include <string.h>
#include "gplib.h"
#include "drv_l1_arm_cp15.h"
#include "drv_l1_dac.h"
#include "drv_l2_display.h"
#include "video_decoder.h"
#include "task_video_decoder.h"
#include "fs.h"
#include "lv_port_video.h"

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define OUTPUT_FORMAT		IMAGE_OUTPUT_FORMAT_RGB565//IMAGE_OUTPUT_FORMAT_GP420	//display format for video encoder/decoder
#define DIP_FORMAT			DISP_FMT_RGB565//DISP_FMT_GP420

#define MY_CLASS            &lv_gp_player_class

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct {
    lv_img_t img;
    lv_timer_t * timer;
    lv_img_dsc_t imgdsc;
    char * file_path;
    INT8S volume; // Max.0x3F
    VIDEO_ARGUMENT arg;
	MEDIA_SOURCE   src;
} lv_gp_player_t;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static void lv_gp_player_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_gp_player_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void Display_Callback(INT16U w, INT16U h, INT32U addr);

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
const lv_obj_class_t lv_gp_player_class = {
    .constructor_cb = lv_gp_player_constructor,
    .destructor_cb = lv_gp_player_destructor,
    .instance_size = sizeof(lv_gp_player_t),
    .base_class = &lv_img_class
};

static lv_gp_player_t * gpPlayer = NULL;
static INT8U isNewSrc = 0, isInit = 0, nCntPlayer = 0;

static INT16U disp_width=320, disp_height=240;

/**************************************************************************
 *                     F U N C T I O N S   D E F I N E                    *
 **************************************************************************/
static void Display_Callback(INT16U w, INT16U h, INT32U addr)
{
    //drv_l2_display_update(DISPLAY_DEVICE,(INT32U)addr);
    if(gpPlayer) {
        drv_l1_cp15_invalidate_dcache_range(addr, (w*h)<<1);
        gpPlayer->imgdsc.data = (uint8_t *)addr;
        lv_img_set_src(&gpPlayer->img.obj, &(gpPlayer->imgdsc));
        //DBG_PRINT("frame=%d\r\n", video_decode_get_current_number());
    }
    return;
}

static void lv_gp_player_constructor(const lv_obj_class_t * class_p,
                                         lv_obj_t * obj)
{
    LV_TRACE_OBJ_CREATE("begin");

    lv_gp_player_t * player = (lv_gp_player_t *)obj;

    if(isInit == 0) {
        video_decode_register_display_callback(Display_Callback);
        video_decode_entrance();
        drv_l1_dac_init(0,0);
        isInit = 1;
    }
    nCntPlayer++;

    player->file_path = NULL;
    player->volume = 0x1F;

    player->arg.bScaler = TRUE;
    player->arg.bUseDefBuf = FALSE;
    player->arg.OutputFormat = OUTPUT_FORMAT;

    player->src.type = SOURCE_TYPE_FS;
    player->src.type_ID.FileHandle = -1;
    //player->src.Format.VideoFormat = MJPEG;//only MJPEG

    #if DISPLAY_ROT_LINE_ENABLE == 1
    drv_l2_display_get_size(DISPLAY_DEVICE, &disp_height, &disp_width);
    #else
    drv_l2_display_get_size(DISPLAY_DEVICE, &disp_width, &disp_height);
    #endif

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_gp_player_destructor(const lv_obj_class_t * class_p,
                                        lv_obj_t * obj)
{
    LV_TRACE_OBJ_CREATE("begin");

    lv_gp_player_t * player = (lv_gp_player_t *)obj;

    lv_img_cache_invalidate_src(lv_img_get_src(obj));

    if(video_decode_status() == VIDEO_CODEC_PROCESSING)
        video_decode_stop();

    nCntPlayer--;
    if(nCntPlayer == 0){
        video_decode_exit();
        isInit = 0;
    }

    if (player->src.type_ID.FileHandle >= 0)
        fs_close(player->src.type_ID.FileHandle);
    player->src.type_ID.FileHandle = -1;

    gpPlayer = NULL;

    LV_TRACE_OBJ_CREATE("finished");
}

lv_obj_t * lv_gp_player_create(lv_obj_t * parent)
{
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

lv_res_t lv_gp_player_set_src(lv_obj_t * obj, const char * path)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_res_t res = LV_RES_INV;

    lv_gp_player_t * player = (lv_gp_player_t *)obj;
    VIDEO_INFO	information;
    VIDEO_CODEC_STATUS status;

    if(video_decode_status() == VIDEO_CODEC_PROCESSING) {
        video_decode_stop();
        osDelay(10);
    }

    if (player->src.type_ID.FileHandle >= 0) {
        fs_close(player->src.type_ID.FileHandle);
    }

    player->src.type_ID.FileHandle = fs_open((char *)path, O_RDONLY);
    if((player->src.type_ID.FileHandle < 0))
    {
        DBG_PRINT("file open fail\r\n");
        player->src.type_ID.FileHandle = -1;
        return res;
    }

    status = video_decode_paser_header(&information, player->arg, player->src);
    if(status != VIDEO_CODEC_STATUS_OK)
    {
        DBG_PRINT("parser header fail !!!\r\n");
        fs_close(player->src.type_ID.FileHandle);
        player->src.type_ID.FileHandle = -1;
        return res;
    }
    DBG_PRINT("Aud SampleRate = %d\r\n", information.AudSampleRate);
    DBG_PRINT("-----Video Track [0] information\r\n");
    DBG_PRINT("Vid FrameRate = %d\r\n", information.VidFrameRate);
    DBG_PRINT("Vid video_total_time = %d\r\n", information.TotalDuration);
    DBG_PRINT("resolution = %d x %d\r\n", information.Width, information.Height);
    DBG_PRINT("Total Time = %d seconds\r\n", information.TotalDuration);
    DBG_PRINT("---------------------------\r\n");

    int width = 0, height = 0;
    uint32_t data_size = 0;
    lv_style_value_t sty_val;

    // get content size setting
    res = lv_obj_get_local_style_prop(obj, LV_STYLE_WIDTH, &sty_val, 0);
    if(res == LV_RES_OK && sty_val.num < LV_SIZE_CONTENT){
        width = (sty_val.num + 0x7) & ~(0x7);   // scaler need 8 align
    }else{
        width = (information.Width + 0x7) & ~(0x7); // scaler need 8 align
    }
    if(width > disp_width){
        width = disp_width;
    }

    res = lv_obj_get_local_style_prop(obj, LV_STYLE_HEIGHT, &sty_val, 0);
    if(res == LV_RES_OK && sty_val.num < LV_SIZE_CONTENT) {
        height = sty_val.num;
    }else{
        height = information.Height;
    }
    if(height > disp_height){
        height = disp_height;
    }

    // set video decode size
    DBG_PRINT("Decode Resolution = %d x %d\r\n", width, height);
    player->arg.DisplayWidth = width;
    player->arg.DisplayBufferWidth = width;
    player->arg.DisplayHeight = height;
    player->arg.DisplayBufferHeight = height;
    data_size = player->arg.DisplayWidth * player->arg.DisplayHeight * LV_COLOR_SIZE / 8;

    player->imgdsc.header.always_zero = 0;
    player->imgdsc.header.w = player->arg.DisplayWidth;
    player->imgdsc.header.h = player->arg.DisplayHeight;
    player->imgdsc.data_size = data_size;
    player->imgdsc.header.cf = LV_IMG_CF_TRUE_COLOR;
    player->imgdsc.data = NULL;
    lv_img_set_src(&player->img.obj, &(player->imgdsc));

    isNewSrc = 1;
    player->file_path = (char *)path;

    if(gpPlayer) {
        /* clear img data */
        gpPlayer->imgdsc.data = NULL;
        lv_img_set_src(&gpPlayer->img.obj, &(gpPlayer->imgdsc));
    }
    gpPlayer = player;

    res = LV_RES_OK;
    return res;
}

lv_res_t lv_gp_player_set_volume(lv_obj_t * obj, INT8S vol)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_gp_player_t * player = (lv_gp_player_t *)obj;

    if(vol > 0x3F)
        vol = 0x3F;
    player->volume = vol;

    return LV_RES_OK;
}

void lv_gp_player_set_cmd(lv_obj_t * obj, lv_gp_player_cmd_t cmd)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_gp_player_t * player = (lv_gp_player_t *)obj;

    VIDEO_CODEC_STATUS status;
    VIDEO_INFO	information;

    if(player->file_path == NULL) {
        DBG_PRINT("player is NULL\r\n");
        return;
    }

    if(gpPlayer != player) {
        DBG_PRINT("player needs to set the source first!!!\r\n");
        return;
    }

    switch(cmd) {
        case LV_GP_PLAYER_CMD_START:
            DBG_PRINT("video_decode_start\r\n");
            if(isNewSrc)
            {
                isNewSrc = 0;
                video_decode_start(player->arg, player->src);
                audio_decode_volume(player->volume);
            }
            else if(video_decode_status() == VIDEO_CODEC_PROCESS_END)
            {
                if (player->src.type_ID.FileHandle >= 0)
                    fs_close(player->src.type_ID.FileHandle);

                player->src.type_ID.FileHandle = fs_open(player->file_path, O_RDONLY);
                if((player->src.type_ID.FileHandle < 0))
                {
                    DBG_PRINT("file open fail\r\n");
                    fs_close(player->src.type_ID.FileHandle);
                    player->src.type_ID.FileHandle = -1;
                    return;
                }

                status = video_decode_paser_header(&information, player->arg, player->src);
                if(status != VIDEO_CODEC_STATUS_OK)
                {
                    DBG_PRINT("parser header fail !!!\r\n");
                    return;
                }

                video_decode_start(player->arg, player->src);
                audio_decode_volume(player->volume);
            }
            break;

        case LV_GP_PLAYER_CMD_STOP:
            DBG_PRINT("video_decode_stop\r\n");
            if(video_decode_status() != VIDEO_CODEC_PROCESS_END)
            {
                video_decode_stop();

                if (player->src.type_ID.FileHandle >= 0)
                    fs_close(player->src.type_ID.FileHandle);
                player->src.type_ID.FileHandle = -1;
            }
            break;

        case LV_GP_PLAYER_CMD_PAUSE:
            DBG_PRINT("video_decode_pause\r\n");
            if(video_decode_status() == VIDEO_CODEC_PROCESSING)
            {
                video_decode_pause();
                DBG_PRINT("cur time = %d, num = %d\r\n", video_decode_get_current_time(), video_decode_get_current_number());
            }
            break;

        case LV_GP_PLAYER_CMD_RESUME:
            DBG_PRINT("video_decode_resume\r\n");
            if(video_decode_status() == VIDEO_CODEC_PROCESS_PAUSE)
            {
                video_decode_resume();
            }
            break;

        default:
            DBG_PRINT("Error cmd: %d\r\n", cmd);
            break;
    }
}

INT8U lv_gp_player_get_status(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_gp_player_t * player = (lv_gp_player_t *)obj;

    if(player->file_path == NULL) {
        DBG_PRINT("player is NULL\r\n");
        return VIDEO_CODEC_RESOURCE_NO_FOUND;
    }
    return video_decode_status();
}
