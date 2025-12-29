#!/usr/bin/env python3
"""
把目录内所有文件重命名为 {类别_序号}.ext
类别规则：
1-100   → 0
101-200 → 1
201-300 → 3
301-400 → 5
401-500 → 6
501-600 → 7
"""
import os
import re
from pathlib import Path

# ========== 配置 ==========
ROOT_DIR = r"C:\Users\13175\Desktop\test"   # 需要重命名的目录
# ==========================

# 区间 → 类别 映射表
RANGE_MAP = [
    (1,   100, 0),
    (101, 200, 1),
    (201, 300, 3),
    (301, 400, 5),
    (401, 500, 6),
    (501, 600, 7),
]

def extract_number(name: str) -> int:
    """从文件名提取第一个连续数字"""
    m = re.search(r'\d+', name)
    return int(m.group()) if m else None

def category_of(n: int) -> int:
    """根据数字返回类别"""
    for lo, hi, cat in RANGE_MAP:
        if lo <= n <= hi:
            return cat
    return None

def main():
    root = Path(ROOT_DIR)
    if not root.is_dir():
        print("目录不存在:", root)
        return

    files = [f for f in root.iterdir() if f.is_file()]
    files.sort(key=lambda f: extract_number(f.stem) or 0)

    counter = {}  # 每个类别已使用到的序号
    for f in files:
        num = extract_number(f.stem)
        if num is None:
            print(f"跳过（无数字）: {f.name}")
            continue

        cat = category_of(num)
        if cat is None:
            print(f"跳过（数字 {num} 不在任何区间）: {f.name}")
            continue

        idx = counter.get(cat, 0)
        new_name = f"{cat}_{idx}{f.suffix}"
        counter[cat] = idx + 1

        new_path = f.with_name(new_name)
        if not new_path.exists():
            f.rename(new_path)
        else:
            print(f"目标文件已存在，跳过: {new_name}")

    print("重命名完成！")
    for cat, cnt in counter.items():
        print(f"类别 {cat}: {cnt} 个文件")

if __name__ == "__main__":
    main()