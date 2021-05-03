/******************************************************************************/
/** ģ�����ƣ�ModbusͨѶ                                                     **/
/** �ļ����ƣ�mbrtu.h                                                        **/
/** ��    ����V1.0.0                                                         **/
/** ��    �飺����ʵ��Modbus RTU����Э��ջADU�ķ�װ                          **/
/**           �����Ͷ���Modbus�����ڴ�����·�ϵ�ʵ�ֵ�������Ժͷ���         **/
/**           �Դ�����·RTU��ʽ���о�������                                  **/
/**                                                                          **/
/* һ�����͵�Modbus����֡�����²�����ɣ�                                     */
/* <------------------- MODBUS������·Ӧ�����ݵ�Ԫ��ADU�� ----------------->  */
/*              <------ MODBUS��Э�����ݵ�Ԫ��PDU�� ------->                */
/*  +-----------+---------------+----------------------------+-------------+  */
/*  |  ��ַ��   |    ������     | ������                     | CRC/LRC     |  */
/*  +-----------+---------------+----------------------------+-------------+  */
/**--------------------------------------------------------------------------**/
/** �޸ļ�¼��                                                               **/
/**     �汾      ����              ����              ˵��                   **/
/**     V1.0.0  2016-04-17          ���Ҿ�            �����ļ�               **/
/**                                                                          **/
/******************************************************************************/ 

#ifndef __mbrtu_h
#define __mbrtu_h

#include "mbpdu.h"


/*ͨ��CRCУ��У����յ���Ϣ�Ƿ���ȷ*/
bool CheckRTUMessageIntegrity (uint8_t *message,uint8_t length);

/*���ɶ�д��վ���ݶ��������,����Ȱ���2��У���ֽ�*/
uint16_t SyntheticReadWriteSlaveCommand(ObjAccessInfo slaveInfo,bool *statusList,uint16_t *registerList,uint8_t *commandBytes);

/*���ɴ�վӦ����վ����Ӧ*/
uint16_t SyntheticSlaveAccessRespond(uint8_t *receivedMesasage,bool *statusList,uint16_t *registerList,uint8_t *respondBytes);

uint16_t GenerateCRC16CheckCode(uint8_t *puckMsg, uint8_t usDataLen);

#endif


/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/
