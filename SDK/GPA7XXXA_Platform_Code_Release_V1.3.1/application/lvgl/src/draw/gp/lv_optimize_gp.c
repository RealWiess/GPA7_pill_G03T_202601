#include "lv_optimize_gp.h"
#include "lv_transform_gp.h"
#include "gplib_mm_gplus.h"
#include "gplib_print_string.h"

#if LV_USE_OPTIMIZE_GP
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static void lv_draw_gp_img_decoded(lv_draw_ctx_t * draw_ctx, const lv_draw_img_dsc_t * dsc,
                                    const lv_area_t * coords, const uint8_t * map_p, lv_img_cf_t cf);

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

void lv_draw_gp_ctx_init(lv_disp_drv_t * drv, lv_draw_ctx_t * draw_ctx)
{
    lv_draw_sw_init_ctx(drv, draw_ctx);

    lv_draw_gp_ctx_t * gp_draw_ctx = (lv_draw_sw_ctx_t *)draw_ctx;

#if LV_COLOR_DEPTH == 16
    gp_draw_ctx->base_draw.draw_img_decoded = lv_draw_gp_img_decoded;
    //gp_draw_ctx->base_draw.draw_img_decoded = lv_draw_sw_img_decoded;
#endif
    gp_draw_ctx->blend = lv_draw_sw_blend_basic;
}

void lv_draw_gp_ctx_deinit(lv_disp_drv_t * drv, lv_draw_ctx_t * draw_ctx)
{
    lv_draw_sw_deinit_ctx(drv, draw_ctx);
}

/**************************************************************************
 *                   S T A T I C    F U N C T I O N S                     *
 **************************************************************************/

static void lv_draw_gp_img_decoded(struct _lv_draw_ctx_t * draw_ctx, const lv_draw_img_dsc_t * draw_dsc,
                                                  const lv_area_t * coords, const uint8_t * src_buf, lv_img_cf_t cf)
{
    bool only_zoom = draw_dsc->zoom != LV_IMG_ZOOM_NONE && draw_dsc->angle == 0 ? true : false;

    if(!only_zoom || draw_dsc->recolor_opa != LV_OPA_TRANSP
        || lv_draw_mask_is_any(draw_ctx->clip_area)
        || (cf != LV_IMG_CF_RGB565A8 && cf != LV_IMG_CF_TRUE_COLOR_ALPHA) )
    {
        lv_draw_sw_img_decoded(draw_ctx, draw_dsc, coords, src_buf, cf);
        return;
    }

    lv_area_t blend_area;
    lv_draw_sw_blend_dsc_t blend_dsc;

    lv_memset_00(&blend_dsc, sizeof(lv_draw_sw_blend_dsc_t));
    blend_dsc.opa = draw_dsc->opa;
    blend_dsc.blend_mode = draw_dsc->blend_mode;
    blend_dsc.blend_area = &blend_area;

    #if 0
        uint8_t flag = 0;
        TickType_t t_start, t_end;
        //if(lv_area_get_width(draw_ctx->clip_area) > 250) || (coords->x1 == 189 && coords->y1 == 87)) {
            //DBG_PRINT("dest:0x%x, area[%d,%d,%d,%d], ", (uint32_t)(draw_ctx->buf), coords->x1, coords->y1, coords->x2, coords->y2);
            DBG_PRINT("dest:0x%x, area[%d,%d,%d,%d], ", (uint32_t)(draw_ctx->buf), draw_ctx->clip_area->x1, draw_ctx->clip_area->y1, draw_ctx->clip_area->x2, draw_ctx->clip_area->y2);
            flag = 1;
        //}
        if(flag)
            t_start = xTaskGetTickCount();
    #endif
        gp_zoom(draw_ctx, draw_dsc, coords, src_buf, cf, &blend_dsc);
    #if 0
        if(flag) {
            t_end = xTaskGetTickCount();
            DBG_PRINT("scale draw time:%d\r\n", t_end-t_start);
        }
    #endif

    return;
}
#endif  // LV_USE_OPTIMIZE_GP
