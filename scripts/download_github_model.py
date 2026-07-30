"""
GPA7 Pill Counter - GitHub & Roboflow Model / Dataset Helper Script
This script helps download and inspect pre-trained pill counter models and dataset samples from GitHub & Roboflow.
"""

import os
import sys
import json
import urllib.request

MODELS_DIR = os.path.join(os.path.dirname(__file__), '..', 'models', 'pretrained')
os.makedirs(MODELS_DIR, exist_ok=True)

# List of notable open GitHub repositories and resources for Pill Detection / Counting
GITHUB_RESOURCES = [
    {
        "name": "Ultralytics Medical Pills Dataset & YOLO Model",
        "url": "https://docs.ultralytics.com/datasets/detect/medical-pills/",
        "description": "Standard medical pill detection dataset with 115 annotated images, ideal for training & fine-tuning YOLOv5/v8.",
        "model_type": "YOLOv5n / YOLOv8n"
    },
    {
        "name": "YOLO-model for Pills Detection (im-syn/safevision)",
        "url": "https://github.com/im-syn/safevision",
        "description": "Object detection model with specific 'pill' and 'pill_bottle' class labels.",
        "model_type": "YOLO ONNX"
    },
    {
        "name": "Pill-Counter-Studio",
        "url": "https://github.com/vertexaisearch/Pill-Counter-Studio",
        "description": "Pharmacist automated pill counting web system utilizing YOLOv8 for table & capsule localization.",
        "model_type": "YOLOv8 PyTorch / ONNX"
    },
    {
        "name": "Pills Counter ESP32 Edge ML (rykovv/pills_counter)",
        "url": "https://github.com/rykovv/pills_counter",
        "description": "Embedded edge pill counting system with classical and neural network inference.",
        "model_type": "Lightweight Edge ML"
    }
]

def print_summary():
    print("=" * 70)
    print("  GPA7 Pill Counter - GitHub Pre-trained Models & Resources  ")
    print("=" * 70)
    for idx, item in enumerate(GITHUB_RESOURCES, 1):
        print(f"[{idx}] {item['name']}")
        print(f"    URL: {item['url']}")
        print(f"    Type: {item['model_type']}")
        print(f"    Desc: {item['description']}")
        print("-" * 70)

def download_sample_placeholder():
    info_file = os.path.join(MODELS_DIR, "model_sources.json")
    with open(info_file, "w", encoding="utf-8") as f:
        json.dump(GITHUB_RESOURCES, f, indent=4, ensure_ascii=False)
    print(f"\n[INFO] Resource metadata saved to: {os.path.abspath(info_file)}")

if __name__ == "__main__":
    print_summary()
    download_sample_placeholder()
