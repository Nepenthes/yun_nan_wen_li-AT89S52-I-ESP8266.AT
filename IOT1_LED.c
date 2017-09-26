/********************************************************************************
*��Ȩ���� (C)2017���ڹ�̩�����������ɷ����޹�˾
*-------------------------------------------------------------------------------
* ���뻷��: Keil uVision5
* Ӳ������: GTA-GPMA12CA-V1.0 + GTA-GECA11CA-V1.0
* �ļ�˵����ͨ���������ַ��ͣ�
* ��ʼ�汾�����ܼҾ�ʵ��_LED����V1.0
* ��    �ߣ�������Ʋ�
* ������ڣ�2017��8��
*-------------------------------------------------------------------------------
*---------------------------------ʵ��ض�--------------------------------------
* 		��ʵ�鿪ʼǰ������ĳ�������253���趨WIFI��SSID����WIFI�ȵ����ƣ�������
*	���ɹ涨�����ƣ�����������ֹ���ģ��������ֻ�APP�޷�ʶ��ʵ�齫����������
*	�ɺ����±�������.hex�ļ�����д����Ƭ����
*	
*	ʾ����
*	
*	ʵ����3 	SSID�趨ΪGTAWJ_WX_02_003��
*	ʵ����10 	SSID�趨ΪGTAWJ_WX_02_010��
*	ʵ����16 	SSID�趨ΪGTAWJ_WX_02_016��
*	
*	...�Դ�����
*
*-------------------------------------------------------------------------------
*---------------------------------�޸ļ�¼--------------------------------------
* �޸ļ�¼1��
* �޸����� ��
* �� �� �� ��
* �� �� �� ��
* �޸����� ��
*
* �޸ļ�¼2:
*-------------------------------------------------------------------------------
********************************************************************************/

#include <reg52.h>
#include "string.h"

#define uchar 					unsigned char
#define uint 						unsigned int
#define CACHE						buffer[0]
#define CMDFLG_RESET		COMMAND_FLAG = 0

#define RX_MAXSIZE			30						//ָ���
#define COMMAND_SIZE		5							//ָ�����
#define	BUFFER_CORTEX		3							//����ָ��㼶

uchar idata buffer[COMMAND_SIZE][RX_MAXSIZE] = {							//���ڻ������

	"a","b","c","d","e"
};		

volatile uchar idata COMMAND_FLAG = 255;											//ָ��ʶ���ʶ
volatile uchar idata counter = 0;																		

bit trans_flg;

code const uchar *command[COMMAND_SIZE] = { 									//ָ����ұ�
	
	"Erase","0,CONNECT","0,CLOSED","ERROR","default5"
};

void init(void)																								//���ڳ�ʼ��  ������9600
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

void delay(uint xms)																					//������ʱ
{
	uint j;
	for(;xms>0;xms--)
	for(j=110;j>0;j--);
}

void put_char(uchar chr)																			//���ڵ��ַ�����
{ 
	ES = 0;
	
	SBUF = chr;
	while(!TI);
	TI = 0;
	
	ES = 1;
}	

static void print_string(uchar *str)													//�����ַ�������
{
	while(*str){
		
		put_char(*str);
		str++;
	}
	put_char('\r');
	put_char('\n');
}
	
void USART() interrupt 4																			//�����жϽ��մ���
{
   
    uchar loop;
	
    if(RI)
    {
				RI = 0;
			 
				if(SBUF == 0x0d){  																		//\rʶ��β��ʼָ��ʶ��
					for(loop=0;loop<COMMAND_SIZE;loop++)				
					if(!strcmp(buffer[0],command[loop])){
							trans_flg = 1;
							COMMAND_FLAG = loop;
							break;
					}
				}			
				
				else
					
				if(SBUF != 0x0a){  																		//\nʶ��ָ���������װ�ַ������뻺�����
					if(!counter){	
						
						 if(BUFFER_CORTEX-1)
							 for(loop = 1;loop < BUFFER_CORTEX;loop ++)
									strcpy(buffer[BUFFER_CORTEX-loop],buffer[BUFFER_CORTEX-loop-1]);

						 memset(buffer[0],0,sizeof(uchar)*RX_MAXSIZE);				   
					}
					buffer[0][counter++] = SBUF;
					if(counter > RX_MAXSIZE-2)counter = 0;							//���治�㣬��ͷ��ʼ
				}else counter = 0;
    } 		
}


/********************************************
	���룺    command�� ��Ҫ�����ATָ��
								rec:	��ӦESP8266 ATָ����Ӧ
					wait_time:	��������ȴ�ʱ��
								rep�� ָ���ظ�����

����ֵ��ָ������ȷ��Ӧ����0�����򷵻�1
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
	���룺    command�� ��Ҫ�����ATָ��
								rec:	��ӦESP8266 ATָ����Ӧ
					over_time:	��ʱʱ��

����ֵ��ָ������ȷ��Ӧ����0�����򷵻�1
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
����ʶ�����
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
����ʶ�����
*********************************************/	
void my_test(void)
{

	init();
	
//	ATTX_M2("����˭��","�Ҿ�����",65535);
//	ATTX_M2("���������","�Ҵ�������",65535);
//	ATTX_M2("�㵽��ȥ��","�ҵ�ȥ��ȥ",65535);
	
	ATTX_M1("����˭��","�Ҿ�����",1000,100);
	ATTX_M1("���������","�Ҵ�������",1000,100);
	ATTX_M1("�㵽��ȥ��","�ҵ�ȥ��ȥ",1000,100);
	usart_test();
}

/********************************************
ESP8266�����趨
*********************************************/	
void Wifi_Reset(void)
{
	origin:																																																		 //ͨ��������ESP8266�����趨ָ�ָ��δ�����ȷ��Ӧ�򷵻ش˴������趨
	P0 = 0xff;
	delay(500);
	if(ATTX_M2("AT+RST","ready",2000))																												    goto origin;
	P0 = 0xfe;
	delay(500);
	if(ATTX_M1("ATE0","OK",200,10))																																goto origin; //�ر�ָ��ط�	
	P0 = 0xfc;																																																 //����ָʾ
	if(ATTX_M1("AT","OK",200,10))																																	goto origin; //ȷ��״̬
	P0 = 0xf8;																																																 //����ָʾ
	if(ATTX_M1("AT+CWMODE=2","OK",200,10))																												goto origin; //�趨ESP8266 ΪAPģʽ
	P0 = 0xf0;																																																 //����ָʾ
	if(ATTX_M1("AT+CIPAP=\"192.168.1.1\"","OK",200,10))																						goto origin; //�趨ESP8266 AP��ӦIP��ַ
	P0 = 0xe0;																																																 //����ָʾ
	if(ATTX_M1("AT+CWSAP_CUR=\"GTAWJ_WX_02_001\",\"1234567890\",5,3,2,0","OK",600,10))						goto origin; //�趨ESP8266 SSID��PASSWORD
	P0 = 0xc0;																																																 //����ָʾ
	if(ATTX_M1("AT+CIPMUX=1","OK",200,10))																												goto origin; //�趨ESP8266 ��������ģʽ
	P0 = 0x80;																																																 //����ָʾ
	if(ATTX_M1("AT+CIPSERVER=1,8888","OK",200,10))																								goto origin; //�趨ESP8266 ����server����Ӧ�˿ں�
	P0 = 0x00;																																																 //����ָʾ
	delay(1000);
}

void Thread_M(void)					//���߳�
{
	uchar idata *p;						//���û���
	uchar error_count,keep;		//��ȡ������Ӧ����
	error_count = keep = 0;
	
	P0 = 0xaa;
	CMDFLG_RESET;							//ָ��ʶ���ʾ��λ
	strcpy(CACHE,"Erase");		//�������ڻ��棬���޹������
	wait_for_connect:while(COMMAND_FLAG != 1){delay(200);P0 = ~P0;}		//�ȴ��ֻ�����
	delay(300);
	print_string("AT+CIPSEND=0,1");				//�ֻ������ӣ��ظ��ֻ����ӱ��0xff
	delay(200);
	put_char('A');
	
	while(strcmp("+IPD,0,3:A",CACHE)){		//�ֻ��ظ�'A'��ʾ������ˮ�ƣ��ֻ��ظ�'B'��ʾ���ƴ���
	
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

	memset(buffer[0],0,sizeof(uchar)*RX_MAXSIZE);				//�������ڻ���
	while(1){
	
		if(COMMAND_FLAG == 2 || error_count > 5)break;		//��ESP8266��Ӧ�ֻ�ʧ�����ֻ������Ͽ����ӣ����߻�ô�����Ӧ�����ﵽ5���������趨ESP8266
		if(COMMAND_FLAG == 3){														//���ESP8266������Ӧ�����������ͬʱ�������ڻ���
			
				error_count++;
				memset(buffer[0],0,sizeof(uchar)*RX_MAXSIZE);
				CMDFLG_RESET;
		}
		if(strlen(CACHE) < 11 && strlen(CACHE) > 8)				//����ֻ�����ͬʱ��ESP8266����������ݰ������䳤�Ƚ���ȷ�ϣ����ⶪ��
		if(!strcmp("+IPD",strtok(CACHE,","))){						//�������ݣ����ָ���ȡ�ַ���"+IPD,0,1:"�������Ȼ���LED���ж�Ӧ����
		
				p = strtok(NULL,":");
				p = strtok(NULL,":");
				P0 = ~(*p);																		//���ݿͻ��˼�����ť���ֵ����8��LED
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
	P0 = 0xff;																					//�����̽�������ر�����LED
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