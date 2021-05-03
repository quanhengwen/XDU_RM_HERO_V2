/******************************************************************************/
/** �ļ���   dataobject.h                                                    **/
/** ��  ��   ���Ҿ�                                                          **/
/** ��  ��   V1.0.0                                                          **/
/** ��  ��   2015��6��17��                                                   **/
/** ��  ��   ��������modbus���ݶ����������Ժͷ���                          **/
/******************************************************************************/

#ifndef __dataobject_h
#define __dataobject_h

#include "stdint.h"
#include "stddef.h"
#include <math.h>
#include "stdbool.h"

//�������ݶ���0��Ȧ��1����״̬��3����Ĵ�����4���ּĴ���
enum DataObjectType {
	Coil=0,				//��Ȧ����
	InputStatus=1,                  //����״̬
	InputRegister=2,		//����Ĵ���
	HoldingRegister=3		//���ּĴ���
};

//����һ���ֽڰ�λ�����Ľṹ
typedef struct bindex
{
	uint8_t bit0:1;
	uint8_t bit1:1;
	uint8_t bit2:1;
	uint8_t bit3:1;
	uint8_t bit4:1;
	uint8_t bit5:1;
	uint8_t bit6:1;
	uint8_t bit7:1;
}BitIndex;

//��������Modbus�Ĵ������ݴ洢�Ľṹ��
typedef struct rnode
{
  uint16_t index;     	//�������
  uint8_t hiByte;     	//����ֵ���ֽ�
  uint8_t loByte;		//����ֵ���ֽ�
  struct rnode * next; 	//��һ���ڵ�
}RegisterNode;

//��������Modbus״̬�����ݴ洢�Ľṹ��
typedef struct snode
{
	uint16_t index;     	//�������
	uint8_t statusByte;     //���ֽڵķ�ʽ�洢״̬��
	struct snode * next; 	//��һ���ڵ�
}StatusNode;

//��������Modbus�е����ݶ��󣬰�������,��ʼ��ַ��������enum DataObjectType�й涨�����ࣩ
typedef struct {
	enum DataObjectType type;
	uint16_t startingAddress;
	uint16_t quantity;
}DataObject;

//����״̬���Ķ���洢�ṹ��������ʼ�ڵ�ĵ�ַ�����ݶ������ʼ��ַ���ܵ�����
typedef struct {
	StatusNode * startNode;
	uint16_t startingAddress;
	uint16_t quantity;
}StatusObject;

//����Ĵ������Ķ���洢�ṹ��������ʼ�ڵ�ĵ�ַ�����ݶ������ʼ��ַ���ܵ�����
typedef struct {
	RegisterNode * startNode;
	uint16_t startingAddress;
	uint16_t quantity;
}RegisterObject;

//��ʼ�����ݴ洢���򣨴�����Ȧ��������״̬�������ּĴ���������Ĵ����Ĵ洢����
//����Ϊ�ṹ�����飬��Ӧ�ó����ж���
void InitializeDataStorageStructure(DataObject dataObject[]);

//�Ӷ����ַ��ȡֵ
uint16_t GetObjectValue(uint8_t result[],DataObject dataObject);

//���ö����ֵ
void SetObjectValue(uint8_t value[],DataObject dataObject);
#endif
/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/