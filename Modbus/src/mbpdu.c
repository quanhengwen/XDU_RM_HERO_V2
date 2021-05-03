/******************************************************************************/
/** ģ�����ƣ�ModbusͨѶ                                                     **/
/** �ļ����ƣ�mbpdu.c                                                        **/
/** ��    ����V1.0.0                                                         **/
/** ��    �飺����ʵ��modbus����Э��ջPDU���ַ�װ                            **/
/**--------------------------------------------------------------------------**/
/* һ�����͵�Modbus����֡�����²�����ɣ�                                     */
/* <------------------- MODBUS������·Ӧ�����ݵ�Ԫ��ADU�� ----------------->  */
/*              <------ MODBUS��Э�����ݵ�Ԫ��PDU�� ------->                */
/*  +-----------+---------------+----------------------------+-------------+  */
/*  |  ��ַ��   |    ������     | ������                     | CRC/LRC     |  */
/*  +-----------+---------------+----------------------------+-------------+  */
/*                                                                            */
/* һ��TCP/IP Modbus����֡�����²�����ɣ�                                    */
/* <------------------- MODBUS TCP/IPӦ�����ݵ�Ԫ��ADU�� ------------------>  */
/*                       <-------- MODBUS��Э�����ݵ�Ԫ��PDU�� ---------->  */
/*  +--------------------+---------------+---------------------------------+  */
/*  |  MBAPͷ��          |    ������     | ������                          |  */
/*  +--------------------+---------------+---------------------------------+  */
/**--------------------------------------------------------------------------**/
/** �޸ļ�¼��                                                               **/
/**     �汾      ����              ����              ˵��                   **/
/**     V1.0.0  2015-06-17          ���Ҿ�            �����ļ�               **/
/**                                                                          **/
/******************************************************************************/ 

#include "mbpdu.h"

/*������������Ȧ������״̬������ת��ΪMB�ֽ�����,������������ĳ���*/
static uint16_t ConvertBoolArrayToMBByteArray(bool *sData,uint16_t length,uint8_t * oData);
/*���Ĵ���������Ĵ����ͱ��ּĴ���������ת��ΪMB�ֽ�����,������������ĳ���*/
static uint16_t ConvertRegisterArrayToMBByteArray(uint16_t *sData,uint16_t length,uint8_t * oData);
/*�����յ���дCoil�ֽ�����ת��Ϊ��������*/
static void ConvertMBByteArrayTotBoolArray(uint8_t * sData,bool *oData);
/*�����յ���д���ּĴ������ֽ�����רΪ�Ĵ�������*/
static void ConvertMBByteArrayToRegisterArray(uint8_t * sData,uint16_t *oData);

/*��ΪRTU��վ��TCP�ͻ��ˣ�ʱ�����ɶ�дRTU��վ��TCP�����������������*/
uint16_t GenerateReadWriteCommand(ObjAccessInfo objInfo,bool *statusList,uint16_t *registerList,uint8_t *commandBytes)
{
  uint16_t index=0;
  commandBytes[index++]=objInfo.unitID;                 //��վ��ַ
  commandBytes[index++]=objInfo.functionCode;           //������
  commandBytes[index++]=objInfo.startingAddress>>8;     //��ʼ��ַ���ֽ�
  commandBytes[index++]=objInfo.startingAddress;        //��ʼ��ַ���ֽ�
  
  /*����վ����*/
  if((objInfo.functionCode>=ReadCoilStatus)&&(objInfo.functionCode <= ReadInputRegister))
  {
    commandBytes[index++]=objInfo.quantity>>8;
    commandBytes[index++]=objInfo.quantity;
  }
  
  /*д������Ȧ���ݶ���*/
  if((WriteSingleCoil==objInfo.functionCode)&&(statusList!=NULL))
  {
    commandBytes[index++]=(*statusList)?0xFF:0x00;
    commandBytes[index++]=0x00;
  }
  
  /*д�����Ĵ������ݶ���*/
  if((objInfo.functionCode==WriteSingleRegister)&&(registerList!=NULL))
  {
    commandBytes[index++]=(*registerList)>>8;
    commandBytes[index++]=(*registerList);
  }
  
  /*д�����Ȧ*/
  if((objInfo.functionCode==WriteMultipleCoil)&&(statusList!=NULL))
  {
    commandBytes[index++]=objInfo.quantity>>8;
    commandBytes[index++]=objInfo.quantity;
    uint8_t byteArray[250];
    uint16_t bytesCount=ConvertBoolArrayToMBByteArray(statusList,objInfo.quantity,byteArray);
    commandBytes[index++]=bytesCount;
    for(int i=0;i<bytesCount;i++)
    {
      commandBytes[index++]=byteArray[i];
    }
  }
  
  /*д����Ĵ���*/
  if((objInfo.functionCode==WriteMultipleRegister)&&(registerList!=NULL))
  {
    commandBytes[index++]=objInfo.quantity>>8;		//�������ֽ�
    commandBytes[index++]=objInfo.quantity;             //�������ֽ�
    uint8_t byteArray[250];
    uint16_t bytesCount=ConvertRegisterArrayToMBByteArray(registerList,objInfo.quantity,byteArray);
    commandBytes[index++]=bytesCount;		//�ֽ�����
    for(int i=0;i<bytesCount;i++)
    {
      commandBytes[index++]=byteArray[i];
    }
  }
  return index;
}

/*������վ���ͻ��ˣ��ӷ�������ȡ������*/
void TransformClientReceivedData(uint8_t * receivedMessage,uint16_t quantity,bool *statusList,uint16_t *registerLister)
{
  FunctionCode fc=(FunctionCode)receivedMessage[1];
  uint16_t bytesCount=(uint16_t)receivedMessage[2];

  /*ת����Ȧ״̬������״̬����*/
  if(((fc==ReadInputStatus)||(fc==ReadCoilStatus))&&(statusList!=NULL))
  {
    for(int i=0;i<bytesCount;i++)
    {
      for(int j=0;j<8;j++)
      {
        if((i*8+j)<quantity)
        {
          statusList[i*8+j]=((receivedMessage[i+3]>>j)&0x01)?true:false;
        }
      }
    }
  }
  
  /*ת�����ּĴ���������Ĵ�������*/
  if(((fc==ReadHoldingRegister)||(fc==ReadInputRegister))&&(registerLister!=NULL))
  {
    if(bytesCount==quantity*2)
    {
      for(int i=0;i<quantity;i++)
      {
        registerLister[i]=(uint16_t)(receivedMessage[i*2+3]);
        registerLister[i]=(registerLister[i]<<8)+(uint16_t)(receivedMessage[i*2+4]);
      }
    }
  }
}

/*������վ�����ʵ���Ӧ������0x01��0x02��0x03��0x04������,������Ӧ��Ϣ�ĳ���*/
uint16_t GenerateMasterAccessRespond(uint8_t *receivedMessage,bool *statusList,uint16_t *registerList,uint8_t *respondBytes)
{
  uint16_t index=0;
  FunctionCode functionCode=(FunctionCode)(*(receivedMessage+1));
  if(CheckFunctionCode(functionCode)!=MB_OK)
  {
    return 0;
  }
  
  respondBytes[index++]=*receivedMessage;			//��վ��ַ
  respondBytes[index++]=*(receivedMessage+1);			//������
  
  /*����Ȧ��״̬��*/
  if(((functionCode==ReadCoilStatus)||(functionCode==ReadInputStatus))&&(statusList!=NULL))
  {
    uint16_t bitsQuantity=*(receivedMessage+4);
    bitsQuantity=(bitsQuantity<<8)+*(receivedMessage+5);
    uint8_t byteArray[250];
    memset(byteArray,0,sizeof(byteArray));
    uint16_t bytesCount=ConvertBoolArrayToMBByteArray(statusList,bitsQuantity,byteArray);
    respondBytes[index++]=bytesCount;//�ֽ���
    for(int i=0;i<bytesCount;i++)
    {
      respondBytes[index++]=byteArray[i];	//��д��λ���ֽ�
    }
  }

  /*���Ĵ�������*/
  if(((functionCode==ReadHoldingRegister)||(functionCode==ReadInputRegister))&&(registerList!=NULL))
  {
    uint16_t registerCount=*(receivedMessage+4);
    registerCount=(registerCount<<8)+*(receivedMessage+5);
    uint8_t byteArray[250];
    uint16_t bytesCount=ConvertRegisterArrayToMBByteArray(registerList,registerCount,byteArray);
    respondBytes[index++]=bytesCount;		//�ֽ�����
    for(int i=0;i<bytesCount;i++)
    {
      respondBytes[index++]=byteArray[i];	//��д����
    }
  }
  
  /*д�������*/
  if(functionCode>ReadInputRegister)
  {
    respondBytes[index++]=*(receivedMessage+2);
    respondBytes[index++]=*(receivedMessage+3);
    respondBytes[index++]=*(receivedMessage+4);
    respondBytes[index++]=*(receivedMessage+5);
    
    /*д�����Ȧ����*/
    if((functionCode==WriteMultipleCoil)&&(statusList!=NULL))
    {
      ConvertMBByteArrayTotBoolArray(receivedMessage,statusList);
    }
    
    /*д����Ĵ���*/
    if((functionCode==WriteMultipleRegister)&&(registerList!=NULL))
    {
      ConvertMBByteArrayToRegisterArray(receivedMessage,registerList);
    }
  }

  return index;
}

/*������������Ȧ������״̬������ת��ΪMB�ֽ�����,������������ĳ���*/
static uint16_t ConvertBoolArrayToMBByteArray(bool *sData,uint16_t length,uint8_t * oData)
{
  uint16_t returnLength=0;
  if(length>0)
  {
    returnLength=(length-1)/8+1;

    for(int i=0;i<returnLength;i++)
    {
      for(int j=0;j<8;j++)
      {
        if((i*8+j)<length)
        {
          oData[i]=oData[i]+((uint8_t)sData[i*8+j]<<j);
        }
      }
    }
  }
  
  return returnLength;
}

/*���Ĵ���������Ĵ����ͱ��ּĴ���������ת��ΪMB�ֽ�����,������������ĳ���*/
static uint16_t ConvertRegisterArrayToMBByteArray(uint16_t *sData,uint16_t length,uint8_t * oData)
{
  uint16_t returnLength=0;
  if(length>0)
  {
    for(int i=0;i<length;i++)
    {
      oData[returnLength++]=(sData[i]>>8);
      oData[returnLength++]=sData[i];
    }
  }
  return returnLength;
}

/*��鹦�����Ƿ���ȷ*/
ModbusStatus CheckFunctionCode(FunctionCode fc)
{
  ModbusStatus status=MB_OK;
  if((fc<ReadCoilStatus)||((fc>WriteSingleRegister)&&(fc<WriteMultipleCoil))||(fc>WriteMultipleRegister))
  {
    status=InvalidFunctionCode;
  }
  return status;
}

/*�����յ���дCoil�ֽ�����ת��Ϊ��������*/
static void ConvertMBByteArrayTotBoolArray(uint8_t * sData,bool *oData)
{
  uint16_t Count=(uint16_t)(*(sData+4));
  Count=(Count<<8)+(uint16_t)(*(sData+5));
  uint16_t byteCount=(uint16_t)(*(sData+6));
  
  for(int i=0;i<byteCount;i++)
  {
    for(int j=0;j<8;j++)
    {
      if((i*8+j)<Count)
      {
        oData[i*8+j]=((sData[i+7]>>j)&0x01)?true:false;
      }
    }
  }
}

/*�����յ���д���ּĴ������ֽ�����רΪ�Ĵ�������*/
static void ConvertMBByteArrayToRegisterArray(uint8_t * sData,uint16_t *oData)
{
  uint16_t Count=(uint16_t)(*(sData+4));
  Count=(Count<<8)+(uint16_t)(*(sData+5));
  
  for(int i=0;i<Count;i++)
  {
    oData[i]=(uint16_t)(sData[i*2+7]);
    oData[i]=(oData[i]<<8)+(uint16_t)(sData[i*2+8]);
  }
}

/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/
