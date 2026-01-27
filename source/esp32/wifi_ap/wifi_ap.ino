#include <WiFi.h>
#include <WiFiUdp.h>

// 1. 修改为你的WiFi名称和密码
const char* ssid = "esp_ap";
const char* password = "12345678";

// 2. 设置UDP监听端口
unsigned int localPort = 8080;
WiFiUDP udp;                    // UDP对象
uint8_t packetBuffer[64];       // 接收缓冲区

void setup() {
  Serial.begin(115200);
  
  WiFi.softAP(ssid, password);
  
  Serial.println("✓ ESP32 AP Mode Started");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());  // 固定为192.168.4.1
  
  udp.begin(localPort);
  Serial.print("✓ UDP listening on port: ");
  Serial.println(localPort);
}

void loop() {
  // 检查是否有UDP数据包到达
  int packetSize = udp.parsePacket();
  if (packetSize) {
    // 读取数据包内容
    int len = udp.read(packetBuffer, sizeof(packetBuffer));
    Serial.println("1");
    if (len == 5) {
      uint16_t cx, cy;
      uint8_t flag;
      
      memcpy(&cx, packetBuffer, 2);
      memcpy(&cy, packetBuffer + 2, 2);
      flag = packetBuffer[4];

      // 显示接收到的数据
      Serial.printf("(%d, %d)\n", cx, cy);
    }
  }
}
