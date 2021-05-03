/******************************************************************************/
/** �ļ���   dataobject.c                                                    **/
/** ��  ��   ���Ҿ�                                                          **/
/** ��  ��   V1.0.0                                                          **/
/** ��  ��   2015��6��17��                                                   **/
/** ��  ��   ���ڶ���modbus���ݶ����������Ժͷ��������������            **/
/***1����ʼ���洢���򣬴������ݶ���********************************************/
/***2����ȡ�洢���򣬻�ȡ���ݶ����ֵ******************************************/
/***3��д��洢�����޸����ݶ����ֵ******************************************/
/******************************************************************************/

#include "dataobject.h"

//���������¼ÿ�����ݴ洢�ṹ�ĵ�ַ����ʼλ�ü�����
StatusObject coilObject={NULL,0,0};
StatusObject inputStatusObject={NULL,0,0};
RegisterObject inputRegisterObject={NULL,0,0};
RegisterObject holdingRegisterObject={NULL,0,0};

/******Begin ����˽�к���******/
//��ʼ����Ȧ����Ĵ洢����
void GenerateCoilStorageStructure(DataObject dataObject);
//��ʼ������״̬����Ĵ洢����
void GenerateInputStatusStorageStructure(DataObject dataObject);
//��ʼ������Ĵ�������Ĵ洢����
void GenerateInputRegisterStorageStructure(DataObject dataObject);
//��ʼ�����ּĴ�������Ĵ洢����
void GenerateHoldingRegisterStorageStructure(DataObject dataObject);
//���ɴ洢״̬�����Ĵ洢���򣬰�����Ȧ������״̬
StatusNode *GenerateStatusObjectStorageStructure(DataObject dataObject);
//���ɴ洢�Ĵ������Ĵ洢����,�������ּĴ���������Ĵ���
RegisterNode *GenerateRegisterObjectStorageStructure(DataObject dataObject);

//��ȡ��Ȧ�����ֵ
uint16_t GetCoilObjectValue(uint8_t result[],DataObject dataObject);
//��ȡ����״̬�����ֵ
uint16_t GetInputStatusObjectValue(uint8_t result[],DataObject dataObject);
//��ȡ���ּĴ��������ֵ
uint16_t GetHoldingRegisterObjectValue(uint8_t result[],DataObject dataObject);
//��ȡ����Ĵ���״̬��ֵ
uint16_t GetInputRegisterObjectValue(uint8_t result[],DataObject dataObject);
//��ȡ״̬����ֵ������ֵ���ֽ���,������Ȧ������״̬
uint16_t GetStatusObjectValue(StatusObject sobject,uint8_t result[],DataObject dataObject);
//��ȡ�Ĵ��������ֵ������ֵ���ֽ������������ּĴ���������Ĵ���
uint16_t GetRegisterObjectValue(RegisterObject robject,uint8_t result[],DataObject dataObject);

//����Coil�����ֵ
void SetCoilObjectValue(uint8_t value[],DataObject dataObject);
//��������״̬�����ֵ
void SetInputStatusObjectValue(uint8_t value[],DataObject dataObject);
//��������Ĵ��������ֵ
void SetInputRegisterObjectValue(uint8_t value[],DataObject dataObject);
//���ñ��ּĴ���״̬��ֵ
void SetHoldingRegisterObjectValue(uint8_t value[],DataObject dataObject);
//����״̬�����ֵ
void SetStatusObjectValue(StatusObject sobject,uint8_t value[],DataObject dataObject);
//���üĴ��������ֵ
void SetRegisterObjectValue(RegisterObject robject,uint8_t value[],DataObject dataObject);
/******End ����˽�к���******/		

/******Begin ��ʼ���洢���򴴣������ݶ���******/
void (*GenerateDataObjectStructure[])(DataObject dataObject)={GenerateCoilStorageStructure,\
															  GenerateInputStatusStorageStructure,\
															  GenerateInputRegisterStorageStructure,\
															  GenerateHoldingRegisterStorageStructure};

//��ʼ�����ݴ洢���򣨴�����Ȧ��������״̬�������ּĴ���������Ĵ����Ĵ洢����
//����Ϊ�ṹ�����飬��Ӧ�ó����ж���
void InitializeDataStorageStructure(DataObject dataObject[])
{
	uint16_t length=sizeof(dataObject)/sizeof(dataObject[0]);
	if((length<1)||(length>4))
	{
		return;
	}
	for(int i=0;i<length;i++)
	{
		(GenerateDataObjectStructure[dataObject[i].type])(dataObject[i]);
	}
}

//��ʼ����Ȧ����Ĵ洢����
void GenerateCoilStorageStructure(DataObject dataObject)
{
	coilObject.startNode=GenerateStatusObjectStorageStructure(dataObject);
	coilObject.startingAddress=dataObject.startingAddress;
	coilObject.quantity=dataObject.quantity;
}

//��ʼ������״̬����Ĵ洢����
void GenerateInputStatusStorageStructure(DataObject dataObject)
{
	inputStatusObject.startNode=GenerateStatusObjectStorageStructure(dataObject);
	inputStatusObject.startingAddress=dataObject.startingAddress;
	inputStatusObject.quantity=dataObject.quantity;
}

//��ʼ������Ĵ�������Ĵ洢����
void GenerateInputRegisterStorageStructure(DataObject dataObject)
{
	inputRegisterObject.startNode=GenerateRegisterObjectStorageStructure(dataObject);
	inputRegisterObject.startingAddress=dataObject.startingAddress;
	inputRegisterObject.quantity=dataObject.quantity;
}

//��ʼ�����ּĴ�������Ĵ洢����
void GenerateHoldingRegisterStorageStructure(DataObject dataObject)
{
	holdingRegisterObject.startNode=GenerateRegisterObjectStorageStructure(dataObject);
	holdingRegisterObject.startingAddress=dataObject.startingAddress;
	holdingRegisterObject.quantity=dataObject.quantity;
}

//���ɴ洢״̬�����Ĵ洢���򣬰�����Ȧ������״̬
StatusNode *GenerateStatusObjectStorageStructure(DataObject dataObject)
{
	StatusNode *startNode=NULL;
	StatusNode *lastNode=NULL;
	uint16_t length=dataObject.quantity/8;

	for(int i=0;i<length;i++)
	{
		StatusNode node={dataObject.startingAddress+i*8,0,NULL};
		
		if(!i)
		{
			startNode=&node;
		}
		else
		{
			lastNode->next=&node;
		}
		lastNode=&node;
	}
	
	return startNode;
}

//���ɴ洢�Ĵ������Ĵ洢����,�������ּĴ���������Ĵ���
RegisterNode *GenerateRegisterObjectStorageStructure(DataObject dataObject)
{
	RegisterNode *startNode=NULL;
	RegisterNode *lastNode=NULL;
	for(int i=0;i<dataObject.quantity;i++)
	{
		RegisterNode node={i,0,0,NULL};
		if(!i)
		{
			startNode=&node;
		}
		else
		{
			lastNode->next=&node;
		}
		lastNode=&node;
	}
	
	return startNode;
}

/******End ��ʼ���洢���򣬽����ݶ���******/

/******Begin ��ȡ�����ֵ******/
uint16_t (*GetDataObjectValue[])(uint8_t result[],DataObject dataObject)={GetCoilObjectValue,\
																		  GetInputStatusObjectValue,\
																		  GetInputRegisterObjectValue,\
																		  GetHoldingRegisterObjectValue};

//�Ӷ����ַ��ȡֵ
uint16_t GetObjectValue(uint8_t result[],DataObject dataObject)
{
	uint16_t byteCount=0;
	byteCount=GetDataObjectValue[dataObject.type](result,dataObject);
	return byteCount;
}

//��ȡ��Ȧ�����ֵ
uint16_t GetCoilObjectValue(uint8_t result[],DataObject dataObject)
{
	//�жϸ������Ƿ񿪱ٴ洢�ռ�,�жϵ�ַ�Ƿ��ںϷ�����
	if((coilObject.startNode==NULL)||\
	   (dataObject.startingAddress<coilObject.startingAddress)||\
	   ((dataObject.startingAddress+dataObject.quantity)>(coilObject.startingAddress+coilObject.quantity)))
	{
		return 0;
	}
	
	return GetStatusObjectValue(coilObject,result,dataObject);
}

//��ȡ����״̬�����ֵ
uint16_t GetInputStatusObjectValue(uint8_t result[],DataObject dataObject)
{
	//�жϸö��������Ƿ񿪱��˴洢�ռ�,�жϷ��ʵĵ�ַ�Ƿ�Ϸ�
	if((inputStatusObject.startNode==NULL)||\
	   (dataObject.startingAddress<inputStatusObject.startingAddress)||\
	   ((dataObject.startingAddress+dataObject.quantity)>(inputStatusObject.startingAddress+inputStatusObject.quantity)))
	{
		return 0;
	}
	return GetStatusObjectValue(inputStatusObject,result,dataObject);
}

//��ȡ���ּĴ��������ֵ
uint16_t GetHoldingRegisterObjectValue(uint8_t result[],DataObject dataObject)
{
	//�жϸö��������Ƿ񿪱��˴洢�ռ�,�жϷ��ʵĵ�ַ�Ƿ�Ϸ�
	if((holdingRegisterObject.startNode==NULL)||\
	   (dataObject.startingAddress<holdingRegisterObject.startingAddress)||\
	   ((dataObject.startingAddress+dataObject.quantity)>(holdingRegisterObject.startingAddress+holdingRegisterObject.quantity)))
	{
		return 0;
	}
	return GetRegisterObjectValue(holdingRegisterObject,result,dataObject);
}

//��ȡ����Ĵ���״̬��ֵ
uint16_t GetInputRegisterObjectValue(uint8_t result[],DataObject dataObject)
{
	//�жϸö��������Ƿ񿪱��˴洢�ռ�,�жϷ��ʵĵ�ַ�Ƿ�Ϸ�
	if((inputRegisterObject.startNode==NULL)||\
	   (dataObject.startingAddress<inputRegisterObject.startingAddress)||\
	   ((dataObject.startingAddress+dataObject.quantity)>(inputRegisterObject.startingAddress+inputRegisterObject.quantity)))
	{
		return 0;
	}
	return GetRegisterObjectValue(inputRegisterObject,result,dataObject);
}

//��ȡ״̬����ֵ������ֵ���ֽ���,������Ȧ������״̬
uint16_t GetStatusObjectValue(StatusObject sobject,uint8_t result[],DataObject dataObject)
{
	StatusNode *node=sobject.startNode;
	uint16_t startPosition=((dataObject.startingAddress-sobject.startingAddress)/8)*8+sobject.startingAddress;
	uint16_t endPosition=((dataObject.startingAddress+dataObject.quantity-1-sobject.startingAddress)/8)*8+sobject.startingAddress;
	uint16_t length=(dataObject.startingAddress+dataObject.quantity-1-sobject.startingAddress)/8+1;
	
	uint8_t value[10];

	for(int i=0;i<length;i++)
	{
		if((startPosition<=node->index)&&(node->index<=endPosition))
		{
			//����Ӧ��ֵȡ����
			value[i]=node->statusByte;
		}
		node=node->next;
	}
	uint16_t byteCount=(uint16_t)ceil(dataObject.quantity/8);
	uint16_t offset=(dataObject.startingAddress-sobject.startingAddress)%8;
	uint16_t eoffset=7-((dataObject.startingAddress-sobject.startingAddress+dataObject.quantity-1)%8);
	uint16_t startIndex=(startPosition-sobject.startingAddress)/8;
	uint16_t endIndex=(endPosition-sobject.startingAddress)/8;
	value[endIndex]=(value[endIndex]<<eoffset)>>eoffset;
	
	for(int i=startIndex;i<=endIndex;i++)
	{
		if((i+1)<=endIndex)
		{
			result[i-startIndex]=(value[i]>>offset)+(value[i+1]<<(7-offset));
		}
		else
		{
			if((i-startIndex)<byteCount)
			{
				result[i-startIndex]=value[i]>>offset;
			}
		}
	}
	
	return byteCount;
}

//��ȡ�Ĵ��������ֵ������ֵ���ֽ������������ּĴ���������Ĵ���
uint16_t GetRegisterObjectValue(RegisterObject robject,uint8_t result[],DataObject dataObject)
{
	RegisterNode *node=robject.startNode;
	uint16_t byteCount=0;
	for(int i=robject.startingAddress;i<dataObject.startingAddress+dataObject.quantity;i++)
	{
		if((node!=NULL)&&(node->index>=dataObject.startingAddress))
		{
			result[byteCount++]=node->hiByte;
			result[byteCount++]=node->loByte;
		}
		
		node=node->next;
	}
	return byteCount;
}
/******End ��ȡ�����ֵ******/

/******Begin ���ö����ֵ******/
void (*SetDataObjectValue[])(uint8_t value[],DataObject dataObject)={SetCoilObjectValue,SetInputStatusObjectValue,SetInputRegisterObjectValue,SetHoldingRegisterObjectValue};

//���ö����ֵ
void SetObjectValue(uint8_t value[],DataObject dataObject)
{
	SetDataObjectValue[dataObject.type](value,dataObject);
}

//����Coil�����ֵ
void SetCoilObjectValue(uint8_t value[],DataObject dataObject)
{
	//�жϸ������Ƿ񿪱ٴ洢�ռ�,�жϵ�ַ�Ƿ��ںϷ�����
	if((coilObject.startNode==NULL)||\
	   (dataObject.startingAddress<coilObject.startingAddress)||\
	   ((dataObject.startingAddress+dataObject.quantity)>(coilObject.startingAddress+coilObject.quantity)))
	{
		return;
	}
	SetStatusObjectValue(coilObject,value,dataObject);
}

//��������״̬�����ֵ
void SetInputStatusObjectValue(uint8_t value[],DataObject dataObject)
{
	//�жϸö��������Ƿ񿪱��˴洢�ռ�,�жϷ��ʵĵ�ַ�Ƿ�Ϸ�
	if((inputStatusObject.startNode==NULL)||\
	   (dataObject.startingAddress<inputStatusObject.startingAddress)||\
	   ((dataObject.startingAddress+dataObject.quantity)>(inputStatusObject.startingAddress+inputStatusObject.quantity)))
	{
		return;
	}
	SetStatusObjectValue(inputStatusObject,value,dataObject);
}

//��������Ĵ��������ֵ
void SetInputRegisterObjectValue(uint8_t value[],DataObject dataObject)
{
	//�жϸö��������Ƿ񿪱��˴洢�ռ�,�жϷ��ʵĵ�ַ�Ƿ�Ϸ�
	if((inputRegisterObject.startNode==NULL)||\
	   (dataObject.startingAddress<inputRegisterObject.startingAddress)||\
	   ((dataObject.startingAddress+dataObject.quantity)>(inputRegisterObject.startingAddress+inputRegisterObject.quantity)))
	{
		return;
	}
	SetRegisterObjectValue(inputRegisterObject,value,dataObject);
}

//���ñ��ּĴ���״̬��ֵ
void SetHoldingRegisterObjectValue(uint8_t value[],DataObject dataObject)
{
	//�жϸö��������Ƿ񿪱��˴洢�ռ�,�жϷ��ʵĵ�ַ�Ƿ�Ϸ�
	if((holdingRegisterObject.startNode==NULL)||\
	   (dataObject.startingAddress<holdingRegisterObject.startingAddress)||\
	   ((dataObject.startingAddress+dataObject.quantity)>(holdingRegisterObject.startingAddress+holdingRegisterObject.quantity)))
	{
		return;
	}
	SetRegisterObjectValue(holdingRegisterObject,value,dataObject);
}

//����״̬�����ֵ
void SetStatusObjectValue(StatusObject sobject,uint8_t value[],DataObject dataObject)
{
/*	StatusNode *node=sobject.startNode;
	uint16_t byteCount=(dataObject.startingAddress+dataObject.quantity-sobject.startingAddress)/8+1;
	uint16_t leftmove=(dataObject.startingAddress-sobject.startingAddress)%8;
	uint16_t startByte=((dataObject.startingAddress-sobject.startingAddress)/8)*8+sobject.startingAddress;
	uint16_t endByte=((dataObject.startingAddress+dataObject.quantity-sobject.startingAddress)/8)*8+sobject.startingAddress;
	for(int i=sobject.startingAddress;i<=endByte;i++)
	{
		
	}*/
}

//���üĴ��������ֵ
void SetRegisterObjectValue(RegisterObject robject,uint8_t value[],DataObject dataObject)
{
	RegisterNode *node=robject.startNode;
	int index=0;
	for(int i=robject.startingAddress;i<(dataObject.startingAddress+dataObject.quantity);i++)
	{
		if((i>=dataObject.startingAddress)&&(index<dataObject.quantity*2))
		{
			node->hiByte=value[index*2];
			node->loByte=value[index*2+1];
			index++;
		}
		node=node->next;
	}
}
/******End ���ö����ֵ******/

/*********** (C) COPYRIGHT 1999-2016 Moonan Technology *********END OF FILE****/