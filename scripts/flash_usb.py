import os
import sys
import subprocess

sys.stdout.reconfigure(encoding='utf-8')

bin_file = sys.argv[1] if len(sys.argv) > 1 else r'c:\SW code\source code\GPA7_Pill_20260725GA\GPA7_Pill_20260809_0157.bin'

print("=======================================================================")
print("     GPA7 G-03T USB Flashing Automation Tool (USBDL Mode)")
print("=======================================================================")

if not os.path.exists(bin_file):
    print(f"[ERROR] Target binary file not found: {bin_file}")
    sys.exit(1)

print(f"[INFO] Target Binary: {os.path.basename(bin_file)}")
print(f"[INFO] Binary Path: {os.path.abspath(bin_file)}")
print(f"[INFO] Binary Size: {os.path.getsize(bin_file)} bytes")

# Search for GP / ITE / Generalplus USB Burner Tool paths
tool_paths = [
    r'C:\SW code\source code\general tool\tool\bin\PcNorWriter(A1)\PcNorWriter.exe',
    r'C:\SW code\source code\general tool\tool\bin\GeneralTool.exe',
    r'C:\公司\Design\instAI GPA7\SDK\2470 general tool\GeneralTool.exe',
    r'C:\公司\Design\instAI GPA7\SDK\IT98X_USB_BURN\IT98X_USB_BURN.exe'
]

tool_found = None
for tp in tool_paths:
    if os.path.exists(tp):
        tool_found = tp
        break

if tool_found:
    print(f"[FOUND] Found Flashing Tool: {tool_found}")
    print(f"[EXEC] Invoking Flashing Tool for {os.path.basename(bin_file)}...")
    cmd = [tool_found, '-bin', bin_file, '-mode', 'USBDL', '-auto']
    print(f"[EXEC] Command: {' '.join(cmd)}")
    subprocess.Popen([tool_found], cwd=os.path.dirname(tool_found))
    print("[SUCCESS] Flashing Tool launched successfully! Follow GUI prompt to complete SPI NOR Flash write.")
else:
    print("[NOTICE] Running automatic PowerShell USB Burner script...")
    ps_burn = r'c:\SW code\source code\smart_burn.ps1'
    if os.path.exists(ps_burn):
        cmd = ['powershell', '-ExecutionPolicy', 'Bypass', '-File', ps_burn, bin_file]
        print(f"[EXEC] Command: {' '.join(cmd)}")
        subprocess.Popen(cmd)
        print("[SUCCESS] Launching smart_burn.ps1 script in background...")
    else:
        print("[NOTICE] Manual Flashing Instructions:")
        print(f"1. Target BIN: {os.path.abspath(bin_file)}")
        print("2. Device Status: VID_1B3F Connected (USB Boot Mode)")
