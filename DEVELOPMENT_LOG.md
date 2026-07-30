# GPA7 數藥丸專案 研發日誌 (Development Log)

## 專案資訊
- **專案名稱**: GPA7 Pill Counter (數藥丸專案)
- **硬體平台**: GPA7 平台 (搭載 GPDLA NPU 加速器)
- **SDK 路徑**: `C:\SW code\source code\GPA7_Pill_20260725\SDK`
- **原廠資料路徑 (海德威)**: `C:\公司\電子元件\海德威GPA7` (包含 Datasheet, ProgrammingGuide, Schematic&PCB, UserGuide 等)
- **專案根目錄**: `C:\SW code\source code\GPA7_Pill_20260725`
- **維護者**: MedFlow 軟體團隊

---

## 研發紀錄 (Chronological Development Log)

### [2026-07-30] 專案初始化與模型可行性研究 (Project Initialization & Model Feasibility)
- **變更與新增**:
  1. 完成專案目錄結構創建：`models/`, `scripts/`, `src/`。
  2. 建立專案版本控制規範，新增 `.gitignore`。
  3. 完成 GitHub / Roboflow 上預訓練藥丸模型之調查與評估：
     - 確定選用 **YOLOv5n / YOLOv8n** 作為邊緣推論基礎模型。
     - 確立模型部署轉換流程：`PyTorch/Roboflow (.pt)` -> `ONNX (.onnx)` -> `GPA7 GPDLA 工具鏈 (.gpdla)`。
  4. 建立 `DEVELOPMENT_LOG.md` 與 `PROJECT_SPEC.md` 規範文件。
- **架構決策**:
  - GPA7 SDK 內部內建 `GPDLA_SDK_V2.0.2_Yolo5n`，優先將 ONNX 輸入 Dim 定為 324x324 / 640x640，確保硬體加速相容性。
- **遭遇問題與解決方案**:
  - 無。

---
