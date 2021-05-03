/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       chassis_power_control.c/h
  * @brief      chassis power control.底盘功率控制
  * @note       this is only controling 80 w power, mainly limit motor current set.
  *             if power limit is 40w, reduce the value JUDGE_TOTAL_CURRENT_LIMIT 
  *             and POWER_CURRENT_LIMIT, and chassis max speed (include max_vx_speed, min_vx_speed)
  *             只控制80w功率，主要通过控制电机电流设定值,如果限制功率是40w，减少
  *             JUDGE_TOTAL_CURRENT_LIMIT和POWER_CURRENT_LIMIT的值，还有底盘最大速度
  *             (包括max_vx_speed, min_vx_speed)
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

//初始状态的底盘功率
#define POWER_LIMIT_LEVEL0         			50.0f

//选择血量优先的底盘功率
#define HPfirst_POWER_LIMIT_LEVEL1         	55.0f
#define HPfirst_POWER_LIMIT_LEVEL2         	60.0f
#define HPfirst_POWER_LIMIT_LEVEL3         	65.0f
//选择功率优先的底盘功率
#define POWfirst_POWER_LIMIT_LEVEL1         	60.0f//70.0f
#define POWfirst_POWER_LIMIT_LEVEL2         	90.0f
#define POWfirst_POWER_LIMIT_LEVEL3         	120.0f



#define WARNING_POWER_LEVEL0        POWER_LIMIT_LEVEL0 * 0.8 

//选择血量优先的底盘功率警告
#define HPfirst_WARNING_POWER_LEVEL1        HPfirst_POWER_LIMIT_LEVEL1 * 0.9 
#define HPfirst_WARNING_POWER_LEVEL2        HPfirst_POWER_LIMIT_LEVEL2 * 0.9    
#define HPfirst_WARNING_POWER_LEVEL3        HPfirst_POWER_LIMIT_LEVEL3 * 0.9    
//选择血量优先的底盘功率警告
#define POWfirst_WARNING_POWER_LEVEL1       POWfirst_POWER_LIMIT_LEVEL1 * 0.9 
#define POWfirst_WARNING_POWER_LEVEL2       POWfirst_POWER_LIMIT_LEVEL2 * 0.9  
#define POWfirst_WARNING_POWER_LEVEL3       POWfirst_POWER_LIMIT_LEVEL3 * 0.9  


#define WARNING_POWER_BUFF  		60.0f   			//底盘缓冲功率




/*************************************************************************************
当没有用到缓冲功率并且功率不超过WARNING_POWER时，电机的最大电流限制是 BUFFER_TOTAL_CURRENT_LIMIT + POWER_TOTAL_CURRENT_LIMIT
当没有用到缓冲功率并且功率超过WARNING_POWER时，电机的最大电流限制是 BUFFER_TOTAL_CURRENT_LIMIT + POWER_TOTAL_CURRENT_LIMIT * scale
当用到缓存功率时，电机的最大电流限制是 BUFFER_TOTAL_CURRENT_LIMIT * scale
**************************************************************************************/
//缓存功率限制内的最大总电流
#define BUFFER_TOTAL_CURRENT_LIMIT      14000.0f
//最大功率限制内的最大总电流
#define POWER_TOTAL_CURRENT_LIMIT       50000.0f

/**
  * @brief          limit the power, mainly limit motor current
  * @param[in]      chassis_power_control: chassis data 
  * @retval         none
  */
/**
  * @brief          限制功率，主要限制电机电流
  * @param[in]      chassis_power_control: 底盘数据
  * @retval         none
  */


//初始功率上限为1级时候的机器人的功率上限
uint32_t warning_power = WARNING_POWER_LEVEL0;  
float power_limit = POWER_LIMIT_LEVEL0;  
float remain_current;				//当前剩余给超级电容的电流
float available_current;			//当前可用电流
fp32 LimitCurrent_temp = 0;
fp32 chassis_volt = 0;   //单位是V
void chassis_power_control(chassis_move_t *chassis_power_control)
{
    fp32 chassis_power = 0.0f;
    fp32 chassis_power_buffer = 0.0f;
    fp32 total_current_limit = 0.0f;
    fp32 total_current = 0.0f;
	
	

	//获取机器人的状态数据
    uint8_t robot_id = get_robot_id();
	
	//计算给电容的充电电流
		chassis_volt = power_heat_data_t.chassis_volt / 1000;		//mv转化为V
		available_current = power_limit / chassis_volt;  			//计算当前最大功率和当前电压下允许的最大电流，单位A
		remain_current = available_current - (fp32)power_heat_data_t.chassis_current / 1000; //计算当前可用的电流，单位A
		
		if(remain_current<=0)
			remain_current = 0;
		//CMS_Hub.LimitCurrent = 1200;
		
		LimitCurrent_temp = remain_current *1.2 * 100;								//当前给超级电容供电的电流为剩余电流的80%
		CMS_Hub.LimitCurrent = (uint16_t)LimitCurrent_temp ;							//将电流格式转化为电容主控板的控制格式
	
	
	//电池直供模式下，用裁判系统功率做功率闭环限制
	if(CMS_Hub.power_routin == CMS_PR_BattDirect)
	{
		get_chassis_power_and_buffer(&chassis_power, &chassis_power_buffer);		//获取裁判系统的底盘功率	
		// power > 80w and buffer < 60j, because buffer < 60 means power has been more than 80w
        //功率超过80w 和缓冲能量小于60j,因为缓冲能量小于60意味着功率超过80w
        if(chassis_power_buffer < WARNING_POWER_BUFF)
        {
            fp32 power_scale;
			//当剩余缓冲功率超过10时
            if(chassis_power_buffer > 10.0f)
            {
                //scale down WARNING_POWER_BUFF
                //缩小WARNING_POWER_BUFF
				//随着缓冲功率使用变多，底盘剩余缓冲功率变小，power_scale变小
                power_scale = chassis_power_buffer / WARNING_POWER_BUFF;
            }
            else
            {
				//当底盘缓冲功率不足10时,当前输出电流为0
                //only left 10% of WARNING_POWER_BUFF
                power_scale = 0.0;//2.0f / WARNING_POWER_BUFF;
				
            }
            //scale down
            //缩小
			//根据当前的剩余缓冲功率，算出底盘四个电机的电流总值
			//当使用到缓冲功率时，底盘四个电机的电流总和不超过BUFFER_TOTAL_CURRENT_LIMIT
            total_current_limit = BUFFER_TOTAL_CURRENT_LIMIT * power_scale;
        }
		//当没有使用到缓存功率时
        else
        {
			//根据当前机器人的等级更新机器人的最大功率
			switch(robot_state.robot_level)
			{
			
			
				//根据需要选择功率优先或者血量优先
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
            //功率大于WARNING_POWER(警告功率)，但是没有超过最大功率，未用到缓冲功率
            if(chassis_power > warning_power)
            {
                fp32 power_scale;
                //功率当前最大限制功率
                if(chassis_power < power_limit)
                {
                    //scale down
                    //缩小
                    power_scale = (power_limit - chassis_power) / (power_limit - warning_power);
                    
                }
				//当前功率大于最大功率时，控制当前的输出电流为0
                else
                {
                    power_scale = 0.0f;
                }
                
                total_current_limit = BUFFER_TOTAL_CURRENT_LIMIT + POWER_TOTAL_CURRENT_LIMIT * power_scale;
            }
            //power < WARNING_POWER
            //功率小于WARNING_POWER
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
    //计算原本电机电流设定
	//计算底盘四个电机总共的电流值
    for(uint8_t i = 0; i < 4; i++)
    {
        total_current += fabs(chassis_power_control->motor_speed_pid[i].out);
    }
    
	//缩小底盘输出电流，当超过限制电流越大，尺度缩小越明显
    if(total_current > total_current_limit)
    {
        fp32 current_scale = total_current_limit / total_current;
        chassis_power_control->motor_speed_pid[0].out*=current_scale;
        chassis_power_control->motor_speed_pid[1].out*=current_scale;
        chassis_power_control->motor_speed_pid[2].out*=current_scale;
        chassis_power_control->motor_speed_pid[3].out*=current_scale;
    }
	
	
	
	
	
	
	
	
	
	
	
}
