import socket
import struct

PC_PORT = 8080  # 跟 K230 的 server_port 保持一致

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', PC_PORT))

print(f'电脑端已启动，端口 {PC_PORT} ...')

while True:
    data, addr = sock.recvfrom(1024)
    
    # 解包 2 个 float + 1 个 byte（共 9 字节）
    if len(data) >= 9:
        x, y, flag = struct.unpack('<2fB', data[:9])
        print(f'收到: x={x:.2f}, y={y:.2f}, flag={flag}')
    else:
        print(f'收到异常数据长度: {len(data)}')