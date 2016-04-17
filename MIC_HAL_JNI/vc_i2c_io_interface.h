/*------------------------------------------------------------------------------

	File Name:  vc_i2c_io_interface.h

Description:  voice clean ��ʹ������Ҫ�������豸��ʵ�ֵ������ӿں���������i2c I/O����
��sleep��������voice clean ctrl api���ã���Ҫ�����ǰ��չ̶���ʱ�򣬶�/дi2c���ߺ�˯��

------------------------------------------------------------------------------*/
#ifndef VOICE_CLEAN_IIC_IO_INTERFACE_H_
#define VOICE_CLEAN_IIC_IO_INTERFACE_H_

#ifdef __cplusplus
extern "C" { 
#endif 

extern int  VCI2CInit(const char * devname);
extern void  VCI2CTerminate(void);

/*-----------------------------------------------------------------------------
  ����ƽ̨���ӿڣ���ʱ���ȡ�����ź���Ҫ���߳�ʵ��
  �ӿڱ�Ҫʹ��������  
 -----------------------------------------------------------------------------*/
 
 void VCAcqureLock(void);
 void VCReleaseLock(void);
 

/*------------------------------------------------------------------------------
  Function:   VCI2CWrite

  Purpose:    �û�ʵ�ֵĽӿں���,����Ϊ��I2C���߰��չ̶���ʱ��д����,ʱ������ͼ��
  
 |START|0x8E(8bit)|ACK|register_address��8λ|ACK|register_address��8λ|ACK|buffer[0]|ACK|buffer[1]|ACK|...|buffer[size-1]|ACK|STOP|
		

  Inputs:     register_address	- 16bit�Ŀ��Ƶ�ַ 
              buffer			- д��I2C�������ݵĴ洢��ַ
			  size				- д��I2C�������ݵĳ���, 1 for Byte

  Outputs:	  return			- succeed : 1; 
								  failed  : 0;
------------------------------------------------------------------------------*/
int VCI2CWrite(int register_address, unsigned char *buffer, int size);


/*------------------------------------------------------------------------------
  Function:   VCI2CRead

  Purpose:     �û�ʵ�ֵĽӿں���,����Ϊ��I2C���߰��չ̶���ʱ�������,ʱ������ͼ��

 |START|0x8E(8bit)|ACK|register_address��8λ|ACK|register_address��8λ|ACK|START|0x8F(8bit)|ACK|register_address��8λ|ACK|register_address��8λ|ACK|buffer[0]|ACK|buffer[1]|ACK|...|buffer[size-1]|ACK|STOP|


  Inputs:     register_address	- 16bit�Ŀ��Ƶ�ַ
              buffer			- ��I2C���߶�ȡ���ݵĴ洢��ַ
			  size				- ��I2C���߶�ȡ���ݵĳ���, 1 for Byte

  Outputs:	  return			- succeed : 1; 
								  failed  : 0;
------------------------------------------------------------------------------*/
int VCI2CRead(int register_address, unsigned char*buffer, int size);


/*------------------------------------------------------------------------------
  Function:   VCI2CSleep

  Purpose:    The I2C driver layer interface which user need to develop.This function
            just call the sleep fun on host. 

  Inputs:     intreval_ms		- time for sleep, 1 for 1 millisecond 
 
  Outputs:	  return			- void
------------------------------------------------------------------------------*/
void VCI2CSleep (int intreval_ms);
#ifdef __cplusplus
}
#endif 
#endif  // VOICE_CLEAN_IIC_IO_INTERFACE_H_

