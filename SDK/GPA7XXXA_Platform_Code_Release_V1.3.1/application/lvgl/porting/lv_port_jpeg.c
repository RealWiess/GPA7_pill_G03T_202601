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
#include "gp_stdlib.h"
#include "drv_l1_arm_cp15.h"
#include "drv_l1_jpeg.h"
#include "drv_l2_jpeg_api.h"
#include "drv_l2_jpeg_decode.h"
#include "drv_l1_scaler.h"
#include "drv_l2_scaler.h"
#include "fs.h"
#include "lv_port_jpeg.h"

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#if LV_COLOR_DEPTH == 32
    #define SCALER_OUT_FORMAT          C_SCALER_CTRL_OUT_ARGB8888
#else
    #define SCALER_OUT_FORMAT          C_SCALER_CTRL_OUT_RGB565
#endif
#define HWIDX	JPEGA	//JPEGA, JPEGB

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static INT32S convert_to_display(INT32U outBuf, INT32U outWidth, INT32U outHeight, JpegDec_t *pJpegDec);

static lv_res_t gp_jpeg_decoder_info(struct _lv_img_decoder_t * decoder, const void * src, lv_img_header_t * header);
static lv_res_t gp_jpeg_decoder_open(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc);
static void gp_jpeg_decoder_close(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc);

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

/**************************************************************************
 *                     F U N C T I O N S   D E F I N E                    *
 **************************************************************************/
/**
 * Register the JPEG decoder functions in LVGL
 */
void lv_gp_jpeg_init(void)
{
    lv_img_decoder_t * dec = lv_img_decoder_create();
    lv_img_decoder_set_info_cb(dec, gp_jpeg_decoder_info);
    lv_img_decoder_set_open_cb(dec, gp_jpeg_decoder_open);
    lv_img_decoder_set_close_cb(dec, gp_jpeg_decoder_close);
}

/**
 * Get info about a JPEG image
 * @param src is file name
 * @param header store the info here
 * @return LV_RES_OK: no error; LV_RES_INV: can't get the info
 */
static lv_res_t gp_jpeg_decoder_info(struct _lv_img_decoder_t * decoder, const void * src, lv_img_header_t * header)
{
    (void) decoder; /*Unused*/
    lv_img_src_t src_type = lv_img_src_get_type(src);          /*Get the source type*/

    /*If it's a JPEG file...*/
    if(src_type == LV_IMG_SRC_FILE) {
        const char * fn = src;
        if((strcmp(lv_fs_get_ext(fn), "jpg") == 0) || (strcmp(lv_fs_get_ext(fn), "JPG") == 0)) {              /*Check the extension*/
            /* Parser header to get the width and height from the file. */
            INT16S fileHandle;
            INT32S ret;
            struct sfn_info file_info;
            INT32U fileSize;
            INT8U *fileBuf;
            INT16U jpeg_yuv_mode, jpeg_width, jpeg_height;

            fileHandle = fs_open((char *)fn, O_RDONLY);
            if(fileHandle < 0){
                DBG_PRINT("Open file fail !\r\n");
                return LV_RES_INV;
            }
            ret = sfn_stat((INT16S)fileHandle, &file_info);
            if(ret != 0){
                DBG_PRINT("Get file info fail %d!\r\n", ret);
                fs_close(fileHandle);
                return LV_RES_INV;
            }
            fileSize = file_info.f_size;
            fs_lseek(fileHandle, 0, SEEK_SET);
            //DBG_PRINT("file size = %d\r\n", fileSize);

            fileBuf = (INT8U *)gp_malloc_align(fileSize, 64);
            if(!fileBuf) {
                DBG_PRINT("file alloc memory fail !\r\n");
                fs_close(fileHandle);
                return LV_RES_INV;
            }
            ret = fs_read((long)fileHandle, (INT32U)fileBuf, fileSize);
            if(ret != fileSize) {
                DBG_PRINT("Read file failed %d!\r\n", ret);
                gp_free(fileBuf);
                fs_close(fileHandle);
                return LV_RES_INV;
            }

            jpeg_decode_init(HWIDX);
            ret = jpeg_api_decode_get_size(HWIDX, (INT32U)fileBuf, fileSize, &jpeg_width, &jpeg_height, &jpeg_yuv_mode);
            if(ret) {
                DBG_PRINT("Parse header fail %d!\r\n", ret);
                gp_free(fileBuf);
                fs_close(fileHandle);
                return LV_RES_INV;
            }

            DBG_PRINT("jpeg_yuv_mode=%d ", jpeg_yuv_mode);
            DBG_PRINT("jpeg_valid=%dx%d\r\n", jpeg_width, jpeg_height);
            //DBG_PRINT("jpeg_extend=%dx%d\r\n", decode.jpeg_extend_w, decode.jpeg_extend_h);

            /*Save the data in the header*/
            header->always_zero = 0;
            header->cf = LV_IMG_CF_TRUE_COLOR;
            header->w = (lv_coord_t)jpeg_width;
            header->h = (lv_coord_t)jpeg_height;

            gp_free(fileBuf);
            fs_close(fileHandle);
            return LV_RES_OK;
        }
    }

    return LV_RES_INV;         /*If didn't succeeded earlier then it's an error*/
}

/**
 * Open a JPEG image and return the decided image, refrence jpeg_api_decode_output(JpegDec_t *pJpegDec)
 * @param src is file name
 * @param style style of the image object (unused now but certain formats might use it)
 * @return pointer to the decoded image or `LV_IMG_DECODER_OPEN_FAIL` if failed
 */
static lv_res_t gp_jpeg_decoder_open(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc)
{

    (void) decoder; /*Unused*/

    /*If it's a JPEG file...*/
    if(dsc->src_type == LV_IMG_SRC_FILE) {
        const char * fn = dsc->src;
        if((strcmp(lv_fs_get_ext(fn), "jpg") == 0) || (strcmp(lv_fs_get_ext(fn), "JPG") == 0)) {              /*Check the extension*/
            INT16S fileHandle;
            INT32S ret;
            struct sfn_info file_info;
            INT32U fileSize;
            INT8U *fileBuf, *decodeBuf, *rgbBuf;
            JpegDec_t decode;

            /*Load the JPEG file into buffer. It's still compressed (not decoded)*/
            fileHandle = fs_open((char *)fn, O_RDONLY);
            if(fileHandle < 0){
                DBG_PRINT("Open file fail !\r\n");
                return LV_RES_INV;
            }
            ret = sfn_stat((INT16S)fileHandle, &file_info);
            if(ret != 0){
                DBG_PRINT("Get file info fail %d!\r\n", ret);
                fs_close(fileHandle);
                return LV_RES_INV;
            }
            fileSize = file_info.f_size;
            fs_lseek(fileHandle, 0, SEEK_SET);
            //DBG_PRINT("file size = %d\r\n", fileSize);

            fileBuf = gp_malloc_align(fileSize, 64);
            if(!fileBuf) {
                DBG_PRINT("file alloc memory fail !\r\n");
                fs_close(fileHandle);
                return LV_RES_INV;
            }
            ret = fs_read((long)fileHandle, (INT32U)fileBuf, fileSize);
            if(ret != fileSize) {
                DBG_PRINT("Read file fail %d!\r\n", ret);
                gp_free(fileBuf);
                fs_close(fileHandle);
                return LV_RES_INV;
            }

            memset((void *)&decode, 0, sizeof(JpegDec_t));
            decode.hwidx = HWIDX;
            decode.raw_data_addr = (INT32U)fileBuf;
            decode.raw_data_size = fileSize;

            jpeg_decode_init(decode.hwidx);
            ret = jpeg_api_decode_get_size(HWIDX, (INT32U)fileBuf, fileSize, (INT16U *)&(decode.jpeg_valid_w), (INT16U *)&(decode.jpeg_valid_h), (INT16U *)&(decode.jpeg_yuv_mode));
            if(ret) {
                DBG_PRINT("Parse header fail %d!\r\n", ret);
                gp_free(fileBuf);
                fs_close(fileHandle);
                return LV_RES_INV;
            }
            decodeBuf = gp_malloc_align(decode.jpeg_valid_w * decode.jpeg_valid_h * 2, 64);
            if(!decodeBuf) {
                DBG_PRINT("decode alloc memory fail !\r\n");
                gp_free(fileBuf);
                fs_close(fileHandle);
                return LV_RES_INV;
            }

            /*Decode the loaded image in YUYV */
            decode.output_addr = (INT32U)decodeBuf;
            decode.output_format = C_SCALER_CTRL_OUT_YUYV; //C_SCALER_CTRL_OUT_GP420 for GP420
            decode.output_boundary_color = 0x008080;
            ret = jpeg_api_decode_lvgl(&decode);
            if(ret) {
                DBG_PRINT("Jpeg decode fail %d!\r\n", ret);
                gp_free(decodeBuf);
                gp_free(fileBuf);
                fs_close(fileHandle);
                return LV_RES_INV;
            }

            gp_free(fileBuf);
            fs_close(fileHandle);

            /*Convert the image to display format*/
            decode.output_w = decode.jpeg_valid_w;
            decode.output_h = decode.jpeg_valid_h;
            rgbBuf = gp_malloc_align(decode.output_w * decode.output_h * LV_COLOR_DEPTH / 8, 64);
            if(!rgbBuf) {
                DBG_PRINT("scale alloc memory fail !\r\n");
                gp_free(decodeBuf);
                return LV_RES_INV;
            }
            ret = convert_to_display((INT32U)rgbBuf, decode.output_w, decode.output_h, &decode);
            if(ret) {
                DBG_PRINT("scale fail %d!\r\n", ret);
                gp_free(decodeBuf);
                gp_free(rgbBuf);
                return LV_RES_INV;
            }

            dsc->img_data = rgbBuf;
            gp_free(decodeBuf);
            return LV_RES_OK;     /*The image is fully decoded. Return with its pointer*/
        }
    }

    return LV_RES_INV;    /*If not returned earlier then it failed*/
}

/**
 * Free the allocated resources
 */
static void gp_jpeg_decoder_close(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc)
{
    LV_UNUSED(decoder); /*Unused*/
    if(dsc->img_data) {
        gp_free((uint8_t *)dsc->img_data);
        dsc->img_data = NULL;
    }
}

/**************************************************************************
 *                         D E C O D E			                          *
 **************************************************************************/
static INT32S convert_to_display(INT32U outBuf, INT32U outWidth, INT32U outHeight, JpegDec_t *pJpegDec)
{
    INT32U* ss;
	INT32U* dd;
	INT32U A,Y,U,V, x, y, R,G,B;

	INT32S ret;
	ScalerFormat_t ScaleDisp;
	ScalerPara_t para;
    INT32U inFormat = C_SCALER_CTRL_IN_YUYV;

    switch(pJpegDec->output_format)
	{
	case C_SCALER_CTRL_OUT_ARGB8888:
		inFormat = C_SCALER_CTRL_IN_ARGB8888;
		ss = (INT32U*)pJpegDec->output_addr;
		dd = (INT32U*)outBuf;
		for (y = 0; y < outHeight; y++)
		{
			for (x = 0; x < outWidth; x++)
			{
				A = (*ss) >> 24;
				R = ((*ss) >> 16) & 0xFF;
				R = (R*A >> 8) & 0xFF;
				G = ((*ss) >> 8) & 0xFF;
				G = (G*A >> 8) & 0xFF;
				B = (*ss) & 0xFF;
				B = (B*A >> 8) & 0xFF;
				(*ss) = (A << 24) | (R << 16) | (G << 8) | B;
				ss++;
			}
		}
		drv_l1_cp15_clean_dcache_range(pJpegDec->output_addr, outWidth*outHeight*4);
		break;
	case C_SCALER_CTRL_OUT_YUV444: //AYUV
		inFormat = C_SCALER_CTRL_IN_YUYV;
		ss = (INT32U*)pJpegDec->output_addr;
		dd = (INT32U*)outBuf;
		for (y = 0; y < outHeight; y++)
		{
			for (x = 0; x < outWidth; x++)
			{
				A = (*ss) >> 24;
				Y = ((*ss) >> 16) & 0xFF;
				Y = (Y*A >> 8) & 0xFF;
				U = ((*ss) >> 8) & 0xFF;
				V = (*ss) & 0xFF;
				if (x & 0x1)
					*dd++ |= (Y << 24) | (U << 16);
				else
					*dd = (Y << 8) | V;
				ss++;
			}
		}
		drv_l1_cp15_clean_dcache_range(outBuf, outWidth*outHeight*2);
		return STATUS_OK;
	case C_SCALER_CTRL_OUT_GP420:
		inFormat = C_SCALER_CTRL_IN_GP420;
		break;
	case C_SCALER_CTRL_OUT_YUYV:
		inFormat = C_SCALER_CTRL_IN_YUYV;
		break;
	}

	memset((void *)&ScaleDisp, 0x00, sizeof(ScaleDisp));
	ScaleDisp.input_format = inFormat;
	ScaleDisp.input_width = pJpegDec->jpeg_valid_w;
	ScaleDisp.input_height = pJpegDec->jpeg_valid_h;
	ScaleDisp.input_visible_width = 0;
	ScaleDisp.input_visible_height = 0;
	ScaleDisp.input_x_offset = 0;
	ScaleDisp.input_y_offset = 0;

	ScaleDisp.input_y_addr = (INT32U)pJpegDec->output_addr;
	ScaleDisp.input_u_addr = 0;
	ScaleDisp.input_v_addr = 0;


	ScaleDisp.output_format = SCALER_OUT_FORMAT;
	ScaleDisp.output_width = outWidth;
	ScaleDisp.output_height = outHeight;
	ScaleDisp.output_buf_width = outWidth;
	ScaleDisp.output_buf_height = outHeight;
	ScaleDisp.output_x_offset = 0;

	ScaleDisp.output_y_addr = (INT32U)outBuf;
	ScaleDisp.output_u_addr = 0;
	ScaleDisp.output_v_addr = 0;

	ScaleDisp.fifo_mode = C_SCALER_CTRL_FIFO_DISABLE;
	ScaleDisp.scale_mode = C_SCALER_BY_RATIO;
	ScaleDisp.digizoom_m = 10;
	ScaleDisp.digizoom_n = 10;

	memset((void *)&para, 0x00, sizeof(para));
	para.boundary_color = 0x008080;

	ret = drv_l2_scaler_trigger(SCALER_0, 1, &ScaleDisp, &para);
	if(ret != C_SCALER_STATUS_DONE && ret != C_SCALER_STATUS_STOP) {
		DBG_PRINT("Scale1 Fail\r\n");
		ret = STATUS_FAIL;
	}else{
        ret = STATUS_OK;
	}

	drv_l2_scaler_stop(SCALER_0);
	return ret;
}
