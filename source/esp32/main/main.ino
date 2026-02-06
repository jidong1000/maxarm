#include <Arduino.h>
#include "PID.h"
#include "ESPMax.h"
#include "Buzzer.h"
#include "SuctionNozzle.h"
#include "LobotSerialServoControl.h"
#include "udp.h"
#include "_espmax.h"

#define LED_BUILTIN                   2                     //定义LED控制引脚
#define INITIAL_HEIGHT                L0 + L2 - 50          //定义起始 z坐标
#define RISE_HEIGHT                   INITIAL_HEIGHT - 20   //定义拿起后上升高度

#define VALVE_HEIGHT                  109                   //定义valve的下降高度
#define END_POSITION_VALVE_Y_DIS      150                   //定义放置区valve y坐标距离
#define END_POSITION_VALVE_X_DIS      150                   //定义放置区valve x坐标距离

#define STUFF_HEIGHT                  83                    //定义valve的下降高度
#define END_POSITION_STUFF1_Y_DIS     80                    //定义放置区stuff1 y坐标距离
#define END_POSITION_STUFF1_X_DIS     150                   //定义放置区stuff1 x坐标距离
#define END_POSITION_STUFF2_Y_DIS     80                    //定义放置区stuff2 y坐标距离
#define END_POSITION_STUFF2_X_DIS     220                   //定义放置区stuff2 x坐标距离

#define BATTERY_HEIGHT                111                   //定义valve的下降高度
#define END_POSITION_BATTERY_Y_DIS    150                   //定义放置区valve y坐标距离
#define END_POSITION_BATTERY_X_DIS    220                   //定义放置区valve x坐标距离


// PID控制器
float Kp = 0.11;
float Ki = 0;
float Kd = 0.0001;
arc::PID<double> x_pid(Kp, Ki, Kd);  
arc::PID<double> y_pid(Kp, Ki, Kd);

// UDP接收器
UdpReceiver udpReceiver("esp_ap", "12345678", 8080);

// 全局变量
float p1 = 0, p2 = 0, p3 = 0;
float current_pos[3] = {0, -(L1 + L3 + L4), INITIAL_HEIGHT};
int stable_count = 0;
int target_x = 320, target_y = 417;
bool flag_target_locked = false; //判断是否xy调整完成
bool flag_z = false;  //防止多次进入z轴任务
bool flag_stuff = false;  //标志第几个stuff
uint8_t screenFlag = 0;
bool screenFlagUpdated = false;


void setup() {
  // 初始化硬件
  Buzzer_init();
  ESPMax_init();
  Nozzle_init();
  setBuzzer(100);
  
  // 初始化通信
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 32, 33);

  udpReceiver.begin();

  // 初始化引脚LED_BUILTIN输出模式
  pinMode(LED_BUILTIN, OUTPUT); 
  
  // 机械臂归位
  status_ready(1500);
  
}

void loop() {
  // ===== 处理UDP数据 =====
  UdpData data = udpReceiver.receive();

  handleSerialScreen();

  if(screenFlagUpdated)
  {
      screenFlagUpdated = false;
      udpReceiver.sendFlag(screenFlag);        
  }
  
  if (data.isValid && (!flag_target_locked))   
  { 
    int color_x = data.cx;
    int color_y = data.cy;
    
    Serial1.printf("page0.t2.txt=\"looking for target\"\xff\xff\xff");
    Serial.printf("%d %d %d\r\n", data.cx, data.cy, data.flag);
    
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

    if (fabs(error_x) < 5 && fabs(error_y) < 5) 
    {
      if (++stable_count > 5) 
      { 
        stable_count = 0;
        setBuzzer(100);  // 到位提示音
        flag_target_locked = 1;
        Serial1.printf("page0.t2.txt=\"placing\"\xff\xff\xff");
      }
    } 
    else 
    {
      stable_count = 0;  // 重置计数器
    }
  }
  //必须是if 不能是else if
  if(flag_target_locked)
  {
    // 打开LED
    digitalWrite(LED_BUILTIN, HIGH); 
    if(!flag_z)
    {
      flag_z = true;
      
      //判断下降距离
      float end_pos[3];
      switch(data.flag)
      {
        case 0:
          current_pos[2] -= STUFF_HEIGHT;
          if(!flag_stuff)
          {
            end_pos[0] = ORIGIN[0] + END_POSITION_STUFF1_X_DIS;
            end_pos[1] = ORIGIN[1] + END_POSITION_STUFF1_Y_DIS;  
          }
          else
          {
            end_pos[0] = ORIGIN[0] + END_POSITION_STUFF2_X_DIS;
            end_pos[1] = ORIGIN[1] + END_POSITION_STUFF2_Y_DIS;       
          } 
          flag_stuff = !flag_stuff;
          break;
        case 1:   
          current_pos[2] -= VALVE_HEIGHT;
          end_pos[0] = ORIGIN[0] + END_POSITION_VALVE_X_DIS;
          end_pos[1] = ORIGIN[1] + END_POSITION_VALVE_Y_DIS;
          break;
        case 2:   
          current_pos[2] -= BATTERY_HEIGHT;
          end_pos[0] = ORIGIN[0] + END_POSITION_BATTERY_X_DIS;
          end_pos[1] = ORIGIN[1] + END_POSITION_BATTERY_Y_DIS;
          break;
      }
      
      //执行下降
      set_position(current_pos, 2000);
      delay(2000);
      
      delay(500);  //缓冲，让吸盘和物体充分接触 
      Pump_on();
      delay(1500);  //给气泵足够多的时间去吸

      //上升到固定坐标
      current_pos[2] = RISE_HEIGHT; 
      set_position(current_pos, 1000);
      delay(1000);
    
      //放置到放置区内
      end_pos[2] = current_pos[2];
      set_position(end_pos, 2000);
      delay(2000);

      //放下并放气
      current_pos[0] = end_pos[0]; current_pos[1] = end_pos[1]; 
      switch(data.flag)
      {
        case 0: current_pos[2] = INITIAL_HEIGHT - STUFF_HEIGHT + 5;break;
        case 1: current_pos[2] = INITIAL_HEIGHT - VALVE_HEIGHT + 5;break;
        case 2: current_pos[2] = INITIAL_HEIGHT - BATTERY_HEIGHT + 5;break;
      }
      
      set_position(current_pos, 2000);
      delay(2000);
      Pump_off();
      delay(1000);
      //重置目标检测开关
      flag_target_locked = false;  
      flag_z = false;
      Serial1.printf("page0.t2.txt=\"finish\"\xff\xff\xff");
       
      // 关闭LED并复位机械臂  
      current_pos[2] = INITIAL_HEIGHT;
      set_position(current_pos, 500);  //防止撞到放好的物体
      delay(500);                 
      status_ready(1500);   
      delay(1500);
      current_pos[0] = 0; current_pos[1] = -(L1 + L3 + L4);   
      udpReceiver.flush();  //重要，清空udp缓存 
      Serial1.printf("page0.t2.txt=\"\"\xff\xff\xff");                   
    }
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void handleSerialScreen()
{
    static uint8_t state = 0;   // 0=等包头, 1=等数据, 2=等包尾
    static uint8_t data_flag = 0;
    
    while (Serial1.available())
    {
        uint8_t b = Serial1.read();
        
        switch (state)
        {
          case 0: // 等包头
              if (b == 0x2C)
                  state = 1; 
              break;
  
          case 1: // 收数据
              data_flag = b;
              state = 2;
              break;
  
          case 2: // 等包尾
              if (b == 0x5B)
              {
                  screenFlag = data_flag;
                  screenFlagUpdated = true;
                  digitalWrite(LED_BUILTIN, HIGH); 
              }
              state = 0; // 不管对不对，都回到等包头
              break;
        }
    }
}
