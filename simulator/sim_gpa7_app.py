"""
GPA7 Dual-Pane HDMI Pill Counter - Premium LVGL v8.3.4 UI Visual Simulator
Author: GPA7 Application Chief Engineer (MedFlow)
Resolution: 1920x1080 Dual-Pane (Left: 1280x1080 Camera & AI, Right: 640x1080 Stats Panel)
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

class GPA7PremiumUISimulator:
    def __init__(self, root):
        self.root = root
        self.root.lift()
        self.root.attributes("-topmost", True)
        self.root.after(1000, lambda: self.root.attributes("-topmost", False))
        self.root.focus_force()
        self.root.title("GPA7 Pill Counter - LVGL v8.3.4 HDMI 1920x1080 Dual-Pane UI (MedFlow)")
        self.root.geometry("1280x720")
        self.root.configure(bg="#11111b") # Catppuccin Crust Dark

        self.camera_mode = "SYNTHETIC" # "WEBCAM", "FILE", "SYNTHETIC"
        self.cap = None
        self.current_file_path = None

        # Pill counts by category
        self.count_capsule = 5
        self.count_round = 4
        self.count_oval = 3

        self.npu_latency_ms = 14.2
        self.fps = 60.0

        self.setup_ui()
        self.update_loop()

    def setup_ui(self):
        # 1. Top Header Toolbar
        toolbar = tk.Frame(self.root, bg="#1e1e2e", height=42)
        toolbar.pack(side=tk.TOP, fill=tk.X)

        title_lbl = tk.Label(
            toolbar,
            text="🏥 MedFlow GPA7 Dual-Pane HDMI 1920x1080 UI Simulator",
            font=("Segoe UI", 11, "bold"),
            fg="#89b4fa",
            bg="#1e1e2e",
            padx=12
        )
        title_lbl.pack(side=tk.LEFT)

        # Source Selector
        lbl_mode = tk.Label(toolbar, text="相機來源:", font=("Segoe UI", 9, "bold"), fg="#a6adc8", bg="#1e1e2e")
        lbl_mode.pack(side=tk.LEFT, padx=(15, 5))

        self.mode_combo = ttk.Combobox(
            toolbar,
            values=["🎨 模擬相機 (Synthetic)", "📹 USB WebCam 實體鏡頭", "🖼️ 圖片/影片檔案 (File)"],
            state="readonly",
            width=24
        )
        self.mode_combo.current(0)
        self.mode_combo.pack(side=tk.LEFT, padx=5)
        self.mode_combo.bind("<<ComboboxSelected>>", self.on_mode_change)

        # Interactive Control Buttons for User Tweaks
        btn_add = tk.Button(toolbar, text="➕ 增加膠囊", command=self.add_capsule, bg="#a6e3a1", fg="#11111b", font=("Segoe UI", 8, "bold"))
        btn_add.pack(side=tk.RIGHT, padx=4, pady=5)

        btn_add_r = tk.Button(toolbar, text="➕ 增加圓片", command=self.add_round, bg="#89b4fa", fg="#11111b", font=("Segoe UI", 8, "bold"))
        btn_add_r.pack(side=tk.RIGHT, padx=4, pady=5)

        btn_sub = tk.Button(toolbar, text="➖ 清空藥丸", command=self.clear_pills, bg="#f38ba8", fg="#11111b", font=("Segoe UI", 8, "bold"))
        btn_sub.pack(side=tk.RIGHT, padx=4, pady=5)

        # 2. Main Split View Container (16:9 Aspect Ratio)
        self.split_frame = tk.Frame(self.root, bg="#11111b")
        self.split_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=8, pady=8)

        # Left Panel (1280x1080 Viewport)
        self.left_panel = tk.Frame(self.split_frame, bg="#181825", bd=2, relief=tk.RIDGE)
        self.left_panel.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        # Right Panel (640x1080 Viewport)
        self.right_panel = tk.Frame(self.split_frame, bg="#181825", width=380, bd=2, relief=tk.RIDGE)
        self.right_panel.pack(side=tk.RIGHT, fill=tk.Y, padx=(8, 0))
        self.right_panel.pack_propagate(False)

        # Left Camera Viewport Canvas
        self.canvas = tk.Canvas(self.left_panel, bg="#11111b", highlightthickness=0)
        self.canvas.pack(fill=tk.BOTH, expand=True)

        # Right LVGL v8.3.4 UI Layout Containers
        hdr_box = tk.Frame(self.right_panel, bg="#1e1e2e", padx=15, pady=12)
        hdr_box.pack(side=tk.TOP, fill=tk.X, padx=10, pady=10)

        ui_title = tk.Label(hdr_box, text="MEDFLOW AI PILL COUNTER", font=("Segoe UI", 12, "bold"), fg="#89b4fa", bg="#1e1e2e")
        ui_title.pack(anchor="w")
        ui_sub = tk.Label(hdr_box, text="GPA7 GPDLAv2 INT8 Hardware Accelerator", font=("Segoe UI", 8), fg="#a6adc8", bg="#1e1e2e")
        ui_sub.pack(anchor="w")

        # 1. Giant Total Pill Count Card (Hero Element)
        count_card = tk.Frame(self.right_panel, bg="#313244", bd=1, relief=tk.GROOVE, padx=15, pady=15)
        count_card.pack(side=tk.TOP, fill=tk.X, padx=10, pady=5)

        cnt_title = tk.Label(count_card, text="即時藥丸計數 (TOTAL COUNT)", font=("Segoe UI", 9, "bold"), fg="#cdd6f4", bg="#313244")
        cnt_title.pack()

        self.val_lbl = tk.Label(count_card, text="12", font=("Segoe UI", 96, "bold"), fg="#a6e3a1", bg="#313244")
        self.val_lbl.pack()

        # Breakdown Pills Category Badges
        breakdown_box = tk.Frame(count_card, bg="#313244")
        breakdown_box.pack(fill=tk.X, pady=(5, 0))

        self.lbl_cap = tk.Label(breakdown_box, text="💊 膠囊: 5", font=("Segoe UI", 9, "bold"), fg="#f9e2af", bg="#313244")
        self.lbl_cap.pack(side=tk.LEFT, expand=True)

        self.lbl_rnd = tk.Label(breakdown_box, text="⚪ 圓片: 4", font=("Segoe UI", 9, "bold"), fg="#89b4fa", bg="#313244")
        self.lbl_rnd.pack(side=tk.LEFT, expand=True)

        self.lbl_ovl = tk.Label(breakdown_box, text="🥚 橢圓: 3", font=("Segoe UI", 9, "bold"), fg="#cba6f7", bg="#313244")
        self.lbl_ovl.pack(side=tk.LEFT, expand=True)

        # 2. Performance Stats Card
        stats_card = tk.Frame(self.right_panel, bg="#2b2b3b", padx=12, pady=10)
        stats_card.pack(side=tk.TOP, fill=tk.X, padx=10, pady=5)

        self.lbl_npu = tk.Label(stats_card, text="⚡ NPU 推論耗時: 14.2 ms (GPDLAv2 INT8)", font=("Segoe UI", 9), fg="#cdd6f4", bg="#2b2b3b", anchor="w")
        self.lbl_npu.pack(fill=tk.X, pady=2)

        self.lbl_fps = tk.Label(stats_card, text="📺 HDMI 輸出率: 60.0 FPS (1920x1080)", font=("Segoe UI", 9), fg="#cdd6f4", bg="#2b2b3b", anchor="w")
        self.lbl_fps.pack(fill=tk.X, pady=2)

        self.lbl_status = tk.Label(stats_card, text="🟢 系統狀態: 穩定運作中 (NPU Normal)", font=("Segoe UI", 9, "bold"), fg="#a6e3a1", bg="#2b2b3b", anchor="w")
        self.lbl_status.pack(fill=tk.X, pady=2)

        # 3. Debug Console Log
        log_hdr = tk.Label(self.right_panel, text="Console Output & Status Log", font=("Segoe UI", 9, "bold"), fg="#74c7ec", bg="#181825", anchor="w")
        log_hdr.pack(side=tk.TOP, fill=tk.X, padx=12, pady=(10, 2))

        self.log_txt = tk.Text(self.right_panel, bg="#11111b", fg="#a6adc8", font=("Consolas", 8), height=9, state=tk.DISABLED)
        self.log_txt.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=10, pady=(0, 10))

        self.log("[SYS] GPA7 Dual-Pane HDMI UI initialized.")
        self.log("[NPU] Engine: GPDLAv2 Model loaded (models/gpdla_compiled/pill_yolov5n.gpdla)")
        self.log("[UI] LVGL v8.3.4 Layout: Left 1280x1080, Right 640x1080.")

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

        if idx == 0:
            self.camera_mode = "SYNTHETIC"
            self.log("[MODE] Switched to Synthetic Camera Generator.")
        elif idx == 1:
            if not HAS_OPENCV:
                self.log("[ERROR] OpenCV not installed.")
                return
            self.cap = cv2.VideoCapture(0)
            if self.cap.isOpened():
                self.camera_mode = "WEBCAM"
                self.log("[MODE] Connected to USB WebCam Device 0.")
            else:
                self.log("[ERROR] Failed to open USB WebCam.")
                self.cap = None
        elif idx == 2:
            file_path = filedialog.askopenfilename(
                title="選擇藥物照片或影片",
                filetypes=[("Image/Video", "*.jpg *.png *.jpeg *.mp4 *.avi")]
            )
            if file_path:
                self.current_file_path = file_path
                if file_path.lower().endswith(('.mp4', '.avi')) and HAS_OPENCV:
                    self.cap = cv2.VideoCapture(file_path)
                    self.camera_mode = "VIDEO_FILE"
                else:
                    self.camera_mode = "IMAGE_FILE"
                self.log(f"[MODE] Opened: {os.path.basename(file_path)}")

    def add_capsule(self):
        self.count_capsule += 1
        self.log(f"[USER] Added Capsule -> Total: {self.get_total_count()}")

    def add_round(self):
        self.count_round += 1
        self.log(f"[USER] Added Round Pill -> Total: {self.get_total_count()}")

    def clear_pills(self):
        self.count_capsule = 0
        self.count_round = 0
        self.count_oval = 0
        self.log("[USER] Cleared all pills.")

    def get_total_count(self):
        return self.count_capsule + self.count_round + self.count_oval

    def update_loop(self):
        w = self.canvas.winfo_width()
        h = self.canvas.winfo_height()

        total = self.get_total_count()
        self.val_lbl.config(text=str(total))
        self.lbl_cap.config(text=f"💊 膠囊: {self.count_capsule}")
        self.lbl_rnd.config(text=f"⚪ 圓片: {self.count_round}")
        self.lbl_ovl.config(text=f"🥚 橢圓: {self.count_oval}")

        if w > 50 and h > 50:
            img = None

            if self.camera_mode in ["WEBCAM", "VIDEO_FILE"] and self.cap:
                ret, frame = self.cap.read()
                if not ret and self.camera_mode == "VIDEO_FILE":
                    self.cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                    ret, frame = self.cap.read()
                if ret:
                    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                    img = Image.fromarray(frame_rgb).resize((w, h))

            elif self.camera_mode == "IMAGE_FILE" and self.current_file_path:
                try:
                    img = Image.open(self.current_file_path).convert("RGB").resize((w, h))
                except Exception:
                    pass

            if img is None:
                img = Image.new("RGB", (w, h), (17, 17, 27))
                draw = ImageDraw.Draw(img)

                # Draw Sample Tray & Grid Lines
                cx, cy = w // 2, h // 2
                r = min(w, h) // 2 - 25
                draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(30, 30, 46), outline=(88, 91, 112), width=3)

                # Draw Pill Shapes & Green Bounding Boxes
                random.seed(42)
                pills_list = []
                for _ in range(self.count_capsule): pills_list.append(("capsule", (243, 139, 168)))
                for _ in range(self.count_round): pills_list.append(("round_pill", (137, 180, 250)))
                for _ in range(self.count_oval): pills_list.append(("oval_pill", (203, 166, 247)))

                for i, (name, color) in enumerate(pills_list):
                    angle = (i * 137.5) * (3.14159 / 180.0)
                    dist = (i ** 0.5) * (r * 0.18) + 25
                    px = int(cx + dist * math.cos(angle))
                    py = int(cy + dist * math.sin(angle))
                    pw, ph = (36, 22) if name != "round_pill" else (28, 28)

                    # Pill Body
                    draw.ellipse([px - pw//2, py - ph//2, px + pw//2, py + ph//2], fill=color, outline=(255, 255, 255))

                    # AI Green Bounding Box Overlay
                    bx1, by1 = px - pw//2 - 6, py - ph//2 - 6
                    bx2, by2 = px + pw//2 + 6, py + ph//2 + 6
                    draw.rectangle([bx1, by1, bx2, by2], outline=(166, 227, 161), width=2)
                    draw.text((bx1, by1 - 13), f"{name} 0.96", fill=(166, 227, 161))

            # Overlay Bounding Boxes if using real WebCam / File
            if self.camera_mode in ["WEBCAM", "IMAGE_FILE", "VIDEO_FILE"] and img:
                draw = ImageDraw.Draw(img)
                random.seed(10)
                for i in range(total):
                    bx1 = random.randint(50, w - 150)
                    by1 = random.randint(50, h - 150)
                    bx2 = bx1 + random.randint(40, 70)
                    by2 = by1 + random.randint(40, 70)
                    draw.rectangle([bx1, by1, bx2, by2], outline=(166, 227, 161), width=3)
                    draw.text((bx1, by1 - 14), f"pill 0.95", fill=(166, 227, 161))

            # Render to Canvas
            self.tk_img = ImageTk.PhotoImage(img)
            self.canvas.delete("all")
            self.canvas.create_image(0, 0, anchor=tk.NW, image=self.tk_img)

        self.root.after(33, self.update_loop)

if __name__ == "__main__":
    root = tk.Tk()
    app = GPA7PremiumUISimulator(root)
    root.mainloop()
