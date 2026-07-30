#include "app_hdmi_ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * GPA7 HDMI Dual-Pane UI Implementation:
 * Left Pane (1280x1080): Camera Live Feed + AI Green Bounding Boxes
 * Right Pane (640x1080): Digital Pill Counter + Real-time Statistics & Status
 */

static bool g_ui_initialized = false;
static uint32_t g_last_pill_count = 0;

bool App_HDMI_UI_Init(void)
{
    printf("[HDMI_UI] Initializing Dual-Pane Display (Resolution: 1920x1080)...\n");
    printf("[HDMI_UI] Left Pane: Video Live Feed & AI Boxes (1280x1080)\n");
    printf("[HDMI_UI] Right Pane: Digital Pill Counter & Stats (640x1080)\n");

    // TODO: Initialize LVGL containers (lv_obj_create) and HDMI display driver
    g_ui_initialized = true;
    g_last_pill_count = 0;
    return true;
}

void App_HDMI_UI_Update(const uint8_t *frame_rgb, uint32_t width, uint32_t height, const PillCounterResult_t *result)
{
    if (!g_ui_initialized || result == NULL) {
        return;
    }

    // 1. Draw Green Bounding Boxes for AI Detected Pills on Left Video Pane
    for (uint32_t i = 0; i < result->total_count && i < MAX_DETECTED_PILLS; i++) {
        const PillBoundingBox_t *box = &result->boxes[i];
        // Coordinate mapping to 1280x1080 video viewport
        // printf("[HDMI_UI] Pill #%u: BoundingBox [%.1f, %.1f, %.1f, %.1f], Conf: %.2f\n",
        //        i + 1, box->x_min, box->y_min, box->x_max, box->y_max, box->confidence);
    }

    // 2. Refresh Right Panel Pill Count Display
    if (result->total_count != g_last_pill_count) {
        g_last_pill_count = result->total_count;
        printf("[HDMI_UI] Updated Counter Panel: Total Pills = %u\n", g_last_pill_count);
    }
}

void App_HDMI_UI_Deinit(void)
{
    if (g_ui_initialized) {
        printf("[HDMI_UI] Releasing HDMI UI resources...\n");
        g_ui_initialized = false;
    }
}
