#ifndef _UI_BSP_H
#define _UI_BSP_H



#include "main.h"
#include "protocol.h"





#define USER_HERO_RED_ID    1               //用户英雄机器人ID(红)
#define USER_HERO_BUL_ID    101             //用户英雄机器人ID(蓝)

#define MASTER_HERO_RED_ID   0x0101         //客户端英雄机器人ID(红)
#define MASTER_HERO_BLU_ID   0x0165         //客户端英雄机器人ID(蓝)



#define DELETE_graphic		 0x0100	
#define ADD_one_graphic		 0x0101
#define ADD_two_graphic		 0x0102
#define ADD_five_graphic     0x0103
#define ADD_seven_graphic	 0x0104
#define ADD_char_graphic     0x0110	


//数据段数据格式
//数据内容ID+发送者ID+接收者ID+数据（长度小于113字节）

//数据ID
typedef __PACKED_STRUCT
{
 uint16_t data_cmd_id;
 uint16_t sender_ID;
 uint16_t receiver_ID;
}dataHeaderFrame;

//客户端删除图形
typedef __PACKED_STRUCT
{
uint8_t operate_tpye;       //操作类型 0：空操作   1：删除图层  2：删除所有
uint8_t layer;              //操作图层 0~9
} ext_client_custom_graphic_delete_t;



//图形数据
typedef __PACKED_STRUCT
{ 	
uint8_t graphic_name[3]; 		//图形名
uint32_t operate_tpye:3; 		//图形操作(位操作) 0:空操作   1:增加    3:删除
uint32_t graphic_tpye:3; 		//图形类型   0:直线  1:矩形 2:整圆 3:椭圆 4:圆弧 5:浮点数 6:整型数 7:字符
uint32_t layer:4; 				//图层数:0-9
uint32_t color:4; 				//颜色: 0:红蓝主色 1:黄色 2:绿色 3:橙色 4:紫红色 5:粉色 6:青色 7:黑色  8:白色
uint32_t start_angle:9;			//起始角度，单位：°，范围[0,360]
uint32_t end_angle:9;			//终止角度，单位：°，范围[0,360]
uint32_t width:10; 				//线宽
uint32_t start_x:11; 			//起点 x 坐标
uint32_t start_y:11; 			//起点 y 坐标
uint32_t radius:10; 			//字体大小或者半径
uint32_t end_x:11; 				//终点 x 坐标
uint32_t end_y:11; 				//终点 y 坐标
} graphic_data_struct_t;

//客户端绘制一个图形
typedef __PACKED_STRUCT
{
 graphic_data_struct_t grapic_data_struct;
} ext_client_custom_graphic_single_t;

//客户端绘制二个图形
typedef __PACKED_STRUCT
{
graphic_data_struct_t grapic_data_struct[2];
} ext_client_custom_graphic_double_t;

//客户端绘制五个图形
typedef __PACKED_STRUCT
{
graphic_data_struct_t grapic_data_struct[5];
} ext_client_custom_graphic_five_t;


//客户端绘制七个图形
typedef __PACKED_STRUCT
{
graphic_data_struct_t grapic_data_struct[7];
} ext_client_custom_graphic_seven_t;

//客户端绘制字符
typedef __PACKED_STRUCT
{
graphic_data_struct_t grapic_data_struct;
uint8_t data[30];
} ext_client_custom_character_t;


//UI界面整帧数据结构体
typedef __PACKED_STRUCT
{
	frame_header_struct_t 					UI_frame_header;		//数据帧头   SOF+data_length+seq+CRC8    
	uint16_t  			   					Cmd_id;					//命令码，恒为0x301
	dataHeaderFrame	UI_dataFrameHeader;		//数据段头结构
	graphic_data_struct_t					UI_graphic_data[7];		//数据段
	uint16_t    		   					frame_tail;				//数据帧尾

}UI_data_frame_t;



//裁判系统UI界面绘制命令发送
extern void UI_paint_send(int length);
						 
extern uint8_t 		   UI_DATA_SEND[128];      //UI界面交互最大为18个字节


#endif