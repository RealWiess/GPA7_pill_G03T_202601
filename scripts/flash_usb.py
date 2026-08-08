import os
import sys
import subprocess

sys.stdout.reconfigure(encoding='utf-8')

bin_file = sys.argv[1] if len(sys.argv) > 1 else None

print("=======================================================================")
print("     GPA7 G-03T USB Flashing Automation Tool (USBDL Mode)")
print("=======================================================================")

if not bin_file or not os.path.exists(bin_file):
    print(f"[ERROR] Target binary file not found: {bin_file}")
    sys.exit(1)

print(f"[INFO] Target Binary: {os.path.basename(bin_file)}")
print(f"[INFO] Binary Size: {os.path.getsize(bin_file)} bytes")

# Search for GP USB Burner Tool paths
tool_paths = [
    r'C:\公司\Design\instAI GPA7\SDK\2470 general tool\GeneralTool.exe',
    r'C:\公司\Design\instAI GPA7\SDK\IT98X_USB_BURN\IT98X_USB_BURN.exe',
    r'C:\Program Files\Generalplus\USB_Burner\USB_Burner.exe'
]

tool_found = None
for tp in tool_paths:
    if os.path.exists(tp):
        tool_found = tp
        break

if tool_found:
    print(f"[FOUND] Found USB Burner Tool: {tool_found}")
    print(f"[EXEC] Flashing {os.path.basename(bin_file)} to G-03T SPI NOR Flash via USB...")
    # Invoke burner tool
    cmd = [tool_found, '-bin', bin_file, '-mode', 'USBDL', '-auto']
    print(f"[EXEC] Command: {' '.join(cmd)}")
    subprocess.Popen(cmd)
    print("[SUCCESS] USB Burner Tool launched! Follow screen instructions to complete flashing.")
else:
    print("[NOTICE] USB Burner GUI Tool not found at default SDK path.")
    print("-----------------------------------------------------------------------")
    print("USB Flashing Instructions (USBDL Mode):")
    print("1. Plug G-03T Type-C cable into PC USB port.")
    print("2. Check Windows Device Manager for 'Generalplus USB Boot'.")
    print(f"3. Select target binary: {os.path.abspath(bin_file)}")
    print("4. Click 'Burn SPI Flash' button in Generalplus Tool.")
    print("5. Once complete, unplug and re-plug Type-C power to boot new firmware.")
    print("-----------------------------------------------------------------------")
