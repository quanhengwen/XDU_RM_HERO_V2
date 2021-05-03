/******************************************************************************/
/** ģ�����ƣ�ModbusͨѶ                                                     **/
/** �ļ����ƣ�mbasciimaster.c                                                **/
/** ��    ����V1.0.0                                                         **/
/** ��    �飺��������Modbus ASCII��վ������Լ�����                         **/
/**--------------------------------------------------------------------------**/
/** �޸ļ�¼��                                                               **/
/**     �汾      ����              ����              ˵��                   **/
/**     V1.0.0  2018-09-11          ���Ҿ�            �����ļ�               **/
/**                                                                          **/
/******************************************************************************/

#include "mbasciimaster.h"

/*�������վ״̬��������Ϣ������Ȧ״̬λ0x01������*/
static void HandleReadCoilStatusRespond(AsciiLocalMasterType *master,uint8_t *receivedMesasage, uint16_t startAddress, uint16_t quantity);
/*�������վ״̬��������Ϣ��������״̬λ0x02������*/
static void HandleReadInputStatusRespond(AsciiLocalMasterType *master,uint8_t *receivedMesasage, uint16_t startAddress, uint16_t quantity);
/*�������վ�Ĵ���ֵ�ķ�����Ϣ�������ּĴ���0x03�����룩*/
static void HandleReadHoldingRegisterRespond(AsciiLocalMasterType *master,uint8_t *receivedMesasage, uint16_t startAddress, uint16_t quantity);
/*�������վ�Ĵ���ֵ�ķ�����Ϣ��������Ĵ���0x04������*/
static void HandleReadInputRegisterRespond(AsciiLocalMasterType *master,uint8_t *receivedMesasage, uint16_t startAddress, uint16_t quantity);
/*�жϽ��յ�����Ϣ�Ƿ��Ƿ�������ķ�����Ϣ*/
static bool CheckMessageAgreeWithCommand(uint8_t *recievedMessage, uint8_t *command);

void (*HandleAsciiSlaveRespond[])(AsciiLocalMasterType *master,uint8_t *, uint16_t, uint16_t) = {HandleReadCoilStatusRespond,
                                                                    HandleReadInputStatusRespond,
                                                                    HandleReadHoldingRegisterRespond,
                                                                    HandleReadInputRegisterRespond};

/*��������CreateAccessSlaveCommand�����ɷ��ʷ�����������*/
/*������ObjAccessInfo objInfo,Ҫ���ɷ�������Ķ�����Ϣ*/
/*      void *dataList,д�������б��Ĵ���Ϊuint16_t���ͣ�״̬��Ϊbool����*/
/*      uint8_t *commandBytes,���ɵ������б�*/
/*����ֵ��uint16_t�����ɵ�����ĳ���*/
uint16_t CreateAccessAsciiSlaveCommand(ObjAccessInfo objInfo, void *dataList, uint8_t *commandBytes)
{
    uint16_t commandLength = 0;
    /*���ɶ���������������������0x01��0x02��0x03��0x04,�����17���ֽ�*/
    if ((objInfo.functionCode >= ReadCoilStatus) && (objInfo.functionCode <= ReadInputRegister))
    {
        commandLength = SyntheticReadWriteAsciiSlaveCommand(objInfo, NULL, NULL, commandBytes);
    }

    /*����Ԥ�÷�������������������0x05,0x0F,������淢�����ݶ���*/
    if ((objInfo.functionCode == WriteSingleCoil) || (objInfo.functionCode == WriteMultipleCoil))
    {
        bool *statusList = (bool *)dataList;
        commandLength = SyntheticReadWriteAsciiSlaveCommand(objInfo, statusList, NULL, commandBytes);
    }

    /*����Ԥ�÷�������������������0x06,0x10,������淢�����ݶ���*/
    if ((objInfo.functionCode == WriteSingleRegister) || (objInfo.functionCode == WriteMultipleRegister))
    {
        uint16_t *registerList = (uint16_t *)dataList;
        commandLength = SyntheticReadWriteAsciiSlaveCommand(objInfo, NULL, registerList, commandBytes);
    }

    return commandLength;
}

/*�����յ��ķ�������Ӧ��Ϣ*/
/*uint8_t *recievedMessage,���յ�����Ϣ�б�*/
/*uint8_t *command,���͵Ķ����������ΪNULL���������б��в���*/
void ParsingAsciiSlaveRespondMessage(AsciiLocalMasterType *master,uint8_t *recievedMessage, uint8_t *command,uint16_t rxLength)
{
    int i=0;
    int j=0;
    uint8_t *cmd=NULL;
    
    /*�ж��Ƿ�ΪModbus ASCII��Ϣ*/
    if (0x3A != recievedMessage[0])
    {
        return ;
    }

    /*�ж���Ϣ�Ƿ��������*/
    if ((rxLength < 17) || (recievedMessage[rxLength - 2] != 0x0D) || (recievedMessage[rxLength - 1] != 0x0A))
    {
        return ;
    }

    uint16_t length = rxLength - 3;
    uint8_t hexMessage[256];

    if (!CovertAsciiMessageToHex(recievedMessage + 1, hexMessage, length))
    {
        return ;
    }
    
    /*У����յ��������Ƿ���ȷ*/
    if (!CheckASCIIMessageIntegrity(hexMessage, length/2))
    {
        return ;
    }
    
    /*�жϹ������Ƿ�����*/
    FunctionCode fuctionCode = (FunctionCode)hexMessage[1];
    if (CheckFunctionCode(fuctionCode) != MB_OK)
    {
        return;
    }

    if ((command == NULL)||(!CheckMessageAgreeWithCommand(recievedMessage, command)))
    {
        while(i<master->slaveNumber)
        {
            if(master->pSlave[i].stationAddress==hexMessage[0])
            {
                break;
            }
            i++;
        }
        
        if(i>=master->slaveNumber)
        {
            return;
        }
    
        if((master->pSlave[i].pLastCommand==NULL)||(!CheckMessageAgreeWithCommand(recievedMessage,master->pSlave[i].pLastCommand)))
        {
            j=FindAsciiCommandForRecievedMessage(recievedMessage,master->pSlave[i].pReadCommand,master->pSlave[i].commandNumber);
      
            if(j<0)
            {
                return;
            }
      
            cmd=master->pSlave[i].pReadCommand[j];
        }
        else
        {
            cmd=master->pSlave[i].pLastCommand;
        }
    }
    else
    {
        cmd=command;
    }

    uint8_t hexCommand[256];
    CovertAsciiMessageToHex(cmd + 1, hexCommand, 14);

    uint16_t startAddress = (uint16_t)hexCommand[2];
    startAddress = (startAddress << 8) + (uint16_t)hexCommand[3];
    uint16_t quantity = (uint16_t)hexCommand[4];
    quantity = (quantity << 8) + (uint16_t)hexCommand[5];

    if ((fuctionCode >= ReadCoilStatus) && (fuctionCode <= ReadInputRegister))
    {
        HandleAsciiSlaveRespond[fuctionCode - 1](master,hexMessage,startAddress,quantity);
    }
}

/*���յ�������Ϣ���ж��Ƿ��Ƿ��������б�������ķ�����Ϣ��*/
/*��������ѷ��������б���ɾ�������������������������Ϣ*/
int FindAsciiCommandForRecievedMessage(uint8_t *recievedMessage, uint8_t (*commandList)[17], uint16_t commandNumber)
{
    int cmdIndex = -1;

    for (int i = 0; i < commandNumber; i++)
    {
        if (CheckMessageAgreeWithCommand(recievedMessage, commandList[i]) == true)
        {
            cmdIndex = i;
            break;
        }
    }
    return cmdIndex;
}

/*�жϽ��յ�����Ϣ�Ƿ��Ƿ�������ķ�����Ϣ*/
static bool CheckMessageAgreeWithCommand(uint8_t *recievedMessage, uint8_t *command)
{
    bool aw = false;

    if ((recievedMessage[1] == command[1]) && (recievedMessage[2] == command[2]) && (recievedMessage[3] == command[3]) && (recievedMessage[4] == command[5]))
    {
        uint8_t hexMessage[256];
        uint8_t hexCommand[256];

        CovertAsciiMessageToHex(command + 1, hexCommand, 14);
        CovertAsciiMessageToHex(recievedMessage + 1, hexMessage, 6);

        uint16_t quantity = (uint16_t)hexCommand[4];
        quantity = (quantity << 8) + (uint16_t)hexCommand[5];
        uint8_t bytescount = 0;
        if ((hexMessage[1] == ReadCoilStatus) || (hexMessage[1] == ReadInputStatus))
        {
            bytescount = (uint8_t)((quantity - 1) / 8 + 1);
        }

        if ((hexMessage[1] == ReadHoldingRegister) || (hexMessage[1] == ReadInputRegister))
        {
            bytescount = quantity * 2;
        }

        if (hexMessage[2] == bytescount)
        {
            aw = true;
        }
    }

    return aw;
}
/*�������վ״̬��������Ϣ������Ȧ״̬λ0x01������*/
static void HandleReadCoilStatusRespond(AsciiLocalMasterType *master,uint8_t *receivedMessage, uint16_t startAddress, uint16_t quantity)
{
    bool coilStatus[256];

    TransformClientReceivedData(receivedMessage, quantity, coilStatus, NULL);
    
    uint8_t slaveAddress=receivedMessage[0];

    master->pUpdateCoilStatus(slaveAddress,startAddress, quantity, coilStatus);
}

/*�������վ״̬��������Ϣ��������״̬λ0x02������*/
static void HandleReadInputStatusRespond(AsciiLocalMasterType *master,uint8_t *receivedMessage, uint16_t startAddress, uint16_t quantity)
{
    bool inputStatus[256];

    TransformClientReceivedData(receivedMessage, quantity, inputStatus, NULL);
    
    uint8_t slaveAddress=receivedMessage[0];

    master->pUpdateInputStatus(slaveAddress,startAddress, quantity, inputStatus);
}

/*�������վ�Ĵ���ֵ�ķ�����Ϣ�������ּĴ���0x03�����룩*/
static void HandleReadHoldingRegisterRespond(AsciiLocalMasterType *master,uint8_t *receivedMessage, uint16_t startAddress, uint16_t quantity)
{
    uint16_t holdingRegister[125];

    TransformClientReceivedData(receivedMessage, quantity, NULL, holdingRegister);
    
    uint8_t slaveAddress=receivedMessage[0];

    master->pUpdateHoldingRegister(slaveAddress,startAddress, quantity, holdingRegister);
}

/*�������վ�Ĵ���ֵ�ķ�����Ϣ��������Ĵ���0x04������*/
static void HandleReadInputRegisterRespond(AsciiLocalMasterType *master,uint8_t *receivedMessage, uint16_t startAddress, uint16_t quantity)
{
    uint16_t inputRegister[125];

    TransformClientReceivedData(receivedMessage, quantity, NULL, inputRegister);
    
    uint8_t slaveAddress=receivedMessage[0];

    master->pUpdateInputResgister(slaveAddress,startAddress, quantity, inputRegister);
}

/*��ʼ��ASCII��վ����*/
void InitializeASCIIMasterObject(AsciiLocalMasterType *master,uint16_t slaveNumber,
                            AsciiAccessedSlaveType *pSlave,
                            UpdateCoilStatusType pUpdateCoilStatus,
                            UpdateInputStatusType pUpdateInputStatus,
                            UpdateHoldingRegisterType pUpdateHoldingRegister,
                            UpdateInputResgisterType pUpdateInputResgister
                            )
{
  master->slaveNumber=slaveNumber>255?255:slaveNumber;
  
  master->readOrder=0;
  
  master->pSlave=pSlave;
  
  for(int i=0;i<8;i++)
  {
    master->flagWriteSlave[i]=0x00000000;
  }
  
  master->pUpdateCoilStatus=pUpdateCoilStatus!=NULL?pUpdateCoilStatus:UpdateCoilStatus;
  

  master->pUpdateInputStatus=pUpdateInputStatus!=NULL?pUpdateInputStatus:UpdateInputStatus;
  
  master->pUpdateHoldingRegister=(pUpdateHoldingRegister!=NULL)?pUpdateHoldingRegister:UpdateHoldingRegister;
  
  master->pUpdateInputResgister=(pUpdateInputResgister!=NULL)?pUpdateInputResgister:UpdateInputResgister;
}

/* ʹ�ܻ���ʧ��д��վ������־λ���޸Ĵ�վ��дʹ�ܱ�־λ�� */
void ModifyWriteASCIISlaveEnableFlag(AsciiLocalMasterType *master,uint8_t slaveAddress,bool en)
{
  uint8_t row=0;
  uint8_t column=0;

  row=slaveAddress/32;
  column=slaveAddress%32;
  
  if(en)
  {
    master->flagWriteSlave[row]|=(0x00000001<<column);
  }
  else
  {
    master->flagWriteSlave[row]&=(~(0x00000001<<column));
  }
}

/* ��ô�վ��дʹ�ܱ�־λ��״̬ */
bool GetWriteASCIISlaveEnableFlag(AsciiLocalMasterType *master,uint8_t slaveAddress)
{
  bool status=false;
  uint8_t row=0;
  uint8_t column=0;

  row=slaveAddress/32;
  column=slaveAddress%32;
  
  if((master->flagWriteSlave[row]&(0x00000001<<column))==(0x00000001<<column))
  {
    status=true;
  }
  
  return status;
}

/* �жϵ�ǰ�Ƿ���д����ʹ�� */
bool CheckWriteASCIISlaveNone(AsciiLocalMasterType *master)
{
  bool status=true;
  
  for(int i=0;i<8;i++)
  {
    if(master->flagWriteSlave[i]>0x00000000)
    {
      status=false;
    }
  }
  
  return status;
}

/*********** (C) COPYRIGHT 1999-2018 Moonan Technology *********END OF FILE****/