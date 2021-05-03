/******************************************************************************/
/** ģ�����ƣ�ModbusͨѶ                                                     **/
/** �ļ����ƣ�modbusrtuslave.c                                               **/
/** ��    ����V1.0.0                                                         **/
/** ��    �飺��������Modbus RTU��վ������Լ�����                           **/
/**--------------------------------------------------------------------------**/
/** �޸ļ�¼��                                                               **/
/**     �汾      ����              ����              ˵��                   **/
/**     V1.0.0  2016-04-17          ���Ҿ�            �����ļ�               **/
/**                                                                          **/
/******************************************************************************/

#include "mbrtuslave.h"

//#define StationAddress 0x01   /*���屾վ��ַ*/

/*�������Ȧ״̬����*/
static uint16_t HandleReadCoilStatusCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes);
/*���������״̬����*/
static uint16_t HandleReadInputStatusCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes);
/*��������ּĴ�������*/
static uint16_t HandleReadHoldingRegisterCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes);
/*���������Ĵ�������*/
static uint16_t HandleReadInputRegisterCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes);
/*����д������Ȧ����*/
static uint16_t HandleWriteSingleCoilCommand(uint16_t coilAddress, uint16_t coilValue, uint8_t *receivedMessage, uint8_t *respondBytes);
/*����д�����Ĵ�������*/
static uint16_t HandleWriteSingleRegisterCommand(uint16_t registerAddress, uint16_t registerValue, uint8_t *receivedMessage, uint8_t *respondBytes);
/*����д�����Ȧ״̬*/
static uint16_t HandleWriteMultipleCoilCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes);
/*����д����Ĵ���״̬*/
static uint16_t HandleWriteMultipleRegisterCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes);

uint16_t (*HandleMasterCommand[])(uint16_t, uint16_t, uint8_t *, uint8_t *) = {HandleReadCoilStatusCommand,
                                                                               HandleReadInputStatusCommand,
                                                                               HandleReadHoldingRegisterCommand,
                                                                               HandleReadInputRegisterCommand,
                                                                               HandleWriteSingleCoilCommand,
                                                                               HandleWriteSingleRegisterCommand,
                                                                               HandleWriteMultipleCoilCommand,
                                                                               HandleWriteMultipleRegisterCommand};

/*�������յ�����Ϣ�������غϳɵĻظ���Ϣ����Ϣ���ֽڳ��ȣ�ͨ���ص�����*/
uint16_t ParsingMasterAccessCommand(uint8_t *receivedMessage, uint8_t *respondBytes, uint16_t rxLength, uint8_t StationAddress)
{
  uint16_t respondLength = 0;
  /*�ж��Ƿ��Ǳ�վ���粻�ǲ�����*/
  uint8_t slaveAddress = *receivedMessage;
  if (slaveAddress != StationAddress)
  {
    return 0;
  }

  /*�жϹ������Ƿ�����*/
  FunctionCode fc = (FunctionCode)(*(receivedMessage + 1));
  if (CheckFunctionCode(fc) != MB_OK)
  {
    return 0;
  }

  /*��ϢУ�飬�粻��ȷ��Ϊ������Ϣ���ô���*/
  uint16_t dataLength = 8;
  if ((fc == WriteMultipleCoil) || (fc == WriteMultipleRegister))
  {
    dataLength = (uint16_t)(*(receivedMessage + 6)) + 9;
    if (rxLength < dataLength) //��δ��������
    {
      return 65535;
    }
  }
  bool chechMessageNoError = CheckRTUMessageIntegrity(receivedMessage, dataLength);
  if (!chechMessageNoError)
  {
    return 0;
  }

  uint16_t startAddress = (uint16_t)(*(receivedMessage + 2));
  startAddress = (startAddress << 8) + (uint16_t)(*(receivedMessage + 3));
  uint16_t quantity = (uint16_t)(*(receivedMessage + 4));
  quantity = (quantity << 8) + (uint16_t)(*(receivedMessage + 5));

  uint8_t index = (fc > 0x08) ? (fc - 0x09) : (fc - 0x01);

  respondLength = HandleMasterCommand[index](startAddress, quantity, receivedMessage, respondBytes);  //������ɵĺ������һ�μ�

  return respondLength;
}

/*�������Ȧ״̬����*/
static uint16_t HandleReadCoilStatusCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
  uint16_t length = 0;
  bool statusList[250];

  GetCoilStatus(startAddress, quantity, statusList);

  length = SyntheticSlaveAccessRespond(receivedMessage, statusList, NULL, respondBytes);

  return length;
}

/*���������״̬����*/
static uint16_t HandleReadInputStatusCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
  uint16_t length = 0;

  bool statusList[250];

  GetInputStatus(startAddress, quantity, statusList);

  length = SyntheticSlaveAccessRespond(receivedMessage, statusList, NULL, respondBytes);

  return length;
}

/*��������ּĴ�������*/
static uint16_t HandleReadHoldingRegisterCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
  uint16_t length = 0;

  uint16_t registerList[125];

  GetHoldingRegister(startAddress, quantity, registerList);

  length = SyntheticSlaveAccessRespond(receivedMessage, NULL, registerList, respondBytes);

  return length;
}

/*���������Ĵ�������*/
static uint16_t HandleReadInputRegisterCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
  uint16_t length = 0;

  uint16_t registerList[125];

  GetInputRegister(startAddress, quantity, registerList);

  length = SyntheticSlaveAccessRespond(receivedMessage, NULL, registerList, respondBytes);

  return length;
}

/*����д������Ȧ����*/
static uint16_t HandleWriteSingleCoilCommand(uint16_t coilAddress, uint16_t coilValue, uint8_t *receivedMessage, uint8_t *respondBytes)
{
  uint16_t length = 0;
  bool value;

  length = SyntheticSlaveAccessRespond(receivedMessage, NULL, NULL, respondBytes);

  GetCoilStatus(coilAddress, 1, &value);

  value = CovertSingleCommandCoilToBoolStatus(coilValue, value);
  SetSingleCoil(coilAddress, value);

  return length;
}

/*����д�����Ĵ�������*/
static uint16_t HandleWriteSingleRegisterCommand(uint16_t registerAddress, uint16_t registerValue, uint8_t *receivedMessage, uint8_t *respondBytes)
{
  uint16_t length = 0;

  length = SyntheticSlaveAccessRespond(receivedMessage, NULL, NULL, respondBytes);

  SetSingleRegister(registerAddress, registerValue);

  return length;
}

/*����д�����Ȧ״̬*/
static uint16_t HandleWriteMultipleCoilCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
  uint16_t length = 0;
  bool statusValue[250];

  length = SyntheticSlaveAccessRespond(receivedMessage, statusValue, NULL, respondBytes);

  SetMultipleCoil(startAddress, quantity, statusValue);

  return length;
}

/*����д����Ĵ���״̬*/
static uint16_t HandleWriteMultipleRegisterCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
  uint16_t length = 0;
  uint16_t registerValue[125];

  length = SyntheticSlaveAccessRespond(receivedMessage, NULL, registerValue, respondBytes);

  SetMultipleRegister(startAddress, quantity, registerValue);
  return length;
}

/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/
