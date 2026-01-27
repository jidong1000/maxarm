from libs.PipeLine import PipeLine
from libs.YOLO import YOLO11
from libs.Utils import *
import os,sys,gc
import ulab.numpy as np
import image

#main函数
if __name__=="__main__":

    kmodel_path = "/data/yolo11/best.kmodel"
    labels = ["stuff","valve","battery"]
    model_input_size = [320,320]

    display_mode = "lcd"
    rgb888p_size = [640,360]
    confidence_threshold = 0.6
    nms_threshold = 0.3

    pl = PipeLine(rgb888p_size=rgb888p_size,display_mode=display_mode)
    pl.create()
    display_size = pl.get_display_size()

    yolo = YOLO11(task_type="detect",mode="video",kmodel_path=kmodel_path,labels=labels,rgb888p_size=rgb888p_size,model_input_size=model_input_size,display_size=display_size,conf_thresh=confidence_threshold,nms_thresh=nms_threshold,max_boxes_num=2,debug_mode=0)
    yolo.config_preprocess()

#    映射坐标
    dw, dh = pl.get_display_size()
    scale_x = dw / rgb888p_size[0]
    scale_y = dh / rgb888p_size[1]

    while True:
        img = pl.get_frame()
        res = yolo.run(img)

#        filter i dont need, r[5] is class_id
        filtered = []
        res = [r for r in res if r[5] == 1]

        class0 = [r for r in res if r[5] == 0]
        others = [r for r in res if r[5] != 0]

        # 只对 class_id == 0 做筛选
        if class0:
            best0 = max(class0, key=lambda r: (r[1] + r[3]) / 2)
            filtered.append(best0)

        # 其他类别本来就只有一个，直接加
        filtered.extend(others)

        res = filtered


        yolo.draw_result(res,pl.osd_img)

        for b in res:
            x1, y1, x2, y2 = b[0:4]

            cx = int((x1 + x2) / 2 * scale_x)
            cy = int((y1 + y2) / 2 * scale_y)

            print(cx, cy)

            pl.osd_img.draw_circle(cx, cy, 4, color=(255, 0, 0), fill=True)

        pl.show_image()
        gc.collect()
    yolo.deinit()
    pl.destroy()
