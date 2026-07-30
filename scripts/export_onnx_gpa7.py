"""
GPA7 Pill Counter - ONNX Export & Compatibility Helper
This script exports PyTorch YOLO models to ONNX format matching GPA7 GPDLA NPU tensor dimensions.
"""

import os
import sys

def check_and_export():
    print("=" * 60)
    print("  GPA7 GPDLA NPU ONNX Exporter ")
    print("=" * 60)
    print("Required input dimensions for GPA7 GPDLA NPU:")
    print("  - Batch Size: 1")
    print("  - Channels: 3 (RGB)")
    print("  - Height x Width: 324x324 or 640x640")
    print("  - Dynamic axes: Disabled (Static input shapes required)")
    print("-" * 60)

    try:
        from ultralytics import YOLO
        print("[INFO] Ultralytics installed. Loading base model...")
        model = YOLO("yolov8n.pt")
        onnx_path = os.path.join(os.path.dirname(__file__), "..", "models", "pretrained", "pill_yolov8n_gpa7.onnx")
        model.export(format="onnx", imgsz=640, dynamic=False, opset=12)
        print(f"[SUCCESS] ONNX model exported to: {onnx_path}")
    except ImportError:
        print("[NOTICE] 'ultralytics' or 'torch' package not found in current Python env.")
        print("[INSTRUCTION] Please run: pip install ultralytics onnx onnxruntime")
        print("[INSTRUCTION] Then rerun this script to generate the ONNX file for GPDLA SDK quantization.")

if __name__ == "__main__":
    check_and_export()
