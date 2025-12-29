import os
import random
import shutil
from pathlib import Path

# ========== 可修改的参数 ==========
SRC_DIR   = Path('E:/datasets_images/num/images/val')   # 原始 val 图目录
DST_DIR   = Path('E:/datasets_images/num/images/test')            # 输出的校准目录
N_SAMPLE  = 80                           # 想抽多少张
SEED      = 42                            # 固定随机种子，结果可复现
# ===================================

random.seed(SEED)
DST_DIR.mkdir(exist_ok=True)

# 过滤常见图片后缀
imgs = [p for p in SRC_DIR.iterdir()
        if p.suffix.lower() in {'.jpg', '.jpeg', '.png', '.bmp'}]

# 如果图片不足 N 张，则全取
picked = random.sample(imgs, k=min(N_SAMPLE, len(imgs)))

for p in picked:
    shutil.copy2(p, DST_DIR / p.name)

print(f'已复制 {len(picked)} 张图片到 {DST_DIR.resolve()}')