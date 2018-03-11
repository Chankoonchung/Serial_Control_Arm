#include <Servo.h>               //声明舵机库
String command = "";   //声明一个字符串数组
#define SER1_BAUD 115200
#define  BEEP_PIN    12          //定义蜂鸣器引脚D12
#define  SERVO_MS_BETWEEN       20.000
#define NUM  6                //宏定义伺服舵机的个数  
Servo myservo[NUM];           //创建舵机伺服对象数组
unsigned int servo_index;       //声明舵机变量 （舵机号）
static unsigned long last_time = 0;
//static unsigned int z=0;
unsigned int delay_ms = 1500;
unsigned int time_max = 0;
//unsigned int time2=0;
byte servo_pin[NUM] = {10, A2, A3, A0, A1, 7};//定义一个数组，存储引脚数字

typedef struct {  //舵机结构体变量声明
  unsigned int aim = 1500;   //舵机目标值
  float cur = 1500;          //舵机当前值
  unsigned  int time1 = 1500;//舵机执行时间
  float inc = 0.00;         //舵机值增量，以20ms为周期
} duoji_t;


duoji_t duoji_doing[NUM];           //用结构体变量声明一个舵机变量数组
void getcommand();                   //读取指令
void move();                   //舵机写入指令
void check();                          //检验函数，检测执行动作后是否在初始位置
void initialize();                     //初始化函数
void get_status();                     //获取所有舵机角度
unsigned char handle_ms_between( unsigned long *time_ms, unsigned int ms_between);
void setup() {
  for (byte i = 0; i < NUM; i++)
  { //循环使相应的舵机伺服对象连接到对应的引脚
    myservo[i].attach(servo_pin[i]);
  }
  for (byte i = 0; i < NUM; i++)
  {
    myservo[i].writeMicroseconds(duoji_doing[i].aim);//循环写入初始PWM目标值
  }
  Serial.begin(SER1_BAUD);
  //beep();
  Serial.println("uart1 init OK.");

}

void loop()
{
  //beep();
  //Serial.begin(SER1_BAUD);
  //setup();
  //initialize();
  getcommand();
  move();
  //delay(z);
  //check();
}

void getcommand() {
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

          if ((index >= NUM)
              || (pwm > 2500)
              || (pwm < 500)) {          //如果舵机号 和 PWM数值超出约定值，则跳出不处理
            Serial.println("Error!Please input right data.");
            break;
          }
          //检测完后赋值
          duoji_doing[index].aim = pwm;         //舵机PWM赋值
          duoji_doing[index].time1 = time1;      //舵机执行时间赋值
          float pos_err = duoji_doing[index].aim - duoji_doing[index].cur;
          duoji_doing[index].inc = (pos_err * 1.000) / (duoji_doing[index].time1 / SERVO_MS_BETWEEN); //根据时间计算舵机PWM增量
          time_max = max(time_max, time1);
          //time2=time_max;
          //z=time1+5000;
#if 1 //调试的时候读取数据用  0/1
          Serial.print("index = ");
          Serial.println(index);
          Serial.print("pwm = ");
          Serial.println( duoji_doing[index].aim);
          Serial.print("time = ");
          Serial.println(duoji_doing[index].time1);
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
      for (byte i = 0; i < NUM; i++)
      {
        duoji_doing[i].aim =  (int)duoji_doing[i].cur;
        myservo[i].writeMicroseconds((int)duoji_doing[i].aim);
        //initialize();
        /*for(byte k=0;k<NUM;k++)
          {
          duoji_doing[k].aim=1500;
          }*/
      }
    }



    else if (command[0] == 'r' && (command[1] == 'e') && (command[2] == 's')
             && (command[3] == 'e') && (command[4] == 't')) { //reset the arm
      //for(byte i = 0; i < NUM; i++)

      //pwm=time1=index=0;

      //initialize();
      for (byte k = 0; k < NUM; k++)
      {
        duoji_doing[k].aim = 1500;

        duoji_doing[k].aim = 1500;         //舵机PWM赋值
        duoji_doing[k].time1 = 1000;      //舵机执行时间赋值
        float pos_err = duoji_doing[k].aim - duoji_doing[k].cur;
        duoji_doing[k].inc = (pos_err * 1.000) / (duoji_doing[k].time1 / SERVO_MS_BETWEEN); //根据时间计算舵机PWM增量
        time_max = max(time_max, time1);
      }

      Serial.println("The arm has been initialized!");
      pwm = time1 = index = 0;
    }

    else if (command[0] == 'p' && (command[1] == 'i') && (command[2] == 'n')
             && (command[3] == 'g')) { //ping指令
      //for(byte i = 0; i < NUM; i++)
      {
        /*duoji_doing[i].aim =  (int)duoji_doing[i].cur;
          myservo[i].writeMicroseconds((int)duoji_doing[i].aim); */
        get_status();
      }
    }
    //index = pwm = time1 = 0;

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
void move() {
  static unsigned long systick_ms_bak = 0;
  //static unsigned int w=0;
  if (!handle_ms_between(&systick_ms_bak, SERVO_MS_BETWEEN))return;
  //Serial.end();    //关闭串口（避免串口中断影响舵机处理过程）
  for (byte i = 0; i < NUM; i++) {
    if (abs( duoji_doing[i].aim - duoji_doing[i].cur) == 0 )
    {
      //z=z+5000;
      //Serial.println("Action completed.");
    }
    else if (abs( duoji_doing[i].aim - duoji_doing[i].cur) <= abs (duoji_doing[i].inc) )
    {
      duoji_doing[i].cur = duoji_doing[i].aim;
      myservo[i].writeMicroseconds((int)duoji_doing[i].aim);
      //z++;
      //z=z+5000;
      Serial.println("Action completed.");//test if servo doesn't work.
    }
    else
    {
      duoji_doing[i].cur +=  duoji_doing[i].inc;
      myservo[i].writeMicroseconds((int)duoji_doing[i].cur);
      //z++;
      //z=z+5000;
      //Serial.println("Action completed.");
    }
    /******************************************************************************/
    //Serial.begin(SER1_BAUD);
    /*for(byte j=0;j<NUM;j++){
      if(duoji_doing[j].aim!=1500){
        w++;
      if(w>0)
      {
        delay(5000);//delay for 5s
        initialize();
        //Serial.end();
        w=0;
        break;
      }
      if(w==0)
      {//w=0;
      break;}
      //w=0;
      }
      }*/
    /*******************************************************************************/

    /*z=z+5000;
      delay(z);
      for(byte j=0;j<NUM;j++){
      myservo[j].writeMicroseconds(1500);
      }
      z=0;*/
    /*Serial.begin(SER1_BAUD);
      for(byte j=0;j<NUM;j++){
      if(duoji_doing[j].aim!=1500){
        w++;
      if(w>0)
      {
        delay(5000);//delay for 5s
        initialize();
        //Serial.end();
        w=0;
        break;
      }
      if(w==0)
      {//w=0;
      break;}
      //w=0;
      }
      }*/

    //w=0;
    //Serial.begin(SER1_BAUD); //开启串口

  }
  /* for(byte j=0;j<NUM;j++)
    {
      if(myservo[j].read()<80||myservo[j].read()>100)
      {
        delay(time2+5000);
        initialize();
      }
    }//输出舵机角度
    time2=0;*/
}

unsigned char handle_ms_between( unsigned long *time_ms, unsigned int ms_between) {
  if (millis() - *time_ms < ms_between) {
    return 0;
  } else {
    *time_ms = millis();
    return 1;
  }
}

void beep() {//u8 times, u8 frequency
  //for(byte i = 0; i < times; i++ ) {
  /*digitalWrite(BEEP_PIN, LOW);
    delay(1);*/
  pinMode(BEEP_PIN, OUTPUT);
  digitalWrite(BEEP_PIN, HIGH );
  Serial.println("Beep!");
  delay(500); //delay for 500ms
  digitalWrite(BEEP_PIN, LOW );
  //}
}

void initialize() { //初始化函数
  for (byte j = 0; j < NUM; j++)
  {
    //myservo[j].writeMicroseconds(1500);//循环写入初始PWM目标值
    //duoji_doing[j].aim=1500;
    //duoji_doing[j].time1=2000;
    myservo[j].writeMicroseconds(1500);
    //duoji_doing[j].aim=1500;
    //Serial.println( duoji_doing[j].aim);
  }

  Serial.println("The arm has been initialized!");
}

void check() {
  static unsigned int w = 0;
  for (byte j = 0; j < NUM; j++) {
    if (duoji_doing[j].aim != 1500) {
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
  for (byte i = 0; i < NUM; i++)
    Serial.println(myservo[i].read() );
}

