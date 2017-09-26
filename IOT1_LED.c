/********************************************************************************
*版权所有 (C)2017深圳国泰安教育技术股份有限公司
*-------------------------------------------------------------------------------
* 编译环境: Keil uVision5
* 硬件环境: GTA-GPMA12CA-V1.0 + GTA-GECA11CA-V1.0
* 文件说明：通过串口助手发送，
* 初始版本：智能家居实验_LED控制V1.0
* 作    者：电子设计部
* 完成日期：2017年8月
*-------------------------------------------------------------------------------
*---------------------------------实验必读--------------------------------------
* 		本实验开始前必须更改程序代码第253行设定WIFI的SSID（即WIFI热点名称）必须设
*	定成规定的名称，其他参数禁止更改，否则导致手机APP无法识别，实验将出错，更改完
*	成后重新编译生成.hex文件并烧写进单片机；
*	
*	示例：
*	
*	实验箱3 	SSID设定为GTAWJ_WX_02_003；
*	实验箱10 	SSID设定为GTAWJ_WX_02_010；
*	实验箱16 	SSID设定为GTAWJ_WX_02_016；
*	
*	...以此类推
*
*-------------------------------------------------------------------------------
*---------------------------------修改记录--------------------------------------
* 修改记录1：
* 修改日期 ：
* 版 本 号 ：
* 修 改 人 ：
* 修改内容 ：
*
* 修改记录2:
*-------------------------------------------------------------------------------
********************************************************************************/

#include <reg52.h>
#include "string.h"

#define uchar 					unsigned char
#define uint 						unsigned int
#define CACHE						buffer[0]
#define CMDFLG_RESET		COMMAND_FLAG = 0

#define RX_MAXSIZE			30						//指令长度
#define COMMAND_SIZE		5							//指令个数
#define	BUFFER_CORTEX		3							//开放指令层级

uchar idata buffer[COMMAND_SIZE][RX_MAXSIZE] = {							//串口缓冲队列

	"a","b","c","d","e"
};		

volatile uchar idata COMMAND_FLAG = 255;											//指令识别标识
volatile uchar idata counter = 0;																		

bit trans_flg;

code const uchar *command[COMMAND_SIZE] = { 									//指令查找表
	
	"Erase","0,CONNECT","0,CLOSED","ERROR","default5"
};

void init(void)																								//串口初始化  波特率9600
{
	SCON = 0x50;
	TMOD |= 0x20;
	PCON |= 0x80;
	TH1 = 0xfa;
	TR1  = 1;
	REN  = 1;
	SM0  = 0;
	SM1  = 1;
	EA   = 1;
	ES   = 1;
	PS	 = 1;
}

void delay(uint xms)																					//毫秒延时
{
	uint j;
	for(;xms>0;xms--)
	for(j=110;j>0;j--);
}

void put_char(uchar chr)																			//串口单字符发送
{ 
	ES = 0;
	
	SBUF = chr;
	while(!TI);
	TI = 0;
	
	ES = 1;
}	

static void print_string(uchar *str)													//串口字符串发送
{
	while(*str){
		
		put_char(*str);
		str++;
	}
	put_char('\r');
	put_char('\n');
}
	
void USART() interrupt 4																			//串口中断接收处理
{
   
    uchar loop;
	
    if(RI)
    {
				RI = 0;
			 
				if(SBUF == 0x0d){  																		//\r识别串尾开始指令识别
					for(loop=0;loop<COMMAND_SIZE;loop++)				
					if(!strcmp(buffer[0],command[loop])){
							trans_flg = 1;
							COMMAND_FLAG = loop;
							break;
					}
				}			
				
				else
					
				if(SBUF != 0x0a){  																		//\n识别指令结束，封装字符串送入缓冲队列
					if(!counter){	
						
						 if(BUFFER_CORTEX-1)
							 for(loop = 1;loop < BUFFER_CORTEX;loop ++)
									strcpy(buffer[BUFFER_CORTEX-loop],buffer[BUFFER_CORTEX-loop-1]);

						 memset(buffer[0],0,sizeof(uchar)*RX_MAXSIZE);				   
					}
					buffer[0][counter++] = SBUF;
					if(counter > RX_MAXSIZE-2)counter = 0;							//缓存不足，从头开始
				}else counter = 0;
    } 		
}


/********************************************
	输入：    command： 需要输入的AT指令
								rec:	对应ESP8266 AT指令响应
					wait_time:	单次输入等待时间
								rep： 指令重复次数

返回值：指令获得正确响应返回0，否则返回1
*********************************************/
bit ATTX_M1(uchar *command,uchar *rec,uint wait_time,uchar rep)
{
		uchar time_point = 1;
	
		delay(100);
		while(strcmp(rec,buffer[0])){
		
				print_string(command);
				delay(wait_time);
				time_point++;
				if(time_point > rep)return 1;
		}
		strcpy(buffer[0],"Erase");
		delay(100);
		return 0;
}

/********************************************
	输入：    command： 需要输入的AT指令
								rec:	对应ESP8266 AT指令响应
					over_time:	超时时长

返回值：指令获得正确响应返回0，否则返回1
*********************************************/		
bit	ATTX_M2(uchar *command,uchar *rec,uint over_time)
{
		
		uint time_point = 1;
	
		delay(100);
		print_string(command);
		while(strcmp(rec,buffer[0])){
			
				delay(200);
				if(time_point > over_time/200)return 1;
				time_point++;
		}
		strcpy(buffer[0],"Erase");
		delay(100);
		return 0;
}

/********************************************
串口识别测试
*********************************************/	
void usart_test(void)
{
	uchar *p;
	init();
	while(1){
		
			put_char('1'+COMMAND_FLAG);
			print_string("\r\n");
			p = strtok(buffer[0],":");
			p = strtok(NULL,":");
			print_string(p);
			print_string("\r\n");
			print_string(buffer[1]);
			print_string("\r\n");
			print_string(buffer[2]);
			print_string("\r\n");
			delay(1000);
	}		
}

/********************************************
串口识别测试
*********************************************/	
void my_test(void)
{

	init();
	
//	ATTX_M2("你是谁？","我就是我",65535);
//	ATTX_M2("你从哪来？","我从来处来",65535);
//	ATTX_M2("你到哪去？","我到去处去",65535);
	
	ATTX_M1("你是谁？","我就是我",1000,100);
	ATTX_M1("你从哪来？","我从来处来",1000,100);
	ATTX_M1("你到哪去？","我到去处去",1000,100);
	usart_test();
}

/********************************************
ESP8266参数设定
*********************************************/	
void Wifi_Reset(void)
{
	origin:																																																		 //通过串口向ESP8266发送设定指令，指令未获得正确响应则返回此处重新设定
	P0 = 0xff;
	delay(500);
	if(ATTX_M2("AT+RST","ready",2000))																												    goto origin;
	P0 = 0xfe;
	delay(500);
	if(ATTX_M1("ATE0","OK",200,10))																																goto origin; //关闭指令回发	
	P0 = 0xfc;																																																 //进程指示
	if(ATTX_M1("AT","OK",200,10))																																	goto origin; //确认状态
	P0 = 0xf8;																																																 //进程指示
	if(ATTX_M1("AT+CWMODE=2","OK",200,10))																												goto origin; //设定ESP8266 为AP模式
	P0 = 0xf0;																																																 //进程指示
	if(ATTX_M1("AT+CIPAP=\"192.168.1.1\"","OK",200,10))																						goto origin; //设定ESP8266 AP对应IP地址
	P0 = 0xe0;																																																 //进程指示
	if(ATTX_M1("AT+CWSAP_CUR=\"GTAWJ_WX_02_001\",\"1234567890\",5,3,2,0","OK",600,10))						goto origin; //设定ESP8266 SSID及PASSWORD
	P0 = 0xc0;																																																 //进程指示
	if(ATTX_M1("AT+CIPMUX=1","OK",200,10))																												goto origin; //设定ESP8266 多线连接模式
	P0 = 0x80;																																																 //进程指示
	if(ATTX_M1("AT+CIPSERVER=1,8888","OK",200,10))																								goto origin; //设定ESP8266 开启server及对应端口号
	P0 = 0x00;																																																 //进程指示
	delay(1000);
}

void Thread_M(void)					//主线程
{
	uchar idata *p;						//设置缓冲
	uchar error_count,keep;		//获取错误响应计数
	error_count = keep = 0;
	
	P0 = 0xaa;
	CMDFLG_RESET;							//指令识别表示复位
	strcpy(CACHE,"Erase");		//擦除串口缓存，用无关字填充
	wait_for_connect:while(COMMAND_FLAG != 1){delay(200);P0 = ~P0;}		//等待手机连接
	delay(300);
	print_string("AT+CIPSEND=0,1");				//手机已连接，回复手机连接标记0xff
	delay(200);
	put_char('A');
	
	while(strcmp("+IPD,0,3:A",CACHE)){		//手机回复'A'表示控制流水灯，手机回复'B'表示控制窗帘
	
		delay(200);P0 = ~P0;
		if(COMMAND_FLAG == 2)goto wait_for_connect;
	}			

	P0 = 0;  delay(100);
	P0 = ~P0;delay(100);
	P0 = ~P0;delay(100);
	P0 = ~P0;delay(100);
	P0 = ~P0;delay(100);
	P0 = ~P0;delay(100);
	P0 = ~P0;delay(100);
	P0 = ~P0;

	memset(buffer[0],0,sizeof(uchar)*RX_MAXSIZE);				//擦除串口缓存
	while(1){
	
		if(COMMAND_FLAG == 2 || error_count > 5)break;		//若ESP8266响应手机失联（手机主动断开连接）或者获得错误响应次数达到5次则重新设定ESP8266
		if(COMMAND_FLAG == 3){														//获得ESP8266错误响应，错误计数，同时擦除串口缓存
			
				error_count++;
				memset(buffer[0],0,sizeof(uchar)*RX_MAXSIZE);
				CMDFLG_RESET;
		}
		if(strlen(CACHE) < 11 && strlen(CACHE) > 8)				//获得手机发送同时被ESP8266处理过的数据包，对其长度进行确认，避免丢包
		if(!strcmp("+IPD",strtok(CACHE,","))){						//解析数据，获得指令，截取字符串"+IPD,0,1:"后的数据然后对LED进行对应操作
		
				p = strtok(NULL,":");
				p = strtok(NULL,":");
				P0 = ~(*p);																		//根据客户端监听按钮获得值控制8颗LED
				strcpy(CACHE,"Erase");
				CMDFLG_RESET;
				delay(50);
		}
		delay(50);
		keep ++;
		
		if(keep > 100){
		
				if(!ATTX_M2("AT+CIPSEND=0,1","ERROR",200))break;
				put_char(0xff);
				keep = 0;
		}
	}
	P0 = 0xff;																					//若进程结束，则关闭所有LED
	print_string("i'm out");
}

void user_task(void)
{
	init();
	
	Wifi_Reset();
	Thread_M();
}

void main(void)
{
	user_task();
}