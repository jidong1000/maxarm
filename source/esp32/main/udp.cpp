#include "udp.h"
#include <Arduino.h>

// 构造函数
UdpReceiver::UdpReceiver(const char* ssid, const char* password, unsigned int port) 
    : _ssid(ssid), _password(password), _localPort(port) {}

// 关闭UDP（新增）
void UdpReceiver::stop() 
{
    _udp.stop();
    Serial.println("✓ UDP stopped");
}

// 初始化UDP通信
bool UdpReceiver::begin() 
{
    stop();        // ← 关键：启动前先关闭
    delay(100);
    
    WiFi.softAP(_ssid, _password);
    Serial.println("✓ ESP32 AP Mode Started");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    
    if (!_udp.begin(_localPort)) 
    {
        Serial.println("✗ UDP begin failed!");
        return false;
    }
    
    Serial.print("✓ UDP listening on port: ");
    Serial.println(_localPort);
    return true;
}

// 接收数据函数（保持不变）
UdpData UdpReceiver::receive() 
{
    UdpData data = {0, 0, 0, false};
    int packetSize = _udp.parsePacket();
    
    if (packetSize) 
    {
        int len = _udp.read(_packetBuffer, sizeof(_packetBuffer));
        
        if (len == 5) 
        {
            memcpy(&data.cx, _packetBuffer, 2);
            memcpy(&data.cy, _packetBuffer + 2, 2);
            data.flag = _packetBuffer[4];
            data.isValid = true;
        } 
        else 
        {
            Serial.printf("✗ Invalid packet length: %d bytes\n", len);
        }
    }
    return data;
}

// 获取本地IP地址
IPAddress UdpReceiver::getLocalIP() 
{
    return WiFi.softAPIP();
}
