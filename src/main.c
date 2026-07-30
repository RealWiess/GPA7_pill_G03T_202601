#include <stdio.h>
#include <stdlib.h>
#include "pill_counter.h"
#include "app_hdmi_ui.h"

int main(int argc, char *argv[])
{
    printf("====================================================\n");
    printf("     GPA7 Dual-Pane HDMI Pill Counter App V1.1      \n");
    printf("====================================================\n");

    const char *model_file = "models/gpdla_compiled/pill_yolov5n.gpdla";
    if (argc > 1) {
        model_file = argv[1];
    }

    if (!PillCounter_Init(model_file)) {
        printf("[MAIN] Failed to initialize Pill Counter NPU Engine.\n");
        return -1;
    }

    if (!App_HDMI_UI_Init()) {
        printf("[MAIN] Failed to initialize HDMI Dual-Pane UI.\n");
        PillCounter_Deinit();
        return -1;
    }

    printf("[MAIN] Starting Camera Capture, NPU Inference & HDMI UI Update Loop...\n");

    // Simulated 640x640 RGB image frame
    uint32_t img_w = 640;
    uint32_t img_h = 640;
    uint8_t *dummy_frame = (uint8_t *)calloc(1, img_w * img_h * 3);

    PillCounterResult_t result;
    if (PillCounter_ProcessFrame(dummy_frame, img_w, img_h, &result)) {
        printf("[MAIN] Frame Processed Successfully!\n");
        printf("[MAIN] Detected Pills Count: %u (Processing Time: %u ms)\n", result.total_count, result.process_time_ms);
        App_HDMI_UI_Update(dummy_frame, img_w, img_h, &result);
    }

    free(dummy_frame);
    App_HDMI_UI_Deinit();
    PillCounter_Deinit();
    printf("[MAIN] Application terminated gracefully.\n");
    return 0;
}
