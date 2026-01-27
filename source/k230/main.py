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
    wlan = network.WLAN(network.STA_IF)
    wlan.connect(ssid, password)

    for _ in range(50):
        if wlan.isconnected():
            ip = wlan.ifconfig()[0]
            print("✓ WiFi连接成功:", ip)
            return ip
        time.sleep(0.1)

    print("✗ WiFi连接失败")
    return None


def send_localIP(sock, esp_ip, esp_port, retry=20):
    pkt = struct.pack('<B', 0xA5)   # 只发类型就够了

    for i in range(retry):
        try:
            sock.sendto(pkt, (esp_ip, esp_port))
            print("✓ sent HELLO to ESP32")
            return True
        except OSError as e:
            print("send_localIP failed:", e, "retry", i)
            time.sleep(0.5)
    return False

def main():
    gc.collect()

    # 1. WiFi
    my_ip = connect_wifi()
    if not my_ip:
        return

    time.sleep(1.0)  # 必须给 ESP32 AP 时间

    # 2. UDP（关键）
    server_ip = '192.168.4.1'
    server_port = 8080
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_socket.bind(('0.0.0.0', 0))
    udp_socket.setblocking(False)   # ⭐⭐⭐ 核心

    send_localIP(udp_socket, server_ip, server_port)

    # 3. YOLO
    kmodel_path = "/data/yolo11/best.kmodel"
    labels = ["stuff", "valve", "battery"]
    rgb888p_size = [640, 360]

    pl = PipeLine(rgb888p_size=rgb888p_size, display_mode="lcd")
    pl.create()

    yolo = YOLO11(
        task_type="detect",
        mode="video",
        kmodel_path=kmodel_path,
        labels=labels,
        rgb888p_size=rgb888p_size,
        model_input_size=[320, 320],
        display_size=pl.get_display_size(),
        conf_thresh=0.6,
        nms_thresh=0.3,
        max_boxes_num=50
    )
    yolo.config_preprocess()

    dw, dh = pl.get_display_size()
    scale_x = dw / rgb888p_size[0]
    scale_y = dh / rgb888p_size[1]

    print("✓ Start loop")

    current_flag = None   # ESP32 控制的 class_id

    try:
        while True:
            img = pl.get_frame()

            # ===== 非阻塞接收 ESP32 flag =====
            try:
                data, _ = udp_socket.recvfrom(8)
                if len(data) >= 1:
                    current_flag = data[0]
                    print("← flag:", current_flag)
            except OSError:
                pass
            if current_flag is not None:
                res = yolo.run(img)
                res = [r for r in res if r[5] == current_flag]

                # ===== class 0 只取 y 最大 =====
                class0 = [r for r in res if r[5] == 0]
                others = [r for r in res if r[5] != 0]

                filtered = []
                if class0:
                    best0 = max(class0, key=lambda r: (r[1] + r[3]) / 2)
                    filtered.append(best0)

                filtered.extend(others)
                res = filtered

                yolo.draw_result(res, pl.osd_img)

                # ===== 发送检测结果 =====
                for b in res:
                    x1, y1, x2, y2 = b[0:4]
                    cls = int(b[5])
                    cx = int((x1 + x2) / 2 * scale_x)
                    cy = int((y1 + y2) / 2 * scale_y)

                    pkt = struct.pack('<B2HB', 0xA6, cx, cy, cls)
                    udp_socket.sendto(pkt, (server_ip, server_port))

            pl.show_image()
            gc.collect()

    finally:
        udp_socket.close()
        yolo.deinit()
        pl.destroy()
        gc.collect()


if __name__ == '__main__':
    main()
