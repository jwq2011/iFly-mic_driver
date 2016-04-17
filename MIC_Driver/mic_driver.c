
#include "mic_gpio.h"



//#define I2CADDR (0x47 << 1)
#define I2CADDR 0x47
static unsigned char vc_i2c_buffer[128];


/***********************************************************************************/


int wait_ret(Command_t *command)
{
	int ret_val;
	do {
		command->data[0] = IV_I2C_CMD_GET_RET;
		ret_val = send_cmd(command, APP_ID_CAPT | capt_nr, IVNR_CMD_GET_I2C_CONTROL, 1);
	} while (ret_val > 0 && ((int)(command->data[0]) == IV_I2C_RET_ACK_CMD || (int)(command->data[0]) == IV_I2C_RET_BUSY));
	if(ret_val > 0 && (int)(command->data[0]) == IV_I2C_RET_SUCCESS_STANDBY) {
		return 0;
	}
	else if (ret_val > 0) {
		return command->data[0];
	} else {
		return -1;
	}
}

void wait_ms(int ms)
{
//CI2CSleep(ms);
}

/************************************************************************************
intermediate function
*************************************************************************************/

static int icI2C_Send(int reg_addr, unsigned char *buffer, int size)
{	
	struct i2c_msg msg;
	struct i2c_adapter *adapter = mic_i2c_client->adapter;
	unsigned char buf[1024];
	unsigned int num;

	if (NULL==adapter){
		printk("%s: adapter==NULL\n", __FUNCTION__);
		return 0;
	}
#if 1
	vc_i2c_buffer[0] = reg_addr >> 8;
	vc_i2c_buffer[1] = reg_addr&0xF;
	memcpy(&vc_i2c_buffer[2], buffer, size);
	
#else
	for(num = size-1; num >= 0; num--)
	{
		buffer[num + 2] = buffer[num];
	}
	buffer[0] = reg_addr >> 8;
	buffer[1] = reg_addr|0xF;
#endif
	msg.addr  = I2CADDR;
	msg.flags = 0x00;
	msg.len   = size+2;
	msg.buf   = vc_i2c_buffer;
//	msg.scl_rate = 100 * 1000;
//	msg.udelay = 5;

	return i2c_transfer(adapter, &msg, 1);
}


static int icI2C_Recv(int reg_addr, unsigned char *buffer, int size)

{
	struct i2c_msg msg[2];
	unsigned char buf[3];
	unsigned int ret = -1;
	unsigned int num;
	struct i2c_adapter *adapter = mic_i2c_client->adapter;

	if (NULL==adapter){
		printk("%s: adapter==NULL\n", __FUNCTION__);
		return -1;
	}

	vc_i2c_buffer[0] = reg_addr >> 8;
	vc_i2c_buffer[1] = reg_addr&0xF;

	msg[0].addr  = I2CADDR;
	msg[0].flags = 0x00;
	msg[0].len   = 2;
	msg[0].buf   = vc_i2c_buffer;
//  msg[0].scl_rate = 100 * 1000;
//  msg[0].udelay = 5;
	//i2c_transfer(adapter, &msg[0], 1);

	msg[1].addr  = I2CADDR;
	msg[1].flags = 0x01;
	msg[1].len   = size;
	msg[1].buf   = buffer;
//    msg[0].scl_rate = 100 * 1000;
//    msg[0].udelay = 5;
	ret = i2c_transfer(adapter, msg, 2);
	//ret = i2c_transfer(adapter, &msg[1], 1);

	return ret;
}

int VCI2CWrite(int register_address, unsigned char *buffer, int size)
{
	return icI2C_Send(register_address, buffer, size);
}

int VCI2CRead(int register_address, unsigned char*buffer, int size)
{
	return icI2C_Recv(register_address, buffer, size);
}

void VCI2CSleep (int intreval_ms)
{
	mdelay(intreval_ms);
}


/************************************************************************************
							COMMAND  function 
*************************************************************************************/
static int m_time_out_ms;

void reset_time_out(void)
{
	m_time_out_ms = 0;
}

int get_time_wait(void)
{
	return m_time_out_ms; 
}

static void byte_order_swap_cmd_head(unsigned char * head)
{
	unsigned char t;
	unsigned short hint = 0xFEFF;
	unsigned char *p = (unsigned char *)&hint;
	if (p[0] == (unsigned char)0xFE) {
		// Big Endian platform
		p = head;
		//swap p[0] p[1]
		t = p[0];
		p[0] = p[1];
		p[1] = t;
		//swap p[2] p[3]
		t = p[2];
		p[2] = p[3];
		p[3] = t;
	}
}

static void byte_order_swap_cmd_data(unsigned char * data, uint32_t size)
{
	unsigned char t;
	uint32_t i;
	unsigned short hint = 0xFEFF;
	unsigned char *p = (unsigned char *)&hint;
	if (p[0] == (unsigned char)0xFE) {
		// Big Endian platform
		for (i = 0, p = data; i < size; i += 4, p += 4) {
			// swap p[0] p[3]
			t = p[0];
			p[0] = p[3];
			p[3] = t;
			// swap p[1] p[2]
			t = p[1];
			p[1] = p[2];
			p[2] = t;
		}
	}
}

#define REPLY_POLL_INTERVAL_MSEC     1
#define REPLY_POLL_TIMEOUT_MSEC   200
static int m_time_out_ms;

int send_cmd (Command_t *cmd, uint32_t  app_module_id, uint32_t  command_id, uint32_t num_16b_words)
{
	unsigned int *i2c_data = (unsigned int *)cmd;
	int size = (num_16b_words + 2) * 4;
	int elapsed_ms = 0;
	int ret = 0;

	// at least two words of header
	if (num_16b_words > MAX_COMMAND_SIZE)
	{
		return(-1);
	}
	cmd->num_16b_words = (uint16_t)((command_id&CMD_GET(0)) ? MAX_COMMAND_SIZE : num_16b_words);
	cmd->command_id    = (uint16_t)command_id;
//#if 0
//	cmd->reply         = 0;
//#else
	cmd->command_id = cmd->command_id & 0x7FFF;
//#endif
	cmd->app_module_id = app_module_id;
//	printk("==========command_id = %d\n", cmd->command_id);
	// write words 1 to N-1 , to addresses 4 to 4+4*N-1
	byte_order_swap_cmd_data((unsigned char *)&i2c_data[1], (size - 4));
	ret = VCI2CWrite(0x4, (unsigned char *)(&i2c_data[1]), (size - 4));
//	printk ("cmd->data: %d.%d.%d.%d\n",cmd->data[0],
//		cmd->data[1], cmd->data[2], cmd->data[3]);
//	printk("==========command_id = %d\n", cmd->command_id);
	if (ret == 0) {
		
		return -1;
	}
	// write word 0 to address 0o
	byte_order_swap_cmd_head((unsigned char *)&i2c_data[0]);
	ret = VCI2CWrite(0x0, (unsigned char *)(&i2c_data[0]), 4);
//	printk ("cmd->data: %d.%d.%d.%d\n",cmd->data[0],
//		cmd->data[1], cmd->data[2], cmd->data[3]);
//	printk("==========command_id = %d\n", cmd->command_id);
	if (ret == 0) {
		return -1;
	}

	#if 1
	ret = VCI2CRead(0x0, (unsigned char *)&i2c_data[0], 4);
//	printk("==========VCI2CRead:0x00 =%d\n", ret);
//	printk ("cmd->data: %d.%d.%d.%d\n",cmd->data[0],
//		cmd->data[1], cmd->data[2], cmd->data[3]);
//	printk("==========command_id = %d\n", cmd->command_id);
	#endif
	while (elapsed_ms < REPLY_POLL_TIMEOUT_MSEC)
	{
		// only read the first word and check the reply bit
//		ret = VCI2CRead(0x0, (unsigned char *)&i2c_data[0], 4);
//		printk("==========VCI2CRead:0x00 =%d\n", ret);
		byte_order_swap_cmd_head((unsigned char *)&i2c_data[0]);
//#if 0
//		if (cmd->reply==1)
//#else
		if (ret != 0 && (cmd->command_id & 0x8000) == 0x8000)
//#endif
			break;
//		VCI2CSleep(REPLY_POLL_INTERVAL_MSEC);
		elapsed_ms += REPLY_POLL_INTERVAL_MSEC;
	}
	m_time_out_ms += elapsed_ms;

//#if 0
//	if (cmd->reply==1)
//#else
	if ((cmd->command_id & 0x8000) == 0x8000)
//#endif
	{
		if (cmd->num_16b_words > 0)
		{
			ret = VCI2CRead(0x8, (unsigned char *)&i2c_data[2], (cmd->num_16b_words)*4);
//			printk("==========VCI2CRead:0x08 =%d\n", ret);
			byte_order_swap_cmd_data((unsigned char *)&i2c_data[2], (cmd->num_16b_words)*4);
			if (ret == 0) {
				return -1;
			}
		}
//		VCI2CSleep(REPLY_POLL_INTERVAL_MSEC);
		return(cmd->num_16b_words);
	}
	return(-1);
}

int test_version(Command_t *command)
{
	int ret_val;
	ret_val = send_cmd(command, (uint32_t)APP_ID_CTRL, CMD_GET(CONTROL_APP_VERSION), 0);

	printk ("version number: %d.%d.%d.%d\n",command->data[0],
		command->data[1], command->data[2], command->data[3]);

	if (ret_val<0)
	{
		printk ("failed to get FW version:%d\n", ret_val);
		return ret_val; /* err code*/
	}

	return(0);
}


/************************************************************************************
							API  function 
*************************************************************************************/
const uint32_t g_work_mode_gain[WORK_MODE_MAX][2] = {
	{0x36360022, 0x1E351E35},
	{0x3636003D, 0x1E351E35},
	{0x0B0B003D, 0x1E0B1E0B},
	{0x3B3B003D, 0x1E3A1E3A}
};

int VCGetVersion(Command_t * command)
{
	int ret_val;

	command->data[0] = IV_I2C_CMD_GET_VERSION;
	ret_val = send_cmd(command, APP_ID_CAPT | capt_nr, (IV_I2C_CMD_CTRL | 0x0100), 2);
	if(ret_val < 0)
	{
		ret_val = test_version(command);
		if(ret_val == 0)
		{
			if(command->data[0] == 2 && command->data[1] == 30 &&
				command->data[2] == 0 && command->data[3] == 0)
			{
				return(2);
			}
		}
		return 0;
	}
	return command->data[0];
}

bool VCWaitReady(Command_t * command, int msecond)
{
	int ret_val, time_wait;
	reset_time_out();
	
	time_wait = 0;
	while( time_wait <= msecond)
	{
		ret_val = test_version(command);
		if(ret_val == 0)
		{
			return(1);
		}
		time_wait = get_time_wait();
	}
	return (0);
}


// 复位操作耗时1s
int VCReset(Command_t * command)
{
	int ntry = 3;
	int32_t ret;

	// reset on release version 4.1010
	command->data[0] = IV_I2C_CMD_GET_VERSION;
	ret = send_cmd(command, APP_ID_CAPT | capt_nr,  (IV_I2C_CMD_CTRL | 0x0100), 1);
	printk("VCReset:ret=%d, %d, %d\n", ret, command->data[0], command->data[1]);
	if(ret < 0) return 0;
//	if (command->data[0] != 4 && command->data[1] != 0x10100) return 0;
	if (!(command->data[0] == 4 && command->data[1] == 0x10100)) return 0;

	while (ntry-- > 0) {
		//check watchdog
		ret  = send_cmd(command, APP_ID_CAPT|capt_nr, CMD_GET(IVNR_CMD_WATCHDOG), 0);
		printk("get watchdog:%d %d\n", ret, command->data[0]);
		if( ret < 0 ) continue;
		
		if (command->data[0] == 0) {
			// start watchdog
			command->data[0] = 1;
			ret  = send_cmd(command, APP_ID_CAPT|capt_nr, CMD_SET(IVNR_CMD_WATCHDOG), 1);
			printk("set watchdog:%d %d\n", ret, command->data[0]);
			if( ret < 0 ) continue;
		}	
		// close app
		command->data[0] = APP_ID_CAPT;
		command->data[1] = 0;
//		ret  = send_cmd(command, APP_ID_STRM, STREAMER_APP_MODULE_APP_ENABLE, 2);
		if (ret < 0) continue;
		
		mdelay(1000);// 复位后要等1s才能操作净音模块,这里用了阻塞式等待
		
		//check watchdog
		ret  = send_cmd(command, APP_ID_CAPT|capt_nr, CMD_GET(IVNR_CMD_WATCHDOG), 0);
		printk("get watchdog:%d %d\n", ret, command->data[0]);
		if( ret < 0 ) continue;
		if (command->data[0] == 1) break;		
	}
	
	return 0;
}

#if 1
int VCChangeWorkMode(Command_t *command, cmd_work_mode_id work_mode_id)
{
	int ret_val;
	uint32_t *pdata32;

	pdata32 = (uint32_t *)&g_work_mode_gain[work_mode_id];
	command->data[0] = IV_I2C_CMD_WORK_MODE;
	command->data[1] = work_mode_id;
	command->data[2] = *pdata32;
	command->data[3] = *(pdata32 + 1);

	ret_val = send_cmd(command, APP_ID_CAPT | capt_nr, (IV_I2C_CMD_CTRL & ~0x0100), 4);
	if (ret_val<0)
	{
		return ret_val; /* err code*/
	}
	ret_val = wait_ret(command);
	return (0);
}
#endif

int VCGetWorkMode(Command_t * command)
{
	int ret_val;

	command->data[0] = IV_I2C_CMD_WORK_MODE;
	ret_val = send_cmd(command, APP_ID_CAPT | capt_nr, CMD_GET(IV_I2C_CMD_CTRL), 1);
	if (ret_val<0)
	{
		return ret_val; /* err code*/
	}
	return command->data[0];	
}

int VCChangeFunc(Command_t * command, cmd_func_id func_id)
{
	int ret_val;

	command->data[0] = func_id;
	ret_val = send_cmd(command, APP_ID_CAPT | capt_nr, IVNR_CMD_SET_I2C_CONTROL, 1);
	if (ret_val<0)
	{
		return ret_val; /* err code*/
	}
	ret_val = wait_ret(command);
	return ret_val;	
}

int VCGetFunc(Command_t * command)
{
	int ret_val;

	command->data[0] = IV_I2C_CMD_GET_FUNC;
	ret_val = send_cmd(command, APP_ID_CAPT | capt_nr, CMD_GET(IV_I2C_CMD_CTRL), 1);
	if (ret_val<0)
	{
		return ret_val; /* err code*/
	}
	return command->data[0];	
}

int VCConfigWakeupGpio(Command_t * command, bool key_level, int msecond)
{
	int ret_val;
	command->data[0] = IV_I2C_CMD_WK_SIGN;
	command->data[1] = key_level;
	command->data[2] = msecond;
	ret_val = send_cmd(command, APP_ID_CAPT | capt_nr, (IV_I2C_CMD_CTRL & ~0x0100), 3);
	if (ret_val<0)
	{
		return ret_val; /* err code*/
	}
	return (0);
}

int VCConfigWakeupID(Command_t * command, int wakeup_id)
{
	int ret_val;

	command->data[0] = IV_I2C_CMD_WK_ID;
	command->data[1] = wakeup_id;
	ret_val = send_cmd(command, APP_ID_CAPT | capt_nr, (IV_I2C_CMD_CTRL & ~0x0100), 2);
	if (ret_val<0)
	{
		return ret_val; /* err code*/
	}
	ret_val = wait_ret(command);
	return ret_val;
}

int VCSetDACVolume(Command_t * command, cmd_dac_gain_id dac_gain_id)
{
	int ret_val, mem_info;

	reset_time_out();
	mem_info = dac_gain_id;
	command->data[0] = 0;
	command->data[1] = 0;
	command->data[2] = 1;
	command->data[3] = mkword(0x34, mem_info);
	ret_val = send_cmd(command, APP_ID_CTRL, CMD_SET(CONTROL_APP_I2C_TUNNEL_DATA), 4);
	if (ret_val<0)
	{
		return ret_val; /* err code*/
	}
	command->data[0] = 0;
	ret_val = send_cmd(command, APP_ID_CTRL, CONTROL_APP_I2C_TUNNEL_APPLY, 1);
	if (ret_val<0)
	{
		return ret_val; /* err code*/
	}

	mem_info |= 0x100;
	command->data[0] = 0;
	command->data[1] = 0;
	command->data[2] = 1;
	command->data[3] = mkword(0x35, mem_info);
	ret_val = send_cmd(command, APP_ID_CTRL, CMD_SET(CONTROL_APP_I2C_TUNNEL_DATA), 4);
	if (ret_val<0)
	{
		return ret_val; /* err code*/
	}
	command->data[0] = 0;
	ret_val = send_cmd(command, APP_ID_CTRL, CONTROL_APP_I2C_TUNNEL_APPLY, 1);
	if (ret_val<0)
	{
		return ret_val; /* err code*/
	}
	return(0);
}

int VCGetWakeupSign(Command_t * command, int * pmsecond)
{
	int ret_val;

	command->data[0] = IV_I2C_CMD_GET_WK_SIGN;
	ret_val = send_cmd(command, APP_ID_CAPT | capt_nr, CMD_GET(IV_I2C_CMD_CTRL), 2);
	if (ret_val<0)
	{
		return ret_val; /* err code*/
	}
	if(pmsecond)
	{
		*pmsecond = command->data[1];
	}
	return command->data[0];
}

int mic_i2cdev_ioctl_rdrw(struct i2c_client *client,
		unsigned long arg)
{
	struct i2c_rdwr_ioctl_data rdwr_arg;
	struct i2c_msg *rdwr_pa;
	u8 __user **data_ptrs;
	int i, res;

	if (copy_from_user(&rdwr_arg,
			   (struct i2c_rdwr_ioctl_data __user *)arg,
			   sizeof(rdwr_arg)))
		return -EFAULT;
	printk("mic_i2cdev_ioctl_rdrw:%d, res=%d\n", __LINE__, res);
	/* Put an arbitrary limit on the number of messages that can
	 * be sent at once */
	if (rdwr_arg.nmsgs > I2C_RDRW_IOCTL_MAX_MSGS)
		return -EINVAL;
	printk("mic_i2cdev_ioctl_rdrw:%d, res=%d\n", __LINE__, res);
	rdwr_pa = memdup_user(rdwr_arg.msgs,
			      rdwr_arg.nmsgs * sizeof(struct i2c_msg));
	if (IS_ERR(rdwr_pa))
		return PTR_ERR(rdwr_pa);
	printk("mic_i2cdev_ioctl_rdrw:%d, res=%d\n", __LINE__, res);
	data_ptrs = kmalloc(rdwr_arg.nmsgs * sizeof(u8 __user *), GFP_KERNEL);
	if (data_ptrs == NULL) {
		kfree(rdwr_pa);
		return -ENOMEM;
	}
	printk("mic_i2cdev_ioctl_rdrw:%d, res=%d\n", __LINE__, res);
	res = 0;
	for (i = 0; i < rdwr_arg.nmsgs; i++) {
		/* Limit the size of the message to a sane amount;
		 * and don't let length change either. */
		if ((rdwr_pa[i].len > 8192) ||
		    (rdwr_pa[i].flags & I2C_M_RECV_LEN)) {
			res = -EINVAL;
			break;
		}
		data_ptrs[i] = (u8 __user *)rdwr_pa[i].buf;
		rdwr_pa[i].buf = memdup_user(data_ptrs[i], rdwr_pa[i].len);
		if (IS_ERR(rdwr_pa[i].buf)) {
			res = PTR_ERR(rdwr_pa[i].buf);
			break;
		}
	}
	if (res < 0) {
		int j;
		for (j = 0; j < i; ++j)
			kfree(rdwr_pa[j].buf);
		kfree(data_ptrs);
		kfree(rdwr_pa);
		return res;
	}
	printk("mic_i2cdev_ioctl_rdrw:%d, res=%d\n", __LINE__, res);
	res = i2c_transfer(client->adapter, rdwr_pa, rdwr_arg.nmsgs);
	while (i-- > 0) {
		if (res >= 0 && (rdwr_pa[i].flags & I2C_M_RD)) {
			if (copy_to_user(data_ptrs[i], rdwr_pa[i].buf,
					 rdwr_pa[i].len))
				res = -EFAULT;
		}
		kfree(rdwr_pa[i].buf);
	}
	kfree(data_ptrs);
	kfree(rdwr_pa);
	printk("mic_i2cdev_ioctl_rdrw:%d, res=%d\n", __LINE__, res);
	return res;
}

