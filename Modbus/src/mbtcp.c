/******************************************************************************/
/** ģ�����ƣ�ModbusͨѶ                                                     **/
/** �ļ����ƣ�mbtcp.c                                                        **/
/** ��    ����V1.0.0                                                         **/
/** ��    �飺����ʵ��Modbus TCP����Э��վADU�ķ�װ                          **/
/**           1��ʵ����Ϊ�ͻ���ʱ���ʷ������������������                    **/
/**           2��ʵ����Ϊ��������ʱ���Կͻ����������Ӧ��Ϣ������            **/
/**                                                                          **/
/* MBAP����ͷ�İ��������ݣ�                                                   */
/* +-------------+---------+--------+--------+------------------------------+ */
/* |     ��      |  ����   | �ͻ��� | ������ |              ����            | */
/* +-------------+---------+--------+--------+------------------------------+ */
/* |����Ԫ��ʶ�� | 2���ֽ� |  ����  |  ����  |����/��Ӧ�������ʶ����     | */
/* +-------------+---------+--------+--------+------------------------------+ */
/* |Э���ʶ��   | 2���ֽ� |  ����  |  ����  |0=MODBUS Э��                 | */
/* +-------------+---------+--------+--------+------------------------------+ */
/* |    ����     | 2���ֽ� |  ����  |  ����  |�����ֽڵ�����                | */
/* +-------------+---------+--------+--------+------------------------------+ */
/* |��Ԫ��ʶ��   | 1���ֽ� |  ����  |  ����  |���ӵ�Զ�̴�վ��ʶ����        | */
/* +-------------+---------+--------+---------------------------------------+ */
/**--------------------------------------------------------------------------**/
/** �޸ļ�¼��                                                               **/
/**     �汾      ����              ����              ˵��                   **/
/**     V1.0.0  2016-04-17          ���Ҿ�            �����ļ�               **/
/**                                                                          **/
/******************************************************************************/ 

#include "mbtcp.h"

uint16_t transactionIDCounter;

/*����MBAP����ͷ*/
static uint16_t CreateMbapHeadPart(uint8_t * mbapHead);

/*����MBAPͷ����,Ϊ�˷�����RTU��ͬ��������Ԫ��ʶ����Զ�̴�վ��ַ��������������ʱ����*/
/*�˴�MBAPͷֻ��7���ֽڣ����Ԫ��ʶ��ΪĬ��ֵ0x01*/
static uint16_t CreateMbapHeadPart(uint8_t * mbapHead)
{
	uint16_t index=0;
	//���������־
	if(transactionIDCounter==65535)
	{
		transactionIDCounter=1;
	}
	else
	{
		transactionIDCounter=transactionIDCounter+1;
	}
	mbapHead[index++]=transactionIDCounter>>8;//�����ʶ��
	mbapHead[index++]=transactionIDCounter;//�����ʶ��
	mbapHead[index++]=0x00;//Э���ʶ����modbus��ʶ��Ϊ0
	mbapHead[index++]=0x00;//Э���ʶ����modbus��ʶ��Ϊ0
	mbapHead[index++]=0x00;//�ֽڳ��ȣ��˴��в���ȷ�����ں�������
	mbapHead[index++]=0x01;//�ֽڳ��ȣ��˴��в���ȷ�����ں�������
	mbapHead[index++]=0x01;//��Ԫ��ʶ��Ĭ��ֵ
	return index;
}

/*���ɶ�д���������������*/
uint16_t SyntheticReadWriteTCPServerCommand(ObjAccessInfo objInfo,bool *statusList,uint16_t *registerList,uint8_t *commandBytes)
{
	uint8_t command[256];
	CreateMbapHeadPart(commandBytes);
	uint16_t index=4;
	uint16_t bytesCount=GenerateReadWriteCommand(objInfo,statusList,registerList,command);
	commandBytes[index++]=bytesCount>>8;
	commandBytes[index++]=bytesCount;	
	for(int i=0;i<bytesCount;i++)
	{
		commandBytes[index++]=command[i];
	}
	return index;
}

/*�ϳɶԷ��������ʵ���Ӧ,����ֵΪ�����*/
uint16_t SyntheticServerAccessRespond(uint8_t *receivedMessage,bool *statusList,uint16_t *registerList,uint8_t *respondBytes)
{
  uint16_t index=0;
  respondBytes[index++]=*receivedMessage;
  respondBytes[index++]=*(receivedMessage+1);
  respondBytes[index++]=*(receivedMessage+2);
  respondBytes[index++]=*(receivedMessage+3);
  uint8_t respond[260];
  uint16_t bytesCount=GenerateMasterAccessRespond(receivedMessage+6,statusList,registerList,respondBytes);
  respondBytes[index++]=(bytesCount>>8);
  respondBytes[index++]=bytesCount;
  
  for(int i=0;i<bytesCount;i++)
  {
    respondBytes[index++]=respond[i];
  }
  return index;
}

/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/