/******************************************************************************/
/** ģ�����ƣ�ModbusͨѶ                                                     **/
/** �ļ����ƣ�mbcommon.h                                                     **/
/** ��    ����V1.0.0                                                         **/
/** ��    �飺����ʵ��Modbus��������µĹ��ò���                             **/
/**--------------------------------------------------------------------------**/
/** �޸ļ�¼��                                                               **/
/**     �汾      ����              ����              ˵��                   **/
/**     V1.0.0  2015-07-18          ���Ҿ�            �����ļ�               **/
/**                                                                          **/
/******************************************************************************/ 

#ifndef __mbcommon_h
#define __mbcommon_h

#include "stdbool.h"
#include "stdint.h"

/*������¶�ȡ�����Ķ���ֵ�ĺ���ָ������*/
/*���¶���������Ȧ״̬*/
typedef void (*UpdateCoilStatusType)(uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,bool *stateValue);

/*���¶�����������״ֵ̬*/
typedef void (*UpdateInputStatusType)(uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,bool *stateValue);

/*���¶������ı��ּĴ���*/
typedef void (*UpdateHoldingRegisterType)(uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,uint16_t *registerValue);

/*���¶�����������Ĵ���*/
typedef void (*UpdateInputResgisterType)(uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,uint16_t *registerValue);

/*�����յ���д����Coilֵת��Ϊ����������Ӧ0x05������*/
bool CovertSingleCommandCoilToBoolStatus(uint16_t coilValue,bool value);

/*������д�����Ƿ����������Ҫ��Χ������(�����ȸ�����)*/
float CheckWriteFloatDataIsValid(float value,float range,float zero);
 
 /*������д�����Ƿ����������Ҫ��Χ������(˫���ȸ�����)*/
double CheckWriteDoubleDataIsValid(double value,double range,double zero);

/*������д�����Ƿ����������Ҫ��Χ������(16λ����)*/
uint16_t CheckWriteInt16DataIsValid(uint16_t value,uint16_t range,uint16_t zero);
 
 /*������д�����Ƿ����������Ҫ��Χ������(32λ����)*/
uint32_t CheckWriteInt32DataIsValid(uint32_t value,uint32_t range,uint32_t zero);

/*��ȡ��Ҫ��ȡ��Coil����ֵ*/
void GetCoilStatus(uint16_t startAddress,uint16_t quantity,bool *statusList);

/*��ȡ��Ҫ��ȡ��InputStatus����ֵ*/
void GetInputStatus(uint16_t startAddress,uint16_t quantity,bool *statusValue);

/*��ȡ��Ҫ��ȡ�ı��ּĴ�����ֵ*/
void GetHoldingRegister(uint16_t startAddress,uint16_t quantity,uint16_t *registerValue);

/*��ȡ��Ҫ��ȡ������Ĵ�����ֵ*/
void GetInputRegister(uint16_t startAddress,uint16_t quantity,uint16_t *registerValue);

/*���õ�����Ȧ��ֵ*/
void SetSingleCoil(uint16_t coilAddress,bool coilValue);

/*���õ����Ĵ�����ֵ*/
void SetSingleRegister(uint16_t registerAddress,uint16_t registerValue);

/*���ö����Ȧ��ֵ*/
void SetMultipleCoil(uint16_t startAddress,uint16_t quantity,bool *statusValue);

/*���ö���Ĵ�����ֵ*/
void SetMultipleRegister(uint16_t startAddress,uint16_t quantity,uint16_t *registerValue);

/*���¶���������Ȧ״̬*/
void UpdateCoilStatus(uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,bool *stateValue);

/*���¶�����������״ֵ̬*/
void UpdateInputStatus(uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,bool *stateValue);

/*���¶������ı��ּĴ���*/
void UpdateHoldingRegister(uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,uint16_t *registerValue);

/*���¶�����������Ĵ���*/
void UpdateInputResgister(uint8_t salveAddress,uint16_t startAddress,uint16_t quantity,uint16_t *registerValue);

#endif //__mbcommon_h

/*********** (C) COPYRIGHT 1999-2019 Moonan Technology *********END OF FILE****/
