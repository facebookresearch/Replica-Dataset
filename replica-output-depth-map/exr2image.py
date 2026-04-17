#!/usr/bin/env python3
import os
import glob
import numpy as np
import cv2
import OpenEXR
import Imath

def convert_blender_depth_to_24bit(exr_path, output_dir):
    file_size = os.path.getsize(exr_path)
    if file_size < 100:
        print(f"[SKIP] {os.path.basename(exr_path)} is empty ({file_size} bytes).")
        return

    try:
        # 1. Open the EXR file natively
        exr_file = OpenEXR.InputFile(exr_path)
        header = exr_file.header()
        
        # 2. Get image dimensions from the data window
        dw = header['dataWindow']
        width = dw.max.x - dw.min.x + 1
        height = dw.max.y - dw.min.y + 1
        
        # 3. Dynamically find the depth channel
        available_channels = list(header['channels'].keys())
        
        if 'Depth.V' in available_channels:
            target_channel = 'Depth.V'
        elif 'V' in available_channels:
            target_channel = 'V'
        elif 'Z' in available_channels:
            target_channel = 'Z'
        else:
            # Fallback: grab whatever channel is available
            target_channel = available_channels[0]
            
        # 4. Extract the raw 32-bit float data directly into NumPy
        pt = Imath.PixelType(Imath.PixelType.FLOAT)
        depth_str = exr_file.channel(target_channel, pt)
        depth_np = np.frombuffer(depth_str, dtype=np.float32).reshape(height, width)
        
    except Exception as e:
        print(f"[ERROR] OpenEXR failed to read {os.path.basename(exr_path)}: {e}")
        return

    # 5. Filter out Blender's "Infinite Sky" (1e10 values)
    valid_mask = np.isfinite(depth_np) & (depth_np < 1e9)
    valid_depths = depth_np[valid_mask]

    if len(valid_depths) == 0:
        print(f"[WARNING] {os.path.basename(exr_path)} contains no valid geometry depth.")
        return

    d_min = valid_depths.min()
    d_max = valid_depths.max()

    # Apply the true room geometry max back to the infinite background pixels
    depth_np = np.where(valid_mask, depth_np, d_max)

    # 6. Normalization (0.0 to 1.0)
    den = d_max - d_min
    if den < 1e-12:
        depth_norm = np.zeros_like(depth_np)
    else:
        depth_norm = (depth_np - d_min) / den

    # 7. 24-bit Packing (Scale to 16777215)
    depth_int24 = np.round(depth_norm * 16777215).astype(np.uint32)

    # Extract Red, Green, and Blue channels
    r = (depth_int24 >> 16).astype(np.uint8)
    g = (depth_int24 >> 8).astype(np.uint8)
    b = depth_int24.astype(np.uint8)

    # Stack into BGR format (cv2.imwrite expects BGR)
    depth_bgr = np.stack([b, g, r], axis=-1)

    # 8. Output file execution
    name = os.path.basename(exr_path)[:-4] 
    rgb_path = os.path.join(output_dir, f"{name}_rgb_depth.png")
    
    cv2.imwrite(rgb_path, depth_bgr)
    print(f"[SUCCESS] Processed {os.path.basename(exr_path)} -> Range: {d_min:.2f}m to {d_max:.2f}m")


if __name__ == "__main__":
    input_dir = "." 
    output_dir = os.path.join(input_dir, "results")
    
    os.makedirs(output_dir, exist_ok=True)
    exr_files = glob.glob(os.path.join(input_dir, "*.exr"))
    
    if not exr_files:
        print("No .exr files found in the target directory.")
        exit()
        
    print(f"Found {len(exr_files)} EXR files. Beginning conversion...\n")
    
    for exr_path in exr_files:
        convert_blender_depth_to_24bit(exr_path, output_dir)
        
    print("\nBatch Conversion Complete!")