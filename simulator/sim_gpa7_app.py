"""
GPA7 Dual-Pane HDMI Pill Counter - Desktop Visual Simulator (OpenRTOS / LVGL PC Sim)
Author: MedFlow Chief Software Engineer (Google Antigravity AI)

This simulator runs on Windows PC to simulate:
1. Camera live feed / sample images (Left Pane: 1280x1080)
2. AI Bounding Box detection & confidence scores
3. LVGL v8.3.4 Dual-Pane HDMI UI (Right Pane: 640x1080)
4. RTOS Task timing, FPS, and pill counting logic
"""

import sys
import os
import time
import random
import tkinter as tk
from tkinter import ttk, filedialog
from PIL import Image, ImageTk, ImageDraw, ImageFont

class GPA7SimulatorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("GPA7 Pill Counter - OpenRTOS & LVGL Desktop Simulator (MedFlow)")
        self.root.geometry("1280x720") # Scaled 16:9 view for PC desktop
        self.root.configure(bg="#1e1e2e")

        self.running = True
        self.simulated_pills_count = 12
        self.process_time_ms = 14.8
        self.fps = 60.0

        self.setup_ui()
        self.update_loop()

    def setup_ui(self):
        # Header Toolbar
        toolbar = tk.Frame(self.root, bg="#2b2b3b", height=40)
        toolbar.pack(side=tk.TOP, fill=tk.X)

        title_lbl = tk.Label(
            toolbar,
            text="🖥️ GPA7 OpenRTOS / LVGL Simulator (HDMI 1920x1080 Dual-Pane)",
            font=("Segoe UI", 12, "bold"),
            fg="#cdd6f4",
            bg="#2b2b3b",
            padx=10
        )
        title_lbl.pack(side=tk.LEFT)

        # Control Buttons
        btn_add = tk.Button(toolbar, text="➕ 增加藥丸", command=self.add_pill, bg="#a6e3a1", fg="#11111b", font=("Segoe UI", 9, "bold"))
        btn_add.pack(side=tk.RIGHT, padx=5, pady=5)

        btn_sub = tk.Button(toolbar, text="➖ 減少藥丸", command=self.sub_pill, bg="#f38ba8", fg="#11111b", font=("Segoe UI", 9, "bold"))
        btn_sub.pack(side=tk.RIGHT, padx=5, pady=5)

        # Main Split View Frame (Aspect Ratio 16:9)
        self.split_frame = tk.Frame(self.root, bg="#11111b")
        self.split_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=10, pady=10)

        # Left Canvas (1280x1080 -> Scaled)
        self.left_panel = tk.Frame(self.split_frame, bg="#181825", bd=2, relief=tk.RIDGE)
        self.left_panel.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        # Right Panel (640x1080 -> Scaled)
        self.right_panel = tk.Frame(self.split_frame, bg="#1e1e2e", width=350, bd=2, relief=tk.RIDGE)
        self.right_panel.pack(side=tk.RIGHT, fill=tk.Y, padx=(10, 0))
        self.right_panel.pack_propagate(False)

        # Left View Canvas
        self.canvas = tk.Canvas(self.left_panel, bg="#11111b", highlightthickness=0)
        self.canvas.pack(fill=tk.BOTH, expand=True)

        # Right View UI Widgets (Simulating LVGL Containers)
        ui_title = tk.Label(self.right_panel, text="MEDFLOW PILL COUNTER", font=("Segoe UI", 14, "bold"), fg="#89b4fa", bg="#1e1e2e", pady=15)
        ui_title.pack(side=tk.TOP)

        # Large Count Display Box
        count_box = tk.Frame(self.right_panel, bg="#313244", bd=2, relief=tk.GROOVE, padx=20, pady=20)
        count_box.pack(side=tk.TOP, fill=tk.X, padx=15, pady=10)

        count_lbl_title = tk.Label(count_box, text="即時藥丸計數 (TOTAL PILLS)", font=("Segoe UI", 10), fg="#a6adc8", bg="#313244")
        count_lbl_title.pack()

        self.val_lbl = tk.Label(count_box, text="0", font=("Segoe UI", 48, "bold"), fg="#a6e3a1", bg="#313244")
        self.val_lbl.pack()

        # Stats Card
        stats_box = tk.Frame(self.right_panel, bg="#2b2b3b", padx=15, pady=15)
        stats_box.pack(side=tk.TOP, fill=tk.X, padx=15, pady=10)

        self.lbl_npu = tk.Label(stats_box, text="NPU 推論耗時: 15.0 ms", font=("Segoe UI", 10), fg="#cdd6f4", bg="#2b2b3b", anchor="w")
        self.lbl_npu.pack(fill=tk.X, pady=2)

        self.lbl_fps = tk.Label(stats_box, text="Display 刷新率: 60 FPS", font=("Segoe UI", 10), fg="#cdd6f4", bg="#2b2b3b", anchor="w")
        self.lbl_fps.pack(fill=tk.X, pady=2)

        self.lbl_status = tk.Label(stats_box, text="系統狀態: 🟢 正常運行 (NPU OK)", font=("Segoe UI", 10), fg="#a6e3a1", bg="#2b2b3b", anchor="w")
        self.lbl_status.pack(fill=tk.X, pady=2)

        # Log Console Widget
        log_title = tk.Label(self.right_panel, text="Console Debug Log", font=("Segoe UI", 9, "bold"), fg="#74c7ec", bg="#1e1e2e", anchor="w")
        log_title.pack(side=tk.TOP, fill=tk.X, padx=15, pady=(15, 2))

        self.log_txt = tk.Text(self.right_panel, bg="#11111b", fg="#a6adc8", font=("Consolas", 8), height=10, state=tk.DISABLED)
        self.log_txt.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=15, pady=(0, 15))

        self.log("[SIM] GPA7 OpenRTOS / LVGL Simulator started.")
        self.log("[SIM] Frame Resolution: 1920x1080 (Left: 1280, Right: 640)")
        self.log("[SIM] NPU Acceleration Engine: GPDLAv2 INT8 Mock ON.")

    def log(self, text):
        self.log_txt.config(state=tk.NORMAL)
        self.log_txt.insert(tk.END, text + "\n")
        self.log_txt.see(tk.END)
        self.log_txt.config(state=tk.DISABLED)

    def add_pill(self):
        self.simulated_pills_count += 1
        self.log(f"[USER] Manual Add Pill -> Count: {self.simulated_pills_count}")

    def sub_pill(self):
        if self.simulated_pills_count > 0:
            self.simulated_pills_count -= 1
            self.log(f"[USER] Manual Sub Pill -> Count: {self.simulated_pills_count}")

    def update_loop(self):
        # Update Counter UI Value
        self.val_lbl.config(text=str(self.simulated_pills_count))
        self.lbl_npu.config(text=f"NPU 推論耗時: {self.process_time_ms:.1f} ms")
        self.lbl_fps.config(text=f"Display 刷新率: {self.fps:.0f} FPS")

        # Draw Simulated Camera Frame with AI Green Bounding Boxes
        w = self.canvas.winfo_width()
        h = self.canvas.winfo_height()

        if w > 50 and h > 50:
            # Create synthetic frame image
            img = Image.new("RGB", (w, h), (20, 20, 30))
            draw = ImageDraw.Draw(img)

            # Draw Simulated Tray Circle
            cx, cy = w // 2, h // 2
            r = min(w, h) // 2 - 20
            draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(40, 42, 54), outline=(100, 100, 140), width=3)

            # Draw Random Pills based on count
            random.seed(42) # Fixed seed for stable positions
            pills_labels = ["capsule", "round_pill", "oval_pill"]

            for i in range(self.simulated_pills_count):
                angle = (i * 137.5) * (3.14159 / 180.0) # Golden ratio spacing
                dist = (i ** 0.5) * (r * 0.18) + 30
                px = int(cx + dist * math.cos(angle))
                py = int(cy + dist * math.sin(angle))

                pw, ph = 36, 24
                # Draw Pill Shape
                draw.ellipse([px - pw//2, py - ph//2, px + pw//2, py + ph//2], fill=(220, 120, 120), outline=(255, 255, 255))

                # Draw AI Green Bounding Box
                bx1, by1 = px - pw//2 - 6, py - ph//2 - 6
                bx2, by2 = px + pw//2 + 6, py + ph//2 + 6
                draw.rectangle([bx1, by1, bx2, by2], outline=(166, 227, 161), width=2)

                # Draw Label Text
                lbl_str = f"{pills_labels[i % 3]} 0.95"
                draw.text((bx1, by1 - 12), lbl_str, fill=(166, 227, 161))

            # Convert to ImageTk
            self.tk_img = ImageTk.PhotoImage(img)
            self.canvas.delete("all")
            self.canvas.create_image(0, 0, anchor=tk.NW, image=self.tk_img)

        # Schedule Next Frame (Simulating 60 FPS RTOS Loop)
        self.root.after(33, self.update_loop)

import math

if __name__ == "__main__":
    root = tk.Tk()
    app = GPA7SimulatorApp(root)
    root.mainloop()
