#include <Arduino.h>
#include "PID.h"
#include "ESPMax.h"
#include "Buzzer.h"
#include "SuctionNozzle.h"
#include "LobotSerialServoControl.h"
#include "udp.h"
#include "_espmax.h"

#define LED_BUILTIN 2 // 定义LED控制引脚

// PID控制器
arc::PID<double> x_pid(0.041, 0.0001, 0.021);  
arc::PID<double> y_pid(0.041, 0.0001, 0.021);

// UDP接收器
UdpReceiver udpReceiver("esp_ap", "12345678", 8080);

// 全局变量
float p1 = 0, p2 = 0, p3 = 0;
float current_pos[3] = {0, -(L1 + L3 + L4), (L0 + L2)};
int stable_count = 0;
int target_x = 320, target_y = 420;
uint8_t flag_target_locked = 0;

void setup() {
  // 初始化硬件
  Buzzer_init();
  ESPMax_init();
  Nozzle_init();
  setBuzzer(100);
  
  // 初始化通信
  Serial.begin(115200);
  udpReceiver.begin();

  // 初始化引脚LED_BUILTIN输出模式
  pinMode(LED_BUILTIN, OUTPUT); 
  
  // 机械臂归位
  go_home(1500);
  delay(1500);
  
}

void loop() {
  // ===== 处理UDP数据 =====
  UdpData data = udpReceiver.receive();
  
  if (data.isValid && flag_target_locked == 0)   
  {
    Serial.printf("%d %d\r\n", data.cx, data.cy);
    int color_x = data.cx;
    int color_y = data.cy;

    // PID计算
    x_pid.setTarget(target_x);
    x_pid.setInput(color_x);
    float dx = x_pid.getOutput();
    
    y_pid.setTarget(target_y);
    y_pid.setInput(color_y);
    float dy = y_pid.getOutput();

    // 输出限幅,防止积分饱和
    dx = constrain(dx, -15.0, 15.0);  
    dy = constrain(dy, -15.0, 15.0);

    // 更新位置
    current_pos[0] -= dx;
    current_pos[1] -= dy;

    // 位置限幅
    current_pos[0] = constrain(current_pos[0], -100, 100);
    current_pos[1] = constrain(current_pos[1], -240, -60);
    
    // 驱动机械臂
    set_position(current_pos, 50);

    // 稳定性检测
    float error_x = target_x - color_x;
    float error_y = target_y - color_y;

    if (fabs(error_x) < 10 && fabs(error_y) < 10) 
    {
      if (++stable_count > 10) 
      { 
        stable_count = 0;
        setBuzzer(100);  // 到位提示音
        flag_target_locked = 1;
        Serial.println("目标锁定！");
      }
    } 
    else 
    {
      stable_count = 0;  // 重置计数器
    }
  }
  else if(flag_target_locked == 1)
  {
    // 打开LED
    digitalWrite(LED_BUILTIN, HIGH);   
    delay(1000);     

    // 关闭LED                 
    digitalWrite(LED_BUILTIN, LOW);    
    delay(1000);                       
  }
}
