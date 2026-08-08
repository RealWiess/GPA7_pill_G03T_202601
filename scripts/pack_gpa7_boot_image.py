import os
import sys
import struct
import hashlib

sys.stdout.reconfigure(encoding='utf-8')

raw_bin = r'C:\SW code\source code\GPA7_Pill_20260725GA\GPA7_Pill_20260809_0157.bin'
boot_bin = r'C:\SW code\source code\GPA7_Pill_20260725GA\GPA7_Pill_20260809_0157_BOOT.bin'

print("=======================================================================")
print("     🛠️ GPA7 Boot Header TAG Auto-Packer (MedFlow Chief Engineer)")
print("     Ref: GPA7XXXAV10_Internal_ROM_Code_Reference_Manual.pdf Sec 1.3/2.0")
print("=======================================================================")

if not os.path.exists(raw_bin):
    print(f"[ERROR] Target binary file not found: {raw_bin}")
    sys.exit(1)

with open(raw_bin, 'rb') as f:
    app_code = f.read()

code_size = len(app_code)
print(f"[INFO] Raw Binary: {os.path.basename(raw_bin)} ({code_size} bytes)")

# Construct GPA7 Mask ROM Boot Header (512-byte aligned header block)
# Magic Signature: 'G' 'P' 'A' '7' (0x37415047)
magic_tag = b'GPA7_BOOT_HEADER\x00'
dram_params = struct.pack('<IIII', 0x80000000, code_size, 0x00000001, 0x00000000)
header_padding = b'\x00' * (512 - len(magic_tag) - len(dram_params))

header_block = magic_tag + dram_params + header_padding
full_boot_image = header_block + app_code

with open(boot_bin, 'wb') as f:
    f.write(full_boot_image)

boot_size = len(full_boot_image)
boot_md5 = hashlib.md5(full_boot_image).hexdigest()

print(f"\n🎉 [SUCCESS] GPA7 Bootable ROM Image Generated: {os.path.basename(boot_bin)}")
print(f"[INFO] Boot Image Size: {boot_size} bytes ({boot_size / 1024:.2f} KB)")
print(f"[INFO] Boot Image MD5: {boot_md5}")
print("=======================================================================")
