# GPA7 數藥丸專案 系統規格書 (PROJECT_SPEC.md)

## 1. 系統簡介
本專案為 MedFlow 藥物管理系統中之 **藥丸自動計數與辨識 (Pill Counter)** 模組。
系統運行於 **GPA7 平台**，利用 **GPDLA NPU 硬體加速器** 進行實時影像中藥丸之檢測 (Detection)、定位 (Localization) 與計數 (Counting)。

---

## 2. 硬體與 SDK 規格
- **處理器與 NPU**: GPA7 晶片，整合 GPDLA NPU 加速單元。
- **SDK 版本**: GPA7 Platform Code Release V1.3.1 / GPDLA SDK V2.0.2.
- **SDK 路徑**: `C:\SW code\source code\GPA7_Pill_20260725\SDK`
- **海德威原廠資料**: `C:\公司\電子元件\海德威GPA7` (含 Datasheet, ProgrammingGuide, Schematic&PCB, UserGuide, PlatformCode)
- **影像輸入規格**: CMOS Camera / USB Video Class (UVC) 輸入，解析度 720P / 1080P。

---

## 3. 深度學習模型規格 (Deep Learning Specification)
- **基礎模型 (Base Model)**: YOLOv5n / YOLOv8n (Lightweight Object Detection Model)
- **模型輸入 (Model Input)**: `1 x 3 x 324 x 324` 或 `1 x 3 x 640 x 640` (RGB Image, Normalized [0, 1])
- **模型輸出 (Model Output)**: `[Batch, Anchors, 5 + Num_Classes]` (Bounding Boxes `(x_center, y_center, width, height)`, Confidence, Class Scores)
- **目標類別 (Classes)**:
  - `0`: Pill / Capsule / Tablet (通用藥丸標籤)
- **NPU 模型格式**: `.gpdla` (由 GPDLA SDK 之 ONNX 轉換器產生)

---

## 4. 藥丸計數後處理與演算法邏輯 (Post-Processing & Counter Logic)
1. **Confidence Threshold**: 信心度低於 `0.45` 之 Detection 劃界予以過濾。
2. **Non-Maximum Suppression (NMS)**: IoU Threshold 設定為 `0.45`，消除重複重疊之框選。
3. **區域劃分與邊界檢測 (ROI Boundary & Counting)**:
   - 僅計算落在指定 ROI (Region of Interest) 內的藥丸數量。
   - 計算藥丸中心點 `(cx, cy)` 並遞增計數器 `Pill_Count`。

---

## 5. 介面與輸出規格 (Interface & Output Specification)
- **UI 顯示**: LVGL 介面即時顯示畫幅、藥丸 Bounding Box 綠框與 `Total Count: N`。
- **通訊輸出**: 支援 UART / Network (MQTT / Sockets) 將計數結果傳送至主控端。
