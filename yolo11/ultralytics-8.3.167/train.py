# -*- coding: utf-8 -*-
import warnings
warnings.filterwarnings('ignore')
from ultralytics import YOLO

if __name__ == '__main__':
    model = YOLO('yolo11s.pt')
    model.train(
        data=r'C:\Users\13175\Desktop\project\maxarm\yolo11\data.yaml',
        imgsz=320,              
        epochs=200,
        batch=16,                # batch随imgsz增大而减小
        lr0=0.001,              # AdamW适合的学习率
        lrf=0.01,
        weight_decay=0.0005,    # 关键：权重衰减防过拟合
        warmup_epochs=5,        # 预热训练
        cos_lr=True,            # 余弦退火

        freeze=8,              # 冻结backbone（非neck）

        # ===== 数据增强（针对工业场景调优）=====
        hsv_h=0.015,            # 色调微调
        hsv_s=0.4,              # 饱和度
        hsv_v=0.2,              # 亮度（降为0.2）
        mosaic=1.0,
        erasing=0.3,

        close_mosaic=10,
        patience=20,

        workers=4,
        device=0,
        cache=True,             # 小数据集启用缓存
        project='C:/Users/13175/Desktop/project/maxarm/yolo11/ultralytics-8.3.167/runs/train',
        name='exp3',            # 新实验名称
        single_cls=False,
        amp=False
                )
