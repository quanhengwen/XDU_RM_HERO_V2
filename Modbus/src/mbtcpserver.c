/******************************************************************************/
/** ģ�����ƣ�ModbusͨѶ                                                     **/
/** �ļ����ƣ�mbtcpserver.c                                                  **/
/** ��    ����V1.0.0                                                         **/
/** ��    �飺����ʵ��Modbus TCP�������˵�������Լ�����                     **/
/**           1����ʼ����ΪServer����ز�������洢���                      **/
/**           2���������յ�����Ϣ���������ݶ��������Ӧ������������Ӧ��Ϣ    **/
/**--------------------------------------------------------------------------**/
/** �޸ļ�¼��                                                               **/
/**     �汾      ����              ����              ˵��                   **/
/**     V1.0.0  2016-04-17          ���Ҿ�            �����ļ�               **/
/**                                                                          **/
/******************************************************************************/ 

#include "mbtcpserver.h"

/*�������Ȧ״̬����*/
static uint16_t HandleReadCoilStatusCommand(uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes);
/*���������״̬����*/
static uint16_t HandleReadInputStatusCommand(uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes);
/*��������ּĴ�������*/
static uint16_t HandleReadHoldingRegisterCommand(uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes);
/*���������Ĵ�������*/
static uint16_t HandleReadInputRegisterCommand(uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes);
/*����д������Ȧ����*/
static uint16_t HandleWriteSingleCoilCommand(uint16_t coilAddress,uint16_t coilValue,uint8_t *receivedMessage,uint8_t *respondBytes);
/*����д�����Ĵ�������*/
static uint16_t HandleWriteSingleRegisterCommand(uint16_t registerAddress,uint16_t registerValue,uint8_t *receivedMessage,uint8_t *respondBytes);
/*����д�����Ȧ״̬*/
static uint16_t HandleWriteMultipleCoilCommand(uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes);
/*����д����Ĵ���״̬*/
static uint16_t HandleWriteMultipleRegisterCommand(uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes);

uint16_t (*HandleClientCommand[])(uint16_t,uint16_t,uint8_t *,uint8_t *)={HandleReadCoilStatusCommand,
                                                                          HandleReadInputStatusCommand,
                                                                          HandleReadHoldingRegisterCommand,
                                                                          HandleReadInputRegisterCommand,
                                                                          HandleWriteSingleCoilCommand,
                                                                          HandleWriteSingleRegisterCommand,
                                                                          HandleWriteMultipleCoilCommand,
                                                                          HandleWriteMultipleRegisterCommand};

/*�������յ�����Ϣ��������Ӧ����ĳ���*/
uint16_t ParsingClientAccessCommand(uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  FunctionCode fc=(FunctionCode)(*(receivedMessage+7));
  if(CheckFunctionCode(fc)!=MB_OK)
  {
    return 0;
  }
  
  uint16_t startAddress=(uint16_t)(*(receivedMessage+8));
  startAddress=(startAddress<<8)+(uint16_t)(*(receivedMessage+9));
  uint16_t quantity=(uint16_t)(*(receivedMessage+10));
  quantity=(quantity<<8)+(uint16_t)(*(receivedMessage+11));
  
  uint8_t index=(fc>0x08)?(fc-0x09):(fc-0x01);
  
  length=HandleClientCommand[index](startAddress,quantity,receivedMessage,respondBytes);

  return length;
}

/*�������Ȧ״̬����*/
static uint16_t HandleReadCoilStatusCommand(uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  bool statusList[250];
  
  GetCoilStatus(startAddress,quantity,statusList);
  
  length=SyntheticServerAccessRespond(receivedMessage,statusList,NULL,respondBytes);
    
  return length;
}

/*���������״̬����*/
static uint16_t HandleReadInputStatusCommand(uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  
  bool statusList[250];
  
  GetInputStatus(startAddress,quantity,statusList);
  
  length=SyntheticServerAccessRespond(receivedMessage,statusList,NULL,respondBytes);
  
  return length;
}

/*��������ּĴ�������*/
static uint16_t HandleReadHoldingRegisterCommand(uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  
  uint16_t registerList[125];
  
  GetHoldingRegister(startAddress,quantity,registerList);
  
  length=SyntheticServerAccessRespond(receivedMessage,NULL,registerList,respondBytes);
  
  return length;
}

/*���������Ĵ�������*/
static uint16_t HandleReadInputRegisterCommand(uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  
  uint16_t registerList[125];
  
  GetInputRegister(startAddress,quantity,registerList);
  
  length=SyntheticServerAccessRespond(receivedMessage,NULL,registerList,respondBytes);
  
  return length;
}

/*����д������Ȧ����*/
static uint16_t HandleWriteSingleCoilCommand(uint16_t coilAddress,uint16_t coilValue,uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  bool value;

  length=SyntheticServerAccessRespond(receivedMessage,NULL,NULL,respondBytes);
  
  GetCoilStatus(coilAddress,1,&value);
  
  value=CovertSingleCommandCoilToBoolStatus(coilValue,value);
  SetSingleCoil(coilAddress,value);
  
  return length;
}

/*����д�����Ĵ�������*/
static uint16_t HandleWriteSingleRegisterCommand(uint16_t registerAddress,uint16_t registerValue,uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;

  length=SyntheticServerAccessRespond(receivedMessage,NULL,NULL,respondBytes);
  
  SetSingleRegister(registerAddress,registerValue);
  
  return length;
}

/*����д�����Ȧ״̬*/
static uint16_t HandleWriteMultipleCoilCommand(uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  bool statusValue[250];
  
  length=SyntheticServerAccessRespond(receivedMessage,statusValue,NULL,respondBytes);
  
  SetMultipleCoil(startAddress,quantity,statusValue);

  return length;
}

/*����д����Ĵ���״̬*/
static uint16_t HandleWriteMultipleRegisterCommand(uint16_t startAddress,uint16_t quantity,uint8_t *receivedMessage,uint8_t *respondBytes)
{
  uint16_t length=0;
  uint16_t registerValue[125];

  length=SyntheticServerAccessRespond(receivedMessage,NULL,registerValue,respondBytes);
  
  SetMultipleRegister(startAddress,quantity,registerValue);
  return length;
}

/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/