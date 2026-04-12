#!/usr/bin/env python3
"""
Stitches a PNG frame sequence into a 2-D sprite atlas (grid layout).
Usage:  python3 make_atlas.py <frames_dir> <output.png> [frame_size [cols]]
  frame_size : resize each frame to NxN pixels  (default: keep original)
  cols       : frames per row                   (default: 16)
  Example:  python3 make_atlas.py images/ knob01.png 128 16
Frames must be named so that alphabetical sort = correct order (e.g. 0001.png).
"""
import sys
import glob
import os
import math
from PIL import Image

def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)

    frames_dir = sys.argv[1]
    output     = sys.argv[2]
    frame_size = int(sys.argv[3]) if len(sys.argv) >= 4 else None
    cols       = int(sys.argv[4]) if len(sys.argv) >= 5 else 16

    paths = sorted(glob.glob(os.path.join(frames_dir, "*.png")))
    if not paths:
        print(f"No PNG files found in {frames_dir}")
        sys.exit(1)

    def load(path):
        img = Image.open(path).convert("RGBA")
        if frame_size:
            img = img.resize((frame_size, frame_size), Image.LANCZOS)
        return img

    imgs = [load(p) for p in paths]
    fw, fh = imgs[0].size
    n    = len(imgs)
    rows = math.ceil(n / cols)

    atlas = Image.new("RGBA", (fw * cols, fh * rows))
    for i, img in enumerate(imgs):
        x = (i % cols) * fw
        y = (i // cols) * fh
        atlas.paste(img, (x, y))

    atlas.save(output)
    print(f"Saved {output}  ({atlas.width}x{atlas.height}, {n} frames, grid {cols}x{rows} @ {fw}x{fh} each)")

if __name__ == "__main__":
    main()
