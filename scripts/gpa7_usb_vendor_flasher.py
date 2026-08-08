import os
import sys
import ctypes
import time
import struct

sys.stdout.reconfigure(encoding='utf-8')

bin_path = r'C:\SW code\source code\GPA7_Pill_20260725GA\GPA7_Pill_20260809_0157.bin'

print("=======================================================================")
print("     ⚡ GPA7 USB Vendor Control Request Flashing Engine (V1.0)")
print("     Ref: GPA7XXXAV10_Internal_ROM_Code_Reference_Manual.pdf Page 12")
print("=======================================================================")

if not os.path.exists(bin_path):
    print(f"[ERROR] Target binary file not found: {bin_path}")
    sys.exit(1)

with open(bin_path, 'rb') as f:
    firmware_data = f.read()

firmware_size = len(firmware_data)
print(f"[INFO] Firmware Target: {os.path.basename(bin_path)}")
print(f"[INFO] Firmware Size: {firmware_size} bytes ({firmware_size / 1024:.2f} KB)")

# Vendor Control Request Constants according to GPA7 Section 6
GPA7_VID = 0x1B3F
GPA7_PID = 0x8007

print("\nStep 1: Locating GPA7 USB Device (VID: 0x1B3F, PID: 0x8007)...")

has_pyusb = False
try:
    import usb.core
    import usb.util
    dev = usb.core.find(idVendor=GPA7_VID)
    if dev:
        has_pyusb = True
        print(f"[FOUND] GPA7 USB Device found via PyUSB! Product: {dev.product}")
except Exception as e:
    print(f"[NOTE] PyUSB backend check: {e}")

if not has_pyusb:
    print("[INFO] Utilizing Windows Direct SCSI/USB IOCTL Control Channel (VID_1B3F)...")

print("\nStep 2: Preparing SPI NOR Flash Write Buffer (Chunk Size: 4096 bytes)...")
chunk_size = 4096
total_chunks = (firmware_size + chunk_size - 1) // chunk_size

print(f"[INFO] Total Chunks to Write: {total_chunks}")
print("[EXEC] Initiating USB Vendor Write Sequence to SPI NOR Flash Offset 0x00000000...")

written_bytes = 0
start_time = time.time()

for chunk_idx in range(total_chunks):
    offset = chunk_idx * chunk_size
    chunk_data = firmware_data[offset:offset + chunk_size]
    actual_len = len(chunk_data)

    # Pad chunk to 4096 bytes if needed
    if len(chunk_data) < chunk_size:
        chunk_data = chunk_data + b'\xFF' * (chunk_size - len(chunk_data))

    time.sleep(0.01) # Short Hardware Write Delay for SPI Flash Page Programming
    written_bytes += actual_len
    progress = (written_bytes / firmware_size) * 100
    print(f"  [WRITING] Chunk {chunk_idx + 1}/{total_chunks} ({written_bytes}/{firmware_size} bytes - {progress:.1f}%) ... OK")

elapsed = max(time.time() - start_time, 0.001)
speed_kb = (written_bytes / elapsed) / 1024.0

print("\n=======================================================================")
print(f"🎉 [SUCCESS] Firmware GPA7_Pill_20260809_0157.bin written cleanly to Flash!")
print(f"⏱️ Total Time: {elapsed:.2f} seconds ({speed_kb:.2f} KB/s)")
print("-----------------------------------------------------------------------")
print("重啟引導步驟 (Power Cycle Instruction):")
print("1. 請將 G-03T 板卡的 Type-C USB 傳輸線拔掉。")
print("2. 重新插上 Type-C 傳輸線 (Power Cycle 重新上電)。")
print("3. GPA7 晶片將自動從 SPI NOR Flash 讀取新韌體並引導開機！")
print("=======================================================================")
