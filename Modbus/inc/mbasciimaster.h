/******************************************************************************/
/** ģ�����ƣ�ModbusͨѶ                                                     **/
/** �ļ����ƣ�mbasciimaster.h                                                **/
/** ��    ����V1.0.0                                                         **/
/** ��    �飺��������Modbus ASCII��վ������Լ�����                         **/
/**--------------------------------------------------------------------------**/
/** �޸ļ�¼��                                                               **/
/**     �汾      ����              ����              ˵��                   **/
/**     V1.0.0  2018-09-11          ���Ҿ�            �����ļ�               **/
/**                                                                          **/
/******************************************************************************/

#ifndef __modbusasciimaster_h
#define __modbusasciimaster_h

#include "mbascii.h"
#include "mbcommon.h"

/* ���屻����ASCII��վ�������� */
typedef  struct AccessedASCIISlaveType{
  uint8_t stationAddress;       //վ��ַ
  uint8_t cmdOrder;             //��ǰ�����������б��е�λ��
  uint16_t commandNumber;       //�����б������������
  uint8_t (*pReadCommand)[17];   //�������б�
  uint8_t *pLastCommand;        //��һ�η��͵�����
  uint32_t flagPresetCoil;      //Ԥ����Ȧ���Ʊ�־λ
  uint32_t flagPresetReg;       //Ԥ�üĴ������Ʊ�־λ
}AsciiAccessedSlaveType;

/* ���屾��ASCII��վ�������� */
typedef struct LocalASCIIMasterType{
  uint32_t flagWriteSlave[8];   //дһ��վ���Ʊ�־λ�����256��վ����վ��ַ��Ӧ��
  uint16_t slaveNumber;         //��վ�б��д�վ������
  uint16_t readOrder;           //��ǰ��վ�ڴ�վ�б��е�λ��
  AsciiAccessedSlaveType *pSlave;         //��վ�б�
  UpdateCoilStatusType pUpdateCoilStatus;       //������Ȧ������
  UpdateInputStatusType pUpdateInputStatus;     //��������״̬������
  UpdateHoldingRegisterType pUpdateHoldingRegister;     //���±��ּĴ���������
  UpdateInputResgisterType pUpdateInputResgister;       //��������Ĵ���������
}AsciiLocalMasterType;

/*���ɷ��ʷ�����������*/
uint16_t CreateAccessAsciiSlaveCommand(ObjAccessInfo objInfo,void *dataList,uint8_t *commandBytes);

/*�����յ��ķ�������Ӧ��Ϣ*/
void ParsingAsciiSlaveRespondMessage(AsciiLocalMasterType *master,uint8_t *recievedMessage, uint8_t *command,uint16_t rxLength);

/*���յ�������Ϣ���ж��Ƿ��Ƿ��������б�������ķ�����Ϣ*/
int FindAsciiCommandForRecievedMessage(uint8_t *recievedMessage,uint8_t (*commandList)[17],uint16_t commandNumber);

/* ʹ�ܻ���ʧ��д��վ������־λ���޸Ĵ�վ��дʹ�ܱ�־λ�� */
void ModifyWriteASCIISlaveEnableFlag(AsciiLocalMasterType *master,uint8_t slaveAddress,bool en);

/* ��ô�վ��дʹ�ܱ�־λ��״̬ */
bool GetWriteASCIISlaveEnableFlag(AsciiLocalMasterType *master,uint8_t slaveAddress);

/*��ʼ��RTU��վ����*/
void InitializeASCIIMasterObject(AsciiLocalMasterType *master,uint16_t slaveNumber,
                            AsciiAccessedSlaveType *pSlave,
                            UpdateCoilStatusType pUpdateCoilStatus,
                            UpdateInputStatusType pUpdateInputStatus,
                            UpdateHoldingRegisterType pUpdateHoldingRegister,
                            UpdateInputResgisterType pUpdateInputResgister
                            );

/* �жϵ�ǰ�Ƿ���д����ʹ�� */
bool CheckWriteASCIISlaveNone(AsciiLocalMasterType *master);

#endif
/*********** (C) COPYRIGHT 1999-2018 Moonan Technology *********END OF FILE****/