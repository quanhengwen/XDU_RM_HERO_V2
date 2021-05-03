/**
  ****************************(C) COPYRIGHT 2019 DJI****************************
  * @file       shoot.c/h
  * @brief      射击功能.
  * @note       
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Dec-26-2018     RM              1. 完成
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2019 DJI****************************
  */

#include "shoot_task.h"
#include "main.h"
#include "cmsis_os.h"
#include "arm_math.h"
#include "user_lib.h"
#include "referee.h"

#include "bsp_can.h"
#include "gimbal_behaviour.h"
#include "detect_task.h"
#include "pid.h"

int16_t heat_temp;
int16_t shoot_stop_time = 0;
/**
  * @brief          射击状态机设置，遥控器上拨一次开启，再上拨关闭，下拨1次发射1颗，一直处在下，则持续发射，用于3min准备时间清理子弹
  * @param[in]      void
  * @retval         void
  */
static void shoot_set_mode(void);
/**
  * @brief          射击数据更新
  * @param[in]      void
  * @retval         void
  */
static void shoot_feedback_update(void);

/**
  * @brief          堵转倒转处理
  * @param[in]      void
  * @retval         void
  */
static void trigger_motor_turn_back(void);

/**
  * @brief          射击控制，控制拨弹电机角度，完成一次发射
  * @param[in]      void
  * @retval         void
  */
static void shoot_bullet_control(void);

static void shoot_set_control();

shoot_control_t shoot_control;          //射击数据


/**
  * @brief          射击初始化，初始化PID，遥控器指针，电机指针
  * @param[in]      void
  * @retval         返回空
  */
void shoot_init(void)
{


    shoot_control.shoot_mode = SHOOT_STOP;		//发射机构初始状态为停止发射行为
    //遥控器指针
    shoot_control.shoot_rc = get_remote_control_point();
    //电机指针
    shoot_control.shoot_motor_measure0 = get_shoot_motor_measure_point(0);		//获取摩擦轮电机数据指针
	shoot_control.shoot_motor_measure1 = get_shoot_motor_measure_point(1);		//获取摩擦轮电机数据指针
	shoot_control.trigle_motor_meaure =  get_trigle_motor_measure_point();		//获取拨盘电机数据指针

    //更新数据
    shoot_control.angle = shoot_control.shoot_motor_measure0->ecd * MOTOR_ECD_TO_ANGLE;
    shoot_control.given_current = 0;
    shoot_control.move_flag = 0;
    shoot_control.set_angle = shoot_control.angle;
    shoot_control.speed = 0.0f;
    shoot_control.speed_set = 0.0f;
    shoot_control.key_time = 0;
	shoot_control.fric0_speed_set = 0;
	shoot_control.fric1_speed_set = 0;


	shoot_control.shoot_start_flag = 1;			//发射使能标志位
	shoot_control.shoot_key_up_count = 0;
	 shoot_control.shoot_mode = SHOOT_STOP;
	 
	 
	 shoot_control.fric0_speed_set = SHOOT_SPEED_SET;
	 shoot_control.fric1_speed_set = -SHOOT_SPEED_SET;
	
}

/**
  * @brief          射击循环
  * @param[in]      void
  * @retval         返回无
  */
void shoot_task(void const *pvParameters)
{

	portTickType shoot_task_pre_tick = 0;
	shoot_init();
  while(1)
  {
	fp32 temp_ecd=0;
    shoot_set_mode();        //设置状态机
	shoot_set_control();	 //根据当前状态设置控制变量
    
    shoot_feedback_update(); //更新数据
    osDelayUntil(&shoot_task_pre_tick,1);
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_0);
   
 }
}

/**
  * @brief          射击状态机设置，遥控器上拨一次开启，再上拨关闭，下拨1次发射1颗，一直处在下，则持续发射，用于3min准备时间清理子弹
  * @param[in]      void
  * @retval         void
  */
static void shoot_set_mode(void)
{
    static int8_t last_s = RC_SW_UP;



	if((shoot_control.shoot_rc->key.v & KEY_PRESSED_OFFSET_Q) || (shoot_control.shoot_mode ==SHOOT_READY_FRIC) ||(shoot_control.shoot_mode == SHOOT_DONE) || shoot_control.shoot_mode == SHOOT_BULLET )
		shoot_control.fric_flag = 1;
	else if( (shoot_control.shoot_rc->key.v & KEY_PRESSED_OFFSET_E) || (shoot_control.shoot_mode ==SHOOT_STOP) )
		shoot_control.fric_flag = 0;



//判断摩擦轮是否开始转动
    //上拨判断， 一次开启，再次关闭
    if ((switch_is_up(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && !switch_is_up(last_s) && shoot_control.shoot_mode == SHOOT_STOP))
    {
       shoot_control.shoot_mode = SHOOT_READY_FRIC;
     }
    else if ((switch_is_up(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && !switch_is_up(last_s) && shoot_control.shoot_mode != SHOOT_STOP))
    {
        shoot_control.shoot_mode = SHOOT_STOP;
    }



    //处于中档， 可以使用键盘开启摩擦轮
    if (switch_is_mid(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && (shoot_control.shoot_rc->key.v & SHOOT_ON_KEYBOARD) && shoot_control.shoot_mode == SHOOT_STOP)
    {
        shoot_control.shoot_mode = SHOOT_READY_FRIC;
    }
    //处于中档， 可以使用键盘关闭摩擦轮
    else if (switch_is_mid(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && (shoot_control.shoot_rc->key.v & SHOOT_OFF_KEYBOARD) && shoot_control.shoot_mode != SHOOT_STOP)
    {
        shoot_control.shoot_mode = SHOOT_STOP;
		shoot_control.press_l = 0;
    }



//判断拨盘是否转动
	//摩擦轮开始转动之后进入即可进入预发弹模式
	if(shoot_control.shoot_mode == SHOOT_READY_FRIC)
    {
        //下拨一次或者鼠标左键按下一次，进入射击状态
        if ((switch_is_down(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && !switch_is_down(last_s) )|| ( shoot_control.press_l))//&& shoot_control.last_press_l == 0)  || (shoot_control.shoot_rc->key.v & KEY_PRESSED_OFFSET_R))//|| (shoot_control.press_r && shoot_control.last_press_r == 0))
        {
            shoot_control.shoot_mode = SHOOT_BULLET;
			 //shoot_control.press_l = 0;
        }
    }
	//每次发射完成之后必须经过一段时间才能进入下一次发射
	else if(shoot_control.shoot_mode == SHOOT_DONE)
	{	 
	    if ((switch_is_down(shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL]) && !switch_is_down(last_s)) || (shoot_control.press_l) )// && shoot_control.last_press_l == 0) )//|| (shoot_control.press_r && shoot_control.last_press_r == 0))
        {
            shoot_control.shoot_mode = SHOOT_BULLET;			
			//shoot_control.press_l = 0;
        }
		
		if(shoot_stop_time >= STOP_TIME)
		{
			if(shoot_control.move_flag == 0)
				shoot_control.press_l_time++;
		}
		else 
		{
			shoot_stop_time++;
		}
				
		
	}
	
	
	


    //如果云台状态是 无力状态，就关闭射击
    if (gimbal_cmd_to_shoot_stop())
    {
        //shoot_control.shoot_mode = SHOOT_STOP;
    }

    last_s = shoot_control.shoot_rc->rc.s[SHOOT_RC_MODE_CHANNEL];
}
/**
  * @brief          射击数据更新
  * @param[in]      void
  * @retval         void
  */
static void shoot_feedback_update(void)
{

	//每次发射完成之后进入按键抬起计数，只有鼠标左键抬起超过一定时间，
	if(shoot_control.shoot_mode == SHOOT_DONE)
	{
		if(shoot_control.shoot_rc->mouse.press_l == 0)
		{
			shoot_control.shoot_key_up_count++;
		}
		//防止数值溢出
		if(shoot_control.shoot_key_up_count > KEY_UP_TIME)
			shoot_control.shoot_key_up_count = KEY_UP_TIME + 100;
			
		//发射完成之后此时已经取消发射使能，即使检测到按键按下也不会更新按键按下标志位，发射状态也不会进入SHOOT_BULLET
		//当按键抬起时间超过一定界限之后，才能将发射使能标志位置位,每次发射结束又将发射使能标志位复位
		//防止子弹连发
		if(shoot_control.shoot_key_up_count >= KEY_UP_TIME)
		{
			shoot_control.shoot_start_flag = 1;
		}
	
	
	}
	//不在SHOOT_DONE状态下，不进行按键抬起检测，并将计数器归0
	else
	{
		shoot_control.shoot_key_up_count = 0;
	}
		
			


//控制鼠标按键检测的频率，消抖
		if((shoot_control.shoot_rc->mouse.press_l)&&(!shoot_control.press_l)&&((shoot_control.shoot_mode == SHOOT_READY_FRIC ) || (shoot_control.shoot_mode == SHOOT_DONE && (shoot_control.press_l_time >= PRESS_LONG_TIME) )) && (shoot_control.shoot_start_flag) )
		{
			shoot_control.press_l = 1;
			shoot_control.press_l_time = 0;
			shoot_stop_time = 0;
		}	



//电机圈数重置， 因为输出轴旋转一圈， 电机轴旋转 36圈，将电机轴数据处理成输出轴数据，用于控制输出轴角度
    if (shoot_control.trigle_motor_meaure->ecd - shoot_control.trigle_motor_meaure->last_ecd > HALF_ECD_RANGE)
    {
       shoot_control.ecd_count--;
    }
    else if (shoot_control.trigle_motor_meaure->ecd - shoot_control.trigle_motor_meaure->last_ecd < -HALF_ECD_RANGE)
    {
        shoot_control.ecd_count++;
    }

    if (shoot_control.ecd_count == FULL_COUNT)
    {
        shoot_control.ecd_count = -(FULL_COUNT - 1);
    }
    else if (shoot_control.ecd_count == -FULL_COUNT)
    {
        shoot_control.ecd_count = FULL_COUNT - 1;
    }

    //计算输出轴角度
    shoot_control.angle = (shoot_control.ecd_count * ECD_RANGE + shoot_control.trigle_motor_meaure->ecd) * MOTOR_ECD_TO_ANGLE;
    
	

}




//拨盘卡弹堵转检测保护
static void trigger_motor_turn_back(void)
{
    if( shoot_control.block_time < BLOCK_TIME)
    {
        shoot_control.trigger_speed_set = TRIGGER_SPEED;
    }
    else
    {
        shoot_control.trigger_speed_set = -TRIGGER_SPEED;
    }

    if(fabs(shoot_control.speed) < BLOCK_TRIGGER_SPEED && shoot_control.block_time < BLOCK_TIME)
    {
        shoot_control.block_time++;
        shoot_control.reverse_time = 0;
    }
    else if (shoot_control.block_time == BLOCK_TIME && shoot_control.reverse_time < REVERSE_TIME)
    {
        shoot_control.reverse_time++;
    }
    else
    {
        shoot_control.block_time = 0;
    }
}

/**
  * @brief          射击控制，控制拨弹电机角度，完成一次发射
  * @param[in]      void
  * @retval         void
  */
static void shoot_bullet_control(void)
{
	

	//设置发射紧急制动按键，如果当前按下关闭摩擦轮按键，就进入发射停止状态
	if(shoot_control.key == SHOOT_OFF_KEYBOARD)
    {
        shoot_control.shoot_mode = SHOOT_STOP;
		return ;
    }

    //每次转动一定角度，发射一颗子弹
    if( (shoot_control.move_flag == 0) ) 
    {

		shoot_control.set_angle = rad_format(shoot_control.angle - PI_FOUR);
		//shoot_control.ecd_set -= TRIGLE_ANGLE_ADD;					//减小当前的预设角度，指示拨盘转动
        shoot_control.move_flag = 1;
    }
	
	//系统故障决策和枪管热量限制

	//枪管热量限制
	//获取英雄机器人枪口每秒冷却值
    get_shoot_heat1_limit_and_heat1(&shoot_control.heat_limit, &shoot_control.heat);
	heat_temp = shoot_control.heat + SHOOT_HEAT_REMAIN_VALUE + 20;// - robot_state.shooter_id1_42mm_cooling_rate;
	//裁判系统离线，或者当前热量加上剩余热量超过上限
    if(//!toe_is_error(REFEREE_TOE) &&
	( heat_temp > shoot_control.heat_limit ))
    {	
		//发生故障，如果当前处于发射状态，退回到上一状态——摩擦轮启动状态（取消发射使能）
        if(shoot_control.shoot_mode == SHOOT_BULLET)//|| shoot_control.shoot_mode == SHOOT_CONTINUE_BULLET)
        {
            //shoot_control.shoot_mode = SHOOT_STOP;
			shoot_control.move_flag = 0;
			shoot_control.trigger_speed_set = 0;
        }
		return ;
    }
	
			//到达角度判断
    if(rad_format(shoot_control.set_angle - shoot_control.angle) < 0.003f)
//	( shoot_control.ecd_set > shoot_control.ecd_now)
    {
        //没到达一直设置旋转速度
        shoot_control.trigger_speed_set = TRIGGER_SPEED;
		trigger_motor_turn_back();
		//trigger_motor_turn_back();
    }
	
	else
	{
	
		 shoot_control.move_flag = 0;
		 shoot_control.trigger_speed_set = 0;
	}


}


//根据设计状态进行射击控制
static void shoot_set_control()
{
//拨盘控制电流状态机
    if(shoot_control.shoot_mode == SHOOT_STOP)
    {	
		//停止模式下摩擦轮和拨盘控制电流均为0
		 shoot_control.shoot_current[2] = 0;
		 shoot_control.move_flag = 0;
		 shoot_control.press_l = 0;
		 
		 shoot_control.set_angle = shoot_control.angle;
    }
    else if(shoot_control.shoot_mode  == SHOOT_READY_FRIC)
    {
		 shoot_control.shoot_current[2] = 0;
	}	 
		//摩擦轮准备没有完成时拨盘和摩擦轮电机电流为0
    else if(shoot_control.shoot_mode == SHOOT_BULLET)
        {
          
		  //进入射击拨盘角度控制
		   shoot_bullet_control();
		   
		  //做速度环控制，改变速度可以控制射频

		   if(shoot_control.move_flag == 0)
		   {
				 shoot_control.block_time = 0;
				shoot_control.press_l = 0;
				shoot_control.shoot_start_flag = 0;		//发射完成之后将发射使能关闭，等待检测到鼠标左键抬起来一段时间之后，才重新进行发射使能
				shoot_control.shoot_mode = SHOOT_DONE;
				
		   }
        }
}


