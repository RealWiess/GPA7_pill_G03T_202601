#ifndef APP_HDMI_UI_H
#define APP_HDMI_UI_H

#include <stdint.h>
#include <stdbool.h>
#include "pill_counter.h"

#ifdef __cplusplus
extern "C" {
#endif

// HDMI Display Resolution Definitions
#define HDMI_DISPLAY_WIDTH   1920
#define HDMI_DISPLAY_HEIGHT  1080

// Split Screen UI Panel Dimensions
#define LEFT_VIDEO_PANE_WIDTH    1280
#define LEFT_VIDEO_PANE_HEIGHT   1080

#define RIGHT_STATS_PANE_WIDTH   640
#define RIGHT_STATS_PANE_HEIGHT  1080

/**
 * @brief Initialize HDMI Dual-Pane LVGL UI Layout
 * @return true on success, false on failure
 */
bool App_HDMI_UI_Init(void);

/**
 * @brief Update Dual-Pane UI with live camera frame and pill counter results
 * @param frame_rgb Pointer to video frame buffer
 * @param width Video width
 * @param height Video height
 * @param result Pointer to PillCounterResult_t containing bounding boxes & total count
 */
void App_HDMI_UI_Update(const uint8_t *frame_rgb, uint32_t width, uint32_t height, const PillCounterResult_t *result);

/**
 * @brief Deinitialize HDMI UI layout
 */
void App_HDMI_UI_Deinit(void);

#ifdef __cplusplus
}
#endif

#endif // APP_HDMI_UI_H
