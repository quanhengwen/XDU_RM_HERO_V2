/******************************************************************************/
/** ģ�����ƣ�ModbusͨѶ                                                     **/
/** �ļ����ƣ�mbtcpclient.c                                                  **/
/** ��    ����V1.0.0                                                         **/
/** ��    �飺��������Modbus TCP�ͻ��˵�������Լ�����                       **/
/**           1��Modbus TCP�ͻ����û�Ӧ�õĽӿڲ�                            **/
/**           2��ʵ�ַ�����������������ɲ����䴫��Ӧ�ò�                    **/
/**--------------------------------------------------------------------------**/
/** �޸ļ�¼��                                                               **/
/**     �汾      ����              ����              ˵��                   **/
/**     V1.0.0  2016-04-17          ���Ҿ�            �����ļ�               **/
/**                                                                          **/
/******************************************************************************/ 

#include "mbtcpclient.h"

/*������������ѷ��������б����������*/
void AddCommandBytesToList(TCPLocalClientType *client,uint8_t *commandBytes);
/*���յ�������Ϣ���ж��Ƿ��Ƿ��������б�������ķ�����Ϣ��*/
static int FindCommandForRecievedMessage(TCPLocalClientType *client,uint8_t *recievedMessage);
/*�������վ״̬��������Ϣ������Ȧ״̬λ0x01������*/
static void HandleReadCoilStatusRespond(TCPLocalClientType *client,uint8_t *receivedMesasage,uint16_t startAddress,uint16_t quantity);
/*�������վ״̬��������Ϣ��������״̬λ0x02������*/
static void HandleReadInputStatusRespond(TCPLocalClientType *client,uint8_t *receivedMesasage,uint16_t startAddress,uint16_t quantity);
/*�������վ�Ĵ���ֵ�ķ�����Ϣ�������ּĴ���0x03�����룩*/
static void HandleReadHoldingRegisterRespond(TCPLocalClientType *client,uint8_t *receivedMesasage,uint16_t startAddress,uint16_t quantity);
/*�������վ�Ĵ���ֵ�ķ�����Ϣ��������Ĵ���0x04������*/
static void HandleReadInputRegisterRespond(TCPLocalClientType *client,uint8_t *receivedMesasage,uint16_t startAddress,uint16_t quantity);

void (*HandleServerRespond[])(TCPLocalClientType *client,uint8_t *,uint16_t,uint16_t)={HandleReadCoilStatusRespond,
                                                                HandleReadInputStatusRespond,
                                                                HandleReadHoldingRegisterRespond,
                                                                HandleReadInputRegisterRespond};

/*���ɷ��ʷ�����������*/
uint16_t CreateAccessServerCommand(ObjAccessInfo objInfo,void *dataList,uint8_t *commandBytes)
{
  uint16_t commandLength=0;
  /*���ɶ���������������������0x01��0x02��0x03��0x04,�����12���ֽ�*/
  if((objInfo.functionCode>=ReadCoilStatus)&&(objInfo.functionCode <= ReadInputRegister))
  {
    commandLength=SyntheticReadWriteTCPServerCommand(objInfo,NULL,NULL,commandBytes);
  }

  /*����Ԥ�÷�������������������0x05,0x0F,������淢�����ݶ���*/
  if((objInfo.functionCode==WriteSingleCoil)&&(objInfo.functionCode==WriteMultipleCoil))
  {
    bool *statusList=(bool*)dataList;
    commandLength=SyntheticReadWriteTCPServerCommand(objInfo,statusList,NULL,commandBytes);
  }
  
  /*����Ԥ�÷�������������������0x06,0x10,������淢�����ݶ���*/
  if((objInfo.functionCode==WriteSingleRegister)&&(objInfo.functionCode==WriteMultipleRegister))
  {
    uint16_t *registerList=(uint16_t*)dataList;
    commandLength=SyntheticReadWriteTCPServerCommand(objInfo,NULL,registerList,commandBytes);
  }

  return commandLength;
}

/*�����յ��ķ�������Ӧ��Ϣ*/
void ParsingServerRespondMessage(TCPLocalClientType *client,uint8_t *recievedMessage)
{
  /*�жϽ��յ�����Ϣ�Ƿ�����Ӧ������*/
  int cmdIndex=FindCommandForRecievedMessage(client,recievedMessage);
  
  if((cmdIndex<0))      //û�ж�Ӧ�������������Ų����
  {
    return;
  }
  
  if((recievedMessage[2]!=0x00)||(recievedMessage[3]!=0x00)) //����Modbus TCPЭ��
  {
    return;
  }
  
  if(recievedMessage[7]>0x04)   //���������0x04���Ƕ������
  {
    return;
  }
  
  uint16_t mLength=(recievedMessage[4]<<8)+recievedMessage[4];
  uint16_t dLength=(uint16_t)recievedMessage[8];
  if(mLength!=dLength+3)        //���ݳ��Ȳ�һ��
  {
    return;
  }
  
  FunctionCode fuctionCode=(FunctionCode)recievedMessage[7];
  
  if(fuctionCode!=client->pReadCommand[cmdIndex][7])
  {
    return;
  }
  
  uint16_t startAddress=(uint16_t)client->pReadCommand[cmdIndex][8];
  startAddress=(startAddress<<8)+(uint16_t)client->pReadCommand[cmdIndex][9];
  uint16_t quantity=(uint16_t)client->pReadCommand[cmdIndex][10];
  quantity=(quantity<<8)+(uint16_t)client->pReadCommand[cmdIndex][11];
  
  if(quantity*2!=dLength)       //��������ݳ����뷵�ص����ݳ��Ȳ�һ��
  {
    return;
  }
  
  if((fuctionCode>=ReadCoilStatus)&&(fuctionCode<=ReadInputRegister))
  {
    HandleServerRespond[fuctionCode-1](client,recievedMessage,startAddress,quantity);
  }
}

/*������������ѷ��������б����������*/
void AddCommandBytesToList(TCPLocalClientType *client,uint8_t *commandBytes)
{
  if(client->cmdOrder>=client->cmdNumber)
  {
    client->cmdOrder=0;
  }
  
  for(int i=0;i<12;i++)
  {
    client->pReadCommand[client->cmdOrder][i]=commandBytes[i];
  }

  client->cmdOrder++;
}

/*���յ�������Ϣ���ж��Ƿ��Ƿ��������б�������ķ�����Ϣ��*/
/*��������ѷ��������б���ɾ�������������������������Ϣ*/
static int FindCommandForRecievedMessage(TCPLocalClientType *client,uint8_t *recievedMessage)
{
  int cmdIndex=-1;
  for(int i=0;i<client->cmdNumber;i++)
  {
    if((recievedMessage[0]==client->pReadCommand[i][0])&&(recievedMessage[1]==client->pReadCommand[i][1]))
    {
      cmdIndex=i;
      break;
    }
  }
  
  return cmdIndex;
}

/*�������վ״̬��������Ϣ������Ȧ״̬λ0x01������*/
static void HandleReadCoilStatusRespond(TCPLocalClientType *client,uint8_t *receivedMessage,uint16_t startAddress,uint16_t quantity)
{
  bool coilStatus[256];
  
  TransformClientReceivedData(receivedMessage+6,quantity,coilStatus,NULL);
  
  client->pUpdateCoilStatus(1,startAddress,quantity,coilStatus);
}

/*�������վ״̬��������Ϣ��������״̬λ0x02������*/
static void HandleReadInputStatusRespond(TCPLocalClientType *client,uint8_t *receivedMessage,uint16_t startAddress,uint16_t quantity)
{
  bool inputStatus[256];
  
  TransformClientReceivedData(receivedMessage+6,quantity,inputStatus,NULL);
  
  client->pUpdateInputStatus(1,startAddress,quantity,inputStatus);
}

/*�������վ�Ĵ���ֵ�ķ�����Ϣ�������ּĴ���0x03�����룩*/
static void HandleReadHoldingRegisterRespond(TCPLocalClientType *client,uint8_t *receivedMessage,uint16_t startAddress,uint16_t quantity)
{
  uint16_t holdingRegister[125];
  
  TransformClientReceivedData(receivedMessage+6,quantity,NULL,holdingRegister);
  
  client->pUpdateHoldingRegister(1,startAddress,quantity,holdingRegister);
}

/*�������վ�Ĵ���ֵ�ķ�����Ϣ��������Ĵ���0x04������*/
static void HandleReadInputRegisterRespond(TCPLocalClientType *client,uint8_t *receivedMessage,uint16_t startAddress,uint16_t quantity)
{
  uint16_t inputRegister[125];
  
  TransformClientReceivedData(receivedMessage+6,quantity,NULL,inputRegister);
  
  client->pUpdateInputResgister(1,startAddress,quantity,inputRegister);
}

/* ʵ����TCP���������� */
void InstantiateTCPServerObject(TCPAccessedServerType *server,
                                uint8_t ipSegment1,uint8_t ipSegment2,
                                uint8_t ipSegment3,uint8_t ipSegment4)
{
  server->ipAddress.ipSegment[0]=ipSegment1;
  server->ipAddress.ipSegment[1]=ipSegment2;
  server->ipAddress.ipSegment[2]=ipSegment3;
  server->ipAddress.ipSegment[3]=ipSegment4;
  
  server->flagPresetServer=0;
  
  server->pWritedCoilHeadNode.writedCoilNumber=0;
  server->pWritedCoilHeadNode.pWritedCoilNode=NULL;
  
  server->pWritedRegisterHeadNode.writedRegisterNumber=0;
  server->pWritedRegisterHeadNode.pWritedRegisterNode=NULL;
  
  server->pNextNode=NULL;
}

/*��ʼ��TCP�ͻ��˶���*/
void InitializeTCPClientObject(TCPLocalClientType *client,
                               uint16_t cmdNumber,
                               uint8_t (*pReadCommand)[12],
                               UpdateCoilStatusType pUpdateCoilStatus,
                               UpdateInputStatusType pUpdateInputStatus,
                               UpdateHoldingRegisterType pUpdateHoldingRegister,
                               UpdateInputResgisterType pUpdateInputResgister
                               )
{
  client->transaction=0;
  
  client->cmdNumber=cmdNumber;
  
  client->cmdOrder=0;
  
  client->pReadCommand=pReadCommand;
  
  client->ServerHeadNode.pServerNode=NULL;
  client->ServerHeadNode.serverNumber=0;

  client->pUpdateCoilStatus=pUpdateCoilStatus!=NULL?pUpdateCoilStatus:UpdateCoilStatus;
  
  client->pUpdateInputStatus=pUpdateInputStatus!=NULL?pUpdateInputStatus:UpdateInputStatus;
  
  client->pUpdateHoldingRegister=(pUpdateHoldingRegister!=NULL)?pUpdateHoldingRegister:UpdateHoldingRegister;
  
  client->pUpdateInputResgister=(pUpdateInputResgister!=NULL)?pUpdateInputResgister:UpdateInputResgister;
}

/* ��TCP�ͻ������TCP�������б�ڵ� */
void AddTCPServerNode(TCPLocalClientType *client,TCPAccessedServerType *server)
{
  TCPAccessedServerType *currentNode=NULL;
  
  if((client==NULL)||(server==NULL))
  {
    return;
  }
  
  currentNode=client->ServerHeadNode.pServerNode;
  
  if(currentNode==NULL)
  {
    client->ServerHeadNode.pServerNode=server;
  }
  else if(server->ipAddress.ipSegment[3]<currentNode->ipAddress.ipSegment[3])
  {
    client->ServerHeadNode.pServerNode=server;
    server->pNextNode=currentNode;
  }
  else
  {
    while(currentNode->pNextNode!=NULL)
    {
      if((currentNode->ipAddress.ipSegment[3]<=server->ipAddress.ipSegment[3])||(server->ipAddress.ipSegment[3]<currentNode->pNextNode->ipAddress.ipSegment[3]))
      {
        server->pNextNode=currentNode->pNextNode;
        currentNode->pNextNode=server;
        break;
      }
      else
      {
        currentNode=currentNode->pNextNode;
      }
    }
    
    if(currentNode->pNextNode==NULL)
    {
      currentNode->pNextNode=server;
    }
  }
  client->ServerHeadNode.serverNumber++;
}

/* ʹ�ܻ���ʧ��д��վ������־λ���޸Ĵ�վ��дʹ�ܱ�־λ�� */
void ModifyWriteTCPServerEnableFlag(TCPLocalClientType *client,uint8_t ipAddress,bool en)
{
  TCPAccessedServerType *currentNode;
  currentNode=client->ServerHeadNode.pServerNode;
  
  while(currentNode!=NULL)
  {
    if(currentNode->ipAddress.ipSegment[3]==ipAddress)
    {
      if(en)
      {
        currentNode->flagPresetServer=1;
      }
      else
      {
        currentNode->flagPresetServer=0;
      }
    }
    currentNode=currentNode->pNextNode;
  }
}

/* ��ô�վ��дʹ�ܱ�־λ��״̬ */
bool GetWriteTCPServerEnableFlag(TCPLocalClientType *client,uint8_t ipAddress)
{
  bool status=false;
  
  TCPAccessedServerType *currentNode;
  currentNode=client->ServerHeadNode.pServerNode;
  
  while(currentNode!=NULL)
  {
    if((currentNode->ipAddress.ipSegment[3]==ipAddress)&&(currentNode->flagPresetServer>0))
    {
      status=true;
      break;
    }
    currentNode=currentNode->pNextNode;
  }

  return status;
}

/* �жϵ�ǰ�Ƿ���д����ʹ�� */
bool CheckWriteTCPServerNone(TCPLocalClientType *client)
{
  bool status=true;
  
  TCPAccessedServerType *currentNode;
  currentNode=client->ServerHeadNode.pServerNode;
  
  while(currentNode!=NULL)
  {
    if(currentNode->flagPresetServer>0)
    {
      status=false;
      break;
    }
    currentNode=currentNode->pNextNode;
  }

  return status;
}

/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/