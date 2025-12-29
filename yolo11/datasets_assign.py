#!/usr/bin/env python3
"""
写死参数版：把原始数据集随机划分成 train / val（去掉 test）
"""

import os
import shutil
import random
import json
from pathlib import Path

# ========== 写死参数 ==========
SOURCE_DIR   = r"C:\Users\13175\Desktop\test"   # 原始数据集根目录
TARGET_DIR   = r"E:\datasets_images\num\images"   # 划分后数据集根目录
RATIO        = [0.8, 0.2]        # train / val 比例（两项之和须为 1）
EXTENSIONS   = ["jpg", "jpeg", "png"]
SEED         = 17
COPY_MODE    = True              # True=复制  False=剪切
# ============================

def collect_files(source_dir, extensions):
    source_dir = Path(source_dir)
    ext_set = {e.lower().lstrip(".") for e in extensions}
    files = []
    for ext in ext_set:
        files.extend(source_dir.glob(f"**/*.{ext}"))
    return [f.relative_to(source_dir) for f in sorted(files)]

def split_files(files, ratio, seed=42):
    if abs(sum(ratio) - 1.0) > 1e-6:
        raise ValueError("ratio 两项之和必须等于 1")
    random.seed(seed)
    shuffled = files[:]
    random.shuffle(shuffled)
    n = len(shuffled)
    train_end = int(ratio[0] * n)
    return {
        "train": shuffled[:train_end],
        "val": shuffled[train_end:]
    }

def copy_or_move(files_dict, source_root, target_root, copy=True):
    source_root = Path(source_root)
    target_root = Path(target_root)
    target_root.mkdir(parents=True, exist_ok=True)

    summary = {}
    for split_name, file_list in files_dict.items():
        split_dir = target_root / split_name
        summary[split_name] = 0
        for file_rel in file_list:
            src = source_root / file_rel
            dst = split_dir / file_rel
            dst.parent.mkdir(parents=True, exist_ok=True)
            if copy:
                shutil.copy2(src, dst)
            else:
                shutil.move(str(src), str(dst))
            summary[split_name] += 1
    return summary

def main():
    source_path = Path(SOURCE_DIR).expanduser().resolve()
    target_path = Path(TARGET_DIR).expanduser().resolve()

    if not source_path.is_dir():
        raise FileNotFoundError(f"源目录不存在：{source_path}")

    files = collect_files(source_path, EXTENSIONS)
    if not files:
        print("未找到任何符合扩展名的文件")
        return

    splits = split_files(files, RATIO, SEED)
    summary = copy_or_move(splits, source_path, target_path, COPY_MODE)

    summary_json = target_path / "split_summary.json"
    with open(summary_json, "w", encoding="utf-8") as f:
        json.dump({
            "source_dir": str(source_path),
            "target_dir": str(target_path),
            "ratio": RATIO,
            "copy_mode": COPY_MODE,
            "counts": summary
        }, f, ensure_ascii=False, indent=2)

    print("划分完成！")
    for k, v in summary.items():
        print(f"  {k}: {v} 个文件")

if __name__ == "__main__":
    main()