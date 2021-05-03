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
/*  ����ASCII������Ҫ��ǰ�������ʼ����������0x3A��                           */
/*  �Լ����������س�������0x0D���� �����з�����0x0A��                        **/
/**--------------------------------------------------------------------------**/
/** �޸ļ�¼��                                                               **/
/**     �汾      ����           ����         ˵��                           **/
/**     V1.0.0  2018-09-11      ���Ҿ�     �����ļ�                          **/
/**                                                                          **/
/******************************************************************************/

#include "string.h"
#include "mbascii.h"

/*LRCУ�麯��*/
static uint8_t GenerateLRCCheckCode(uint8_t *usData, uint16_t usLength);
/*��0��F��16������ת��ΪASCII��*/
static uint8_t HexToASCII(uint8_t hData);
/*��ASCII��ת��Ϊ0��F��16������*/
static uint8_t ASCIIToHex(uint8_t aData);
/*��ASCII��Ϣ��תΪ16������Ϣ��*/
static ModbusStatus CovertAsciiMsgToHexMsg(uint8_t *aMsg, uint8_t *hMsg, uint16_t aLen);
/*��16������Ϣ��ת��ΪASCII��Ϣ��*/
static ModbusStatus CovertHexMsgToAsciiMsg(uint8_t *hMsg, uint8_t *aMsg, uint16_t hLen);

/* ���ɶ�д��վ�����Ӧ������վ����У�鼰��ʼ������ */
/* ����slaveInfo�����ڹ�������ķ��ʴ�վ��Ϣ */
/* ����statusList������д��վ��Ȧ��ʱ����ȦԤ��ֵ�б� */
/* ����registerList������д �Ĵ���ʱ���Ĵ�����Ԥ��ֵ�б�*/
/* ����commandBytes�����ɵ��������� */
/* ����ֵ�������ɵ�����ĳ��ȣ����ֽ�Ϊ��λ */
uint16_t SyntheticReadWriteAsciiSlaveCommand(ObjAccessInfo slaveInfo, bool *statusList, uint16_t *registerList, uint8_t *commandBytes)
{
    uint8_t command[256];
    uint16_t bytesCount = GenerateReadWriteCommand(slaveInfo, statusList, registerList, command);
    uint8_t lcr = GenerateLRCCheckCode(command, bytesCount);
    command[bytesCount++] = lcr;

    uint8_t AsciiCommand[256];
    bool status = CovertHexMsgToAsciiMsg(command, AsciiCommand, bytesCount);
    if (status != MB_OK)
    {
        return 0;
    }

    uint16_t index = 0;
    commandBytes[index++] = 0x3A;
    for (int i = 0; i < bytesCount * 2; i++)
    {
        commandBytes[index++] = command[i];
    }
    commandBytes[index++] = 0x0D;
    commandBytes[index++] = 0x0A;

    return index;
}

/* ����Ӧ����վ����Ӧ��Ӧ���ڴ�վ */
/* ������uint8_t *receivedMessage,���յ�����վ���� */
/* ������bool *statusList,��վ�����״̬����ֵ�б� */
/* ������uint16_t *registerList,��վ����ļĴ�������ֵ�б� */
/* ������uint8_t *respondBytes���ͻ������ɵ���Ӧ��վ����Ϣ */
/* ����ֵ�����ɵ���Ӧ��Ϣ�ĳ��� */
uint16_t SyntheticAsciiSlaveAccessRespond(uint8_t *receivedMessage, bool *statusList, uint16_t *registerList, uint8_t *respondBytes)
{
    uint16_t respondLength = 0;
    uint8_t respond[256];
    uint16_t length = GenerateMasterAccessRespond(receivedMessage, statusList, registerList, respond);
    uint8_t lcr = GenerateLRCCheckCode(respond, length);
    respond[length++] = lcr;

    uint8_t AsciiCommand[256];
    bool status = CovertHexMsgToAsciiMsg(respond, AsciiCommand, length);
    if (status != MB_OK)
    {
        return 0;
    }

    respondBytes[respondLength++] = 0x3A;
    for (int i = 0; i < length * 2; i++)
    {
        respondBytes[respondLength++] = AsciiCommand[i];
    }
    respondBytes[respondLength++] = 0x0D;
    respondBytes[respondLength++] = 0x0A;
    
    return respondLength;
}

/*���յ���ASCII��Ϣת��Ϊ16����*/
bool CovertAsciiMessageToHex(uint8_t *aMsg, uint8_t *hMsg, uint16_t aLen)
{
    bool checkResult = CovertAsciiMsgToHexMsg(aMsg, hMsg, aLen);

    if (checkResult == MB_OK)
    {
        return true;
    }

    return false;
}

/*��16������Ϣ��ת��ΪASCII��Ϣ��*/
static ModbusStatus CovertHexMsgToAsciiMsg(uint8_t *hMsg, uint8_t *aMsg, uint16_t hLen)
{
    if (hLen < 1)
    {
        return SlaveFailure;
    }

    for (uint16_t i = 0; i < hLen; i++)
    {
        aMsg[2 * i] = HexToASCII(hMsg[i] >> 4);
        aMsg[2 * i + 1] = HexToASCII(hMsg[i] & 0x0F);
    }

    return MB_OK;
}

/*��ASCII��Ϣ��תΪ16������Ϣ��*/
static ModbusStatus CovertAsciiMsgToHexMsg(uint8_t *aMsg, uint8_t *hMsg, uint16_t aLen)
{
    if ((aLen < 2) || (aLen % 2 != 0))
    {
        return SlaveFailure;
    }

    uint8_t msb, lsb;
    for (uint16_t i = 0; i < aLen; i = i + 2)
    {
        msb = ASCIIToHex(aMsg[i]);
        lsb = ASCIIToHex(aMsg[i + 1]);

        if ((msb == 0xFF) || (lsb == 0xFF))
        {
            return SlaveFailure;
        }

        hMsg[i / 2] = (msb << 4) + lsb;
    }

    return MB_OK;
}

/*��0��F��16������ת��ΪASCII��*/
static uint8_t HexToASCII(uint8_t hData)
{
    uint8_t aData;

    if ((hData <= 0x9))
    {
        aData = hData + 0x30;
    }
    else if ((hData >= 0xA) && (hData <= 0xF))
    {
        aData = hData + 0x37;
    }
    else
    {
        aData = 0xFF;
    }

    return aData;
}

/*��ASCII��ת��Ϊ0��F��16������*/
static uint8_t ASCIIToHex(uint8_t aData)
{
    uint8_t hData;

    if ((aData >= 0x30) && (aData <= 0x39))
    {
        hData = aData - 0x30;
    }
    else if ((aData >= 0x41) && (aData <= 0x46))
    {
        hData = aData - 0x37;
    }
    else if ((aData >= 0x61) && (aData <= 0x66))
    {
        hData = aData - 0x57;
    }
    else
    {
        hData = 0xFF;
    }

    return hData;
}

/*LRCУ�麯��*/
static uint8_t GenerateLRCCheckCode(uint8_t *usData, uint16_t usLength)
{
    uint8_t lrcResult = 0;
    uint8_t sum = 0;

    for (uint16_t i = 0; i < usLength; i++)
    {
        sum = sum + usData[i];
    }

    lrcResult = ~sum + 1;

    return lrcResult;
}

/*�ж�ASCII������Ϣ�Ƿ���ȷ*/
bool CheckASCIIMessageIntegrity(uint8_t *usMsg, uint16_t usLength)
{
    bool checkResult;
    checkResult = (GenerateLRCCheckCode(usMsg, usLength) == 0x00) ? true : false;
    return checkResult;
}

/*********** (C) COPYRIGHT 1999-2018 Moonan Technology *********END OF FILE****/