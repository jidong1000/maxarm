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
        patience=30,
        freeze=8,              # 冻结backbone（非neck）

        # ===== 数据增强（针对工业场景调优）=====
        hsv_h=0.015,            # 色调微调
        hsv_s=0.15,              # 饱和度
        hsv_v=0.15,              # 亮度（降为0.2）
        mosaic=1.0,
        erasing=0.3,

        close_mosaic=10,
        
        cls=0.6,              # 大幅提高分类损失权重（核心参数）
        box=6.5,              # 适度降低框损失权重
        dfl=1.0,

        optimizer='AdamW',    # AdamW通常收敛更稳定
        lr0=0.0002,          # 极低的学习率，缓慢调优
        cos_lr=True,          # 余弦退火
        warmup_epochs=5,      # 5轮预热
        warmup_momentum=0.9,
        warmup_bias_lr=0.1,

        weight_decay=0.0005,   # 权重衰减
        label_smoothing=0.05, # 极低的标签平滑（让预测更"自信"）
        dropout=0.15,        # 较高的dropout防止过拟合

        workers=8,
        device=0,
        cache=True,             # 小数据集启用缓存
        project='C:/Users/13175/Desktop/project/maxarm/yolo11/ultralytics-8.3.167/runs/train',
        name='exp4',            # 新实验名称
        single_cls=False,
        amp=False
                )
