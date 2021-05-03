/******************************************************************************/
/** ģ�����ƣ�ModbusͨѶ                                                     **/
/** �ļ����ƣ�mbasciislave.c                                                 **/
/** ��    ����V1.0.0                                                         **/
/** ��    �飺��������Modbus ASCII��վ������Լ�����                         **/
/**--------------------------------------------------------------------------**/
/** �޸ļ�¼��                                                               **/
/**     �汾      ����              ����              ˵��                   **/
/**     V1.0.0  2018-09-11          ���Ҿ�            �����ļ�               **/
/**                                                                          **/
/******************************************************************************/

#include "mbasciislave.h"

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

uint16_t (*HandleAsciiMasterCommand[])(uint16_t, uint16_t, uint8_t *, uint8_t *) = {HandleReadCoilStatusCommand,
                                                                                    HandleReadInputStatusCommand,
                                                                                    HandleReadHoldingRegisterCommand,
                                                                                    HandleReadInputRegisterCommand,
                                                                                    HandleWriteSingleCoilCommand,
                                                                                    HandleWriteSingleRegisterCommand,
                                                                                    HandleWriteMultipleCoilCommand,
                                                                                    HandleWriteMultipleRegisterCommand};

/*�������յ�����Ϣ�������غϳɵĻظ���Ϣ����Ϣ���ֽڳ��ȣ�ͨ���ص�����*/
uint16_t ParsingAsciiMasterAccessCommand(uint8_t *receivedMessage, uint8_t *respondBytes, uint16_t rxLength, uint8_t StationAddress)
{
    uint16_t respondLength = 0;

    /*�ж��Ƿ�ΪModbus ASCII��Ϣ*/
    if (0x3A != receivedMessage[0])
    {
        return 0;
    }

    /*�ж���Ϣ�Ƿ��������*/
    if ((rxLength < 17) || (receivedMessage[rxLength - 2] != 0x0D) || (receivedMessage[rxLength - 1] != 0x0A))
    {
        return 65535;
    }

    uint16_t length = rxLength - 3;
    uint8_t hexMessage[256];

    if (!CovertAsciiMessageToHex(receivedMessage + 1, hexMessage, length))
    {
        return 0;
    }

    /*У����յ��������Ƿ���ȷ*/
    if (!CheckASCIIMessageIntegrity(hexMessage, length))
    {
        return 0;
    }

    /*�ж��Ƿ��Ǳ�վ���粻�ǲ�����*/
    uint8_t slaveAddress = *hexMessage;
    if (slaveAddress != StationAddress)
    {
        return 0;
    }

    /*�жϹ������Ƿ�����*/
    FunctionCode fc = (FunctionCode)(*(hexMessage + 1));
    if (CheckFunctionCode(fc) != MB_OK)
    {
        return 0;
    }

    uint16_t startAddress = (uint16_t)(*(hexMessage + 2));
    startAddress = (startAddress << 8) + (uint16_t)(*(hexMessage + 3));
    uint16_t quantity = (uint16_t)(*(receivedMessage + 4));
    quantity = (quantity << 8) + (uint16_t)(*(hexMessage + 5));

    uint8_t index = (fc > 0x08) ? (fc - 0x09) : (fc - 0x01);

    respondLength = HandleAsciiMasterCommand[index](startAddress, quantity, hexMessage, respondBytes);

    return respondLength;
}

/*�������Ȧ״̬����*/
static uint16_t HandleReadCoilStatusCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
    uint16_t length = 0;
    bool statusList[250];

    GetCoilStatus(startAddress, quantity, statusList);

    length = SyntheticAsciiSlaveAccessRespond(receivedMessage, statusList, NULL, respondBytes);

    return length;
}

/*���������״̬����*/
static uint16_t HandleReadInputStatusCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
    uint16_t length = 0;

    bool statusList[250];

    GetInputStatus(startAddress, quantity, statusList);

    length = SyntheticAsciiSlaveAccessRespond(receivedMessage, statusList, NULL, respondBytes);

    return length;
}

/*��������ּĴ�������*/
static uint16_t HandleReadHoldingRegisterCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
    uint16_t length = 0;

    uint16_t registerList[125];

    GetHoldingRegister(startAddress, quantity, registerList);

    length = SyntheticAsciiSlaveAccessRespond(receivedMessage, NULL, registerList, respondBytes);

    return length;
}

/*���������Ĵ�������*/
static uint16_t HandleReadInputRegisterCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
    uint16_t length = 0;

    uint16_t registerList[125];

    GetInputRegister(startAddress, quantity, registerList);

    length = SyntheticAsciiSlaveAccessRespond(receivedMessage, NULL, registerList, respondBytes);

    return length;
}

/*����д������Ȧ����*/
static uint16_t HandleWriteSingleCoilCommand(uint16_t coilAddress, uint16_t coilValue, uint8_t *receivedMessage, uint8_t *respondBytes)
{
    uint16_t length = 0;
    bool value;

    length = SyntheticAsciiSlaveAccessRespond(receivedMessage, NULL, NULL, respondBytes);

    GetCoilStatus(coilAddress, 1, &value);

    value = CovertSingleCommandCoilToBoolStatus(coilValue, value);
    SetSingleCoil(coilAddress, value);

    return length;
}

/*����д�����Ĵ�������*/
static uint16_t HandleWriteSingleRegisterCommand(uint16_t registerAddress, uint16_t registerValue, uint8_t *receivedMessage, uint8_t *respondBytes)
{
    uint16_t length = 0;

    length = SyntheticAsciiSlaveAccessRespond(receivedMessage, NULL, NULL, respondBytes);

    SetSingleRegister(registerAddress, registerValue);

    return length;
}

/*����д�����Ȧ״̬*/
static uint16_t HandleWriteMultipleCoilCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
    uint16_t length = 0;
    bool statusValue[250];

    length = SyntheticAsciiSlaveAccessRespond(receivedMessage, statusValue, NULL, respondBytes);

    SetMultipleCoil(startAddress, quantity, statusValue);

    return length;
}

/*����д����Ĵ���״̬*/
static uint16_t HandleWriteMultipleRegisterCommand(uint16_t startAddress, uint16_t quantity, uint8_t *receivedMessage, uint8_t *respondBytes)
{
    uint16_t length = 0;
    uint16_t registerValue[125];

    length = SyntheticAsciiSlaveAccessRespond(receivedMessage, NULL, registerValue, respondBytes);

    SetMultipleRegister(startAddress, quantity, registerValue);
    return length;
}

/*********** (C) COPYRIGHT 1999-2018 Moonan Technology *********END OF FILE****/