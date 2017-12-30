#include <LedControl.h>//led显示的头文件

int DIN = 12;//数据输入引脚
int CS =  11;//片选
int CLK = 10;//时钟

//定义按键，用来判断是否被按下
#define leftkey analogRead(A1)   
#define rightkey analogRead(A4)
#define upkey analogRead(A2)
#define downkey analogRead(A3)
#define resetkey analogRead(A5)

//定义了两个LedControl型变量（LedControl.h中定义了这个类）
LedControl lc=LedControl(13,CLK,CS,4);
LedControl ld=LedControl(DIN,CLK,CS,4);
//存储方块图形
byte allshape[19][4]={
  0x00,0x00,0x18,0x18,
  0x10,0x10,0x10,0x10,
  0x00,0x00,0x00,0x3c,
  0x00,0x08,0x18,0x10,
  0x00,0x00,0x30,0x18,
  0x00,0x10,0x18,0x08,
  0x00,0x00,0x18,0x30,
  0x00,0x08,0x08,0x18,
  0x00,0x00,0x38,0x08,
  0x00,0x18,0x10,0x10,
  0x00,0x00,0x20,0x38,
  0x00,0x10,0x10,0x18,
  0x00,0x00,0x08,0x38,
  0x00,0x18,0x08,0x08,
  0x00,0x00,0x38,0x20,
  0x00,0x00,0x10,0x38,
  0x00,0x08,0x18,0x08,
  0x00,0x00,0x38,0x10,
  0x00,0x10,0x18,0x10};
//欢迎界面
byte welcome[20]={0x00,0x00,0x00,0x00,
    0x3C,0x42,0xA5,0x81,0xA5,0x99,0x42,0x3C,
    0x3C,0x42,0xA5,0x81,0xA5,0x99,0x42,0x3C};//欢迎界面两个笑脸

byte staticdata[20]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//存储上一次的画面，
int shapewidth[19]={2,1,4,2,3,2,3,2,3,2,3,2,3,2,3,3,2,3,2};        
//各个图形的宽度属性
//int shapehig[19]=  {2,4,1,3,2,3,2,3,2,3,2,3,2,3,2,2,3,2,3};
//各个图形的高度属性
int shapex[19]=    {4,4,3,4,3,4,4,4,3,4,3,4,3,4,3,3,4,3,4};
//各个图形的位置属性  1——最左   8——最右
int shaperotate[19]={0,2,1,4,3,6,5,8,9,10,7,12,13,14,11,16,17,18,15};    
//旋转图形时，用于改变图形的编号以实现图形的切换


int shapenum; //形状
int normalspeed=500;    //系统下落速度，此值会随着玩家分数的增加而减小，相应的下落速度会增大
int fastspeed=50;      //当用户按下down键时，方块下落速度为此值
int sped;//下落速度，越大越慢，——————不是我打错了字母，是speed是保留字
int row;//方块位置属性：底部下落的高度，row=0时：刚刚出现，row=15时：下落到屏幕最底部
int x;//方块位置属性：x=1时 在最左面 x=8时 在最右面
int n=0;//判断是否是最开始
byte big[20];//用来动态显示的数组

void setup(){
 lc.shutdown(0,false);       //启动时，MAX72XX处于省电模式
 lc.setIntensity(0,8);       //将亮度设置为最大值
 lc.clearDisplay(0);         //清除显示

 ld.shutdown(0,false);       //启动时，MAX72XX处于省电模式
 ld.setIntensity(0,8);       //将亮度设置为最大值
 ld.clearDisplay(0);         //清除显示

 randomSeed(analogRead(A5));//下一个随机种子，以保证每次的随机数不唯一
}

void loop(){
    
    if(n==0)
    {
     do{
      printByte(welcome); 
      delay(500);
      }while(resetkey!=0);//显示标准脸（只有按下reset键才跳出，开始游戏。
    }
    
    row=4;//初始行数（0——19）为4
    sped=normalspeed;//慢速
    shapenum=random(0,18);//产生随机数
    x=shapex[shapenum]; //确定图形的初始位置
    while(checkdown()==0&&row!=19)//当不能下落时或者下落到嘴下面时跳出循环
   {
     ifileshape(shapenum);
     keyscan();
     down(shapenum);
     keyscan();
   }
   baoliu(); 
   if(staticdata[4]!=0)n=-1;//如果满，则下一循环从新来过。
   n++;
}

//点阵显示函数
void printByte(byte character [])
{
  int i = 0;
  for(i=0;i<8;i++)//第0——3行存了图形
  {
    lc.setColumn(0,i,character[i+4]);//从第4行开始扫描big数组并显示
    ld.setColumn(0,i,character[i+12]);//从第12行开始扫描big数组并显示
  }
}

int checkleft()  //判断方块左移是否会遇到障碍物
{ 
  int m=0;
  for(int i=0;i<4;i++)
    if(staticdata[row-i]&(allshape[shapenum][3-i]<<1)!=0)m++;
  if(m!=0||x==1)return 1;  //返回1，说明有障碍
  else  return 0;                      //返回0，说明没有障碍
}

int checkright()  //判断方块右移是否会遇到障碍物
{
  int m=0;
  for(int i=0;i<4;i++)
   if((staticdata[row-i]&(allshape[shapenum][3-i]>>1))!=0)m++;
  if(m!=0||((shapewidth[shapenum]+x-1)==8))return 1;  //返回1，说明有障碍
  else  return 0;                      //返回0，说明没有障碍
}

int checkdown()  //判断方块下落是否会遇到障碍物
{
  int m=0;
  for(int i=0;i<4;i++)
   { 
    if((staticdata[row+1-i]&allshape[shapenum][3-i])!=0)
      m++;//&运算：二进制同位的数全为1时，运算结果该位为1顾可以判断下一行是否有障碍物
    }
  if(m!=0||row==19) return 1;//
  else return 0;            //m==0，说明不会遇到障碍物
}

int checkup()
{
  int m=0;
  for(int i=0;i<4;i++)
  {
    if((staticdata[row-i]&(allshape[shaperotate[shapenum]][3-i]))!=0)
    m++;//同理
  }
  if(m!=0)return 1;
  else return 0;
}

void ifileshape(int num)
{
   fuyu();//把保留的静态图像数据存入动态显示的数组里
   for(int i=0;i<4;i++)
    {
      if(x>=shapex[num])//说明方块进行了右移（x-shapex[num]）次
      big[row-i]=((allshape[num][3-i]>>(x-shapex[num]))|staticdata[row-i]);
      else //说明方块进行了左移（shapex[num]-x）次
      big[row-i]=((allshape[num][3-i]<<(shapex[num]-x))|staticdata[row-i]);
    }
}

void fuyu()
{
    for(int i=0;i<20;i++)//对应给予
    big[i]=staticdata[i];
    Serial.println("fuyu");
}

void baoliu()
{
  int j,i=0;//把最后落定的图像保留下来，并删去满行
   for(j=0;j<20;j++)
    {
      if(big[j]!=0xff)
         staticdata[i++]=big[j];
    }
}

void down(int num)
{ 
  if(checkdown()!=0)return;//如果遇到障碍物，不进行操作
  else
  {
   fuyu();//同理
  for(int i=0;i<4;i++)
  {
    if(x>=shapex[num])
    big[row+1-i]=((allshape[num][3-i]>>(x-shapex[num]))|staticdata[row-i]);//“|”运算：二进制同位数字有1则为1，全0则0
    else 
    big[row+1-i]=((allshape[num][3-i]<<(shapex[num]-x))|staticdata[row-i]);
   }
    printByte(big);
    delay(sped);
    row++;
  }
}

void keyscan()          //键盘扫锚并执行用户输入的指令
{
  if(leftkey==0&&checkleft()==0)//如果按下左键，并且可以左移
  {
      x--;  //按键命令：方块左移一位
      ifileshape(shapenum);//写入数组
      printByte(big);      // 显示
   }
   / /同理
   if(rightkey==0&&checkright()==0)
  {
      x++;       //按键命令：方块右移一位
      ifileshape(shapenum);
      printByte(big);
   }       
    //同理
  if(upkey==0&&checkup()==0)
     {
       shapenum=shaperotate[shapenum]; //按键命令：将翻转后的图形编号赋给要显示的图形编号
       ifileshape(shapenum);
       printByte(big);
      }
  //同理
  if(downkey==0)
  {
      sped=fastspeed;  //按键命令：将方块下落速度加快
  }
}
