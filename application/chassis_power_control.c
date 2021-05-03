/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       chassis_power_control.c/h
  * @brief      chassis power control.���̹��ʿ���
  * @note       this is only controling 80 w power, mainly limit motor current set.
  *             if power limit is 40w, reduce the value JUDGE_TOTAL_CURRENT_LIMIT 
  *             and POWER_CURRENT_LIMIT, and chassis max speed (include max_vx_speed, min_vx_speed)
  *             ֻ����80w���ʣ���Ҫͨ�����Ƶ�������趨ֵ,������ƹ�����40w������
  *             JUDGE_TOTAL_CURRENT_LIMIT��POWER_CURRENT_LIMIT��ֵ�����е�������ٶ�
  *             (����max_vx_speed, min_vx_speed)
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Nov-11-2019     RM              1. add chassis power control
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2019 DJI****************************
  */
#include "chassis_power_control.h"
#include "referee.h"
#include "arm_math.h"
#include "CMS_interface.h"


#define POW_first  1

//��ʼ״̬�ĵ��̹���
#define POWER_LIMIT_LEVEL0         			50.0f

//ѡ��Ѫ�����ȵĵ��̹���
#define HPfirst_POWER_LIMIT_LEVEL1         	55.0f
#define HPfirst_POWER_LIMIT_LEVEL2         	60.0f
#define HPfirst_POWER_LIMIT_LEVEL3         	65.0f
//ѡ�������ȵĵ��̹���
#define POWfirst_POWER_LIMIT_LEVEL1         	60.0f//70.0f
#define POWfirst_POWER_LIMIT_LEVEL2         	90.0f
#define POWfirst_POWER_LIMIT_LEVEL3         	120.0f



#define WARNING_POWER_LEVEL0        POWER_LIMIT_LEVEL0 * 0.8 

//ѡ��Ѫ�����ȵĵ��̹��ʾ���
#define HPfirst_WARNING_POWER_LEVEL1        HPfirst_POWER_LIMIT_LEVEL1 * 0.9 
#define HPfirst_WARNING_POWER_LEVEL2        HPfirst_POWER_LIMIT_LEVEL2 * 0.9    
#define HPfirst_WARNING_POWER_LEVEL3        HPfirst_POWER_LIMIT_LEVEL3 * 0.9    
//ѡ��Ѫ�����ȵĵ��̹��ʾ���
#define POWfirst_WARNING_POWER_LEVEL1       POWfirst_POWER_LIMIT_LEVEL1 * 0.9 
#define POWfirst_WARNING_POWER_LEVEL2       POWfirst_POWER_LIMIT_LEVEL2 * 0.9  
#define POWfirst_WARNING_POWER_LEVEL3       POWfirst_POWER_LIMIT_LEVEL3 * 0.9  


#define WARNING_POWER_BUFF  		60.0f   			//���̻��幦��




/*************************************************************************************
��û���õ����幦�ʲ��ҹ��ʲ�����WARNING_POWERʱ������������������� BUFFER_TOTAL_CURRENT_LIMIT + POWER_TOTAL_CURRENT_LIMIT
��û���õ����幦�ʲ��ҹ��ʳ���WARNING_POWERʱ������������������� BUFFER_TOTAL_CURRENT_LIMIT + POWER_TOTAL_CURRENT_LIMIT * scale
���õ����湦��ʱ������������������� BUFFER_TOTAL_CURRENT_LIMIT * scale
**************************************************************************************/
//���湦�������ڵ�����ܵ���
#define BUFFER_TOTAL_CURRENT_LIMIT      14000.0f
//����������ڵ�����ܵ���
#define POWER_TOTAL_CURRENT_LIMIT       50000.0f

/**
  * @brief          limit the power, mainly limit motor current
  * @param[in]      chassis_power_control: chassis data 
  * @retval         none
  */
/**
  * @brief          ���ƹ��ʣ���Ҫ���Ƶ������
  * @param[in]      chassis_power_control: ��������
  * @retval         none
  */


//��ʼ��������Ϊ1��ʱ��Ļ����˵Ĺ�������
uint32_t warning_power = WARNING_POWER_LEVEL0;  
float power_limit = POWER_LIMIT_LEVEL0;  
float remain_current;				//��ǰʣ����������ݵĵ���
float available_current;			//��ǰ���õ���
fp32 LimitCurrent_temp = 0;
fp32 chassis_volt = 0;   //��λ��V
void chassis_power_control(chassis_move_t *chassis_power_control)
{
    fp32 chassis_power = 0.0f;
    fp32 chassis_power_buffer = 0.0f;
    fp32 total_current_limit = 0.0f;
    fp32 total_current = 0.0f;
	
	

	//��ȡ�����˵�״̬����
    uint8_t robot_id = get_robot_id();
	
	//��������ݵĳ�����
		chassis_volt = power_heat_data_t.chassis_volt / 1000;		//mvת��ΪV
		available_current = power_limit / chassis_volt;  			//���㵱ǰ����ʺ͵�ǰ��ѹ�����������������λA
		remain_current = available_current - (fp32)power_heat_data_t.chassis_current / 1000; //���㵱ǰ���õĵ�������λA
		
		if(remain_current<=0)
			remain_current = 0;
		//CMS_Hub.LimitCurrent = 1200;
		
		LimitCurrent_temp = remain_current *1.2 * 100;								//��ǰ���������ݹ���ĵ���Ϊʣ�������80%
		CMS_Hub.LimitCurrent = (uint16_t)LimitCurrent_temp ;							//��������ʽת��Ϊ�������ذ�Ŀ��Ƹ�ʽ
	
	
	//���ֱ��ģʽ�£��ò���ϵͳ���������ʱջ�����
	if(CMS_Hub.power_routin == CMS_PR_BattDirect)
	{
		get_chassis_power_and_buffer(&chassis_power, &chassis_power_buffer);		//��ȡ����ϵͳ�ĵ��̹���	
		// power > 80w and buffer < 60j, because buffer < 60 means power has been more than 80w
        //���ʳ���80w �ͻ�������С��60j,��Ϊ��������С��60��ζ�Ź��ʳ���80w
        if(chassis_power_buffer < WARNING_POWER_BUFF)
        {
            fp32 power_scale;
			//��ʣ�໺�幦�ʳ���10ʱ
            if(chassis_power_buffer > 10.0f)
            {
                //scale down WARNING_POWER_BUFF
                //��СWARNING_POWER_BUFF
				//���Ż��幦��ʹ�ñ�࣬����ʣ�໺�幦�ʱ�С��power_scale��С
                power_scale = chassis_power_buffer / WARNING_POWER_BUFF;
            }
            else
            {
				//�����̻��幦�ʲ���10ʱ,��ǰ�������Ϊ0
                //only left 10% of WARNING_POWER_BUFF
                power_scale = 0.0;//2.0f / WARNING_POWER_BUFF;
				
            }
            //scale down
            //��С
			//���ݵ�ǰ��ʣ�໺�幦�ʣ���������ĸ�����ĵ�����ֵ
			//��ʹ�õ����幦��ʱ�������ĸ�����ĵ����ܺͲ�����BUFFER_TOTAL_CURRENT_LIMIT
            total_current_limit = BUFFER_TOTAL_CURRENT_LIMIT * power_scale;
        }
		//��û��ʹ�õ����湦��ʱ
        else
        {
			//���ݵ�ǰ�����˵ĵȼ����»����˵������
			switch(robot_state.robot_level)
			{
			
			
				//������Ҫѡ�������Ȼ���Ѫ������
				#ifdef POW_first
				case 1:
					{warning_power	=	POWfirst_WARNING_POWER_LEVEL1; power_limit = POWfirst_POWER_LIMIT_LEVEL1;break;}
				case 2:
					{warning_power	=	POWfirst_WARNING_POWER_LEVEL2; power_limit = POWfirst_POWER_LIMIT_LEVEL2;break;}
				case 3:
					{warning_power	=	POWfirst_WARNING_POWER_LEVEL3; power_limit = POWfirst_POWER_LIMIT_LEVEL3;break;}
				default:
					{warning_power	=	WARNING_POWER_LEVEL0; power_limit = POWER_LIMIT_LEVEL0;break;}
			
				#else
				case 1:
					{warning_power	=	HPfirst_WARNING_POWER_LEVEL1; power_limit = HPfirst_POWER_LIMIT_LEVEL1;break;}
				case 2:
					{warning_power	=	HPfirst_WARNING_POWER_LEVEL2; power_limit = HPfirst_POWER_LIMIT_LEVEL2;break;}
				case 3:
					{warning_power	=	HPfirst_WARNING_POWER_LEVEL3; power_limit = HPfirst_POWER_LIMIT_LEVEL3;break;}
				default:
					{warning_power	=	WARNING_POWER_LEVEL0; power_limit = POWER_LIMIT_LEVEL0;break;}
					
				#endif
			}
			
			//power > WARNING_POWER
            //���ʴ���WARNING_POWER(���湦��)������û�г�������ʣ�δ�õ����幦��
            if(chassis_power > warning_power)
            {
                fp32 power_scale;
                //���ʵ�ǰ������ƹ���
                if(chassis_power < power_limit)
                {
                    //scale down
                    //��С
                    power_scale = (power_limit - chassis_power) / (power_limit - warning_power);
                    
                }
				//��ǰ���ʴ��������ʱ�����Ƶ�ǰ���������Ϊ0
                else
                {
                    power_scale = 0.0f;
                }
                
                total_current_limit = BUFFER_TOTAL_CURRENT_LIMIT + POWER_TOTAL_CURRENT_LIMIT * power_scale;
            }
            //power < WARNING_POWER
            //����С��WARNING_POWER
            else
            {
                total_current_limit = BUFFER_TOTAL_CURRENT_LIMIT + POWER_TOTAL_CURRENT_LIMIT;
            }
        }

   }
   else if(CMS_Hub.power_routin == CMS_PR_BuckBoost)
   {

			total_current_limit = 64000;

   }
			
    
    total_current = 0.0f;
    //calculate the original motor current set
    //����ԭ����������趨
	//��������ĸ�����ܹ��ĵ���ֵ
    for(uint8_t i = 0; i < 4; i++)
    {
        total_current += fabs(chassis_power_control->motor_speed_pid[i].out);
    }
    
	//��С����������������������Ƶ���Խ�󣬳߶���СԽ����
    if(total_current > total_current_limit)
    {
        fp32 current_scale = total_current_limit / total_current;
        chassis_power_control->motor_speed_pid[0].out*=current_scale;
        chassis_power_control->motor_speed_pid[1].out*=current_scale;
        chassis_power_control->motor_speed_pid[2].out*=current_scale;
        chassis_power_control->motor_speed_pid[3].out*=current_scale;
    }
	
	
	
	
	
	
	
	
	
	
	
}
