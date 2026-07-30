#ifndef LV_OPTIMIZE_GP_H_INCLUDED
#define LV_OPTIMIZE_GP_H_INCLUDED

/**************************************************************************
 *                             I N C U D E S                              *
 **************************************************************************/
#include "../../lv_conf_internal.h"

#if LV_USE_OPTIMIZE_GP
#include "../sw/lv_draw_sw.h"

/**************************************************************************
 *                            T Y P E D E F S                             *
 **************************************************************************/
typedef lv_draw_sw_ctx_t lv_draw_gp_ctx_t;

/**************************************************************************
 *                    G L O B A L   P R O T O T Y P E S                   *
 **************************************************************************/

void lv_draw_gp_ctx_init(struct _lv_disp_drv_t * drv, lv_draw_ctx_t * draw_ctx);

void lv_draw_gp_ctx_deinit(struct _lv_disp_drv_t * drv, lv_draw_ctx_t * draw_ctx);

#endif // LV_USE_OPTIMIZE_GP
#endif // LV_OPTIMIZE_GP_H_INCLUDED



