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

### [2026-07-30] HDMI 雙區域 UI 切分與燒錄檔產出架構 (HDMI Dual-Pane UI & ROM Specs)
- **變更與新增**:
  1. 新增 `src/app_hdmi_ui.h` 與 `src/app_hdmi_ui.c` HDMI 雙畫面 layout UI 模組。
  2. 左半區 (`1280x1080`): 即時 Camera 影像串流與 AI 綠色 Bounding Box 藥丸標記。
  3. 右半區 (`640x1080`): 數位計數面板與實時 NPU 數據分析。
  4. 整合主程式 `src/main.c` 執行流程。
  5. 修正平台工具鏈定義為 GPA7 (Generalplus G+IDE for ARM / G+ Code Packer)。
- **燒錄檔與 Firmware 命名規範**:
  - 編譯並經 G+ Code Packer 打包產出之 `.bin` 燒錄檔，複製至專案根目錄並命名為：`GPA7_PillCounter_YYYYMMDD_HHMMSS.bin`，供 ISP/SPI Flash/SPINAND 燒錄與版號追溯。

---


### [2026-08-08 22:05] 🤖 除錯維護對照清單：當前 AI 模型與韌體 BIN 檔產出規範

- **當前採用的 AI 模型規格 (Model Spec for Debug)**:
  - **模型名稱**: `MobileNetV2-YOLOv3-Lite` / `YOLOv5n`
  - **網路組態**: `Training/configs/yolo_pill_mode_b.cfg`
  - **輸入尺寸**: `320x320x3` (或 `324x324x3`) RGB
  - **偵測類別**: `0: capsule` (膠囊), `1: round_pill` (圓形錠劑), `2: oval_pill` (橢圓形藥丸)
  - **NPU 權重標頭檔**: `Output_GPDLA/HW_wt_param_320X320_YOLO_LITE.h` (INT8 量化)
  - **專案二進位模型檔**: `models/gpdla_compiled/pill_yolov5n.gpdla`

- **韌體 BIN 檔產出與命名 SOP**:
  - **存放目錄**: `C:\SW code\source code\GPA7_Pill_20260725GA\`
  - **命名格式**: `GPA7_Pill_YYYYMMDD_HHMM.bin` (例如 `GPA7_Pill_20260808_2146.bin`)

- **GitHub 儲存庫**: [RealWiess/GPA7_pill_G03T_202601](https://github.com/RealWiess/GPA7_pill_G03T_202601)


### [2026-08-08 22:10] 🤖 AI 全自動化標註、合成數據擴增 (Synthetic Data) 與訓練管線規劃

- **90%+ AI 自動化範疇**:
  1. **合成數據生成 (Cut-and-Paste Augmentation)**: 使用 SAM 2 / OpenCV 去背藥丸圖層，由 Python 腳本隨機旋轉貼合至各式背景，邊界框座標 100% 自動精確計算，免人工標註。
  2. **AI 自動標註 (Auto-Labeling)**: 利用 Grounding DINO / YOLO-World 進行 Zero-Shot 自動標註並匯出 VOC/YOLO 檔。
  3. **模型訓練與轉檔**: Python 自動化啟動訓練 $\rightarrow$ 導出 ONNX.
  4. **GPDLA INT8 量化**: 執行 `Quantization/pipeline_gpa7.py` 自動產生 C 標頭檔與 `.gpdla` 檔。
  5. **韌體打包**: 自動啟動編譯並歸檔為 `GPA7_Pill_YYYYMMDD_HHMM.bin`。

- **5% 人工必要工作 (Human-in-the-Loop)**:
  1. 首批實體環境藥丸照片拍攝。
  2. 重疊堆疊與低置信度 (Conf < 0.85) 案例之快速抽檢與微調。
  3. G-03T 終端真機上電現場驗收。


### [2026-08-08 22:12] ⚠️ 原始碼高風險修改項目、變動範圍與難易度評估 (High-Risk Audit Matrix)

| 修改項目 | 變動檔案 / 模組 | 變動範圍與內容 | 難易度 | 風險評估與潛在隱患 |
| :--- | :--- | :--- | :---: | :--- |
| **1. 記憶體動態分配靜態化** | `src/main.c`<br>`src/pill_counter.c`<br>`src/app_hdmi_ui.c` | 移除主迴圈與運作期間 `calloc`/`free`；改為在 LPDDR2 / IRAM 區一次性配置靜態 Dedicated Ping-Pong Buffers。 | ⭐⭐<br>(中度) | **高風險**<br>若記憶體邊界未適當隔離，大 Frame Buffer 可能碰撞 CPU Cache 或 IRAM 觸發 HardFault。 |
| **2. GPDLAv2 NPU C API 對接** | `src/pill_counter.c`<br>`src/pill_counter.h`<br>`models/gpdla_compiled/` | 引入 GP SDK 原廠 GPDLA 標頭檔，將 Mock stub 替換為實體 DMA 物理位址載入與 INT8 NMS 算子對接。 | ⭐⭐⭐⭐<br>(高度) | **極高風險**<br>若 NPU 輸入 shape ($320\times320$ vs $640\times640$) 或 32-byte 記憶體對齊不合，會引發 NPU 硬體 Exception 與晶片 Reset。 |
| **3. LVGL 雙分頁 UI & Task 切換** | `src/app_hdmi_ui.c`<br>`src/app_hdmi_ui.h` | 實作 LVGL `lv_canvas` / `lv_img` 物件與專用 UI Task，掛載 `lv_timer_handler()` 並加裝 Mutex 資源鎖。 | ⭐⭐⭐<br>(中~高度) | **高風險**<br>若在非 UI Task 繪製 LVGL 物件而未加 Mutex 鎖，會造成 LVGL State Machine 競爭與 UI 畫面撕裂。 |
| **4. RTOS Task 調度與 Sleep 防護** | `src/main.c` | 將單次推論修改為符合 OpenRTOS 規範的「Camera $\rightarrow$ NPU $\rightarrow$ UI」主 Task 迴圈，加入 `vTaskDelay()` / Queue blocked wait。 | ⭐⭐<br>(中度) | **高風險**<br>若主 Task 缺乏 `vTaskDelay()`，會引發 100% CPU 飢餓、Watchdog 逾時重啟或低優先級 Task 卡死。 |


### [2026-08-08 22:12] 📌 高風險原始碼修改待辦事項清單 (High-Risk TODO Checklist)

- [ ] **TODO-01 [高風險] 記憶體分配靜態化與 Ping-Pong Buffer 重構**
  - **難易度**: ⭐⭐ (中度)
  - **變動檔案**: `src/main.c` (L33 `calloc`), `src/pill_counter.c`, `src/app_hdmi_ui.c`
  - **變動內容**: 移除主迴圈與運作期間的 `calloc`/`free` 動態記憶體分配，改為在 LPDDR2 區配置靜態 Dedicated Buffers，防止 Heap 碎片化。

- [ ] **TODO-02 [極高風險] GPDLAv2 NPU C API 實體 DMA 與量化標頭檔對接**
  - **難易度**: ⭐⭐⭐⭐ (高度)
  - **變動檔案**: `src/pill_counter.c`, `src/pill_counter.h`, `models/gpdla_compiled/pill_yolov5n.gpdla`
  - **變動內容**: 引入 GP SDK 原廠 GPDLA 標頭檔，將 Mock 存根替換為實體 DMA 物理位址載入、32-byte 邊界對齊及 INT8 NMS 算子對接，防止 NPU 硬體 Exception。

- [ ] **TODO-03 [高風險] LVGL 1920x1080 雙畫面 UI 畫布與專用 Task Mutex 鎖**
  - **難易度**: ⭐⭐⭐ (中高度)
  - **變動檔案**: `src/app_hdmi_ui.c`, `src/app_hdmi_ui.h`
  - **變動內容**: 實作 LVGL `lv_canvas` / `lv_img` 畫布物件，建立獨立 UI Task 掛載 `lv_timer_handler()`，並於跨 Task 繪製綠框時加裝 Mutex 鎖，防範 State Machine 競態與 UI 畫面撕裂。

- [ ] **TODO-04 [高風險] RTOS 主 Task 迴圈與 `vTaskDelay` 看門狗防護**
  - **難易度**: ⭐⭐ (中度)
  - **變動檔案**: `src/main.c`
  - **變動內容**: 將單次推論修改為符合 OpenRTOS 規範的無窮主 Task 迴圈，加入 `vTaskDelay()` / Queue blocked wait，防止 CPU 100% 飢餓與看門狗 (Watchdog) 逾時重啟。
