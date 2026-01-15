# -*- coding: utf-8 -*-

import warnings
warnings.filterwarnings('ignore')
from ultralytics import YOLO

if __name__ == '__main__':
    model = YOLO('yolo11s.pt')
    model.train(data=r'C:\Users\13175\Desktop\project\maxarm\yolo11\data.yaml',
                imgsz=320,
                epochs=200,
                batch=16,

                lr0=0.0001,             # 初始学习率！必须是0.0001级别（默认值0.01太大）
                lrf=0.01,               # 最终学习率，lr0的1%
                freeze=8,              # 冻结前12层（backbone），防过拟合

                # ===== 抗干扰灵魂（必须加）=====
                hsv_v=0.4,              # 亮度剧烈抖动（模拟明暗变化）
                mosaic=1.0,             # 四图拼接（模拟复杂场景）
                erasing=0.3,            # 随机擦除（模拟黑影/高光遮挡）

                close_mosaic=10,        # 最后10轮关闭mosaic，稳定收敛
                patience=20,            # 早停机制：20轮无提升自动停止

                workers=4,
                device=0,
                optimizer='SGD',
                resume=False,
                project='C:/Users/13175/Desktop/project/maxarm/yolo11/ultralytics-8.3.167/runs/train',
                name='exp',
                single_cls=False,
                cache=False,
                )
