#ifndef PILL_COUNTER_H
#define PILL_COUNTER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Max detected pill count limit per frame
#define MAX_DETECTED_PILLS 256

// Bounding Box Structure for Detected Pill
typedef struct {
    float x_min;
    float y_min;
    float x_max;
    float y_max;
    float confidence;
    uint32_t class_id;
} PillBoundingBox_t;

// Counter Result Structure
typedef struct {
    uint32_t total_count;
    PillBoundingBox_t boxes[MAX_DETECTED_PILLS];
    uint32_t process_time_ms;
} PillCounterResult_t;

/**
 * @brief Initialize GPA7 GPDLA NPU Pill Counter Engine
 * @param model_path Path to the compiled .gpdla NPU model binary
 * @return true on success, false on failure
 */
bool PillCounter_Init(const char *model_path);

/**
 * @brief Deinitialize Pill Counter Engine
 */
void PillCounter_Deinit(void);

/**
 * @brief Process image buffer and perform pill counting via GPDLA NPU
 * @param frame_data Pointer to RGB/YUV image buffer
 * @param width Image width in pixels
 * @param height Image height in pixels
 * @param result Pointer to output PillCounterResult_t structure
 * @return true on success, false on failure
 */
bool PillCounter_ProcessFrame(const uint8_t *frame_data, uint32_t width, uint32_t height, PillCounterResult_t *result);

#ifdef __cplusplus
}
#endif

#endif // PILL_COUNTER_H
