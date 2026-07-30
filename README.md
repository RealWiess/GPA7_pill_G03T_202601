# GPA7 數藥丸 (Pill Counter) 專案

本專案基於 **GPA7 平台 (GPDLA NPU 硬體加速器)** 實作實時藥丸檢測與自動計數系統。

## 專案結構
```
C:\SW code\source code\GPA7_Pill_20260725\
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

### 4. 嵌入式專案編譯 (C/C++)
將 `src/` 中的檔案整合至 GPA7 Platform Code 工程中編譯 ROM，編譯完成後複製出帶有時間戳記之 ROM 檔 (如 `GW202601_YYYYMMDD_HHMMSS.ROM`)。
