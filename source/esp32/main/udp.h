#ifndef UDP_H
#define UDP_H

#include <WiFi.h>
#include <WiFiUdp.h>

// ===== 协议类型 =====
#define PKT_TYPE_IP    0xA5   // K230 → ESP32：上报IP
#define PKT_TYPE_DATA  0xA6   // K230 → ESP32：坐标数据

struct UdpData {
    uint16_t cx;
    uint16_t cy;
    uint8_t  flag;
    bool     isValid;        // 是否收到有效坐标
};

class UdpReceiver {
private:
    const char* _ssid;
    const char* _password;
    unsigned int _localPort;

    WiFiUDP _udp;
    uint8_t _packetBuffer[64];

    IPAddress _clientIP;
    uint16_t  _clientPort;
    bool      _isReceivedIP;

public:
    UdpReceiver(const char* ssid, const char* password, unsigned int port = 8080);

    void stop();
    bool begin();

    UdpData receive();

    bool sendFlag(uint8_t flag);

    IPAddress getClientIP();
    uint16_t  getClientPort();
    bool      isReceivedIP();

    void flush();
    IPAddress getLocalIP();
};

#endif
