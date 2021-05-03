/**
  ****************************�����Ƽ���ѧCOD****************************
  * @file       user_task.c/h-���䳤��
  * @brief      һ����ͨ�����������ƶԴ���лл��ң�д��demo��δ���ü���װ���պϿ���
  ==============================================================================
  @endverbatim
  ****************************�����Ƽ���ѧCOD****************************
  */

#include "User_Task.h"
#include "main.h"
#include "RM_Cilent_UI.h"
#include "string.h"
#include "CAN_Receive.h"//����Ħ���ַ���ջ���ʵת��
#define PI 3.1415936

Graph_Data G1,G2,G3,G4,G5,G6,G7,G8,G9,G10,G11;
String_Data CH_SHOOT;
String_Data CH_FLRB;
char shoot_arr[5]="shoot";
char flrb_arr[4]="FRBL";
void UserTask(void *pvParameters)
{
		memset(&G1,0,sizeof(G1));//���Ĵ���
		memset(&G2,0,sizeof(G2));//�ϻ�����
		memset(&G3,0,sizeof(G3));//����ˮƽ��
		memset(&G4,0,sizeof(G4));//ǹ��������
		memset(&G5,0,sizeof(G5));//�»�����
		memset(&G6,0,sizeof(G6));//Զ���������
		memset(&G7,0,sizeof(G7));//Ħ����״̬
		memset(&CH_SHOOT,0,sizeof(CH_SHOOT));//Ħ���ֱ�ʶ
	    memset(&G8,0,sizeof(G8));//ǰװ�װ�״̬
	    memset(&G9,0,sizeof(G9));//��װ�װ�״̬
	    memset(&G10,0,sizeof(G10));//��װ�װ�״̬
	    memset(&G11,0,sizeof(G11));//��װ�װ�״̬
	    memset(&CH_FLRB,0,sizeof(CH_FLRB));//װ�װ��ʶ

        Line_Draw(&G1,"091",UI_Graph_ADD,9,UI_Color_Purplish_red,1,960,330,960,620);
		Line_Draw(&G2,"092",UI_Graph_ADD,9,UI_Color_Purplish_red,1,880,580,1040,580);
		Line_Draw(&G3,"093",UI_Graph_ADD,9,UI_Color_Purplish_red,1,800,540,1120,540);
		Line_Draw(&G4,"094",UI_Graph_ADD,9,UI_Color_Purplish_red,1,880,500,1040,500);
		Line_Draw(&G5,"095",UI_Graph_ADD,9,UI_Color_Purplish_red,1,900,420,1020,420);
	    Line_Draw(&G6,"096",UI_Graph_ADD,9,UI_Color_Purplish_red,1,920,370,1000,370);
	    Circle_Draw(&G7,"097",UI_Graph_ADD,9,UI_Color_Yellow,15,230,770,15);
	    Char_Draw(&CH_SHOOT,"087",UI_Graph_ADD,8 ,UI_Color_Yellow,24,5,4,80,780,&shoot_arr[0]);
		UI_ReFresh(7,G1,G2,G3,G4,G5,G6,G7);                          //����ͼ��	
        Char_ReFresh(CH_SHOOT);
		static double angle =0,heat=0,heat_door=0;//����angleֱ�Ӹ��ɲ�ֵ
		static int flag_cnn1=0,flag_cnn2=0,heat_flag1=0,heat_flag2=0;;//��ʱ1//��ʱ2//�ܻ����־λ//�ܻ����ñ�־λ;
		Circle_Draw(&G8,"081",UI_Graph_ADD,8,UI_Color_Green,3,960+(int)340*sin((angle)*2*PI/360.0),540+(int)340*cos((angle)*2*PI/360.0),50);
		Circle_Draw(&G9,"082",UI_Graph_ADD,8,UI_Color_Green,3,960+(int)340*sin((angle+90)*2*PI/360.0),540+(int)340*cos((angle+90)*2*PI/360.0),50);
		Circle_Draw(&G10,"083",UI_Graph_ADD,8,UI_Color_Green,3,960+(int)340*sin((angle+180)*2*PI/360.0),540+(int)340*cos((angle+180)*2*PI/360.0),50);
		Circle_Draw(&G11,"084",UI_Graph_ADD,8,UI_Color_Green,3,960+(int)340*sin((angle+270)*2*PI/360.0),540+(int)340*cos((angle+270)*2*PI/360.0),50);
		UI_ReFresh(5,G7,G8,G9,G10,G11);//G7ȱʡ   
		Char_Draw(&CH_FLRB,"077",UI_Graph_ADD,7 ,UI_Color_Yellow,24,1,4,960+(int)340*sin((angle)*2*PI/360.0),540+(int)340*cos((angle)*2*PI/360.0),&flrb_arr[0]);
		Char_ReFresh(CH_FLRB);
		Char_Draw(&CH_FLRB,"076",UI_Graph_ADD,7,UI_Color_Yellow,24,1,4,960+(int)340*sin((angle+90)*2*PI/360.0),540+(int)340*cos((angle+90)*2*PI/360.0),&flrb_arr[1]);
		Char_ReFresh(CH_FLRB);
		Char_Draw(&CH_FLRB,"075",UI_Graph_ADD,7 ,UI_Color_Yellow,24,1,4,960+(int)340*sin((angle+180)*2*PI/360.0),540+(int)340*cos((angle+180)*2*PI/360.0),&flrb_arr[2]);
		Char_ReFresh(CH_FLRB);
		Char_Draw(&CH_FLRB,"074",UI_Graph_ADD,7 ,UI_Color_Yellow,24,1,4,960+(int)340*sin((angle+270)*2*PI/360.0),540+(int)340*cos((angle+270)*2*PI/360.0),&flrb_arr[3]);
		Char_ReFresh(CH_FLRB);
  while(1)
	{
		//ģ��װ�װ��ܻ��ж�||��װ�װ�ID��Ӧangle�����4��angle����ת��ģ���װ�װ��ƶ��뱻���򣡣�������
		if(angle==50) heat=1;
		if(angle==80) heat=2;
		if(angle==85) heat=2;
		if(angle==90) heat=3;
		if(heat) heat_door=heat;
		if(heat_door)
		{		
			if(!heat_flag1){heat_flag1=heat;flag_cnn1=0;}//�״α�����
			else if(heat_flag1==heat) {flag_cnn1=0;}//���α�������ͬһװ�װ�
			else if(heat_flag1!=heat) 
				{ 
					if(!heat_flag2) {heat_flag2=heat;flag_cnn2=0;}
					if(heat_flag2==heat) {flag_cnn2=0;}
				  else if(heat){ if(flag_cnn1>=flag_cnn2){heat_flag1=heat;flag_cnn1=0;}else {heat_flag2=heat;flag_cnn2=0;}}
				}//���λ���λ�ò���ͬһװ�װ�
			if(flag_cnn1>=20) 
				{
					heat_flag1=0;flag_cnn1=0;
				}
			else if(heat_flag1) flag_cnn1+=1;//ʱ���־λ1�ۼ�
			if(flag_cnn2>=20) 
				{
					heat_flag2=0;flag_cnn2=0;
				}
			else if(heat_flag2) flag_cnn2+=1;//ʱ���־λ2�ۼ�
			heat=0;
		}
			//����Ħ���ַ���UI,��ʼ����ɫ�������Ϻ�ɫ��������ɫ
		if(Shoot_Right_Date.V&&Shoot_Right_Date.V)
			Circle_Draw(&G7,"007",UI_Graph_Change,9,UI_Color_Green,15,230,770,15);
		else Circle_Draw(&G7,"007",UI_Graph_Change,9,UI_Color_Purplish_red,15,230,770,15);
		//װ�װ���ת�ƶ���������ɫ���ܻ��Ϻ�ɫ��˸����s��Ӧʱ��
		if(heat_flag1==1||heat_flag2==1)
			Circle_Draw(&G8,"081",UI_Graph_Change,8,UI_Color_Pink,10,960+(int)340*sin((angle)*2*PI/360.0),540+(int)340*cos((angle)*2*PI/360.0),50);
		else Circle_Draw(&G8,"081",UI_Graph_Change,8,UI_Color_Green,3,960+(int)340*sin((angle)*2*PI/360.0),540+(int)340*cos((angle)*2*PI/360.0),50);
		if(heat_flag1==2||heat_flag2==2) 
			Circle_Draw(&G9,"082",UI_Graph_Change,8,UI_Color_Pink,10,960+(int)340*sin((angle+90)*2*PI/360.0),540+(int)340*cos((angle+90)*2*PI/360.0),50);
		else Circle_Draw(&G9,"082",UI_Graph_Change,8,UI_Color_Green,3,960+(int)340*sin((angle+90)*2*PI/360.0),540+(int)340*cos((angle+90)*2*PI/360.0),50);
		if(heat_flag1==3||heat_flag2==3) 	
			Circle_Draw(&G10,"083",UI_Graph_Change,8,UI_Color_Pink,10,960+(int)340*sin((angle+180)*2*PI/360.0),540+(int)340*cos((angle+180)*2*PI/360.0),50);
		else Circle_Draw(&G10,"083",UI_Graph_Change,8,UI_Color_Green,3,960+(int)340*sin((angle+180)*2*PI/360.0),540+(int)340*cos((angle+180)*2*PI/360.0),50);
		if(heat_flag1==4||heat_flag2==4) 	
		  Circle_Draw(&G11,"084",UI_Graph_Change,8,UI_Color_Pink,10,960+(int)340*sin((angle+270)*2*PI/360.0),540+(int)340*cos((angle+270)*2*PI/360.0),50);
		else Circle_Draw(&G11,"084",UI_Graph_Change,8,UI_Color_Green,3,960+(int)340*sin((angle+270)*2*PI/360.0),540+(int)340*cos((angle+270)*2*PI/360.0),50);
		UI_ReFresh(5, G7, G8, G9, G10, G11);
		Char_Draw(&CH_FLRB, "077", UI_Graph_Change, 7, UI_Color_Yellow, 24, 1, 4, 960 + (int)340 * sin((angle) * 2 * PI / 360.0), 540 + (int)340 * cos((angle) * 2 * PI / 360.0), &flrb_arr[0]);
		Char_ReFresh(CH_FLRB);
		Char_Draw(&CH_FLRB, "076", UI_Graph_Change, 7, UI_Color_Yellow, 24, 1, 4, 960 + (int)340 * sin((angle + 90) * 2 * PI / 360.0), 540 + (int)340 * cos((angle + 90) * 2 * PI / 360.0), &flrb_arr[1]);
		Char_ReFresh(CH_FLRB);
		Char_Draw(&CH_FLRB, "075", UI_Graph_Change, 7, UI_Color_Yellow, 24, 1, 4, 960 + (int)340 * sin((angle + 180) * 2 * PI / 360.0), 540 + (int)340 * cos((angle + 180) * 2 * PI / 360.0), &flrb_arr[2]);
		Char_ReFresh(CH_FLRB);
		Char_Draw(&CH_FLRB, "074", UI_Graph_Change, 7, UI_Color_Yellow, 24, 1, 4, 960 + (int)340 * sin((angle + 270) * 2 * PI / 360.0), 540 + (int)340 * cos((angle + 270) * 2 * PI / 360.0), &flrb_arr[3]);
		Char_ReFresh(CH_FLRB);
		//Ƶ�ʿ���10hz
		vTaskDelay(100);
	}		
}




