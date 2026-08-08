import subprocess
import os
import sys

sys.stdout.reconfigure(encoding='utf-8')

ide_exe = r'C:\Program Files (x86)\Generalplus\G+ IDE for ARM 1.2.1\G+IDE for ARM.exe'
prj_file = r'C:\SW code\source code\GPA7_Pill_20260725GA\SDK\PlatformCode\GPA7XXXA Platform Code\GPA7XXXA_Platform_Code_Release_V1.3.1\project\GPDLA_NN_Examples\Yolov5n_320x320_demo\GPA7xxxA_YOLOv5n_object_demo_Project_Release.gpcmprj'

print("=======================================================================")
print("          🚀 啟動 G+ IDE for ARM 1.2.1 並開啟 GPA7 藥丸數粒專案")
print("=======================================================================")

if not os.path.exists(ide_exe):
    print(f"[ERROR] IDE executable not found: {ide_exe}")
    sys.exit(1)

if not os.path.exists(prj_file):
    print(f"[ERROR] Project file not found: {prj_file}")
    sys.exit(1)

print(f"[EXEC] IDE Path: {ide_exe}")
print(f"[EXEC] Opening Project: {os.path.basename(prj_file)}")

try:
    cmd = [ide_exe, prj_file]
    subprocess.Popen(cmd, cwd=os.path.dirname(ide_exe))
    print("🎉 [SUCCESS] G+ IDE for ARM 1.2.1 已成功在視窗中啟動！")
except Exception as e:
    print(f"[ERROR] Launching IDE failed: {e}")

print("=======================================================================")
