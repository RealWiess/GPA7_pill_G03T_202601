#include "drv_l1_scaler.h"
#include "drv_l2_scaler.h"
#include "drv_l1_pscaler.h"
#include "drv_l1_arm_cp15.h"
#include "gp_stdlib.h"
#include "gplib_mm_gplus.h"
#include "gplib_print_string.h"
#include "lvgl.h"

#if LV_USE_OPTIMIZE_GP
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define gpScale1Format_t 	    ScalerFormat_t
#define gpScale1Para_t		    ScalerPara_t

#define PSCALER_DEV             PSCALER_Y
#define SCALER_DEV              SCALER_0

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

static INT32S gp_scaler_rgb(INT32U ScaleBufIn, INT32U ScaleBufOut, INT32U in_h_size, INT32U in_v_size, INT32U out_h_size, INT32U out_v_size, INT32U buf_h_size);
static INT32S gp_scaler_alpha(INT32U ScaleBufIn, INT32U ScaleBufOut, INT32U in_h_size, INT32U in_v_size, INT32U out_h_size, INT32U out_v_size, INT32U buf_h_size);
static INT32S gp_pscaler_alpha(INT32U ScaleBufIn, INT32U ScaleBufOut, INT32U in_h_size, INT32U in_v_size, INT32U out_h_size, INT32U out_v_size, INT32U buf_h_size);
static INT32S gp_wait_scaler_done();
static INT32S gp_wait_pscaler_done();

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
gpScale1Format_t Scale1;

#if 0
void gp_zoom(struct _lv_draw_ctx_t * draw_ctx, const lv_draw_img_dsc_t * draw_dsc,
                      const lv_area_t * coords, const uint8_t * src_buf, lv_img_cf_t cf,
                      lv_draw_sw_blend_dsc_t * blend_dsc)
{
    lv_area_t blend_area, ori_area;
    lv_coord_t ori_w, ori_h, src_stride, src_height;
    /*Create buffers and masks*/
    uint32_t ori_size, buf_size;
    int16_t x, y;

    lv_area_copy(&blend_area, draw_ctx->clip_area);
    lv_coord_t blend_w = lv_area_get_width(&blend_area);
    lv_coord_t blend_h = lv_area_get_height(&blend_area);
    lv_coord_t sca_buf_w = (blend_w + 0x7) & ~(0x7);
    buf_size = sca_buf_w * blend_h;

    src_stride = lv_area_get_width(coords);
    src_height = lv_area_get_height(coords);

    uint32_t rev_zoom = 256*256/draw_dsc->zoom, pivot_256_x = (uint32_t)draw_dsc->pivot.x << 8, pivot_256_y = (uint32_t)draw_dsc->pivot.y << 8;
    ori_area.x1 = (((int32_t)(blend_area.x1 - coords->x1 - draw_dsc->pivot.x) * rev_zoom + 128 + pivot_256_x) >> 8);     /* get mapping src buffer */
    ori_area.y1 = (((int32_t)(blend_area.y1 - coords->y1 - draw_dsc->pivot.y) * rev_zoom + 128 + pivot_256_y) >> 8);
    ori_area.x2 = (((int32_t)(blend_area.x2 - coords->x1 - draw_dsc->pivot.x) * rev_zoom + 128 + pivot_256_x) >> 8);
    ori_area.y2 = (((int32_t)(blend_area.y2 - coords->y1 - draw_dsc->pivot.y) * rev_zoom + 128 + pivot_256_y) >> 8);
    ori_w = lv_area_get_width(&ori_area);
    ori_h = lv_area_get_height(&ori_area);
    ori_size = ori_w * ori_h;

    lv_color_t *rgb_buf = (lv_color_t *)gp_malloc(ori_size * sizeof(lv_color_t));
    lv_opa_t *alpha_buf = (lv_opa_t *)gp_malloc(ori_size);
    lv_color_t *scale_rgb_buf = (lv_color_t *)gp_malloc(buf_size * sizeof(lv_color_t));
    lv_opa_t *scale_alpha_buf = (lv_opa_t *)gp_malloc(buf_size);
    lv_memset(scale_alpha_buf, 0, buf_size);

    if(cf == LV_IMG_CF_TRUE_COLOR_ALPHA) {
        lv_color_t *rgb_ptr = rgb_buf;
        lv_opa_t *alpha_ptr = alpha_buf;
        uint8_t *src_tmp8;
        uint16_t src_x;
        /* Separate the image channels to RGB and Alpha to match LV_COLOR_DEPTH settings*/
        src_tmp8 = (uint8_t *)src_buf + (ori_area.y1 * src_stride + ori_area.x1) * LV_IMG_PX_SIZE_ALPHA_BYTE;
        for(y = ori_area.y1; y <= ori_area.y2; y++) {
            if(y < 0 || y >= src_height) {
                lv_memset(alpha_ptr, 0, ori_w);
                src_tmp8 += (src_stride * LV_IMG_PX_SIZE_ALPHA_BYTE);
            } else {
                for(x = 0; x < ori_w; x++) {
                    src_x = x + ori_area.x1;
                    if(src_x < 0 || src_x > src_stride) {
                        alpha_ptr[x] = 0;
                    } else {
                        alpha_ptr[x] = src_tmp8[LV_IMG_PX_SIZE_ALPHA_BYTE - 1];
                        rgb_ptr[x].full = *src_tmp8 + ((*(src_tmp8 + 1)) << 8);
                    }
                    src_tmp8 += LV_IMG_PX_SIZE_ALPHA_BYTE;
                }
                src_tmp8 += ((src_stride - ori_w) * LV_IMG_PX_SIZE_ALPHA_BYTE);
            }
            alpha_ptr += ori_w;
            rgb_ptr += ori_w;
        }
    }
    else if(cf == LV_IMG_CF_RGB565A8) {
        lv_color_t *rgb_ptr = rgb_buf;
        lv_opa_t *alpha_ptr = alpha_buf;
        uint8_t *src_tmp8, *src_a8;
        uint16_t src_x;
        //rgb_buf = (lv_color_t *)src_buf;
        //alpha_buf = (lv_opa_t *)src_buf + src_size * sizeof(lv_color_t);
        /* Separate the image channels to RGB and Alpha to match LV_COLOR_DEPTH settings*/
        src_tmp8 = (uint8_t *)src_buf + (ori_area.y1 * src_stride + ori_area.x1) * sizeof(lv_color_t);
        src_a8 = (uint8_t *)src_buf + src_stride * src_height * sizeof(lv_color_t) + (ori_area.y1 * src_stride + ori_area.x1);
        for(y = ori_area.y1; y <= ori_area.y2; y++) {
            if(y < 0 || y >= src_height) {
                lv_memset(alpha_ptr, 0, ori_w);
                src_a8 += src_stride;
                src_tmp8 += (src_stride * sizeof(lv_color_t));
            } else {
                for(x = 0; x < ori_w; x++) {
                    src_x = x + ori_area.x1;
                    if(src_x < 0 || src_x > src_stride) {
                        alpha_ptr[x] = 0;
                    } else {
                        alpha_ptr[x] = *src_a8;
                        rgb_ptr[x].full = *src_tmp8 + ((*(src_tmp8 + 1)) << 8);
                    }
                    src_tmp8 += sizeof(lv_color_t);
                    src_a8++;
                }
                src_tmp8 += ((src_stride - ori_w) * sizeof(lv_color_t));
                src_a8 += (src_stride - ori_w);
            }
            alpha_ptr += ori_w;
            rgb_ptr += ori_w;
        }
    }
    else {
        goto finish;
    }

    //DBG_PRINT("rgb_buf=0x%x, alpha_buf=0x%x, w=%d, h=%d\r\n", rgb_buf, alpha_buf, blend_w, blend_h);

    gp_scaler_rgb((INT32U)rgb_buf, (INT32U)scale_rgb_buf, ori_w, ori_h, blend_w, blend_h, sca_buf_w);
    gp_scaler_alpha((INT32U)alpha_buf, (INT32U)scale_alpha_buf, ori_w, ori_h, blend_w, blend_h, sca_buf_w);
    if(blend_w & 0x7)
        blend_area.x2 = blend_area.x1 + sca_buf_w - 1;  /* setting buffer true width*/

    blend_dsc->mask_buf = scale_alpha_buf;
    blend_dsc->mask_area = &blend_area;
    blend_dsc->mask_res = LV_DRAW_MASK_RES_CHANGED;
    blend_dsc->src_buf = scale_rgb_buf;
    blend_dsc->blend_area = &blend_area;
    lv_draw_sw_blend(draw_ctx, blend_dsc);

finish:
    gp_free(rgb_buf);
    gp_free(alpha_buf);
    gp_free(scale_rgb_buf);
    gp_free(scale_alpha_buf);
}
#else
void gp_zoom(struct _lv_draw_ctx_t * draw_ctx, const lv_draw_img_dsc_t * draw_dsc,
                      const lv_area_t * coords, const uint8_t * src_buf, lv_img_cf_t cf,
                      lv_draw_sw_blend_dsc_t * blend_dsc)
{
    lv_area_t blend_area;
    lv_coord_t blend_w, blend_h, src_stride, src_height, sca_buf_w;
    /*Create buffers and masks*/
    uint32_t ori_size, buf_size, x, y;

    src_stride = lv_area_get_width(coords);
    src_height = lv_area_get_height(coords);
    buf_size = src_stride * src_height;

    lv_point_t p_left_top = {0, 0}, p_right_bottom = {src_stride, src_height};
    lv_point_transform(&p_left_top, 0, draw_dsc->zoom, &(draw_dsc->pivot));
    lv_point_transform(&p_right_bottom, 0, draw_dsc->zoom, &(draw_dsc->pivot));

    blend_area.x1 = p_left_top.x - 2 + coords->x1;     /* get mapping src buffer */
    blend_area.y1 = p_left_top.y - 2 + coords->y1;
    blend_area.x2 = p_right_bottom.x + 2 + coords->x1;
    blend_area.y2 = p_right_bottom.y + 2 + coords->y1;
    blend_w = lv_area_get_width(&blend_area);
    blend_h = lv_area_get_height(&blend_area);

    sca_buf_w = (blend_w + 0x7) & ~(0x7);
    ori_size = sca_buf_w * blend_h;

    lv_color_t *rgb_buf;
    lv_opa_t *alpha_buf;
    lv_color_t *scale_rgb_buf = (lv_color_t *)gp_malloc(ori_size * sizeof(lv_color_t));
    lv_opa_t *scale_alpha_buf = (lv_opa_t *)gp_malloc(ori_size);
    lv_memset(scale_alpha_buf, 0, ori_size);

    if(cf == LV_IMG_CF_TRUE_COLOR_ALPHA) {
        rgb_buf = (lv_color_t *)gp_malloc(buf_size * sizeof(lv_color_t));
        alpha_buf = (lv_opa_t *)gp_malloc(buf_size);

        lv_color_t *rgb_ptr = rgb_buf;
        lv_opa_t *alpha_ptr = alpha_buf;
        uint8_t *src_tmp8 = (uint8_t *)src_buf;
        /* Separate the image channels to RGB and Alpha to match LV_COLOR_DEPTH settings*/
        for(y = 0; y < src_height; y++) {
            for(x = 0; x < src_stride; x++) {
                rgb_ptr[x].full = *src_tmp8 + ((*(src_tmp8 + 1)) << 8);
                *(alpha_ptr+x) = *(src_tmp8 + LV_IMG_PX_SIZE_ALPHA_BYTE - 1);
                src_tmp8 += LV_IMG_PX_SIZE_ALPHA_BYTE;
            }
            alpha_ptr += src_stride;
            rgb_ptr += src_stride;
        }
    }
    else if(cf == LV_IMG_CF_RGB565A8) {
        rgb_buf = (lv_color_t *)src_buf;
        alpha_buf = (lv_opa_t *)(src_buf + buf_size * sizeof(lv_color_t));
    }
    else {
        goto finish;
    }
    //DBG_PRINT("rgb_buf=0x%x, alpha_buf=0x%x, w=%d, h=%d\r\n", rgb_buf, alpha_buf, blend_w, blend_h);

    gp_scaler_rgb((INT32U)rgb_buf, (INT32U)scale_rgb_buf, src_stride, src_height, blend_w, blend_h, sca_buf_w);
    if(src_stride & 0x3) {
        gp_wait_scaler_done();
        gp_scaler_alpha((INT32U)alpha_buf, (INT32U)scale_alpha_buf, src_stride, src_height, blend_w, blend_h, sca_buf_w);
    } else {
        gp_pscaler_alpha((INT32U)alpha_buf, (INT32U)scale_alpha_buf, src_stride, src_height, blend_w, blend_h, sca_buf_w);
        gp_wait_pscaler_done();
    }
    gp_wait_scaler_done();

    if(blend_w & 0x7)
        blend_area.x2 = blend_area.x1 + sca_buf_w - 1;  /* setting buffer true width*/

    blend_dsc->mask_buf = scale_alpha_buf;
    blend_dsc->mask_area = &blend_area;
    blend_dsc->mask_res = LV_DRAW_MASK_RES_CHANGED;
    blend_dsc->src_buf = scale_rgb_buf;
    blend_dsc->blend_area = &blend_area;
    lv_draw_sw_blend(draw_ctx, blend_dsc);

finish:
    if(cf == LV_IMG_CF_TRUE_COLOR_ALPHA) {
        gp_free(rgb_buf);
        gp_free(alpha_buf);
    }
    gp_free(scale_rgb_buf);
    gp_free(scale_alpha_buf);
}
#endif

static INT32S gp_wait_scaler_done()
{
    INT32S ret;

    ret = drv_l2_scaler_wait_done(SCALER_DEV, &Scale1);
    if(ret == C_SCALER_STATUS_DONE || ret == C_SCALER_STATUS_STOP)
	{
		drv_l2_scaler_stop(SCALER_DEV);
	}
	else
	{
		DBG_PRINT("Scale1 RGB Fail\r\n");
		while(1);
	}

    return 0;
}

static INT32S gp_wait_pscaler_done()
{
    INT32S ret;
    INT8U flag_frame_done=0, flag_ahb_done=0;

    //t_start = xTaskGetTickCount();
    do{
        ret = drv_l1_pscaler_status_get(PSCALER_DEV);
        //DBG_PRINT("ret=0x%x\r\n", ret);
        if(ret & PIPELINE_SCALER_STATUS_FRAME_DONE)
            flag_frame_done = 1;
        if(ret & PIPELINE_SCALER_STATUS_AHB_DONE)
            flag_ahb_done = 1;

        if(flag_frame_done && flag_ahb_done)
        {
            flag_frame_done = 0;
            flag_ahb_done = 0;
            break;
        }
        //osDelay(1);
    }while(1);
    //t_end = xTaskGetTickCount();
    //DBG_PRINT("t:%d\r\n", t_end-t_start);
    drv_l1_pscaler_stop(PSCALER_DEV);

    return 0;
}

static INT32S gp_scaler_rgb(INT32U ScaleBufIn, INT32U ScaleBufOut, INT32U in_h_size, INT32U in_v_size, INT32U out_h_size, INT32U out_v_size, INT32U buf_h_size)
{
	INT32S ret;
	gpScale1Para_t Matrix;

	//DBG_PRINT("\r\n<< %s >>\r\n", __func__);
	lv_memset((void *)&Scale1, 0x00, sizeof(Scale1));
	lv_memset((void *)&Matrix, 0x00, sizeof(Matrix));

	// start test
	Scale1.input_format = C_SCALER_CTRL_IN_RGB565;
    Scale1.input_width = in_h_size;
    Scale1.input_height = in_v_size;
    Scale1.input_visible_width = in_h_size;
    Scale1.input_visible_height = in_v_size;
	Scale1.input_x_offset = 0;
	Scale1.input_y_offset = 0;
	Scale1.input_y_addr = (INT32U)ScaleBufIn;
	Scale1.input_u_addr = 0;
	Scale1.input_v_addr = 0;

    Scale1.output_format = C_SCALER_CTRL_OUT_RGB565;
	Scale1.output_width = out_h_size;
	Scale1.output_height = out_v_size;
	Scale1.output_buf_width = buf_h_size;
	Scale1.output_buf_height = out_v_size;
	Scale1.output_x_offset = 0;

	Scale1.output_y_addr = ScaleBufOut;
	Scale1.output_u_addr = 0;
	Scale1.output_v_addr = 0;
	Scale1.fifo_mode = C_SCALER_CTRL_FIFO_DISABLE;
	Scale1.scale_mode = C_SCALER_RATIO_USER; //C_SCALER_FULL_SCREEN;
	Scale1.digizoom_m = 10;
	Scale1.digizoom_n = 10;
	Scale1.force_intp_en = 1;

	Matrix.boundary_mode = 1;
	Matrix.boundary_color = 0x8080;
	Matrix.gamma_en = 0;
	Matrix.color_matrix_en = 0;
	Matrix.yuv_type = 0;
    //Matrix.scaler_intmode = 3;

	#if 1
	ret = drv_l2_scaler_trigger(SCALER_DEV, 0, &Scale1, &Matrix);
	#else
	ret = drv_l2_scaler_trigger(SCALER_DEV, 1, &Scale1, &Matrix);
	if(ret == C_SCALER_STATUS_DONE || ret == C_SCALER_STATUS_STOP)
	{
		drv_l2_scaler_stop(SCALER_DEV);
	}
	else
	{
		DBG_PRINT("Scale1 RGB Fail\r\n");
		while(1);
	}
	#endif

	return ret;
}

static INT32S gp_scaler_alpha(INT32U ScaleBufIn, INT32U ScaleBufOut, INT32U in_h_size, INT32U in_v_size, INT32U out_h_size, INT32U out_v_size, INT32U buf_h_size)
{
	INT32S ret;
	gpScale1Format_t Scale1;
	gpScale1Para_t Matrix;

	//DBG_PRINT("\r\n<< %s >>\r\n", __func__);
	gp_memset((void *)&Scale1, 0x00, sizeof(Scale1));
	gp_memset((void *)&Matrix, 0x00, sizeof(Matrix));

	// start test
	Scale1.input_format = C_SCALER_CTRL_IN_Y_ONLY;
    Scale1.input_width = in_h_size;
    Scale1.input_height = in_v_size;
    Scale1.input_visible_width = in_h_size;
    Scale1.input_visible_height = in_v_size;
	Scale1.input_x_offset = 0;
	Scale1.input_y_offset = 0;
	Scale1.input_y_addr = (INT32U)ScaleBufIn;
	Scale1.input_u_addr = 0;
	Scale1.input_v_addr = 0;

    Scale1.output_format = C_SCALER_CTRL_OUT_Y_ONLY;
	Scale1.output_width = out_h_size;
	Scale1.output_height = out_v_size;
	Scale1.output_buf_width = buf_h_size;
	Scale1.output_buf_height = out_v_size;
	Scale1.output_x_offset = 0;

	Scale1.output_y_addr = ScaleBufOut;
	Scale1.output_u_addr = 0;
	Scale1.output_v_addr = 0;
	Scale1.fifo_mode = C_SCALER_CTRL_FIFO_DISABLE;
	Scale1.scale_mode = C_SCALER_RATIO_USER; //C_SCALER_FULL_SCREEN;;
	Scale1.digizoom_m = 10;
	Scale1.digizoom_n = 10;
	Scale1.force_intp_en = 1;

	Matrix.boundary_mode = 1;
	Matrix.boundary_color = 0x8080;
	Matrix.gamma_en = 0;
	Matrix.color_matrix_en = 0;
	Matrix.yuv_type = 0;
	//Matrix.scaler_intmode = 3;

	ret = drv_l2_scaler_trigger(SCALER_DEV, 1, &Scale1, &Matrix);
	if(ret == C_SCALER_STATUS_DONE || ret == C_SCALER_STATUS_STOP)
	{
		drv_l2_scaler_stop(SCALER_DEV);
	}
	else
	{
		DBG_PRINT("Scale1 Alpha Fail\r\n");
		while(1);
	}

	return ret;
}

static INT32S gp_pscaler_alpha(INT32U ScaleBufIn, INT32U ScaleBufOut, INT32U in_h_size, INT32U in_v_size, INT32U out_h_size, INT32U out_v_size, INT32U buf_h_size)
{
    INT32U widthFactor=65536,heightFactor=65536;

    widthFactor = ((in_h_size*65536)/out_h_size);
	heightFactor = ((in_v_size*65536)/out_v_size);

    //DBG_PRINT("area(%d,%d,%d,%d) out_buf=0x%x\r\n", n_x1, area->y1, n_width, n_height, (INT32U)(ScaleBufOut+area->y1*s_width*2+n_x1*2));
    drv_l1_pscaler_clk_ctrl(PSCALER_DEV, ENABLE);
    drv_l1_pscaler_init(PSCALER_DEV);
    //drv_l1_pscaler_callback_register(PSCALER_DEV, psca_callback);
    drv_l1_pscaler_input_format_set(PSCALER_DEV, PIPELINE_SCALER_INPUT_FORMAT_Y);
    drv_l1_pscaler_output_format_set(PSCALER_DEV, PIPELINE_SCALER_OUTPUT_FORMAT_YONLY);
    drv_l1_pscaler_input_source_set(PSCALER_DEV, PIPELINE_SCALER_INPUT_SOURCE_DRAM);
    drv_l1_pscaler_input_X_start_set(PSCALER_DEV, 0);
    drv_l1_pscaler_input_Y_start_set(PSCALER_DEV, 0);
    drv_l1_pscaler_input_pixels_set(PSCALER_DEV, in_h_size, in_v_size);
    drv_l1_pscaler_output_B_buffer_set(PSCALER_DEV, (INT32U)DUMMY_BUFFER_ADDRS);
    drv_l1_pscaler_interrupt_set(PSCALER_DEV, PIPELINE_SCALER_INT_ENABLE_AHB_READ_DONE | PIPELINE_SCALER_INT_ENABLE_FRAME_END);
    //drv_l1_pscaler_integration_mode_set(PSCALER_DEV, 1, 1);

    drv_l1_pscaler_output_pixels_set(PSCALER_DEV, widthFactor, buf_h_size, heightFactor, out_v_size);
    drv_l1_pscaler_output_pixels_X_offset_set(PSCALER_DEV, 0);
    drv_l1_pscaler_output_fifo_line_set(PSCALER_DEV, out_v_size, 0);

    drv_l1_pscaler_input_buffer_set(PSCALER_DEV, ScaleBufIn);
    drv_l1_pscaler_output_A_buffer_set(PSCALER_DEV, ScaleBufOut);

    drv_l1_pscaler_start(PSCALER_DEV);
    #if 0
    INT8U flag_frame_done=0, flag_ahb_done=0;
    INT32S ret = 0;
    //t_start = xTaskGetTickCount();
    do{
        ret = drv_l1_pscaler_status_get(PSCALER_DEV);
        //DBG_PRINT("ret=0x%x\r\n", ret);
        if(ret & PIPELINE_SCALER_STATUS_FRAME_DONE)
            flag_frame_done = 1;
        if(ret & PIPELINE_SCALER_STATUS_AHB_DONE)
            flag_ahb_done = 1;

        if(flag_frame_done && flag_ahb_done)
        {
            flag_frame_done = 0;
            flag_ahb_done = 0;
            break;
        }
        //osDelay(1);
    }while(1);
    //t_end = xTaskGetTickCount();
    //DBG_PRINT("t:%d\r\n", t_end-t_start);
    drv_l1_pscaler_stop(PSCALER_DEV);
    #endif

    return 0;
}

#endif //LV_USE_OPTIMIZE_GP
