

#include <reg52.h>
#include <string.h>
#include <intrins.h>
#define uchar unsigned char
#define uint unsigned int
#define FOSC 11.0592 // 系统晶振频率11.0592MHz
#define FACTOR 0.1720 // (距离计算因子)[(12 / FOSC) * 10^-6] * 340 / 2
	
/*主模块端口定义*/
sbit trig = P2^1; // 电平触发输入引脚
sbit echo = P2^0; // 回声输出引脚

/*LCD1602显示模块端口定义*/
sbit lcdrs = P2^6;
sbit lcdrw = P2^5;
sbit lcden = P2^7;
sbit led1  = P1^3;
sbit led2  = P1^4;
sbit beep  = P1^5;
sbit dj    = P2^3;
sbit key1 =  P1^0;
sbit key2 =	 P1^1;
sbit key3 =  P1^2;

/*主模块函数声明*/
void startUltra();
void timerInit();
void delayUs(uint xus);
void computeDis();

/*LCD1602显示模块函数声明*/
void delay(uint xms);
void writeCommand(uchar command);
void writeDataLCD(uchar dat);
void LCDInit();
void display(float dat,l,m,h);

/*定义全局变量*/
uint time = 0; // 记录回声高电平持续时间
uint distance = 0; // 存放距离
unsigned int sw,l,m,h,d;
uchar dis[3],djb;
uchar zifu[]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39};
uchar lb,mb,hb;
uint db=0;
/*主函数*/
void main(){
	d=160;
	l=50;
	m=70;
	h=120;
	sw=0;
	led1=1;
	led2=1;
	dj=0;
	beep=1;
	trig=0;
	LCDInit();
	timerInit();
	dj=1;
	while(1){
		echo = 0; // 接收回音端置0
		startUltra(); // 启动超声测距
		while(!echo); // 等待回音
		TR0 = 1; // 开始计时
		while(echo);
		TR0 = 0; // 停止计时
		computeDis();		
		distance=d-distance;// 计算距离
		display(distance,l,m,h);		// 显示距离
		if(djb==1)
		{
				dj=1;
				delay(50);
		}
		if(djb==0)
		{
				dj=0;
				delay(10);
		}
				
		if(distance<m)
		{dj=1;
			delay(50);
		}
		if(distance>=m)
		{	dj=0;delay(10);}
		if(distance<l)
			{led2= 1;
			led1 = 0;
			beep = 0;
		
			delay(10);
			}

		if(distance>h)
			{led2=1;
			led1 = 0;
			beep = 0;
			delay(10);}
		if(distance<=h&&distance>=l)
			{
			beep=1;
			led1=1;
			led2=0;
			delay(10);}	
		if(key1==0) {
		delay(10);
		if (key1==0)
		sw+=1;}
		if(sw==1) 
		{	if(key2==0) l+=1;
			if(key3==0) l-=1;}
		if(sw==2) 
		{	if(key2==0) m+=1;
			if(key3==0) m-=1;}
		if(sw==3) 
		{	if(key2==0) h+=1;
			if(key3==0) h-=1;
		}
		if(sw==4) sw=0;

	}
}
/*启动超声波测距*/
void startUltra(){ // 产生一个大于10us的高电平
	trig = 1;
	delayUs(15);
	trig = 0;
}

/*timer初始化函数*/
void timerInit(){
	SCON = 0x50;
	TMOD = 0x21;					 // 同时使用两个定时器
	TH0 = 0x00;						 // 置初值 
	TL0 = 0x00;
	TH1 = 0xFD;               // TH1:  初始值为0xFD  波特率：9600 晶振频率：11.0592MHz  
	TL1 = 0x00;
	ET0 = 1; 						// 开timer0溢出中断
	TR1 = 1;                  // TR1:  开启定时器1                         
	EA  = 1;                  //打开总中断
	ES  = 1;                  //打开串口中断
}
void uart_send_byte(unsigned char dat)
{
	SBUF = dat; // 将数据送到发送缓冲寄存器SBUF，一位一位的发送
	while(!TI); // 等待发送完毕 (发送完毕TI硬件置1)
	TI = 0;// 将TI清零，表示可以发送下一字节数据。
}
// 串口中断处理函数 （串口接收到数据，发送数据完毕都可以引起串口中断）
void uart_send_str(unsigned char *s)
{
	while(*s != '\0')// '\0':字符串结束标志
	{
		uart_send_byte(*s);// 发送1个字节数据，1个字符占8位，1字节
		s++;// 指向下一个字符
	}
}

void uart_interrupt(void) interrupt 4 		//也叫串行中断服务程序
{		unsigned char send_data1[]="ireceive" ;// 要发送的信息

	unsigned char recv_data;// 用来存放接收到的数据
	unsigned int send_data[3] ;
	unsigned char send_data2[3] ;// 要发送的信息

	send_data[0] = distance % 1000/100;
	send_data[1] = distance % 100 / 10;
	send_data[2] = distance % 10;
	send_data2[0] = zifu[send_data[0]];
	send_data2[1] = zifu[send_data[1]];
	send_data2[2] = zifu[send_data[2]];

	if(RI) //接收数据(1字节)完毕，RI会被硬件置1
	{
		RI = 0;            		// 将 接收中断标志位 清零(让串口可以继续接收数据)
		recv_data = SBUF;           	//读取接收到的数据，并存放到data
	if(recv_data==0xff){djb=0;}
	if(recv_data==0xfe){djb=1;}
	if(lb||mb||hb==1)
	{	if(recv_data==0x00) {db= 10*db+0;}
	if(recv_data==0x01){db= 10*db+1;}
	if(recv_data==0x02){db= 10*db+2;}
	if(recv_data==0x03){db= 10*db+3;}
	if(recv_data==0x04){db= 10*db+4;}
	if(recv_data==0x05){db= 10*db+5;}
	if(recv_data==0x06){db= 10*db+6;}
	if(recv_data==0x07){db= 10*db+7;}
	if(recv_data==0x08){db= 10*db+8;}
	if(recv_data==0x09){db= 10*db+9;}
	if(recv_data==0x0a){lb=0;l=db;db=0;}
  	if(recv_data==0x0b){mb=0;m=db;db=0;}
	if(recv_data==0x0c){hb=0;h=db;db=0;}
	if(recv_data==0x0d){hb=0;d=db;db=0;}
	}
		if(recv_data ==0x0a){
		lb=1;	}


		if(recv_data ==0x0b){
		mb=1;}

		if(recv_data ==0x0c){
			hb=1;	}


	
		
	    uart_send_str(send_data2); //收到数据之后，发送字符串"I received."给对方
	}
	if(TI)// 发送数据(1字节)完毕
	{
		TI = 0;// 将 发送中断标志位 清零(让串口可以继续发送数据)
	}
} 











/*timer0溢出中断服务程序*/
void timer0Service() interrupt 1{  // 计数溢出说明超出测距范围
echo = 0; // 强制关闭回音

		}

/*延时xus*/
void delayUs(uint xus){
	uint i;
	for(i = xus; i > 0; i--){
		_nop_();
	}
}


/*计算距离*/
void computeDis(){
	time = TH0 * 256 + TL0;
	distance = FACTOR * time;
	TH0 = 0x00; // 重装初值
	TL0 = 0x00;
}


uchar code table0[] = {"L:  M:  H:  N:  "};
uchar code table2[] = {"0123456789"};

uchar num = 0;


/*初始化LCD1602的设置*/
void LCDInit(){
	uchar i;
	lcden = 0; // 拉低使能端，准备产生使能高脉冲信号
	writeCommand(0x38); // 显示模式设置(16x2， 5x7点阵，8位数据接口)
	writeCommand(0x0c); // 开显示，不显示光标
	writeCommand(0x06); // 写一个字符后地址指针自动加1
	writeCommand(0x01); // 显示清零，数据指针清零
	
	/*LCD上电界面*/
	writeCommand(0x80); // 将数据指针定位到第一行首
	for(i = 0; i < strlen(table0); i++){
		writeDataLCD(table0[i]);
		delay(5);
	}

	writeCommand(0x80 + 0x40); // 将数据指针定位到第二行首

}

/*LCD显示函数*/
void display(float dat,l,m,h){
	uint disINT = (uint)dat;
	uchar  i;
	dis[0] = l % 1000 / 100;
	dis[1] = l % 100 / 10;
	dis[2] = l % 10;
	for(i = 0; i < 3; i++){
		writeCommand(0x80 + 0x40 + i);
		writeDataLCD(table2[dis[i]]);
		delay(5);
	}
	dis[0] = m % 1000 / 100;
	dis[1] = m % 100 / 10;
	dis[2] = m % 10;
	for(i = 0; i < 3; i++){
		writeCommand(0x80 + 0x40 + 4 + i);
		writeDataLCD(table2[dis[i]]);
		delay(5);
	}
	dis[0] = h % 1000 / 100;
	dis[1] = h % 100 / 10;
	dis[2] = h % 10;
	for(i = 0; i < 3; i++){
		writeCommand(0x80 + 0x40 + 8 + i);
		writeDataLCD(table2[dis[i]]);
		delay(5);
	}
	dis[0] = disINT % 1000 / 100;
	dis[1] = disINT % 100 / 10;
	dis[2] = disINT % 10;
	for(i = 0; i < 3; i++){
		writeCommand(0x80 + 0x40 + 12 + i);
		writeDataLCD(table2[dis[i]]);
		delay(5);
	}
	}


/*写指令函数*/
void writeCommand(unsigned char command)
	{
	lcdrs = 0; // 命令选择
	lcdrw = 0;
	P0 = command;
	delay(5);
	
	lcden = 1; // 产生一个正脉冲使能信号
	delay(5);
	lcden = 0;
}

/*写数据函数*/
void writeDataLCD(uchar dat){
	lcdrs = 1; // 数据选择
	lcdrw = 0;

	P0 = dat;
	delay(5);

	lcden = 1;
	delay(5);
	lcden = 0;
}

/*延时xms函数*/
void delay(uint xms){
	uint i, j;
	for(i = xms; i > 0; i--)
		for(j = 110; j > 0; j--);
}

