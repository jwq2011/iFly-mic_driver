#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <pthread.h>

#include "vc_i2c_io_interface.h"

#define I2CADDR (0x47)

/* This is the structure as used in the I2C_RDWR ioctl call */
struct i2c_rdwr_ioctl_data {
	struct i2c_msg __user *msgs;	/* pointers to i2c_msgs */
	__u32 nmsgs;			/* number of i2c_msgs */
};

#define  I2C_RDRW_IOCTL_MAX_MSGS	42

static pthread_mutex_t mutex;
static int vc_i2c_file = -1;
static unsigned char vc_i2c_buffer[128];

int  VCI2CInit(const char * devname)
{
	// open device
	vc_i2c_file = open(devname, O_RDWR);
	if(vc_i2c_file < 0) {
		printf("open_i2c(): open %s failed\n", devname);
		return 0;
	}
	pthread_mutex_init(&mutex, NULL);
	return 1;
}

void  VCI2CTerminate(void)
{
	if (vc_i2c_file >= 0) {
		pthread_mutex_destroy(&mutex);
		close(vc_i2c_file);
		vc_i2c_file = -1;
	}
}

 void VCAcqureLock(void)
 {
	pthread_mutex_lock(&mutex);
 }
 
 void VCReleaseLock(void)
 {
	pthread_mutex_unlock(&mutex);
 }
 
/************************************************************************************
intermediate function
*************************************************************************************/
/*------------------------------------------------------------------------------
  Function:   VCI2CWrite

  Purpose:    用户实现的接口函数,功能为向I2C总线按照固定的时序写数据,时序如下图：
  
 |START|0x8E(8bit)|ACK|register_address高8位|ACK|register_address低8位|ACK|buffer[0]|ACK|buffer[1]|ACK|...|buffer[size-1]|ACK|STOP|
		

  Inputs:     register_address	- 16bit的控制地址 
              buffer			- 写入I2C总线数据的存储地址
			  size				- 写入I2C总线数据的长度, 1 for Byte

  Outputs:	  return			- succeed : 1; 
								  failed  : 0;
------------------------------------------------------------------------------*/
int VCI2CWrite(int register_address, unsigned char *buffer, int size)
{
	int ret;
	struct i2c_msg msgs[1] = { {I2CADDR, 0, size + 2, vc_i2c_buffer} };
	struct i2c_rdwr_ioctl_data ioctl_data = {msgs, 1};
	vc_i2c_buffer[0] = (register_address >> 8) & 0xFF;
	vc_i2c_buffer[1] = (register_address) & 0xFF;
	memcpy(&vc_i2c_buffer[2], buffer, size);
	ret = ioctl(vc_i2c_file, I2C_RDWR, &ioctl_data);
	if (ret != 1) {
		printf("error write :%d\n", ret);
	}
	return ret == 1;
}


/*------------------------------------------------------------------------------
  Function:   VCI2CRead

  Purpose:     用户实现的接口函数,功能为从I2C总线按照固定的时序读数据,时序如下图：

 |START|0x8E(8bit)|ACK|register_address高8位|ACK|register_address低8位|ACK|START|0x8F(8bit)|ACK|buffer[0]|ACK|buffer[1]|ACK|...|buffer[size-1]|ACK|STOP|


  Inputs:     register_address	- 16bit的控制地址
              buffer			- 从I2C总线读取数据的存储地址
			  size				- 从I2C总线读取数据的长度, 1 for Byte

  Outputs:	  return			- succeed : 1; 
								  failed  : 0;
------------------------------------------------------------------------------*/
int VCI2CRead(int register_address, unsigned char*buffer, int size)
{
	int ret;
	struct i2c_msg msgs[2] = { {I2CADDR, 0, 2, vc_i2c_buffer}, {I2CADDR, I2C_M_RD, size, buffer} };
	struct i2c_rdwr_ioctl_data ioctl_data = {msgs, 2};
	vc_i2c_buffer[0] = (register_address >> 8) & 0xFF;
	vc_i2c_buffer[1] = (register_address) & 0xFF;
	ret = ioctl(vc_i2c_file, I2C_RDWR, &ioctl_data);
	if (ret != 2) {
		printf("error read :%d\n", ret);
	}
	return ret == 2;
}


/*------------------------------------------------------------------------------
  Function:   VCI2CSleep

  Purpose:    The I2C driver layer interface which user need to develop.This function
            just call the sleep fun on host. 

  Inputs:     intreval_ms		- time for sleep, 1 for 1 millisecond 
 
  Outputs:	  return			- void
------------------------------------------------------------------------------*/
void VCI2CSleep (int intreval_ms)
{
	usleep(intreval_ms * 1000);
}
