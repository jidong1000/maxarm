#include <Arduino.h>
#include "PID.h"
#include "ESPMax.h"
#include "Buzzer.h"
#include "SuctionNozzle.h"
#include "LobotSerialServoControl.h"
#include "udp.h"
#include "_espmax.h"

#define LED_BUILTIN               2                     //定义LED控制引脚
#define INITIAL_HEIGHT            L0 + L2 - 50          //定义起始 z坐标
#define RISE_HEIGHT               INITIAL_HEIGHT - 20   //定义拿起后上升高度
#define VALVE_HEIGHT              105                   //定义valve的下降高度
#define END_POSITION_Y_DIS        150                   //定义放置区 y坐标
#define END_POSITION_VALVE_X_DIS  150                   //定义放置区valve x坐标

// PID控制器
float Kp = 0.045;
float Ki = 0;
float Kd = 0.02;
arc::PID<double> x_pid(Kp, Ki, Kd);  
arc::PID<double> y_pid(Kp, Ki, Kd);

// UDP接收器
UdpReceiver udpReceiver("esp_ap", "12345678", 8080);

// 全局变量
float p1 = 0, p2 = 0, p3 = 0;
float current_pos[3] = {0, -(L1 + L3 + L4), INITIAL_HEIGHT};
int stable_count = 0;
int target_x = 320, target_y = 405;
bool flag_target_locked = false; //判断是否xy调整完成
bool flag_z = false;  //防止多次进入z轴任务

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
  status_ready(1500);
  
}

void loop() {
  // ===== 处理UDP数据 =====
  UdpData data = udpReceiver.receive();

  if (data.isValid && (!flag_target_locked))   
  { 
    Serial.printf("%d %d\r\n", data.cx, data.cy);
    int color_x = data.cx;
    int color_y = data.cy;
    
    float dis_x, dis_y;

    // PID计算
    x_pid.setTarget(target_x);
    dis_x = target_x - color_x;
    if(abs(dis_x) > 80) //判断是大距离还是小距离
    {
       x_pid.setKd(0);
       x_pid.setInput(color_x);
    }
    else
    {
       x_pid.setKd(Kd);
       x_pid.setInput(color_x); 
    }
    float dx = x_pid.getOutput();
    
    y_pid.setTarget(target_y);
    dis_y = target_y - color_y;
    if(abs(dis_y) > 80) //判断是大距离还是小距离
    {
       y_pid.setKd(0);
       y_pid.setInput(color_y);
    }
    else
    {
       y_pid.setKd(Kd);
       y_pid.setInput(color_y); 
    }
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
  else if(flag_target_locked)
  {
    // 打开LED
    digitalWrite(LED_BUILTIN, HIGH); 

    if(!flag_z)
    {
      //判断下降距离
      float end_pos[3];
      switch(data.flag)
      {
        case 1:   
          flag_z = 1;
          current_pos[2] -= VALVE_HEIGHT;
          end_pos[0] = ORIGIN[0] + END_POSITION_VALVE_X_DIS;
          end_pos[1] = ORIGIN[1] + END_POSITION_Y_DIS;
          break;
      }
      
      //执行下降
      set_position(current_pos, 2000);
      delay(2000);
      
      delay(500);  //缓冲，让吸盘和物体充分接触 
      Pump_on();
      delay(1000);  //给气泵足够多的时间去吸

      //上升到固定坐标
      current_pos[2] = RISE_HEIGHT; 
      set_position(current_pos, 1000);
      delay(1000);
    
      //放置到放置区内
      end_pos[2] = current_pos[2];
      set_position(end_pos, 2000);
      delay(2000);

      //放下并放气
      current_pos[0] = end_pos[0]; current_pos[1] = end_pos[1]; current_pos[2] = INITIAL_HEIGHT - VALVE_HEIGHT + 10;
      set_position(current_pos, 2000);
      delay(2000);
      Pump_off();

      //重置目标检测开关
      flag_target_locked = false;   
    }
    
    // 关闭LED并复位机械臂                 
    digitalWrite(LED_BUILTIN, LOW); 
    status_ready(1500);   
    delay(1500);
    current_pos[0] = 0; current_pos[1] = -(L1 + L3 + L4); current_pos[2] = INITIAL_HEIGHT;   
    udpReceiver.flush();  //重要，清空udp缓存                    
  }
}
