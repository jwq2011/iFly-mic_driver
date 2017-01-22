/*----------------------------------------------------------------------------

	File Name: vc_ctrl_host.cpp

------------------------------------------------------------------------------*/
#include "vc_i2c_io_interface.h"
#include "vc_i2c_command.h"
#include "vc_ctrl_api.h"

#define UNREFERENCED_PARAMETER(P)      {(P) = (P);} 
#define MODULE_I2C_CTRL_FIRST	 (0x00)
#define IV_I2C_CMD_CTRL		(0x0048)
#define IVNR_CMD_SET_NR_OUTPUT   CMD_SET(IVNR_CMD_NR_OUTPUT)
typedef enum IvnrI2CCmdTypeTag {
	IV_I2C_CMD_STANDBY_CMD = MODULE_I2C_CTRL_FIRST - 1,
	IV_I2C_CMD_FUNC_PASSBY = 0x00,
	IV_I2C_CMD_FUNC_NR = 0x01,
	IV_I2C_CMD_FUNC_AECS_PH = 0x02,
	IV_I2C_CMD_FUNC_AECS_WK = 0x03,
	IV_I2C_CMD_WK_INSIDE = 0x04,
	IV_I2C_CMD_WK_SIGN= 0x05,
	IV_I2C_CMD_GET_RET = 0x06,
	IV_I2C_CMD_GET_FUNC = 0x07,
	IV_I2C_CMD_WK_ID = 0x08,
	IV_I2C_CMD_AEC_RIGHT_OUTPUT = 0x09,
	IV_I2C_CMD_GET_VERSION = 0x0A,
	IV_I2C_CMD_WORK_MODE = 0x0B,
	IV_I2C_CMD_GET_WK_SIGN = 0x0C,
	IV_I2C_CMD_END = 0x0D
} IvnrI2CCmdType;
#define WORK_MODE_MAX 4

#define WITH_LOCK 1

#if WITH_LOCK

extern BOOL VCWaitReady_NOLOCK(Command_t * command, int msecond);
extern int VCGetVersion_NOLOCK(Command_t * command);
extern int VCChangeWorkMode_NOLOCK(Command_t *command, cmd_work_mode_id work_mode_id);
extern int VCGetWorkMode_NOLOCK(Command_t * command);
extern int VCChangeFunc_NOLOCK(Command_t * command, cmd_func_id func_id);
extern int VCGetFunc_NOLOCK(Command_t * command);
extern int VCConfigWakeupGpio_NOLOCK(Command_t * command, BOOL key_level, int msecond);
extern int VCConfigWakeupID_NOLOCK(Command_t * command, int wakeup_id);
extern int VCGetWakeupSign_NOLOCK(Command_t * command, int * pmsecond);
extern int VCEnableNR_NOLOCK(Command_t * command, BOOL enable_nr);
extern int VCSetDACVolume_NOLOCK(Command_t * command, cmd_dac_gain_id dac_gain_id);


extern BOOL VCWaitReady(Command_t * command, int msecond)
{
	BOOL rv;
	VCAcqureLock();
	rv = VCWaitReady_NOLOCK(command, msecond);
	VCReleaseLock();
	return rv;
}

extern int VCGetVersion(Command_t * command)
{
	int rv;
	VCAcqureLock();
	rv = VCGetVersion_NOLOCK(command);
	VCReleaseLock();
	return rv;
}

extern int VCChangeWorkMode(Command_t *command, cmd_work_mode_id work_mode_id)
{
	int rv;
	VCAcqureLock();
	rv = VCChangeWorkMode_NOLOCK(command, work_mode_id);
	VCReleaseLock();
	return rv;
}

extern int VCGetWorkMode(Command_t * command)
{
	int rv;
	VCAcqureLock();
	rv = VCGetWorkMode_NOLOCK(command);
	VCReleaseLock();
	return rv;
}

extern int VCChangeFunc(Command_t * command, cmd_func_id func_id)
{
	int rv;
	VCAcqureLock();
	rv = VCChangeFunc_NOLOCK(command, func_id);
	VCReleaseLock();
	return rv;
}
extern int VCGetFunc(Command_t * command)
{
	int rv;
	VCAcqureLock();
	rv = VCGetFunc_NOLOCK(command);
	VCReleaseLock();
	return rv;
}

extern int VCConfigWakeupGpio(Command_t * command, BOOL key_level, int msecond)
{
	int rv;
	VCAcqureLock();
	rv = VCConfigWakeupGpio_NOLOCK(command, key_level, msecond);
	VCReleaseLock();
	return rv;
}

extern int VCConfigWakeupID(Command_t * command, int wakeup_id)
{
	int rv;
	VCAcqureLock();
	rv = VCConfigWakeupID_NOLOCK(command, wakeup_id);
	VCReleaseLock();
	return rv;
}

extern int VCGetWakeupSign(Command_t * command, int * pmsecond)
{
	int rv;
	VCAcqureLock();
	rv = VCGetWakeupSign_NOLOCK(command, pmsecond);
	VCReleaseLock();
	return rv;
}

extern int VCEnableNR(Command_t * command, BOOL enable_nr)
{
	int rv;
	VCAcqureLock();
	rv = VCEnableNR_NOLOCK(command, enable_nr);
	VCReleaseLock();
	return rv;
}

extern int VCSetDACVolume(Command_t * command, cmd_dac_gain_id dac_gain_id)
{
	int rv;
	VCAcqureLock();
	rv = VCSetDACVolume_NOLOCK(command, dac_gain_id);
	VCReleaseLock();
	return rv;
}

#define VCWaitReady VCWaitReady_NOLOCK
#define VCGetVersion VCGetVersion_NOLOCK
#define VCChangeWorkMode VCChangeWorkMode_NOLOCK
#define VCGetWorkMode VCGetWorkMode_NOLOCK
#define VCChangeFunc VCChangeFunc_NOLOCK
#define VCGetFunc VCGetFunc_NOLOCK
#define VCConfigWakeupGpio VCConfigWakeupGpio_NOLOCK
#define VCConfigWakeupID VCConfigWakeupID_NOLOCK
#define VCGetWakeupSign VCGetWakeupSign_NOLOCK
#define VCEnableNR VCEnableNR_NOLOCK
#define VCSetDACVolume VCSetDACVolume_NOLOCK

#endif

const uint32_t g_work_mode_gain[WORK_MODE_MAX][2] = {
	{0x36360022, 0x1E351E35},
	{0x3636003D, 0x1E351E35},
	{0x0B0B003D, 0x1E0B1E0B},
	{0x3B3B003D, 0x1E3A1E3A}
};

/***********************************************************************************/
int wait_ret(Command_t *command);
void wait_ms(int ms);
/************************************************************************************
							API  function 
*************************************************************************************/
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

BOOL VCWaitReady(Command_t * command, int msecond)
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

int VCConfigWakeupGpio(Command_t * command, BOOL key_level, int msecond)
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

/*************************************************************************************/
/*   VCEnableNR 仅用于版本二，于版本三中调用会造成异常   */
/*************************************************************************************/
int VCEnableNR(Command_t * command, BOOL enable_nr)
{
	int ret_val;

	reset_time_out();
	
	command->data[0] = 0;
	command->data[1] = 0x1A;
	command->data[2] = 0x0;
	command->data[3] = 1;
	command->data[4] = 2;
	command->data[5] = 0;
	ret_val = send_cmd(command, APP_ID_CTRL, CONTROL_APP_I2C_TUNNEL_CONFIG, 6);
	if (ret_val < 0) {
		return ret_val; /* err code*/
	}
	
	if(enable_nr)
	{
		command->data[0] = 0;
		command->data[1] = 0;
		command->data[2] = 1;
		command->data[3] = mkword(0x32, 0x15);
		ret_val = send_cmd(command, APP_ID_CTRL, CMD_SET(CONTROL_APP_I2C_TUNNEL_DATA), 4);
		if (ret_val<0) {
			return ret_val; /* err code*/
		}
		
		command->data[0] = 0;
		ret_val = send_cmd(command, APP_ID_CTRL, CONTROL_APP_I2C_TUNNEL_APPLY, 1);
		if (ret_val<0) {
			return ret_val; /* err code*/
		}
		
		command->data[0] = 0;
		command->data[1] = 0;
		command->data[2] = 1;
		command->data[3] = mkword(0x33, 0x15);
		ret_val = send_cmd(command, APP_ID_CTRL, CMD_SET(CONTROL_APP_I2C_TUNNEL_DATA), 4);
		if (ret_val<0) {
			return ret_val; /* err code*/
		}
		
		command->data[0] = 0;
		ret_val = send_cmd(command, APP_ID_CTRL, CONTROL_APP_I2C_TUNNEL_APPLY, 1);
		if (ret_val<0) {
			return ret_val; /* err code*/
		}
		
		command->data[0] = 1;
		ret_val = send_cmd(command, APP_ID_CAPT | capt_nr, IVNR_CMD_SET_NR_OUTPUT, 1);
		if (ret_val<0) {
			return ret_val; /* err code*/
		}
	}
	else
	{
		command->data[0] = 0;
		ret_val = send_cmd(command, APP_ID_CAPT | capt_nr, IVNR_CMD_SET_NR_OUTPUT, 1);
		if (ret_val<0) {
			return ret_val; /* err code*/
		}
		
		command->data[0] = 0;
		command->data[1] = 0;
		command->data[2] = 1;
		command->data[3] = mkword(0x32, 0x16);
		ret_val = send_cmd(command, APP_ID_CTRL, CMD_SET(CONTROL_APP_I2C_TUNNEL_DATA), 4);
		if (ret_val<0) {
			return ret_val; /* err code*/
		}
		
		command->data[0] = 0;
		ret_val = send_cmd(command, APP_ID_CTRL, CONTROL_APP_I2C_TUNNEL_APPLY, 1);
		if (ret_val<0) {
			return ret_val; /* err code*/
		}
		command->data[0] = 0;
		command->data[1] = 0;
		command->data[2] = 1;
		command->data[3] = mkword(0x33, 0x16);
		ret_val = send_cmd(command, APP_ID_CTRL, CMD_SET(CONTROL_APP_I2C_TUNNEL_DATA), 4);
		if (ret_val<0) {
			return ret_val; /* err code*/
		}
		
		command->data[0] = 0;
		ret_val = send_cmd(command, APP_ID_CTRL, CONTROL_APP_I2C_TUNNEL_APPLY, 1);
		if (ret_val<0) {
			return ret_val; /* err code*/
		}
	}
	return(0);
}

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
	VCI2CSleep(ms);
}
