#ifndef UDP_H
#define UDP_H

#include <WiFi.h>
#include <WiFiUdp.h>

struct UdpData {
    uint16_t cx;      // X坐标
    uint16_t cy;      // Y坐标
    uint8_t flag;     // 标志位
    bool isValid;     // 数据是否有效
};

class UdpReceiver {
  private:
      const char* _ssid;
      const char* _password;
      unsigned int _localPort;
      WiFiUDP _udp;
      uint8_t _packetBuffer[64];
  
  public:
      UdpReceiver(const char* ssid, const char* password, unsigned int port = 8080);
      
      void stop();
       
      bool begin();
      
      UdpData receive();
  
      void flush();
      
      IPAddress getLocalIP();
};

#endif
