/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       referee_usart_task.c/h
  * @brief      RM referee system data solve. RM裁判系统数据处理
  * @note       
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Nov-11-2019     RM              1. done
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2019 DJI****************************
  */
#include "referee_usart_task.h"
#include "main.h"
#include "cmsis_os.h"

#include "bsp_usart.h"
#include "detect_task.h"

#include "CRC8_CRC16.h"
#include "fifo.h"
#include "referee.h"


//#include "UI_bsp.h"


/**
  * @brief          single byte upacked 
  * @param[in]      void
  * @retval         none
  */
/**
  * @brief          单字节解包
  * @param[in]      void
  * @retval         none
  */
static void referee_unpack_fifo_data(void);


/*****************user_fuc_begin******************/

#include "RM_Cilent_UI.h"
#include "string.h"
#include "CMS_interface.h"


/*****************user_fuc_end*******************/

/*****************user_define_begin******************/
#define CAP_Y_SALE_ADD		6
#define CAP_LINE_LENGTH		60	
#define CAP_LINE_X_BEGIN    605
#define CAP_LINE_Y_BEGIN    430
/*****************user_define_end*******************/

/*****************user_variable_begin******************/

int32_t UI_COUNT = 0;
//Graph_Data G1,G2,G3,G4,G5;
String_Data CAP_voltate_name;
Float_Data CAP_voltate;
Graph_Data CAP_line1,CAP_line2,CAP_line3,CAP_line4,CAP_line5,CAP_line6;
Graph_Data CAP_line7,CAP_line8,CAP_line9,CAP_line10,CAP_line11,CAP_line12,CAP_line13;
/*****************user_variable_end*******************/


extern UART_HandleTypeDef huart6;

uint8_t usart6_buf[2][USART_RX_BUF_LENGHT];

fifo_s_t referee_fifo;
uint8_t referee_fifo_buf[REFEREE_FIFO_BUF_LENGTH];
unpack_data_t referee_unpack_obj;

//extern uint8_t UI_DATA_SEND[128];      //UI界面交互最大为18个字节


/**
  * @brief          referee task
  * @param[in]      pvParameters: NULL
  * @retval         none
  */
/**
  * @brief          裁判系统任务
  * @param[in]      pvParameters: NULL
  * @retval         none
  */
void judge_task(void const * argument)
{

	portTickType judge_task_pre_tick = 0;
	
	
    init_referee_struct_data();
    fifo_s_init(&referee_fifo, referee_fifo_buf, REFEREE_FIFO_BUF_LENGTH);
    usart6_init(usart6_buf[0], usart6_buf[1], USART_RX_BUF_LENGHT);
	
//	memset(&G1,0,sizeof(G1));
//	memset(&G2,0,sizeof(G2));
//	memset(&G3,0,sizeof(G3));
//	memset(&G4,0,sizeof(G4));
//	memset(&G5,0,sizeof(G5));
	
	
//Line_Draw(&G1,"001",UI_Graph_ADD,9,UI_Color_Orange,3,960,570,960,200);
//Rectangle_Draw(&G2,"002",UI_Graph_ADD,9,UI_Color_Pink,5,700,450,1000,650);
//Arc_Draw(&G3,"003",UI_Graph_ADD,9,UI_Color_Green,70,180,5,960,400,80,80);
//Circle_Draw(&G4,"004",UI_Graph_ADD,9,UI_Color_Cyan,8,700,700,100);
//Line_Draw(&G5,"001",UI_Graph_ADD,9,UI_Color_Orange,3,1100,800,700,800);
//UI_ReFresh(5,G1,G2,G3,G4,G5);                          //绘制图形


    while(1)
    {

        referee_unpack_fifo_data();
		if(UI_COUNT == 19)
		{
			UI_Delete(UI_Data_Del_Layer,9);
			
//			Char_Draw(CAP_voltate_name,"cap",UI_Graph_ADD,UI_Color_Main,9);
			
			Float_Draw(&CAP_voltate,"001",UI_Graph_ADD,9,UI_Color_White,25,2,2,CAP_LINE_X_BEGIN-60,400,CapChageVoltage);
			
			Line_Draw(&CAP_line1,"002",UI_Graph_ADD,9,UI_Color_Main,2,CAP_LINE_X_BEGIN,CAP_LINE_Y_BEGIN,CAP_LINE_X_BEGIN+CAP_LINE_LENGTH,CAP_LINE_Y_BEGIN);
			Line_Draw(&CAP_line2,"003",UI_Graph_ADD,9,UI_Color_Main,2,CAP_LINE_X_BEGIN,CAP_LINE_Y_BEGIN+CAP_Y_SALE_ADD,CAP_LINE_X_BEGIN+CAP_LINE_LENGTH,CAP_LINE_Y_BEGIN+CAP_Y_SALE_ADD);
			Line_Draw(&CAP_line3,"004",UI_Graph_ADD,9,UI_Color_Main,2,CAP_LINE_X_BEGIN,CAP_LINE_Y_BEGIN+2*CAP_Y_SALE_ADD,CAP_LINE_X_BEGIN+CAP_LINE_LENGTH,CAP_LINE_Y_BEGIN+2*CAP_Y_SALE_ADD);
			Line_Draw(&CAP_line4,"005",UI_Graph_ADD,9,UI_Color_Main,2,CAP_LINE_X_BEGIN,CAP_LINE_Y_BEGIN+3*CAP_Y_SALE_ADD,CAP_LINE_X_BEGIN+CAP_LINE_LENGTH,CAP_LINE_Y_BEGIN+3*CAP_Y_SALE_ADD);
			Line_Draw(&CAP_line5,"006",UI_Graph_ADD,9,UI_Color_Main,2,CAP_LINE_X_BEGIN,CAP_LINE_Y_BEGIN+4*CAP_Y_SALE_ADD,CAP_LINE_X_BEGIN+CAP_LINE_LENGTH,CAP_LINE_Y_BEGIN+4*CAP_Y_SALE_ADD);
			Line_Draw(&CAP_line6,"007",UI_Graph_ADD,9,UI_Color_Main,2,CAP_LINE_X_BEGIN,CAP_LINE_Y_BEGIN+5*CAP_Y_SALE_ADD,CAP_LINE_X_BEGIN+CAP_LINE_LENGTH,CAP_LINE_Y_BEGIN+5*CAP_Y_SALE_ADD);
			UI_ReFresh(7,CAP_voltate,CAP_line1,CAP_line2,CAP_line3,CAP_line4,CAP_line5,CAP_line6);
			
			
			Line_Draw(&CAP_line7,"008",UI_Graph_ADD,9,UI_Color_Green,2,CAP_LINE_X_BEGIN,CAP_LINE_Y_BEGIN+6*CAP_Y_SALE_ADD,CAP_LINE_X_BEGIN+CAP_LINE_LENGTH,CAP_LINE_Y_BEGIN+6*CAP_Y_SALE_ADD);
			Line_Draw(&CAP_line8,"009",UI_Graph_ADD,9,UI_Color_Green,2,CAP_LINE_X_BEGIN,CAP_LINE_Y_BEGIN+7*CAP_Y_SALE_ADD,CAP_LINE_X_BEGIN+CAP_LINE_LENGTH,CAP_LINE_Y_BEGIN+7*CAP_Y_SALE_ADD);
			Line_Draw(&CAP_line9,"010",UI_Graph_ADD,9,UI_Color_Green,2,CAP_LINE_X_BEGIN,CAP_LINE_Y_BEGIN+8*CAP_Y_SALE_ADD,CAP_LINE_X_BEGIN+CAP_LINE_LENGTH,CAP_LINE_Y_BEGIN+8*CAP_Y_SALE_ADD);
			Line_Draw(&CAP_line10,"011",UI_Graph_ADD,9,UI_Color_Green,2,CAP_LINE_X_BEGIN,CAP_LINE_Y_BEGIN+9*CAP_Y_SALE_ADD,CAP_LINE_X_BEGIN+CAP_LINE_LENGTH,CAP_LINE_Y_BEGIN+9*CAP_Y_SALE_ADD);
			Line_Draw(&CAP_line11,"012",UI_Graph_ADD,9,UI_Color_Green,2,CAP_LINE_X_BEGIN,CAP_LINE_Y_BEGIN+10*CAP_Y_SALE_ADD,CAP_LINE_X_BEGIN+CAP_LINE_LENGTH,CAP_LINE_Y_BEGIN+10*CAP_Y_SALE_ADD);
			Line_Draw(&CAP_line12,"013",UI_Graph_ADD,9,UI_Color_Green,2,CAP_LINE_X_BEGIN,CAP_LINE_Y_BEGIN+11*CAP_Y_SALE_ADD,CAP_LINE_X_BEGIN+CAP_LINE_LENGTH,CAP_LINE_Y_BEGIN+11*CAP_Y_SALE_ADD);
			Line_Draw(&CAP_line13,"014",UI_Graph_ADD,9,UI_Color_Green,2,CAP_LINE_X_BEGIN,CAP_LINE_Y_BEGIN+12*CAP_Y_SALE_ADD,CAP_LINE_X_BEGIN+CAP_LINE_LENGTH,CAP_LINE_Y_BEGIN+12*CAP_Y_SALE_ADD);

			UI_ReFresh(7,CAP_line7,CAP_line8,CAP_line9,CAP_line10,CAP_line11,CAP_line12,CAP_line13);
			
			
			UI_COUNT=0;
		}
		UI_COUNT++;


        osDelayUntil(&judge_task_pre_tick,5);			//5ms的解算频率
    }
}


/**
  * @brief          single byte upacked 
  * @param[in]      void
  * @retval         none
  */
/**
  * @brief          单字节解包
  * @param[in]      void
  * @retval         none
  */
void referee_unpack_fifo_data(void)
{
  uint8_t byte = 0;
  uint8_t sof = HEADER_SOF;
  unpack_data_t *p_obj = &referee_unpack_obj;

  while ( fifo_s_used(&referee_fifo) )
  {
    byte = fifo_s_get(&referee_fifo);
    switch(p_obj->unpack_step)
    {
      case STEP_HEADER_SOF:
      {
        if(byte == sof)
        {
          p_obj->unpack_step = STEP_LENGTH_LOW;
          p_obj->protocol_packet[p_obj->index++] = byte;
        }
        else
        {
          p_obj->index = 0;
        }
      }break;
      
      case STEP_LENGTH_LOW:
      {
        p_obj->data_len = byte;
        p_obj->protocol_packet[p_obj->index++] = byte;
        p_obj->unpack_step = STEP_LENGTH_HIGH;
      }break;
      
      case STEP_LENGTH_HIGH:
      {
        p_obj->data_len |= (byte << 8);
        p_obj->protocol_packet[p_obj->index++] = byte;

        if(p_obj->data_len < (REF_PROTOCOL_FRAME_MAX_SIZE - REF_HEADER_CRC_CMDID_LEN))
        {
          p_obj->unpack_step = STEP_FRAME_SEQ;
        }
        else
        {
          p_obj->unpack_step = STEP_HEADER_SOF;
          p_obj->index = 0;
        }
      }break;
      case STEP_FRAME_SEQ:
      {
        p_obj->protocol_packet[p_obj->index++] = byte;
        p_obj->unpack_step = STEP_HEADER_CRC8;
      }break;

      case STEP_HEADER_CRC8:
      {
        p_obj->protocol_packet[p_obj->index++] = byte;

        if (p_obj->index == REF_PROTOCOL_HEADER_SIZE)
        {
          if ( verify_CRC8_check_sum(p_obj->protocol_packet, REF_PROTOCOL_HEADER_SIZE) )
          {
            p_obj->unpack_step = STEP_DATA_CRC16;
          }
          else
          {
            p_obj->unpack_step = STEP_HEADER_SOF;
            p_obj->index = 0;
          }
        }
      }break;  
      
      case STEP_DATA_CRC16:
      {
        if (p_obj->index < (REF_HEADER_CRC_CMDID_LEN + p_obj->data_len))
        {
           p_obj->protocol_packet[p_obj->index++] = byte;  
        }
        if (p_obj->index >= (REF_HEADER_CRC_CMDID_LEN + p_obj->data_len))
        {
          p_obj->unpack_step = STEP_HEADER_SOF;
          p_obj->index = 0;

          if ( verify_CRC16_check_sum(p_obj->protocol_packet, REF_HEADER_CRC_CMDID_LEN + p_obj->data_len) )
          {
            referee_data_solve(p_obj->protocol_packet);
          }
        }
      }break;

      default:
      {
        p_obj->unpack_step = STEP_HEADER_SOF;
        p_obj->index = 0;
      }break;
    }
  }
}


void USART6_IRQHandler(void)
{
    static volatile uint8_t res;
    if(USART6->SR & UART_FLAG_IDLE)
    {
        __HAL_UART_CLEAR_PEFLAG(&huart6);

        static uint16_t this_time_rx_len = 0;

        if ((huart6.hdmarx->Instance->CR & DMA_SxCR_CT) == RESET)
        {
            __HAL_DMA_DISABLE(huart6.hdmarx);
            this_time_rx_len = USART_RX_BUF_LENGHT - __HAL_DMA_GET_COUNTER(huart6.hdmarx);
            __HAL_DMA_SET_COUNTER(huart6.hdmarx, USART_RX_BUF_LENGHT);
            huart6.hdmarx->Instance->CR |= DMA_SxCR_CT;
            __HAL_DMA_ENABLE(huart6.hdmarx);
            fifo_s_puts(&referee_fifo, (char*)usart6_buf[0], this_time_rx_len);
            //detect_hook(REFEREE_TOE);
        }
        else
        {
            __HAL_DMA_DISABLE(huart6.hdmarx);
            this_time_rx_len = USART_RX_BUF_LENGHT - __HAL_DMA_GET_COUNTER(huart6.hdmarx);
            __HAL_DMA_SET_COUNTER(huart6.hdmarx, USART_RX_BUF_LENGHT);
            huart6.hdmarx->Instance->CR &= ~(DMA_SxCR_CT);
            __HAL_DMA_ENABLE(huart6.hdmarx);
            fifo_s_puts(&referee_fifo, (char*)usart6_buf[1], this_time_rx_len);
            //detect_hook(REFEREE_TOE);
        }
    }
}







