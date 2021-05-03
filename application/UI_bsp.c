#include "UI_bsp.h"
#include "referee.h"
#include "CRC8_CRC16.h"
#include "string.h"
#include "CMS_interface.h"


uint8_t 		   UI_DATA_SEND[128];      //UI���潻�����Ϊ18���ֽ�
UI_data_frame_t    hero_UI_data_frame;	

graphic_data_struct_t  CAP_volate;




											
/* * * * * * * * * * *USER func define begin* * * * * * * * * * */
//UIָ������֡ͷ����
/* * * * * * * * * * * * * * * * * * * * * * *
0x0100:�ͻ���ɾ��ͼ��
0x0101:�ͻ��˻���һ��ͼ��
0x0102:�ͻ��˻��ƶ���ͼ��
0x0103:�ͻ��˻������ͼ��
0x0104:�ͻ��˻����߸�ͼ��
0x0110:�ͻ��˻����ַ�ͼ��
* * * * * * * * * * * * * * * * * * * * * * */
dataHeaderFrame UI_dataHeaderFrame_conf(dataHeaderFrame dataHeaderFrame,
										uint16_t data_id
														);																	
//���Ƴ������ݵ�ѹ����
graphic_data_struct_t UI_CAPvolate_data_conf();															
																	
																	
/* * * * * * * * * * *USER func define end* * * * * * * * * * */	



//UI��ͼ�嵥
graphic_data_struct_t *UI_graphic_list[] = {	UI_CAPvolate_data_conf			
												



																	};

															
		uint8_t test_1[4];															

void UI_paint_send(int length)
{
    uint8_t temp_frameHeader[5];

	//step1:
	/***********************frame_header begin**********************/
    //frame_header.SOF
	hero_UI_data_frame.UI_frame_header.SOF = 0xA5;
	//frame_header.data_length
	hero_UI_data_frame.UI_frame_header.data_length = 6+length * 15;
	//frame_header.seq
	if(hero_UI_data_frame.UI_frame_header.seq == 255)
		{hero_UI_data_frame.UI_frame_header.seq = 0;}
	hero_UI_data_frame.UI_frame_header.seq++;
	//frame_header.CRC8
	memcpy(temp_frameHeader,&hero_UI_data_frame.UI_frame_header,4);
	append_CRC8_check_sum(temp_frameHeader,5);
	hero_UI_data_frame.UI_frame_header.CRC8 = temp_frameHeader[4];
	/***********************frame_header end************************/
	
	//step2:
	/**********************cmd_id begin************************/
	hero_UI_data_frame.Cmd_id = STUDENT_INTERACTIVE_DATA_CMD_ID;
	/**********************cmd_id  end*************************/
	
	//step3:
	/**********************data_frameHeader begin********************/
	hero_UI_data_frame.UI_dataFrameHeader = UI_dataHeaderFrame_conf(hero_UI_data_frame.UI_dataFrameHeader,
																	ADD_one_graphic);
	/**********************data_frameHeader end***********************/
	
	
	//step4:
	/**********************data_graphic begin************************/
//	int graphic_num = 0;
//	int i;
//	switch(hero_UI_data_frame.UI_dataFrameHeader.data_cmd_id)
//	{
//		case ADD_one_graphic:
//		{graphic_num=1;    break;}
//		case ADD_two_graphic:
//		{graphic_num=2;    break;}
//		case ADD_five_graphic:
//		{graphic_num=5;    break;}
//		case ADD_seven_graphic:
//		{graphic_num=7;    break;}
//	}
//	
//	for(i=0; i<graphic_num; i++)
//	{
//		hero_UI_data_frame.UI_graphic_data[0] = *UI_graphic_list[0];
		hero_UI_data_frame.UI_graphic_data[0] = UI_CAPvolate_data_conf();
//	}
	
	/**********************data_graphic end***************************/
	
	memcpy(UI_DATA_SEND,&hero_UI_data_frame,15+length*15);
	memcpy(&UI_DATA_SEND[24],&CapChageVoltage,4);
	append_CRC16_check_sum(UI_DATA_SEND,30);		//��CRC16��������β
	
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*������UI������������֡ͷ
*dataHeaderFrame: ����֡ͷ���
*data_id	    : ����ָ��

0x0100:�ͻ���ɾ��ͼ��
0x0101:�ͻ��˻���һ��ͼ��
0x0102:�ͻ��˻��ƶ���ͼ��
0x0103:�ͻ��˻������ͼ��
0x0104:�ͻ��˻����߸�ͼ��
0x0110:�ͻ��˻����ַ�ͼ��

* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

dataHeaderFrame UI_dataHeaderFrame_conf(dataHeaderFrame dataHeaderFrame,
										uint16_t data_id
														)
{

	if(robot_state.robot_id == USER_HERO_RED_ID)		//1Ϊ�췽Ӣ�۵�ID
	{
		dataHeaderFrame.receiver_ID = MASTER_HERO_RED_ID;
		dataHeaderFrame.sender_ID   = USER_HERO_RED_ID;
	
	}
	else if(robot_state.robot_id == USER_HERO_BUL_ID)	//101Ϊ����Ӣ��
	{
		dataHeaderFrame.receiver_ID = MASTER_HERO_BLU_ID;
		dataHeaderFrame.sender_ID   = USER_HERO_BUL_ID;
	}
	dataHeaderFrame.data_cmd_id = data_id;

	return dataHeaderFrame;
}




//���Ƴ������ݵ�ѹ����
graphic_data_struct_t UI_CAPvolate_data_conf()
{
	CAP_volate.graphic_name[0] = 100;    //ͼ�������0
	CAP_volate.operate_tpye = 1;
	CAP_volate.graphic_tpye = 5;
	CAP_volate.layer		= 0;	   //ͼ����0
	CAP_volate.color		= 8;	   //��ɫ�ǰ�ɫ
	CAP_volate.start_angle  = 40;
	CAP_volate.end_angle	= 2; 
	CAP_volate.width		= 2;
	CAP_volate.start_x		= 700;
	CAP_volate.start_y		= 700;
	//memcpy(CAP_volate.radius,&CapChageVoltage,4);	//�����������������
	//memcpy(&test_1,&CapChageVoltage,4);
	return CAP_volate;
}







