/****************************************************************************
  Project:Automatic_robot
  Author:Chan
  Version:v1.0
  Excemple command:#1P2000T2000!#2P750T2000!#3P600T2000!#5P600T2000!

****************************************************************************/
#include <Servo.h>               //声明舵机库
String command = "";   //声明一个字符串数组
#define SERIAL_BAUD 115200
#define  BEEP_PIN    12          //定义蜂鸣器引脚D12
#define  SERVO_MS       20.000
#define SERVO_NUM  6                //宏定义伺服舵机的个数  
Servo myservo[SERVO_NUM];           //创建舵机伺服对象数组
unsigned int servo_index;       //声明舵机变量 （舵机号）
static unsigned long last_time = 0;
//static unsigned int z=0;
unsigned int delay_ms = 2000;
unsigned int time_max = 0;
//unsigned int time2=0;
byte servo_pin[SERVO_NUM] = {10, A2, A3, A0, A1, 7};//定义一个数组，存储引脚数字


typedef struct {  //舵机结构体变量声明
  unsigned int aim = 1500;   //舵机目标值
  int initaim[5] = {1500, 900, 700, 600, 1500};
  float cur = 1500;          //舵机当前值
  unsigned  int time1 = 2000;//舵机执行时间
  float inc = 0.00;         //舵机值增量，以20ms为周期
} duoji_t;


duoji_t servo_doing[SERVO_NUM];           //用结构体变量声明一个舵机变量数组
void handle_comm();                   //读取指令
void handle_servo();                   //舵机写入指令
void check();                          //检验函数，检测执行动作后是否在初始位置
void initialize();                     //初始化函数
void get_status();                     //获取所有舵机角度
unsigned char handle_ms_between( unsigned long *time_ms, unsigned int ms_between);
void setup() {
  //guide
  pinMode(7, OUTPUT); // Enable
  pinMode(9, OUTPUT); // Step
  pinMode(8, OUTPUT); // Dir
  digitalWrite(7, LOW); // Set Enable low
  //arm
  for (byte i = 0; i < SERVO_NUM; i++)
  { //循环使相应的舵机伺服对象连接到对应的引脚
    myservo[i].attach(servo_pin[i]);
  }
  for (byte i = 0; i < 5; i++) {
    myservo[i].writeMicroseconds(servo_doing[i].initaim[i]);//循环写入初始PWM目标值
  }
  /*arm(1,900,1000);
    arm(2,600,1000);
    arm(3,2400,1000);
    arm(0,1500,1000);
    arm(4,1500,1000);
    arm(5,1500,1000);*/

  Serial.begin(SERIAL_BAUD);
  //beep();
  Serial.println("Start!");

}

void loop()
{
  //initialize();
  handle_comm();
  handle_servo();
  //delay(z);
  //check();
}

void handle_comm() {
  static unsigned int index = 0, time1 = 0, pwm = 0, i;
  static char get_ok = 0, get_byte;
  unsigned int len;

  while (Serial.available())  {     //如果串口有数据
    //beep();
    get_byte = char(Serial.read());
    command  += get_byte;
    delayMicroseconds(200);

  }
  if (command.length() > 0) {    //如果串口有数据
    time_max = 0;
    if ((command[0] == '#')) {  //解析以“#”开头的指令
      len = command.length();  //获取串口接收数据的长度
      index = 0; pwm = 0; time1 = 0;     //3个参数初始化
      for (i = 0; i < len; i++) {             //如果数据没有接收完
        if (command[i] == '#') {        //判断是否为起始符“#”
          i++;                                  //下一个字符
          while ((command[i] != 'P') && (i < len)) { //判断是否为#后面的数字检测完
            index = index * 10 + command[i] - '0'; //记录P之前的数字
            i++;
          }
          i--;                          //因为上面i多自增一次，所以要减去1个
        } else if (command[i] == 'P') { //检测是否为“P”
          i++;
          while ((command[i] != 'T') && (i < len)) { //P之后的数字检测并保存
            pwm = pwm * 10 + command[i] - '0';
            i++;
          }
          i--;
        } else if (command[i] == 'T') { //判断是否为“T”
          i++;
          while ((command[i] != '!') && (i < len)) {
            time1 = time1 * 10 + command[i] - '0';  //将T后面的数字保存
            i++;
          }

          if ((index >= SERVO_NUM)
              || (pwm > 2500)
              || (pwm < 500)) {          //如果舵机号 和 PWM数值超出约定值，则跳出不处理
            Serial.println("Error!Please input right data.");
            break;
          }
          //检测完后赋值
          servo_doing[index].aim = pwm;         //舵机PWM赋值
          servo_doing[index].time1 = time1;      //舵机执行时间赋值
          float pos_err = servo_doing[index].aim - servo_doing[index].cur;
          servo_doing[index].inc = (pos_err * 1.000) / (servo_doing[index].time1 / SERVO_MS); //根据时间计算舵机PWM增量
          time_max = max(time_max, time1);
#if 1 //调试的时候读取数据用  0/1
          Serial.print("index = ");
          Serial.println(index);
          Serial.print("pwm = ");
          Serial.println( servo_doing[index].aim);
          Serial.print("time = ");
          Serial.println(servo_doing[index].time1);
          Serial.print("time_max = ");
          Serial.println(time_max);
#endif
          index = pwm = time1 = 0;
        }
        //Serial.println("Command recieved.");//check.etc:#3P600T5000
      }
    }

    else if (command[0] == '$' && (command[1] == 'D') && (command[2] == 'S')
             && (command[3] == 'T') && (command[4] == '!')) { //解析以"$"开头的指令
      for (byte i = 0; i < SERVO_NUM; i++)
      {
        servo_doing[i].aim =  (int)servo_doing[i].cur;
        myservo[i].writeMicroseconds((int)servo_doing[i].aim);
        //initialize();
        /*for(byte k=0;k<SERVO_NUM;k++)
          {
          servo_doing[k].aim=1500;
          }*/
      }
    }


    //reset the arm
    else if (command[0] == 'r' && (command[1] == 'e') && (command[2] == 's')
             && (command[3] == 'e') && (command[4] == 't')) {
      for (byte k = 0; k < 5; k++)
      {
        servo_doing[k].aim = servo_doing[k].initaim[k];

        // servo_doing[k].aim = 1500;         //舵机PWM赋值
        servo_doing[k].time1 = 1000;      //舵机执行时间赋值
        float pos_err = servo_doing[k].aim - servo_doing[k].cur;
        servo_doing[k].inc = (pos_err * 1.000) / (servo_doing[k].time1 / SERVO_MS); //根据时间计算舵机PWM增量
        time_max = max(time_max, time1);
      }

      Serial.println("The arm has been initialized!");
      pwm = time1 = index = 0;
    }

    //ping the arm and get the angle
    else if (command[0] == 'p' && (command[1] == 'i') && (command[2] == 'n')
             && (command[3] == 'g')) {
      //for(byte i = 0; i < SERVO_NUM; i++)
      {
        /*servo_doing[i].aim =  (int)servo_doing[i].cur;
          myservo[i].writeMicroseconds((int)servo_doing[i].aim); */
        get_status();
      }
    }
    //index = pwm = time1 = 0;

    else if (command[0] == 'f' && (command[1] == 'r') && (command[2] == 'e')
             && (command[3] == 'e')) { //free指令
      {
        servo_detach();
        Serial.println("Free mode.");
      }
    }

    else if (command[0] == 'h' && (command[1] == 'a') && (command[2] == 'r')
             && (command[3] == 'd')) { //hard指令
      {
        servo_attach();
        Serial.println("Hard mode.");
      }
    }

    //action to box 1
    else if (command[0] == '3')
    {
      //Serial.println("Received '3'");
      //guide();
      //delay(1000);
      //command="";
      arm(0, 1200, 2000);
      //Serial.println("Action completed!");
      index = pwm = time1 = 0;
    }

    last_time =  millis();
    //Serial.println(command);
    command = "";
    get_ok = 0;

  }

}

/*******************************************
  舵机处理函数，处理串口解析出来的数值
  然后让相应的舵机执行相应的动作值
******************************************/
void handle_servo() {
  static unsigned long systick_ms_bak = 0;
  if (!handle_ms_between(&systick_ms_bak, SERVO_MS))return;
  //Serial.end();    //关闭串口（避免串口中断影响舵机处理过程）
  for (byte i = 0; i < SERVO_NUM; i++) {
    if (abs( servo_doing[i].aim - servo_doing[i].cur) == 0 )
    {
      //z=z+5000;
      //Serial.println("Action completed.");
    }
    else if (abs( servo_doing[i].aim - servo_doing[i].cur) <= abs (servo_doing[i].inc) )
    {
      servo_doing[i].cur = servo_doing[i].aim;
      myservo[i].writeMicroseconds((int)servo_doing[i].aim);
      Serial.println("Action completed.");//test if servo doesn't work.
    }
    else
    {
      servo_doing[i].cur +=  servo_doing[i].inc;
      myservo[i].writeMicroseconds((int)servo_doing[i].cur);
    }
  }
}

unsigned char handle_ms_between( unsigned long *time_ms, unsigned int ms_between) {
  if (millis() - *time_ms < ms_between) {
    return 0;
  } else {
    *time_ms = millis();
    return 1;
  }
}

void beep() {
  pinMode(BEEP_PIN, OUTPUT);
  digitalWrite(BEEP_PIN, HIGH );
  Serial.println("Beep!");
  delay(500); //delay for 500ms
  digitalWrite(BEEP_PIN, LOW );
}

void initialize() { //初始化函数
  for (byte i = 0; i < 5; i++) {
    myservo[i].writeMicroseconds(servo_doing[i].initaim[i]);//循环写入初始PWM目标值
  }

  Serial.println("The arm has been initialized!");
}

void check() {
  static unsigned int w = 0;
  for (byte j = 0; j < SERVO_NUM; j++) {
    if (servo_doing[j].aim != 1500) {
      w++;
      if (w > 0)
      {
        delay(5000);//delay for 5s
        initialize();
        //Serial.end();
        w = 0;
        break;
      }
      if (w == 0)
      {
        break;
      }
      //w=0;
    }
  }
}

void get_status() {
  for (byte i = 0; i < SERVO_NUM; i++)
    Serial.println(myservo[i].read() );
}

void servo_detach() {
  for (byte i = 0; i < SERVO_NUM; i++) {
    myservo[i].detach();
  }
}

void servo_attach() {
  for (byte i = 0; i < SERVO_NUM; i++) {
    myservo[i].attach(servo_pin[i]);
  }
}

void guide(unsigned char v)
{
  while (Serial.available()) {
    //char v = 0;
    unsigned long t = 0;
    //v = Serial.read();
    int q = v - 48;
    if (-1 != v) {
      //Serial.print("val=");
      //Serial.println(v);
      t = q * 6000;
      //Serial.print("time=");
      //Serial.println(t);
      digitalWrite(8, LOW); // Set Dir high
      for (unsigned long x = 0; x < t * 10; x++) // Loop 200 times
      {
        digitalWrite(9, HIGH); // Output high
        delayMicroseconds(50); // Wait 1/2 a ms
        digitalWrite(9, LOW); // Output low
        delayMicroseconds(50); // Wait 1/2 a ms
      }
      delay(1000); // pause one second
      digitalWrite(8, HIGH); // Set Dir low
      for (unsigned long x = 0; x < t * 10 ; x++) // Loop 2000 times
      {
        digitalWrite(9, HIGH); // Output high
        delayMicroseconds(50); // Wait 1/2 a ms
        digitalWrite(9, LOW); // Output low
        delayMicroseconds(50); // Wait 1/2 a ms
      }
    }
  }
}

void arm(unsigned int in, unsigned int pw, unsigned int ti) //in:index pw:pwm ti:time1
{
  servo_doing[in].aim = pw;         //舵机PWM赋值
  servo_doing[in].time1 = ti;      //舵机执行时间赋值
  float pos_err = servo_doing[in].aim - servo_doing[in].cur;
  servo_doing[in].inc = (pos_err * 1.000) / (servo_doing[in].time1 / SERVO_MS); //根据时间计算舵机PWM增量
  time_max = max(time_max, ti);
  //time2=time_max;
  //z=time1+5000;
#if 0 //调试的时候读取数据用  0/1
  Serial.print("index = ");
  Serial.println(in);
  Serial.print("pwm = ");
  Serial.println( servo_doing[in].aim);
  Serial.print("time = ");
  Serial.println(servo_doing[in].time1);
  Serial.print("time_max = ");
  Serial.println(time_max);
#endif
  //in = pw = ti = 0;
}


