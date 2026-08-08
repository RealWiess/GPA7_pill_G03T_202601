# GPA7 數藥丸 (Pill Counter G-03T) 專案

> 🤖 **本專案由 Google Antigravity AI 系統自動生成與維護**
> 
> GitHub 儲存庫：[RealWiess/GPA7_pill_G03T_202601](https://github.com/RealWiess/GPA7_pill_G03T_202601)

本專案基於 **instAI GPA7 平台 (GPDLA NPU 硬體加速器)** 實作實時藥丸檢測、定位與自動計數系統，搭配 HDMI 1920x1080 雙畫面 LVGL UI 顯示。

## 專案結構
```
C:\SW code\source code\GPA7_Pill_20260725GA\
├── SDK\                      # GPA7 官方 SDK (包含 PlatformCode, ProgrammingGuide 等)
├── DEVELOPMENT_LOG.md        # 研發日誌 (記錄歷史修訂、Debug 與設計決策)
├── PROJECT_SPEC.md           # 系統規格書與通訊/介面定義
├── README.md                 # 專案說明與操作手冊
├── .gitignore                # Git 版本控制忽略設定
├── models\                   # 存放預訓練模型與 GPDLA 編譯檔
│   ├── pretrained\           # 存放 YOLOv5n/v8n ONNX 或 PyTorch 權重
│   └── gpdla_compiled\       # 經 GPDLA 工具鏈轉換產生之 .gpdla 硬體模型
├── scripts\                  # 模擬與轉檔腳本
│   ├── download_github_model.py # 自動下載 GitHub/Roboflow 預訓練藥丸模型與數據集
│   └── export_onnx_gpa7.py      # 將 YOLO 模型導出為 GPA7 相容之 ONNX
└── src\                      # C/C++ 嵌入式原始碼 (GPA7 Platform)
    ├── main.c                # 專案主程式入口
    ├── pill_counter.c        # NPU 呼叫與藥丸計數演算法邏輯
    └── pill_counter.h        # 標頭檔介面定義
```

## 快速開始

### 1. 模型與數據集準備 (Python)
使用專案提供的腳本進行預訓練模型與測試圖片之準備：
```bash
python scripts/download_github_model.py
```

### 2. 導出 GPA7 NPU 相容 ONNX
```bash
python scripts/export_onnx_gpa7.py
```

### 3. GPDLA 工具鏈量化 (GPA7 SDK)
解壓 `SDK/PlatformCode/GPDLA_SDK_V2.0.2_Yolo5n.zip` 並執行模型轉換命令，產生 `.gpdla` 檔置於 `models/gpdla_compiled/`。

### 4. 嵌入式專案編譯與產出 (C/C++)
將 `src/` 中的檔案整合至 GPA7 Platform Code 工程中（使用 G+ IDE for ARM）進行編譯打包。編譯與 CodePacker 完成後，複製二進位檔至專案根目錄，並依 SOP 命名為：`GPA7_Pill_YYYYMMDD_HHMM.bin` (例如 `GPA7_Pill_20260808_2146.bin`)。
