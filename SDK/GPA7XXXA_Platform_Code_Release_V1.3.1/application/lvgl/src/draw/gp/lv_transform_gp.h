#ifndef LV_TRANSFORM_GP_H_INCLUDED
#define LV_TRANSFORM_GP_H_INCLUDED
/**************************************************************************
 *                             I N C U D E S                              *
 **************************************************************************/
#include "../../lv_conf_internal.h"
#if LV_USE_OPTIMIZE_GP

/**************************************************************************
 *                            T Y P E D E F S                             *
 **************************************************************************/

/**************************************************************************
 *                    G L O B A L   P R O T O T Y P E S                   *
 **************************************************************************/

void gp_zoom(struct _lv_draw_ctx_t * draw_ctx, const lv_draw_img_dsc_t * draw_dsc,
              const lv_area_t * coords, const uint8_t * src_buf, lv_img_cf_t cf,
              lv_draw_sw_blend_dsc_t * blend_dsc);

#endif // LV_USE_OPTIMIZE_GP
#endif // LV_TRANSFORM_GP_H_INCLUDED
