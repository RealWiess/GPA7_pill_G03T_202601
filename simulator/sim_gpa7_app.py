"""
GPA7 Dual-Pane HDMI Pill Counter - Desktop Visual Simulator (OpenCV Camera & File Mode)
Author: MedFlow Chief Software Engineer (Google Antigravity AI)

Features:
1. Mode A: Live USB WebCam Capture (cv2.VideoCapture)
2. Mode B: Static Image / Video File Playback
3. Mode C: Synthetic Pill Generator for Stress Testing
4. Real-time AI Green Bounding Box Overlay & LVGL v8.3.4 Right-Pane UI
"""

import sys
import os
import time
import random
import math
import tkinter as tk
from tkinter import ttk, filedialog
from PIL import Image, ImageTk, ImageDraw, ImageFont

try:
    import cv2
    HAS_OPENCV = True
except ImportError:
    HAS_OPENCV = False

class GPA7CameraSimulatorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("GPA7 Pill Counter - Camera & LVGL Desktop Simulator (MedFlow)")
        self.root.geometry("1280x720")
        self.root.configure(bg="#1e1e2e")

        self.camera_mode = "SYNTHETIC" # "WEBCAM", "FILE", "SYNTHETIC"
        self.cap = None
        self.current_file_path = None
        self.simulated_pills_count = 8
        self.process_time_ms = 14.2
        self.fps = 60.0

        self.setup_ui()
        self.update_loop()

    def setup_ui(self):
        # Header Toolbar
        toolbar = tk.Frame(self.root, bg="#2b2b3b", height=45)
        toolbar.pack(side=tk.TOP, fill=tk.X)

        title_lbl = tk.Label(
            toolbar,
            text="📷 GPA7 Camera & LVGL Simulator (Dual-Pane 1920x1080)",
            font=("Segoe UI", 11, "bold"),
            fg="#cdd6f4",
            bg="#2b2b3b",
            padx=10
        )
        title_lbl.pack(side=tk.LEFT)

        # Camera Mode Selector
        lbl_mode = tk.Label(toolbar, text="鏡頭來源:", font=("Segoe UI", 9), fg="#a6adc8", bg="#2b2b3b")
        lbl_mode.pack(side=tk.LEFT, padx=(20, 5))

        self.mode_combo = ttk.Combobox(
            toolbar,
            values=["合成圖像產生器 (Synthetic)", "電腦 USB WebCam 實體鏡頭", "開啟圖片/影片檔案 (File)"],
            state="readonly",
            width=25
        )
        self.mode_combo.current(0)
        self.mode_combo.pack(side=tk.LEFT, padx=5)
        self.mode_combo.bind("<<ComboboxSelected>>", self.on_mode_change)

        # Control Buttons
        btn_add = tk.Button(toolbar, text="➕ 增加藥丸", command=self.add_pill, bg="#a6e3a1", fg="#11111b", font=("Segoe UI", 9, "bold"))
        btn_add.pack(side=tk.RIGHT, padx=5, pady=5)

        btn_sub = tk.Button(toolbar, text="➖ 減少藥丸", command=self.sub_pill, bg="#f38ba8", fg="#11111b", font=("Segoe UI", 9, "bold"))
        btn_sub.pack(side=tk.RIGHT, padx=5, pady=5)

        # Main Split View Frame
        self.split_frame = tk.Frame(self.root, bg="#11111b")
        self.split_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=10, pady=10)

        # Left Canvas (1280x1080 Viewport)
        self.left_panel = tk.Frame(self.split_frame, bg="#181825", bd=2, relief=tk.RIDGE)
        self.left_panel.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        # Right Panel (640x1080 Viewport)
        self.right_panel = tk.Frame(self.split_frame, bg="#1e1e2e", width=360, bd=2, relief=tk.RIDGE)
        self.right_panel.pack(side=tk.RIGHT, fill=tk.Y, padx=(10, 0))
        self.right_panel.pack_propagate(False)

        # Left View Canvas
        self.canvas = tk.Canvas(self.left_panel, bg="#11111b", highlightthickness=0)
        self.canvas.pack(fill=tk.BOTH, expand=True)

        # Right View UI Widgets
        ui_title = tk.Label(self.right_panel, text="MEDFLOW PILL COUNTER", font=("Segoe UI", 13, "bold"), fg="#89b4fa", bg="#1e1e2e", pady=12)
        ui_title.pack(side=tk.TOP)

        # Counter Box
        count_box = tk.Frame(self.right_panel, bg="#313244", bd=2, relief=tk.GROOVE, padx=15, pady=15)
        count_box.pack(side=tk.TOP, fill=tk.X, padx=15, pady=8)

        count_lbl_title = tk.Label(count_box, text="即時藥丸計數 (TOTAL PILLS)", font=("Segoe UI", 9), fg="#a6adc8", bg="#313244")
        count_lbl_title.pack()

        self.val_lbl = tk.Label(count_box, text="0", font=("Segoe UI", 44, "bold"), fg="#a6e3a1", bg="#313244")
        self.val_lbl.pack()

        # Stats Card
        stats_box = tk.Frame(self.right_panel, bg="#2b2b3b", padx=12, pady=12)
        stats_box.pack(side=tk.TOP, fill=tk.X, padx=15, pady=8)

        self.lbl_src = tk.Label(stats_box, text="鏡頭來源: 🎨 合成圖像產生器", font=("Segoe UI", 9), fg="#fab387", bg="#2b2b3b", anchor="w")
        self.lbl_src.pack(fill=tk.X, pady=2)

        self.lbl_npu = tk.Label(stats_box, text="NPU 推論耗時: 14.2 ms", font=("Segoe UI", 9), fg="#cdd6f4", bg="#2b2b3b", anchor="w")
        self.lbl_npu.pack(fill=tk.X, pady=2)

        self.lbl_status = tk.Label(stats_box, text="系統狀態: 🟢 NPU 推論 OK", font=("Segoe UI", 9), fg="#a6e3a1", bg="#2b2b3b", anchor="w")
        self.lbl_status.pack(fill=tk.X, pady=2)

        # Log Console Widget
        log_title = tk.Label(self.right_panel, text="Console Debug Log", font=("Segoe UI", 9, "bold"), fg="#74c7ec", bg="#1e1e2e", anchor="w")
        log_title.pack(side=tk.TOP, fill=tk.X, padx=15, pady=(10, 2))

        self.log_txt = tk.Text(self.right_panel, bg="#11111b", fg="#a6adc8", font=("Consolas", 8), height=10, state=tk.DISABLED)
        self.log_txt.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=15, pady=(0, 12))

        self.log("[SIM] GPA7 Camera & LVGL Simulator initialized.")
        if HAS_OPENCV:
            self.log("[SIM] OpenCV backend available for WebCam/Video capture.")
        else:
            self.log("[SIM] Note: Install opencv-python for physical USB WebCam support.")

    def log(self, text):
        self.log_txt.config(state=tk.NORMAL)
        self.log_txt.insert(tk.END, text + "\n")
        self.log_txt.see(tk.END)
        self.log_txt.config(state=tk.DISABLED)

    def on_mode_change(self, event):
        idx = self.mode_combo.current()
        if self.cap:
            self.cap.release()
            self.cap = None

        if idx == 0: # Synthetic
            self.camera_mode = "SYNTHETIC"
            self.lbl_src.config(text="鏡頭來源: 🎨 合成圖像產生器")
            self.log("[MODE] Switched to Synthetic Image Generator.")
        elif idx == 1: # USB WebCam
            if not HAS_OPENCV:
                self.log("[ERROR] OpenCV not installed. Run 'pip install opencv-python'.")
                return
            self.cap = cv2.VideoCapture(0)
            if self.cap.isOpened():
                self.camera_mode = "WEBCAM"
                self.lbl_src.config(text="鏡頭來源: 📹 USB WebCam 實體鏡頭")
                self.log("[MODE] USB WebCam initialized successfully.")
            else:
                self.log("[ERROR] Failed to open USB WebCam (Device 0).")
                self.cap = None
        elif idx == 2: # File
            file_path = filedialog.askopenfilename(
                title="選擇藥丸圖片或影片檔案",
                filetypes=[("Image/Video", "*.jpg *.png *.jpeg *.mp4 *.avi")]
            )
            if file_path:
                self.current_file_path = file_path
                if file_path.lower().endswith(('.mp4', '.avi')) and HAS_OPENCV:
                    self.cap = cv2.VideoCapture(file_path)
                    self.camera_mode = "VIDEO_FILE"
                    self.lbl_src.config(text=f"鏡頭來源: 🎬 影片 [{os.path.basename(file_path)}]")
                else:
                    self.camera_mode = "IMAGE_FILE"
                    self.lbl_src.config(text=f"鏡頭來源: 🖼️ 圖片 [{os.path.basename(file_path)}]")
                self.log(f"[MODE] Loaded file: {os.path.basename(file_path)}")

    def add_pill(self):
        self.simulated_pills_count += 1
        self.log(f"[USER] Add Pill -> Count: {self.simulated_pills_count}")

    def sub_pill(self):
        if self.simulated_pills_count > 0:
            self.simulated_pills_count -= 1
            self.log(f"[USER] Sub Pill -> Count: {self.simulated_pills_count}")

    def update_loop(self):
        w = self.canvas.winfo_width()
        h = self.canvas.winfo_height()

        if w > 50 and h > 50:
            img = None

            # Mode A & VIDEO: OpenCV WebCam or Video File
            if self.camera_mode in ["WEBCAM", "VIDEO_FILE"] and self.cap:
                ret, frame = self.cap.read()
                if not ret and self.camera_mode == "VIDEO_FILE":
                    self.cap.set(cv2.CAP_PROP_POS_FRAMES, 0) # Loop video
                    ret, frame = self.cap.read()
                if ret:
                    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                    img = Image.fromarray(frame_rgb).resize((w, h))

            # Mode B: Image File
            elif self.camera_mode == "IMAGE_FILE" and self.current_file_path:
                try:
                    img = Image.open(self.current_file_path).convert("RGB").resize((w, h))
                except Exception:
                    pass

            # Mode C: Synthetic Generator (Default Fallback)
            if img is None:
                img = Image.new("RGB", (w, h), (20, 20, 30))
                draw = ImageDraw.Draw(img)
                cx, cy = w // 2, h // 2
                r = min(w, h) // 2 - 20
                draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(40, 42, 54), outline=(100, 100, 140), width=3)

                random.seed(42)
                pills_labels = ["capsule", "round_pill", "oval_pill"]

                for i in range(self.simulated_pills_count):
                    angle = (i * 137.5) * (3.14159 / 180.0)
                    dist = (i ** 0.5) * (r * 0.18) + 30
                    px = int(cx + dist * math.cos(angle))
                    py = int(cy + dist * math.sin(angle))
                    pw, ph = 36, 24
                    draw.ellipse([px - pw//2, py - ph//2, px + pw//2, py + ph//2], fill=(220, 120, 120), outline=(255, 255, 255))
                    bx1, by1 = px - pw//2 - 6, py - ph//2 - 6
                    bx2, by2 = px + pw//2 + 6, py + ph//2 + 6
                    draw.rectangle([bx1, by1, bx2, by2], outline=(166, 227, 161), width=2)
                    draw.text((bx1, by1 - 12), f"{pills_labels[i % 3]} 0.95", fill=(166, 227, 161))

            # Overlay AI Green Boxes if using Real Camera / Image File
            if self.camera_mode in ["WEBCAM", "IMAGE_FILE", "VIDEO_FILE"] and img:
                draw = ImageDraw.Draw(img)
                random.seed(10)
                # Simulate AI Pill Detection Bounding Boxes on Real Image
                for i in range(self.simulated_pills_count):
                    bx1 = random.randint(50, w - 150)
                    by1 = random.randint(50, h - 150)
                    bx2 = bx1 + random.randint(40, 70)
                    by2 = by1 + random.randint(40, 70)
                    draw.rectangle([bx1, by1, bx2, by2], outline=(166, 227, 161), width=3)
                    draw.text((bx1, by1 - 14), f"pill 0.94", fill=(166, 227, 161))

            # Render to Canvas
            self.tk_img = ImageTk.PhotoImage(img)
            self.canvas.delete("all")
            self.canvas.create_image(0, 0, anchor=tk.NW, image=self.tk_img)

        # Update Counter UI text
        self.val_lbl.config(text=str(self.simulated_pills_count))
        self.lbl_npu.config(text=f"NPU 推論耗時: {self.process_time_ms:.1f} ms")

        self.root.after(33, self.update_loop)

if __name__ == "__main__":
    root = tk.Tk()
    app = GPA7CameraSimulatorApp(root)
    root.mainloop()
