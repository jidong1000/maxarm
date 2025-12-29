import time
from PWMServo import PWMServo

# 控制单个PWM舵机转动

pwm = PWMServo()
pwm.work_with_time()

if __name__ == '__main__':
  
  angle = 90
  width = 11.1 * angle + 500;
  pwm.run(1, width, 100) # 设置1号PWM舵机脉宽500，运行时间1000毫秒(PWM舵机无法读取当前位置，所以首次运行，运行时间不可控)
  time.sleep_ms(2000) # 延时2000毫秒
  
 






