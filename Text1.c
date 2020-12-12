

#include <reg52.h>
#include <string.h>
#include <intrins.h>
#define uchar unsigned char
#define uint unsigned int
#define FOSC 11.0592 // ϵͳ����Ƶ��11.0592MHz
#define FACTOR 0.1720 // (�����������)[(12 / FOSC) * 10^-6] * 340 / 2
	
/*��ģ��˿ڶ���*/
sbit trig = P2^1; // ��ƽ������������
sbit echo = P2^0; // �����������

/*LCD1602��ʾģ��˿ڶ���*/
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

/*��ģ�麯������*/
void startUltra();
void timerInit();
void delayUs(uint xus);
void computeDis();

/*LCD1602��ʾģ�麯������*/
void delay(uint xms);
void writeCommand(uchar command);
void writeDataLCD(uchar dat);
void LCDInit();
void display(float dat,l,m,h);

/*����ȫ�ֱ���*/
uint time = 0; // ��¼�����ߵ�ƽ����ʱ��
uint distance = 0; // ��ž���
unsigned int sw,l,m,h,d;
uchar dis[3],djb;
uchar zifu[]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39};
uchar lb,mb,hb;
uint db=0;
/*������*/
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
		echo = 0; // ���ջ�������0
		startUltra(); // �����������
		while(!echo); // �ȴ�����
		TR0 = 1; // ��ʼ��ʱ
		while(echo);
		TR0 = 0; // ֹͣ��ʱ
		computeDis();		
		distance=d-distance;// �������
		display(distance,l,m,h);		// ��ʾ����
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
/*�������������*/
void startUltra(){ // ����һ������10us�ĸߵ�ƽ
	trig = 1;
	delayUs(15);
	trig = 0;
}

/*timer��ʼ������*/
void timerInit(){
	SCON = 0x50;
	TMOD = 0x21;					 // ͬʱʹ��������ʱ��
	TH0 = 0x00;						 // �ó�ֵ 
	TL0 = 0x00;
	TH1 = 0xFD;               // TH1:  ��ʼֵΪ0xFD  �����ʣ�9600 ����Ƶ�ʣ�11.0592MHz  
	TL1 = 0x00;
	ET0 = 1; 						// ��timer0����ж�
	TR1 = 1;                  // TR1:  ������ʱ��1                         
	EA  = 1;                  //�����ж�
	ES  = 1;                  //�򿪴����ж�
}
void uart_send_byte(unsigned char dat)
{
	SBUF = dat; // �������͵����ͻ���Ĵ���SBUF��һλһλ�ķ���
	while(!TI); // �ȴ�������� (�������TIӲ����1)
	TI = 0;// ��TI���㣬��ʾ���Է�����һ�ֽ����ݡ�
}
// �����жϴ����� �����ڽ��յ����ݣ�����������϶��������𴮿��жϣ�
void uart_send_str(unsigned char *s)
{
	while(*s != '\0')// '\0':�ַ���������־
	{
		uart_send_byte(*s);// ����1���ֽ����ݣ�1���ַ�ռ8λ��1�ֽ�
		s++;// ָ����һ���ַ�
	}
}

void uart_interrupt(void) interrupt 4 		//Ҳ�д����жϷ������
{		unsigned char send_data1[]="ireceive" ;// Ҫ���͵���Ϣ

	unsigned char recv_data;// ������Ž��յ�������
	unsigned int send_data[3] ;
	unsigned char send_data2[3] ;// Ҫ���͵���Ϣ

	send_data[0] = distance % 1000/100;
	send_data[1] = distance % 100 / 10;
	send_data[2] = distance % 10;
	send_data2[0] = zifu[send_data[0]];
	send_data2[1] = zifu[send_data[1]];
	send_data2[2] = zifu[send_data[2]];

	if(RI) //��������(1�ֽ�)��ϣ�RI�ᱻӲ����1
	{
		RI = 0;            		// �� �����жϱ�־λ ����(�ô��ڿ��Լ�����������)
		recv_data = SBUF;           	//��ȡ���յ������ݣ�����ŵ�data
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


	
		
	    uart_send_str(send_data2); //�յ�����֮�󣬷����ַ���"I received."���Է�
	}
	if(TI)// ��������(1�ֽ�)���
	{
		TI = 0;// �� �����жϱ�־λ ����(�ô��ڿ��Լ�����������)
	}
} 











/*timer0����жϷ������*/
void timer0Service() interrupt 1{  // �������˵��������෶Χ
echo = 0; // ǿ�ƹرջ���

		}

/*��ʱxus*/
void delayUs(uint xus){
	uint i;
	for(i = xus; i > 0; i--){
		_nop_();
	}
}


/*�������*/
void computeDis(){
	time = TH0 * 256 + TL0;
	distance = FACTOR * time;
	TH0 = 0x00; // ��װ��ֵ
	TL0 = 0x00;
}


uchar code table0[] = {"L:  M:  H:  N:  "};
uchar code table2[] = {"0123456789"};

uchar num = 0;


/*��ʼ��LCD1602������*/
void LCDInit(){
	uchar i;
	lcden = 0; // ����ʹ�ܶˣ�׼������ʹ�ܸ������ź�
	writeCommand(0x38); // ��ʾģʽ����(16x2�� 5x7����8λ���ݽӿ�)
	writeCommand(0x0c); // ����ʾ������ʾ���
	writeCommand(0x06); // дһ���ַ����ַָ���Զ���1
	writeCommand(0x01); // ��ʾ���㣬����ָ������
	
	/*LCD�ϵ����*/
	writeCommand(0x80); // ������ָ�붨λ����һ����
	for(i = 0; i < strlen(table0); i++){
		writeDataLCD(table0[i]);
		delay(5);
	}

	writeCommand(0x80 + 0x40); // ������ָ�붨λ���ڶ�����

}

/*LCD��ʾ����*/
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


/*дָ���*/
void writeCommand(unsigned char command)
	{
	lcdrs = 0; // ����ѡ��
	lcdrw = 0;
	P0 = command;
	delay(5);
	
	lcden = 1; // ����һ��������ʹ���ź�
	delay(5);
	lcden = 0;
}

/*д���ݺ���*/
void writeDataLCD(uchar dat){
	lcdrs = 1; // ����ѡ��
	lcdrw = 0;

	P0 = dat;
	delay(5);

	lcden = 1;
	delay(5);
	lcden = 0;
}

/*��ʱxms����*/
void delay(uint xms){
	uint i, j;
	for(i = xms; i > 0; i--)
		for(j = 110; j > 0; j--);
}

