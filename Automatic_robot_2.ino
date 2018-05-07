/****************************************************************************
  Project:Automatic_robot
  Author:Chan
  Version:v2.0
  Changelog:Control servos by writing angle instead of writing microsecond
  Serial_Commands:
  #<index>A<angle>S<speed> --move the servo
  ping                     --get the status of servos
  reset                    --reset servos
  disconnect               --detach servos
  connect                  --attach servos
  +<index>                 --increase 10 degree for a servo
  -<index>                 --decrease 10 degree for a servo
****************************************************************************/
#include <VarSpeedServo.h>
String command = ""; // 声明一个字符串数组
#define SERIAL_BAUD 115200
#define BEEP_PIN 12 // 定义蜂鸣器引脚D12
#define SERVO_MS 20.000
#define SERVO_NUM 6 // 宏定义伺服舵机的个数
VarSpeedServo myservo[SERVO_NUM]; // 创建舵机伺服对象数组
unsigned int servo_index; // 声明舵机变量 （舵机号）
static unsigned long last_time = 0;
// static unsigned int z=0;
unsigned int delay_ms = 2000;
unsigned int time_max = 0;
// unsigned int time2=0;
byte servo_pin[SERVO_NUM] =
{
  10, A2, A3, A0, A1, 7
}; // 定义一个数组，存储引脚数字

unsigned int acache[6] =
{
  90, 90, 90, 90, 90, 90
};

typedef struct
{
  // 舵机结构体变量声明
  unsigned int aim = 1500; // 舵机目标值
  int initaim[5] =
  {
    1500, 900, 700, 600, 1500
  };
  float cur = 90; // 舵机当前值
  unsigned int time1 = 2000; // 舵机执行时间
  float inc = 0.00; // 舵机值增量，以20ms为周期
  uint8_t speed = 0;
  int value;
  bool state;
}

duoji_t;
duoji_t servo_doing[SERVO_NUM]; // 用结构体变量声明一个舵机变量数组
void handle_comm(); // 读取指令
void handle_servo(); // 舵机写入指令
void check(); // 检验函数，检测执行动作后是否在初始位置
void initialize(); // 初始化函数
void get_status(); // 获取所有舵机角度
void servo_detach();
void servo_attach();
void forward(char v);
void backward(char v);
unsigned char handle_ms_between(unsigned long * time_ms, unsigned int ms_between);
void setup()
{
  Serial.begin(SERIAL_BAUD);
  // guide
  pinMode(3, OUTPUT); // Enable7
  pinMode(5, OUTPUT); // Step9
  pinMode(6, OUTPUT); // Dir8
  digitalWrite(7, LOW); // Set Enable low
  // arm
  for (byte i = 0; i < SERVO_NUM; i++)
  {
    // 循环使相应的舵机伺服对象连接到对应的引脚
    myservo[i].attach(servo_pin[i]);
  }
  for (byte i = 0; i < 5; i++)
  {
    myservo[i].writeMicroseconds(servo_doing[i].initaim[i]); // 循环写入初始PWM目标值
  }
  // beep();
  Serial.println("Start!");
}

void loop()
{
  handle_comm();
}

void handle_comm()
{
  static unsigned int index, i;
  int freeindex[5];
  int value;
  int total;
  uint8_t speed;
  // bool wait;
  static char get_ok = 0, get_byte;
  unsigned int len;
  while (Serial.available())
  {
    // 如果串口有数据
    // beep();
    get_byte = char(Serial.read());
    command += get_byte;
    delayMicroseconds(200);
  }
  if (command.length() > 0)
  {
    // 如果串口有数据
    time_max = 0;
    // Free mode
    if ((command[0] == '#'))
    {
      // 解析以“#”开头的指令
      len = command.length(); // 获取串口接收数据的长度
      index = 0;
      value = 0;
      speed = 0; // 3个参数初始化
      for (i = 0; i < len; i++)
      {
        // 如果数据没有接收完
        if (command[i] == '#')
        {
          // 判断是否为起始符“#”
          i++; // 下一个字符
          while ((command[i] != 'A') && (i < len))
          {
            // 判断是否为#后面的数字检测完
            index = index * 10 + command[i] - '0'; // 记录A之前的数字
            i++;
          }
          i--; // 因为上面i多自增一次，所以要减去1个
        }
        else if (command[i] == 'A')
        {
          // 检测是否为“A”
          i++;
          while ((command[i] != 'S') && (i < len))
          {
            // A之后的数字检测并保存
            value = value * 10 + command[i] - '0';
            i++;
          }
          i--;
        }
        else if (command[i] == 'S')
        {
          // 判断是否为“S”
          i++;
          while ((command[i] != '!') && (i < len))
          {
            speed = speed * 10 + command[i] - '0'; // 将S后面的数字保存
            i++;
          }
          if ((index >= SERVO_NUM)
              || (speed > 255)
              || (speed < 1) || (value < 0))
          {
            // 如果舵机号 和 value数值超出约定值，则跳出不处理
            Serial.println("Error!Please input right data.");
            break;
          }
          // 检测完后赋值
          servo_doing[index].state = false;
          servo_doing[index].value = value; // 舵机value赋值
          acache[index] = value;
          servo_doing[index].speed = speed; // 舵机执行时间赋值
          float pos_err = servo_doing[index].value - servo_doing[index].cur;
          servo_doing[index].inc = (pos_err * 1.000) / (servo_doing[index].speed / SERVO_MS); // 根据时间计算舵机value增量
          // time_max = max(time_max, time1);
          for (i = 0; i < SERVO_NUM; i++)
          {
            myservo[i].write(servo_doing[i].value, servo_doing[i].speed, true);
            Serial.println("Sequential action completed.");
          }

#if 1 // 调试的时候读取数据用  0/1
          Serial.print("index = ");
          Serial.println(index);
          Serial.print("value = ");
          Serial.println(servo_doing[index].value);
          Serial.print("speed = ");
          Serial.println(servo_doing[index].speed);
          // Serial.print("time_max = ");
          // Serial.println(time_max);
#endif

          index = value = speed = 0;
        }
      }
    }
    else if ((command[0] == '$'))
    {
      // 解析以“#”开头的指令
      len = command.length(); // 获取串口接收数据的长度
      index = 0;
      value = 0;
      speed = 0; // 3个参数初始化
      for (i = 0; i < len; i++)
      {
        // 如果数据没有接收完
        if (command[i] == '$')
        {
          // 判断是否为起始符“#”
          i++; // 下一个字符
          while ((command[i] != 'A') && (i < len))
          {
            // 判断是否为#后面的数字检测完
            index = index * 10 + command[i] - '0'; // 记录A之前的数字
            i++;
          }
          i--; // 因为上面i多自增一次，所以要减去1个
        }
        else if (command[i] == 'A')
        {
          // 检测是否为“A”
          i++;
          while ((command[i] != 'S') && (i < len))
          {
            // S之后的数字检测并保存
            value = value * 10 + command[i] - '0';
            i++;
          }
          i--;
        }
        else if (command[i] == 'S')
        {
          // 判断是否为“S”
          i++;
          while ((command[i] != '!') && (i < len))
          {
            speed = speed * 10 + command[i] - '0'; // 将T后面的数字保存
            i++;
          }
          if ((index >= SERVO_NUM)
              || (speed > 255)
              || (speed < 1) || (value < 0))
          {
            // 如果舵机号 和 value数值超出约定值，则跳出不处理
            Serial.println("Error!Please input right data.");
            break;
          }
          // 检测完后赋值
          servo_doing[index].state = true;
          servo_doing[index].value = value; // 舵机value赋值
          acache[index] = value;
          servo_doing[index].speed = speed; // 舵机执行时间赋值
          float pos_err = servo_doing[index].value - servo_doing[index].cur;
          servo_doing[index].inc = (pos_err * 1.000) / (servo_doing[index].speed / SERVO_MS); // 根据时间计算舵机value增量
          // time_max = max(time_max, time1);
          for (i = 0; i < SERVO_NUM; i++)
          {
            myservo[i].write(servo_doing[i].value, servo_doing[i].speed, true);
            Serial.println("Sequential action completed.");
          }

#if 1 // 调试的时候读取数据用  0/1
          Serial.print("index = ");
          Serial.println(index);
          Serial.print("value = ");
          Serial.println(servo_doing[index].value);
          Serial.print("speed = ");
          Serial.println(servo_doing[index].speed);
          // Serial.print("time_max = ");
          // Serial.println(time_max);
#endif

          index = value = speed = 0;
        }
      }
    }
    else if ((command[0] == '+'))
    {
      // 解析以“+”开头的指令
      len = command.length(); // 获取串口接收数据的长度
      index = 0;
      value = 0;
      speed = 50; // 参数初始化
      for (i = 0; i < len; i++)
      {
        // 如果数据没有接收完
        if (command[i] == '+')
        {
          // 判断是否为起始符“+”
          i++; // 下一个字符
          while ((command[i] != '+') && (i < len))
          {
            // 判断是否为+后面的数字检测完
            index = index * 10 + command[i] - '0'; // 记录+之前的数字
            i++;
            value = acache[index] + 10;
          }
          if ((index >= SERVO_NUM)
              || (speed > 255)
              || (speed < 1) || (value < 0) || (value > 180))
          {
            // 如果舵机号 和 value数值超出约定值，则跳出不处理
            Serial.println("Error!Please input right data.");
            break;
          }
          // 检测完后赋值
          servo_doing[index].state = false;
          servo_doing[index].value = value; // 舵机value赋值
          servo_doing[index].speed = 50; // 舵机执行时间赋值
          float pos_err = servo_doing[index].value - servo_doing[index].cur;
          servo_doing[index].inc = (pos_err * 1.000) / (servo_doing[index].speed / SERVO_MS); // 根据时间计算舵机value增量
          // time_max = max(time_max, time1);
          myservo[index].write(servo_doing[index].value, servo_doing[index].speed, true);
          Serial.println("Calibration completed.");

#if 1 // 调试的时候读取数据用  0/1
          Serial.print("index = ");
          Serial.println(index);
          Serial.print("value = ");
          Serial.println(servo_doing[index].value);
          Serial.print("speed = ");
          Serial.println(servo_doing[index].speed);
          // Serial.print("time_max = ");
          // Serial.println(time_max);
#endif

          acache[index] = value;
          index = value = speed = 0;
        }
      }
    }
    else if ((command[0] == '-'))
    {
      // 解析以“-”开头的指令
      len = command.length(); // 获取串口接收数据的长度
      index = 0;
      value = 0;
      speed = 50; // 参数初始化
      for (i = 0; i < len; i++)
      {
        // 如果数据没有接收完
        if (command[i] == '-')
        {
          // 判断是否为起始符“-”
          i++; // 下一个字符
          while ((command[i] != '-') && (i < len))
          {
            // 判断是否为-后面的数字检测完
            index = index * 10 + command[i] - '0'; // 记录-之前的数字
            i++;
            value = acache[index] - 10;
          }
          if ((index >= SERVO_NUM)
              || (speed > 255)
              || (speed < 1) || (value < 0) || (value > 180))
          {
            // 如果舵机号 和 value数值超出约定值，则跳出不处理
            Serial.println("Error!Please input right data.");
            break;
          }
          // 检测完后赋值
          servo_doing[index].state = false;
          servo_doing[index].value = value; // 舵机value赋值
          servo_doing[index].speed = 50; // 舵机执行时间赋值
          float pos_err = servo_doing[index].value - servo_doing[index].cur;
          servo_doing[index].inc = (pos_err * 1.000) / (servo_doing[index].speed / SERVO_MS); // 根据时间计算舵机value增量
          // time_max = max(time_max, time1);
          myservo[index].write(servo_doing[index].value, servo_doing[index].speed, true);
          Serial.println("Calibration completed.");

#if 1 // 调试的时候读取数据用  0/1
          Serial.print("index = ");
          Serial.println(index);
          Serial.print("value = ");
          Serial.println(servo_doing[index].value);
          Serial.print("speed = ");
          Serial.println(servo_doing[index].speed);
          // Serial.print("time_max = ");
          // Serial.println(time_max);
#endif

          acache[index] = value;
          index = value = speed = 0;
        }
      }
    }
    // reset the arm
    else if (command[0] == 'r' && (command[1] == 'e') && (command[2] == 's')
             && (command[3] == 'e') && (command[4] == 't'))
    {
      initialize();
      // value = speed = index = 0;
    }
    // ping the arm and get the angle
    else if (command[0] == 'p' && (command[1] == 'i') && (command[2] == 'n')
             && (command[3] == 'g'))
    {
      get_status();
    }
    // index = pwm = time1 = 0;
    else if (command[0] == 'd' && (command[1] == 'i') && (command[2] == 's')
             && (command[3] == 'c') && (command[4] == 'o') && (command[5] == 'n')
             && (command[6] == 'n') && (command[7] == 'e') && (command[8] == 'c')
             && (command[9] == 't'))
    {
      // disconnect指令
      {
        servo_detach();
        Serial.println("Disconnected.");
      }
    }
    else if (command[0] == 'c' && (command[1] == 'o') && (command[2] == 'n')
             && (command[3] == 'n') && (command[4] == 'e') && (command[5] == 'c')
             && (command[6] == 't'))
    {
      // hard指令
      {
        servo_attach();
        Serial.println("Connected.");
      }
    }
    else if (command[0] == 'c' && (command[1] == 'l') && (command[2] == 'e')
             && (command[3] == 'a') && (command[4] == 'r'))
    {
      // clear the info
      {
        Serial.flush();
      }
    }
    //personal action
    // action to box 1
    /*
      else if (command[0] == '3')
      {
      // Serial.println("Received '3'");
      // guide(30);
      myservo[0].write(90, 10, false);
      myservo[1].write(151, 12, false);
      myservo[2].write(15, 12, false);
      myservo[3].write(179, 20, false);
      myservo[4].write(90, 10, false);
      myservo[5].write(110, 10, false);
      // arm(0,700,2000);
      // myservo[0].wait();//wait until action completed.
      Serial.println("Action1 completed!");
      delay(3000);
      myservo[5].write(180, 10, true);
      // acache[5] = myservo[5].read();
      // myservo[1].write(60, 10, true);
      // acache[1] = myservo[1].read();
      Serial.println("Action2 completed!");
      initialize();
      myservo[3].wait();
      forward('1');
      delay(1000);
      myservo[1].write(90, 12, false);
      myservo[3].write(160, 20, true);
      myservo[1].write(131, 12, true);
      myservo[3].write(115, 20, false);
      myservo[2].write(92, 20, false);
      myservo[1].write(151, 5, false);
      myservo[4].write(92, 10, false);
      myservo[1].wait();
      myservo[4].wait();
      myservo[5].write(110, 50, true);
      myservo[5].wait();
      delay(1000);
      initialize();
      myservo[1].wait();
      backward('1');
      delay(1000);
      // myservo[5].write(110, 10, false);
      acache[0] = myservo[0].read();
      acache[1] = myservo[1].read();
      acache[2] = myservo[2].read();
      acache[3] = myservo[3].read();
      acache[4] = myservo[4].read();
      // acache[5] = myservo[5].read();
      index = value = speed = 0;
      }
    */
    /*for(i=0;i<SERVO_NUM;i++){
      servo_doing[i].aim = 0;
      servo_doing[i].speed = 0;
      }*/
    last_time = millis();
    // Serial.println(command);
    command = "";
    get_ok = 0;
  }
}

unsigned char handle_ms_between(unsigned long * time_ms, unsigned int ms_between)
{
  if (millis() - * time_ms < ms_between)
  {
    return 0;
  }
  else
  {
    * time_ms = millis();
    return 1;
  }
}

void beep()
{
  pinMode(BEEP_PIN, OUTPUT);
  digitalWrite(BEEP_PIN, HIGH);
  Serial.println("Beep!");
  delay(500); // delay for 500ms
  digitalWrite(BEEP_PIN, LOW);
}

void initialize()
{
  // 初始化函数
  myservo[0].write(90, 15, false); // 循环写入初始value目标值
  myservo[1].write(34, 15, false);
  myservo[2].write(15, 15, false);
  myservo[3].write(5, 13, false);
  myservo[4].write(90, 15, false);
  for (byte i = 0; i < 5; i++)
  {
    acache[i] = myservo[i].read();
  }
  Serial.println("The arm has been initialized!");
}

void check()
{
  static unsigned int w = 0;
  for (byte j = 0; j < SERVO_NUM; j++)
  {
    if (servo_doing[j].aim != 1500)
    {
      w++;
      if (w > 0)
      {
        delay(5000); // delay for 5s
        initialize();
        // Serial.end();
        w = 0;
        break;
      }
      if (w == 0)
      {
        break;
      }
      // w=0;
    }
  }
}

void get_status()
{
  for (byte i = 0; i < SERVO_NUM; i++)
  {
    Serial.print("Servo[");
    Serial.print(i);
    Serial.print("]");
    Serial.print(":");
    Serial.println(myservo[i].read());
  }
}

void servo_detach()
{
  for (byte i = 0; i < SERVO_NUM; i++)
  {
    myservo[i].detach();
  }
}

void servo_attach()
{
  for (byte i = 0; i < SERVO_NUM; i++)
  {
    myservo[i].attach(servo_pin[i]);
  }
}

//guide action
void forward(char v)
{
  int q = v - 48;
  unsigned long t = 0;
  if (-1 != v) {
    Serial.print("val=");
    Serial.println(v);
    t = q * 6000;
    Serial.print("time=");
    Serial.println(t);
    digitalWrite(6, LOW); // Set Dir high
    Serial.println("Moving...");
    for (unsigned long x = 0; x < t * 10; x++) // Loop 200 times
    {
      digitalWrite(5, HIGH); // Output high
      delayMicroseconds(50); // Wait 1/2 a ms
      digitalWrite(5, LOW); // Output low
      delayMicroseconds(50); // Wait 1/2 a ms
    }
    Serial.println("Action completed.");
  }
}

//guide action
void backward(char v)
{
  int q = v - 48;
  unsigned long t = 0;
  if (-1 != v) {
    Serial.print("val=");
    Serial.println(v);
    t = q * 6000;
    Serial.print("time=");
    Serial.println(t);
    Serial.println("Backing...");
    digitalWrite(6, HIGH); // Set Dir low
    for (unsigned long x = 0; x < t * 10 ; x++) // Loop 2000 times
    {
      digitalWrite(5, HIGH); // Output high
      delayMicroseconds(50); // Wait 1/2 a ms
      digitalWrite(5, LOW); // Output low
      delayMicroseconds(50); // Wait 1/2 a ms
    }
    Serial.println("Action completed.");
  }
}
