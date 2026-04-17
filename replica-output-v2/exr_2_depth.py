import os

# THIS MUST BE THE VERY FIRST THING IN THE SCRIPT
# It unlocks OpenCV 4+ OpenEXR header reading
os.environ["OPENCV_IO_ENABLE_OPENEXR"] = "1"

import glob
import numpy as np
import cv2

def exr_to_dpt(exr_path, dpt_path):
    # 1. Byte-size verification
    file_size = os.path.getsize(exr_path)
    if file_size < 100:
        print(f"[FAIL] {os.path.basename(exr_path)} is only {file_size} bytes. Blender generated an empty file.")
        return

    # 2. Read with unlocked OpenCV
    exr_img = cv2.imread(exr_path, cv2.IMREAD_ANYCOLOR | cv2.IMREAD_ANYDEPTH)

    if exr_img is None:
        print(f"[ERROR] OpenCV failed to read {os.path.basename(exr_path)}. The file is corrupted.")
        return

    # 3. Extract Z-depth
    if len(exr_img.shape) >= 3:
        depth_np = exr_img[:, :, 0]
    else:
        depth_np = exr_img

    # 4. Convert to raw 32-bit float binary
    depth_float32 = np.array(depth_np, dtype=np.float32)

    try:
        with open(dpt_path, 'wb') as f:
            f.write(depth_float32.tobytes())
        print(f"[SUCCESS] Saved: {os.path.basename(dpt_path)} ({file_size / 1024 / 1024:.2f} MB)")
    except Exception as e:
        print(f"[ERROR] Failed to write {dpt_path}: {e}")

if __name__ == "__main__":
    input_dir = "."
    output_dir = os.path.join(input_dir, "dpt_maps")
    os.makedirs(output_dir, exist_ok=True)

    exr_files = glob.glob(os.path.join(input_dir, "*.exr"))
    
    if not exr_files:
        print("No .exr files found in the current directory.")
        exit()
    
    for exr_path in exr_files:
        filename = os.path.basename(exr_path)
        dpt_filename = filename.replace(".exr", ".dpt")
        out_path = os.path.join(output_dir, dpt_filename)
        
        exr_to_dpt(exr_path, out_path)
        
    print("\nBatch Conversion to .dpt Complete!")