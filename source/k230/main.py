import socket
import time
import network
import struct
from libs.PipeLine import PipeLine
from libs.YOLO import YOLO11
from libs.Utils import *
import os, sys, gc
import ulab.numpy as np
import image

def connect_wifi(ssid="esp_ap", password="12345678"):
    """连接WiFi并返回IP地址"""
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(ssid, password)

    if wlan.isconnected():
        print("✓ WiFi连接成功!")
        print("K230 IP配置:", wlan.ifconfig())
        return wlan.ifconfig()[0]
    else:
        print("✗ WiFi连接失败!")
        return None

def main():
    # ========== 第1处修改：强制清理残留socket ==========
    for _ in range(3):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.close()
            gc.collect()
        except:
            pass
    # =====================================================

    # 1. 初始化网络
    my_ip = connect_wifi()

    # 2. 配置UDP
    server_ip = '192.168.4.1'   # 改成esp32的IP
    server_port = 8080
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_socket.bind(('0.0.0.0', 0))

    # 3. 初始化YOLO（你的原始配置）
    kmodel_path = "/data/yolo11/best.kmodel"
    labels = ["stuff", "coin", "charge"]
    model_input_size = [320, 320]
    display_mode = "lcd"
    rgb888p_size = [640, 360]
    confidence_threshold = 0.6
    nms_threshold = 0.3

    pl = PipeLine(rgb888p_size=rgb888p_size, display_mode=display_mode)
    pl.create()
    display_size = pl.get_display_size()

    yolo = YOLO11(task_type="detect", mode="video", kmodel_path=kmodel_path,
                  labels=labels, rgb888p_size=rgb888p_size,
                  model_input_size=model_input_size, display_size=display_size,
                  conf_thresh=confidence_threshold, nms_thresh=nms_threshold,
                  max_boxes_num=50, debug_mode=0)
    yolo.config_preprocess()

    # 坐标映射
    dw, dh = pl.get_display_size()
    scale_x = dw / rgb888p_size[0]
    scale_y = dh / rgb888p_size[1]

    print("开始检测并发送坐标...")

    try:
        while True:
            img = pl.get_frame()
            res = yolo.run(img)

            # 过滤，只保留 coin (class_id == 1)
            res = [r for r in res if r[5] == 1]

            yolo.draw_result(res, pl.osd_img)

            # 发送所有检测结果
            for b in res:
                x1, y1, x2, y2 = b[0:4]
                cx = int((x1 + x2) / 2 * scale_x)
                cy = int((y1 + y2) / 2 * scale_y)

                # 画中心点
                pl.osd_img.draw_circle(cx, cy, 4, color=(255, 0, 0), fill=True)

                # 打包并发送坐标
                pkt = struct.pack('<2HB', int(cx), int(cy), 0xAA)
                udp_socket.sendto(pkt, (server_ip, server_port))

            pl.show_image()
            gc.collect()

    finally:
        udp_socket.close()
        yolo.deinit()
        pl.destroy()
        gc.collect()  # ← 第2处修改：添加这一行

if __name__ == '__main__':
    main()
