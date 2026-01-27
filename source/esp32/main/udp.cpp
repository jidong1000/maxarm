#include "udp.h"
#include <Arduino.h>

// 构造函数
UdpReceiver::UdpReceiver(const char* ssid, const char* password, unsigned int port)
    : _ssid(ssid),
      _password(password),
      _localPort(port),
      _clientPort(0),
      _isReceivedIP(false)
{
}

// 关闭 UDP
void UdpReceiver::stop()
{
    _udp.stop();
    Serial.println("✓ UDP stopped");
}

// 初始化 AP + UDP
bool UdpReceiver::begin()
{
    stop();
    delay(100);

    WiFi.softAP(_ssid, _password);
    Serial.println("✓ ESP32 AP Mode Started");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    if (!_udp.begin(_localPort)) {
        Serial.println("✗ UDP begin failed!");
        return false;
    }

    Serial.print("✓ UDP listening on port: ");
    Serial.println(_localPort);
    return true;
}

// 接收数据
UdpData UdpReceiver::receive()
{
    UdpData data = {0, 0, 0, false};
    int packetSize = _udp.parsePacket();
    
    if (packetSize) 
    {
        int len = _udp.read(_packetBuffer, sizeof(_packetBuffer));
        uint8_t type = _packetBuffer[0];
        
        // ===== A6：坐标数据 =====
        if (len == 6 && type == PKT_TYPE_DATA)
        {
            memcpy(&data.cx, _packetBuffer + 1, 2);
            memcpy(&data.cy, _packetBuffer + 3, 2);
            data.flag = _packetBuffer[5];
            data.isValid = true;
        }
        // ===== A5：K230 上报 IP =====
        else if (len == 1 && type == PKT_TYPE_IP)
        {
            _clientIP = _udp.remoteIP();
            _clientPort = _udp.remotePort();
            _isReceivedIP = true;
    
            Serial.printf(
                "✓ Client registered: %d.%d.%d.%d:%d\n",
                _clientIP[0], _clientIP[1],
                _clientIP[2], _clientIP[3],
                _clientPort
            );
        }
        else
        {
            Serial.printf("✗ Invalid packet: len=%d type=0x%02X\n", len, type);
        }
    }
    return data;
}

// 发送 flag（自动回给最近的 K230）
bool UdpReceiver::sendFlag(uint8_t flag)
{
    if (!_isReceivedIP) return false;

    if (!_udp.beginPacket(_clientIP, _clientPort)) {
        Serial.println("✗ beginPacket failed");
        return false;
    }

    _udp.write(&flag, 1);

    if (!_udp.endPacket()) {
        Serial.println("✗ endPacket failed");
        return false;
    }

    return true;
}

IPAddress UdpReceiver::getClientIP()
{
    return _clientIP;
}

uint16_t UdpReceiver::getClientPort()
{
    return _clientPort;
}

bool UdpReceiver::isReceivedIP()
{
    return _isReceivedIP;
}

void UdpReceiver::flush()
{
    while (_udp.parsePacket()) {
        _udp.read(_packetBuffer, sizeof(_packetBuffer));
    }
}

IPAddress UdpReceiver::getLocalIP()
{
    return WiFi.softAPIP();
}
