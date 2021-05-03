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

#ifndef __mbtcpclient_h
#define __mbtcpclient_h

#include "mbtcp.h"
#include "mbcommon.h"

/* ����ɱ�д����Ȧ�����ݶ������� */
typedef struct WritedCoilListType {
  uint16_t coilAddress;                 //��д��Ȧ���ĵ�ַ
  uint8_t writedStatus;                 //д��Ȧ��״̬��1��Ҫд��0����Ҫ
  uint8_t value;                        //��д��ֵ
//  struct WritedCoilListType *pNext;     //��һ���ڵ�ָ��
}WritedCoilListNode;

/* ����ɱ�д���ּĴ������ݶ������� */
typedef struct WritedRegisterListType{
  uint16_t coilAddress;                 //��д���ּĴ�����ַ
  uint16_t writedStatus;                //д�Ĵ�����״̬��1��Ҫд��0����Ҫ
  uint16_t value;                       //��д��ֵ
//  struct WritedRegisterListType *pNext; //��һ���ڵ�ָ��
}WritedRegisterListNode;

/* ����ɱ�д����Ȧ����������ͷ���� */
typedef struct WritedCoilListHeadType{
  WritedCoilListNode *pWritedCoilNode;  //��д��Ȧ���ڵ�ָ��
  uint32_t writedCoilNumber;            //��д��Ȧ���ڵ������
}WritedCoilListHeadNode;

/* ����ɱ�д���ּĴ�����������ͷ���� */
typedef struct WritedRegisterListHeadType{
  WritedRegisterListNode *pWritedRegisterNode;  //��д�Ĵ������ڵ�ָ��
  uint32_t writedRegisterNumber;                //��д�Ĵ������ڵ������
}WritedRegisterListHeadNode;

/* ���屻����TCP�������������� */
typedef struct AccessedTCPServerType{
  union {
    uint32_t ipNumber;
    uint8_t ipSegment[4];
  }ipAddress;                                           //��������IP��ַ
  uint32_t flagPresetServer;                            //д�����������־
  WritedCoilListHeadNode pWritedCoilHeadNode;          //��д����Ȧ���б�
  WritedRegisterListHeadNode pWritedRegisterHeadNode;  //��д�ı��ּĴ����б�
  struct AccessedTCPServerType *pNextNode;              //��һ��TCP�������ڵ�
}TCPAccessedServerType;

/* �������������ͷ���� */
typedef struct ServerListHeadType {
  TCPAccessedServerType *pServerNode;           //�������ڵ�ָ��
  uint32_t serverNumber;                        //������������
}ServerListHeadNode;

/* ���屾��TCP�ͻ��˶������� */
typedef struct LocalTCPClientType{
  uint32_t transaction;                                 //�����ʶ��
  uint16_t cmdNumber;                                  //�����������������
  uint16_t cmdOrder;                                   //��ǰ��վ�ڴ�վ�б��е�λ��
  uint8_t (*pReadCommand)[12];                         //�������б�
  ServerListHeadNode ServerHeadNode;                    //Server���������ͷ�ڵ�
  UpdateCoilStatusType pUpdateCoilStatus;               //������Ȧ������
  UpdateInputStatusType pUpdateInputStatus;             //��������״̬������
  UpdateHoldingRegisterType pUpdateHoldingRegister;     //���±��ּĴ���������
  UpdateInputResgisterType pUpdateInputResgister;       //��������Ĵ���������
}TCPLocalClientType;

/*���ɷ��ʷ�����������*/
uint16_t CreateAccessServerCommand(ObjAccessInfo objInfo,void *dataList,uint8_t *commandBytes);

/*�����յ��ķ�������Ӧ��Ϣ*/
void ParsingServerRespondMessage(TCPLocalClientType *client,uint8_t *recievedMessage);

/*������������ѷ��������б����������*/
void AddCommandBytesToList(TCPLocalClientType *client,uint8_t *commandBytes);

/* ʹ�ܻ���ʧ��д��վ������־λ���޸Ĵ�վ��дʹ�ܱ�־λ�� */
void ModifyWriteTCPServerEnableFlag(TCPLocalClientType *client,uint8_t ipAddress,bool en);

/* ��ô�վ��дʹ�ܱ�־λ��״̬ */
bool GetWriteTCPServerEnableFlag(TCPLocalClientType *client,uint8_t ipAddress);

/* ʵ����TCP���������� */
void InstantiateTCPServerObject(TCPAccessedServerType *server,
                                uint8_t ipSegment1,uint8_t ipSegment2,
                                uint8_t ipSegment3,uint8_t ipSegment4);

/* ��TCP�ͻ������TCP�������б�ڵ� */
void AddTCPServerNode(TCPLocalClientType *client,TCPAccessedServerType *server);

/*��ʼ��TCP�ͻ��˶���*/
void InitializeTCPClientObject(TCPLocalClientType *client,
                               uint16_t cmdNumber,
                               uint8_t (*pReadCommand)[12],
                               UpdateCoilStatusType pUpdateCoilStatus,
                               UpdateInputStatusType pUpdateInputStatus,
                               UpdateHoldingRegisterType pUpdateHoldingRegister,
                               UpdateInputResgisterType pUpdateInputResgister
                               );

/* �жϵ�ǰ�Ƿ���д����ʹ�� */
bool CheckWriteTCPServerNone(TCPLocalClientType *client);


#endif
/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/