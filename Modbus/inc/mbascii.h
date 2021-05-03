/******************************************************************************/
/** ģ�����ƣ�ModbusͨѶ                                                     **/
/** �ļ����ƣ�mbascii.c                                                      **/
/** ��    ����V1.0.0                                                         **/
/** ��    �飺����ʵ��Modbus ASCII����Э��ջADU�ķ�װ                        **/
/**           �����Ͷ���Modbus�����ڴ�����·�ϵ�ʵ�ֵ�������Ժͷ���         **/
/**           �Դ�����·ASCII��ʽ���о�������                                **/
/**                                                                          **/
/* һ�����͵�Modbus����֡�����²�����ɣ�                                     */
/* <------------------- MODBUS������·Ӧ�����ݵ�Ԫ��ADU�� ----------------->  */
/*              <------ MODBUS��Э�����ݵ�Ԫ��PDU�� ------->                */
/*  +-----------+---------------+----------------------------+-------------+  */
/*  |  ��ַ��   |    ������     | ������                     | CRC/LRC     |  */
/*  +-----------+---------------+----------------------------+-------------+  */
/*  ����ASCII������Ҫ��ǰ�������ʼ����������0x3A���Լ����������س�������0x0D���� */
/*  �����з�����0x0A��                                                         **/
/**--------------------------------------------------------------------------**/
/** �޸ļ�¼��                                                               **/
/**     �汾      ����              ����              ˵��                   **/
/**     V1.0.0  2018-09-11          ���Ҿ�            �����ļ�               **/
/**                                                                          **/
/******************************************************************************/

#ifndef __mbascii_h
#define __mbascii_h

#include "stdint.h"
#include "mbpdu.h"

/*���ɶ�д��վ�����Ӧ������վ����У�鼰��ʼ������*/
uint16_t SyntheticReadWriteAsciiSlaveCommand(ObjAccessInfo slaveInfo, bool *statusList, uint16_t *registerList, uint8_t *commandBytes);

/*����Ӧ����վ����Ӧ��Ӧ���ڴ�վ*/
uint16_t SyntheticAsciiSlaveAccessRespond(uint8_t *receivedMessage, bool *statusList, uint16_t *registerList, uint8_t *respondBytes);

/*���յ���ASCII��Ϣת��Ϊ16����*/
bool CovertAsciiMessageToHex(uint8_t *aMsg, uint8_t *hMsg, uint16_t aLen);

/*�ж�ASCII������Ϣ�Ƿ���ȷ*/
bool CheckASCIIMessageIntegrity(uint8_t *usMsg, uint16_t usLength);

#endif
/*********** (C) COPYRIGHT 1999-2018 Moonan Technology *********END OF FILE****/