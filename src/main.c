#include <stdio.h>
#include <stdlib.h>
#include "pill_counter.h"

int main(int argc, char *argv[])
{
    printf("====================================================\n");
    printf("     GPA7 Pill Counter Application V1.0.0            \n");
    printf("====================================================\n");

    const char *model_file = "models/gpdla_compiled/pill_yolov5n.gpdla";
    if (argc > 1) {
        model_file = argv[1];
    }

    if (!PillCounter_Init(model_file)) {
        printf("[MAIN] Failed to initialize Pill Counter NPU Engine.\n");
        return -1;
    }

    printf("[MAIN] Starting Camera Capture and Pill Counting Loop...\n");

    // Dummy frame buffer test (Simulated 640x640 RGB image)
    uint32_t img_w = 640;
    uint32_t img_h = 640;
    uint8_t *dummy_frame = (uint8_t *)calloc(1, img_w * img_h * 3);

    PillCounterResult_t result;
    if (PillCounter_ProcessFrame(dummy_frame, img_w, img_h, &result)) {
        printf("[MAIN] Frame Processed Successfully!\n");
        printf("[MAIN] Detected Pills Count: %u\n", result.total_count);
        printf("[MAIN] NPU Processing Time: %u ms\n", result.process_time_ms);
    }

    free(dummy_frame);
    PillCounter_Deinit();
    printf("[MAIN] Application terminated gracefully.\n");
    return 0;
}
