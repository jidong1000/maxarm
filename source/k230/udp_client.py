import socket
import os
import time
import network
import struct

def connect_wifi(ssid="CMCC-Kq2S", password="EMXZ3796"):
    """
    连接WiFi并返回IP地址
    Connect to WiFi and return IP address

    参数 / Parameters:
    ssid: WiFi名称 / WiFi name
    password: WiFi密码 / WiFi password

    返回 / Returns:
    str: IP地址 / IP address
    """
    wifi_station = network.WLAN(0)  # 创建WiFi站点对象 / Create WiFi station object
    wifi_station.connect(ssid, password)  # 连接到指定WiFi / Connect to specified WiFi

    # 等待直到获取到IP地址 / Wait until IP address is obtained
    while wifi_station.ifconfig()[0] == '0.0.0.0':
        os.exitpoint()
    return wifi_station.ifconfig()[0]  # 返回IP地址 / Return IP address

def start_udp_client():
    """
    启动UDP客户端，发送测试消息
    Start UDP client and send test messages
    """
    # 连接WiFi网络 / Connect to WiFi network
    connect_wifi()

    # 设置服务器参数 / Set server parameters
    server_ip = '192.168.10.84'
    server_port = 8080

    # 获取服务器地址信息 / Get server address information
    address_info = socket.getaddrinfo(server_ip, server_port)
    print("地址信息 / Address info:", address_info)

    server_address = address_info[0][-1]
    print("连接地址 / Connect address:", server_address)

    # 创建UDP套接字 / Create UDP socket
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # 发送测试消息 / Send test messages
    try:
        cnt = 0
        while True:  # 持续发，不限制10次
            # 固定坐标值（x, y, z, 标志位）
            x, y, z = 123.4, 567.8, 90.0
            flag = 0xA1

            # 打包成16字节
            message = struct.pack('<3fB', x, y, z, flag)

            # 发送
            udp_socket.sendto(message, (server_ip, server_port))

            cnt += 1
            if cnt % 1000 == 0:
                cnt = 0
                print(f"已发送{x}, {y}, {z}")

            time.sleep(0.001)

    finally:
        udp_socket.close()
        print("客户端已结束")

# 启动客户端 / Start client
if __name__ == '__main__':
    start_udp_client()
