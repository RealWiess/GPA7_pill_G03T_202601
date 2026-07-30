#include <stdio.h>
#include "audio_decoder.h"
#include "drv_l1_aud_compressor.h"
#include "drv_l1_dac.h"
#include "multimedia_platform.h"
#include "lv_port_audio.h"
#include "lv_port_fs.h"

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct {
    char* file_path;
    AUDIO_ARGUMENT arg;
	MEDIA_SOURCE   src;
} lv_gp_audio_t;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static lv_gp_audio_t *gpPlayer = NULL;
static INT8U isNewSrc = 0;

lv_res_t lv_gp_audio_init(void)
{
    drv_l1_audio_compressor_init(1);
    drv_l1_dac_init(0,0);
    platform_entrance(0);

    gpPlayer = (lv_gp_audio_t *)lv_mem_alloc(sizeof(lv_gp_audio_t));
    if(gpPlayer == NULL) {
        DBG_PRINT("audio init fail!\r\n");
        platform_exit();
        return LV_RES_INV;
    }
    lv_memset(gpPlayer, 0, sizeof(lv_gp_audio_t));
    gpPlayer->arg.volume = 31;			// volume level = 0~63
    gpPlayer->src.type_ID.FileHandle = -1;

    audio_decode_entrance();	// Initialize and allocate the resources needed by audio decoder
    return LV_RES_OK;
}

void lv_gp_audio_uninit(void)
{
    if(gpPlayer == NULL)
        return;

    audio_decode_stop(gpPlayer->arg);
	audio_decode_exit();
	fs_close(gpPlayer->src.type_ID.FileHandle);
    platform_exit();
    lv_mem_free(gpPlayer);
}

lv_res_t lv_gp_set_audio_format(const char * file_path)
{
    lv_res_t res = LV_RES_OK;
    AUDIO_FORMAT audio_format;
    size_t offset = strlen(file_path) - 3;
    char *ptr_path = (char *)file_path + offset;

    if(strcmp(ptr_path, "mp3") || strcmp(ptr_path, "MP3")) {
        audio_format = MP3;
    }else if(strcmp(ptr_path, "wav") || strcmp(ptr_path, "WAV")) {
        audio_format = WAV;
    }else{
        audio_format = AUD_AUTOFORMAT;
    }

    switch(audio_format)
	{
		case MP3:
			gpPlayer->arg.Main_Channel = 1;
			gpPlayer->src.Format.AudioFormat = MP3;
			break;
        case WAV:
            gpPlayer->arg.Main_Channel = 1;
			gpPlayer->src.Format.AudioFormat = WAV;
			gpPlayer->src.type_ID.memptr = 0;
			break;
		default:
            gpPlayer->src.Format.AudioFormat = AUD_AUTOFORMAT;
            res = LV_RES_INV;
			break;
	}
	return res;
}

lv_res_t lv_gp_audio_set_src(const char * file_path)
{
    lv_res_t res = LV_RES_OK;

    if(!gpPlayer) {
        DBG_PRINT("audio player is NULL\r\n");
        return LV_RES_INV;
    }

    if(audio_decode_status(gpPlayer->arg) != AUDIO_CODEC_PROCESS_END)
        audio_decode_stop(gpPlayer->arg);

    if (gpPlayer->src.type_ID.FileHandle >= 0)
        fs_close(gpPlayer->src.type_ID.FileHandle);

    gpPlayer->src.type_ID.FileHandle = fs_open((char *)file_path, O_RDONLY);
    if(gpPlayer->src.type_ID.FileHandle < 0) {
        DBG_PRINT("open file:%s%s fail\r\n", FS_ROOT_PATH, file_path);
        gpPlayer->src.type_ID.FileHandle = -1;
        return LV_RES_INV;
    }

    res = lv_gp_set_audio_format(file_path);
    if(res == LV_RES_INV) {
        DBG_PRINT("file:%s%s format error\r\n", FS_ROOT_PATH, file_path);
        return res;
    }

    gpPlayer->src.type = SOURCE_TYPE_FS;
    gpPlayer->src.audio_config.AudioCompressor_en = DISABLE;

    gpPlayer->arg.L_R_Channel = 3;		// Left + Right Channel
    gpPlayer->arg.mute = 0;
    gpPlayer->arg.midi_index = 0;
    gpPlayer->arg.data_offset = 0;

    isNewSrc = 1;
    gpPlayer->file_path = (char *)file_path;
    return res;
}

lv_res_t lv_gp_audio_set_cmd(lv_gp_audio_cmd_t cmd)
{
    CODEC_START_STATUS status;
    if(!gpPlayer) {
        DBG_PRINT("audio player is NULL\r\n");
        return LV_RES_INV;
    }

    switch(cmd) {
        case LV_GP_AUDIO_CMD_START:
            DBG_PRINT("audio_decode_start\r\n");
            if(isNewSrc) {
                isNewSrc = 0;
                status = audio_decode_start(gpPlayer->arg, gpPlayer->src);
                if(status != START_OK) {
                    DBG_PRINT("audio start fail !!!\r\n");
                    return LV_RES_INV;
                }
            } else if(audio_decode_status(gpPlayer->arg) == AUDIO_CODEC_PROCESS_END) {
                if (gpPlayer->src.type_ID.FileHandle >= 0)
                    fs_close(gpPlayer->src.type_ID.FileHandle);

                gpPlayer->src.type_ID.FileHandle = fs_open(gpPlayer->file_path, O_RDONLY);
                if((gpPlayer->src.type_ID.FileHandle < 0))
                {
                    DBG_PRINT("file open fail\r\n");
                    fs_close(gpPlayer->src.type_ID.FileHandle);
                    gpPlayer->src.type_ID.FileHandle = -1;
                    return LV_RES_INV;
                }

                status = audio_decode_start(gpPlayer->arg, gpPlayer->src);
                if(status != START_OK) {
                    DBG_PRINT("audio start fail !!!\r\n");
                    return LV_RES_INV;
                }
            }
            break;

        case LV_GP_AUDIO_CMD_STOP:
            DBG_PRINT("audio_decode_stop\r\n");
			if(audio_decode_status(gpPlayer->arg) != AUDIO_CODEC_PROCESS_END) {
				audio_decode_stop(gpPlayer->arg);
			}
			if (gpPlayer->src.type_ID.FileHandle >= 0) {
                fs_close(gpPlayer->src.type_ID.FileHandle);
            }
            gpPlayer->src.type_ID.FileHandle = -1;
            break;

        case LV_GP_AUDIO_CMD_PAUSE:
            DBG_PRINT("audio_decode_pause\r\n");
			if(audio_decode_status(gpPlayer->arg) == AUDIO_CODEC_PROCESSING) {
				audio_decode_pause(gpPlayer->arg);
			}
            break;

        case LV_GP_AUDIO_CMD_RESUME:
            DBG_PRINT("audio_decode_resume\r\n");
            if(audio_decode_status(gpPlayer->arg) == AUDIO_CODEC_PROCESS_PAUSED) {
				audio_decode_resume(gpPlayer->arg);
			}
            break;

        default:
            DBG_PRINT("Error cmd: %d\r\n", cmd);
            break;
    }

    return LV_RES_OK;
}

lv_res_t lv_gp_audio_set_volume(INT8U volume)
{
    if(!gpPlayer) {
        DBG_PRINT("audio player is NULL\r\n");
        return LV_RES_INV;
    }

    // volume level = 0~63
    if(volume > 63)
        volume = 63;

    gpPlayer->arg.volume = volume;
    drv_l1_dac_pga_set(gpPlayer->arg.volume);

    return LV_RES_OK;
}

AUDIO_CODEC_STATUS lv_gp_audio_get_status()
{
    if(!gpPlayer) {
        DBG_PRINT("audio player is NULL\r\n");
        return AUDIO_CODEC_BREAK_OFF;
    }

    return audio_decode_status(gpPlayer->arg);
}
