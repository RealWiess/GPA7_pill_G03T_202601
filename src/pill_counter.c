#include "pill_counter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool g_is_initialized = false;
static char g_loaded_model[256] = {0};

bool PillCounter_Init(const char *model_path)
{
    if (model_path == NULL) {
        printf("[PillCounter] Error: NULL model path provided.\n");
        return false;
    }

    snprintf(g_loaded_model, sizeof(g_loaded_model), "%s", model_path);
    printf("[PillCounter] Initializing GPA7 GPDLA NPU Engine with model: %s...\n", g_loaded_model);

    // TODO: Connect with GPA7 GPDLA NPU C API calls from PlatformCode
    g_is_initialized = true;
    printf("[PillCounter] Initialization successful.\n");
    return true;
}

void PillCounter_Deinit(void)
{
    if (g_is_initialized) {
        printf("[PillCounter] Releasing GPA7 GPDLA NPU resources...\n");
        g_is_initialized = false;
        memset(g_loaded_model, 0, sizeof(g_loaded_model));
    }
}

bool PillCounter_ProcessFrame(const uint8_t *frame_data, uint32_t width, uint32_t height, PillCounterResult_t *result)
{
    if (!g_is_initialized) {
        printf("[PillCounter] Error: Engine not initialized.\n");
        return false;
    }
    if (frame_data == NULL || result == NULL) {
        printf("[PillCounter] Error: Invalid argument buffers.\n");
        return false;
    }

    // Clear result structure
    memset(result, 0, sizeof(PillCounterResult_t));

    // TODO: Execute NPU Forward Inference on GPA7 GPDLA Hardware
    // Execute NMS Post-Processing and Count Pills

    result->total_count = 0; // Baseline count
    result->process_time_ms = 15; // Target NPU latency: ~15ms on GPA7

    return true;
}
