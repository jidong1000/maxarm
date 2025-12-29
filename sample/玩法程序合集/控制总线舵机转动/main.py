import time
from BusServo import BusServo

# 控制总线舵机转动例程

bus_servo = BusServo() 

if __name__ == '__main__':
  
  # ID:1->bottom; 2->left; 3->right
  
  angle = 120
  width = int(4.2 * angle)
  
  bus_servo.run(2, width, 1000)
  time.sleep_ms(1000)         # 延时1000毫秒  
  
  bus_servo.run(3, width, 1000)
  time.sleep_ms(1000)         # 延时1000毫秒




