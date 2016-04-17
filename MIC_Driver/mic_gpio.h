#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/i2c.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/uaccess.h> /*copy_from_user */
#include <asm/system.h>
#include <mach/pinmux_reg.h>
#include <mach/ac83xx_pinmux_table.h>
#include <mach/ac83xx_gpio_pinmux.h>


#include <mach/gpio.h> 

//#include "mic_command.h"
//#include "mic_ctrl_api.h"

#include <linux/i2c-dev.h>
extern struct i2c_client  *mic_i2c_client;

extern int mic_i2cdev_ioctl_rdrw(struct i2c_client *client,\
		unsigned long arg);



/******************************COMMAND******************************/
#define CMD_SET(item)   ((item) & ~0x0100)
#define CMD_GET(item)   ((item) |  0x0100)
#define mkword(reg, val)	((val >> 8) | (reg << 1) | ((val & 0xFF) << 8))


// Reserved app IDs
#define APP_ID(a,b,c,d) (uint32_t)(((uint32_t)((a)-0x20)<<8)| \
                                   ((uint32_t)((b)-0x20)<<14)| \
								   ((uint32_t)((c)-0x20)<<20)| \
								   ((uint32_t)((d)-0x20)<<26))
#define APP_ID_SOS  APP_ID('S','O','S',' ')
#define APP_ID_CTRL APP_ID('C','T','R','L')
#define APP_ID_STRM APP_ID('S','T','R','M')
#define APP_ID_CAPT APP_ID('C','A','P','T')
#define APP_ID_GVAD APP_ID('G','V','A','D')
#define APP_ID_PLBK APP_ID('P','L','B','K')
#define APP_ID_BROADCAST     0xFFFFFF00 // to broadcast commands to all apps

typedef enum {
	capt_input_mic   =  1,
	capt_nr          =  2,
	capt_output      =  3
} CaptModuleId;

typedef enum {

	CONTROL_APP_CMD_RESERVED      =  0,
	//  CONTROL_APP_SUSPEND         =  1,
	//  CONTROL_APP_RESUME          =  2,
	CONTROL_APP_VERSION           =  3,
	CONTROL_APP_EXEC_FILE         =  4,
	CONTROL_APP_OCLA_ENABLE       =  5,
	CONTROL_APP_MEMORY            =  6,
	CONTROL_APP_CLOCK	        =  7,

	CONTROL_APP_I2C_TUNNEL_CONFIG =  8,
	CONTROL_APP_I2C_TUNNEL_DATA   =  9,
	CONTROL_APP_I2C_TUNNEL_APPLY  = 10,

	CONTROL_APP_LOGGING_INIT      = 20,
	CONTROL_APP_LOGGING_ENABLE    = 21,

	SOS_TASK_CREATE               = 35,
	SOS_TASK_PRIORITY             = 36,
	SOS_EXEC_DUP                  = 37,
	SOS_EXEC_FREE                 = 38,
	SOS_EXEC_PARM                 = 39,
	SOS_JIFFIES                   = 41,
	SOS_SIGNAL                    = 42,
	SOS_SIGNAL_ALL                = 43,
	SOS_ENABLE_IRQ_NR             = 44,
	SOS_DISABLE_IRQ_NR            = 45,
	SOS_RESOURCE                  = 47,
	SOS_TIME                      = 50,
	SOS_TASK_SLEEP_JIFFIES        = 51,

	CONTROL_APP_USB_START         = 60,
	CONTROL_APP_USB_STOP	        = 61,

} ControlAppCommandCode;


typedef enum IvnrCmdCodeTag {
	IVNR_CMD_SIN_OUTPUT = 0x0040,
	IVNR_CMD_NR = 0x0041,
	IVNR_CMD_WATER_MARK = 0x0042,
	IVNR_CMD_BLINK = 0x0043,
	IVNR_CMD_IO_CONTROL = 0x0044,
	IVNR_CMD_NR_OUTPUT = 0x0045,
	IVNR_CMD_SESSION_DATA = 0x0046,
	IVNR_CMD_WATCHDOG = 0x0047,
	IVNR_CMD_I2C_CONTROL = 0x0048

} IvnrCmdCode;


#define IVNR_CMD_GET_I2C_CONTROL			CMD_GET(IVNR_CMD_I2C_CONTROL)
#define IVNR_CMD_SET_I2C_CONTROL			CMD_SET(IVNR_CMD_I2C_CONTROL)


// The maximum number of 32-bit data elements that a command can contain
#define MAX_COMMAND_SIZE 13
typedef struct {                     
	int16_t   num_16b_words;  
	uint16_t  command_id;    
	uint32_t  app_module_id; 
	uint32_t  data[MAX_COMMAND_SIZE] ;    
}Command_t;


int test_version(Command_t *command);
int test_ret(Command_t *command);
int send_cmd (Command_t *cmd, uint32_t  app_module_id, uint32_t  command_id, uint32_t num_32b_words);

/******************************CTR_API******************************/

#define UNREFERENCED_PARAMETER(P)      {(P) = (P);} 
#define MODULE_I2C_CTRL_FIRST	 (0x00)
#define IV_I2C_CMD_CTRL		(0x0048)

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
#if 0
const uint32_t g_work_mode_gain[WORK_MODE_MAX][2] = {
	{0x36360022, 0x1E351E35},       //默认模式 [NC_mic1][NC_mic2][XX][DAC_CSP],[WK_mic2][WK_mic1][P_mic2][P_mic1] 
	{0x3636003D, 0x1E351E35},       //外挂模式 [NC_mic1][NC_mic2][XX][DAC_CSP],[WK_mic2][WK_mic1][P_mic2][P_mic1]
	{0x0B0B003D, 0x1E0B1E0B},       //顶灯模式 [NC_mic1][NC_mic2][XX][DAC_CSP],[WK_mic2][WK_mic1][P_mic2][P_mic1]
	{0x3B3B003D, 0x1E3A1E3A}        //车机模式 [NC_mic1][NC_mic2][XX][DAC_CSP],[WK_mic2][WK_mic1][P_mic2][P_mic1]
};
#endif

#define MODULE_RC_I2C_FIRST	-2048

typedef enum IvnrI2CRetTypeTag {
	IV_I2C_RET_ACK_CMD = MODULE_RC_I2C_FIRST - 4,
	IV_I2C_RET_NOSUPPORT = MODULE_RC_I2C_FIRST -3,
	IV_I2C_RET_FAILED = MODULE_RC_I2C_FIRST - 2,
	IV_I2C_RET_BUSY = MODULE_RC_I2C_FIRST -1,
	IV_I2C_RET_SUCCESS_STANDBY = 0x00,
} IvnrI2CRetType;

// 复位操作耗时1s
extern int VCReset(Command_t * command);
/*------------------------------------------------------------------------------
  Function: VCWaitReady

  Purpose:    waiting for servant i2c control module ready. 400ms after servant
              turning on, it is ready for receiving orders from the host. so we 
			  need to send command to check ready. if servant is ready, function 
			  returns 1 immediately, otherwise it will continue to check until 
			  the timeout, and return 0.

  Inputs:   cmd				- i2c command struct object pointer
			msecond			- timeout length, in milliseconds

  Outputs:  return BOOL 		- servant ready : 1; 
                                  waiting timeout : 0
------------------------------------------------------------------------------*/
extern bool VCWaitReady(Command_t * command, int msecond);

/*------------------------------------------------------------------------------
  Function: VCGetVersion

  Purpose:  区分讯飞静音模块是二版（仅降噪）还是三版（带唤醒、回声消除）

  Inputs:   command				- i2c command struct object pointer

  Outputs:  return				- 2  二版（仅降噪）
								- 3  三版（带唤醒、回声消除）
								- 0  未知，或I2C读取失败
								  
------------------------------------------------------------------------------*/

extern int VCGetVersion(Command_t * command);

/*------------------------------------------------------------------------------
  Function: VCChangeWorkMode

  Purpose:  send command to Servant for changing the working mode : 
                         1：外挂模式、       2：顶灯模式、       3：车机模式

  Inputs:   cmd                                - i2c command struct object pointer
           work_mode_id    - id of work mode
                            
         WORK_MODE_PERIPHERAL: 外挂模式
         WORK_MODE_TOPLIGHT: 顶灯模式
         WORK_MODE_HOST：车机模式

  Outputs:       return                         - succeed : 0; 
         `                                                       failed  : error code < 0
------------------------------------------------------------------------------*/

typedef enum
{
	WORK_MODE_PERIPHERAL				= 0x1,
	WORK_MODE_TOPLIGHT					= 0x2,
	WORK_MODE_HOST						= 0x3
} cmd_work_mode_id;

extern int VCChangeWorkMode(Command_t *command, cmd_work_mode_id work_mode_id);

/*------------------------------------------------------------------------------
  Function: VCGetWorkMode

  Purpose:  send command to Servant to get the working mode : 

  Inputs:   cmd                                - i2c command struct object pointer

  Outputs:       return                    - failed  : error code < 0
											succeed  :	1：外挂模式
														2：顶灯模式
														3：车机模式 
------------------------------------------------------------------------------*/
extern int VCGetWorkMode(Command_t * command);


/*------------------------------------------------------------------------------
  Function: VCChangeFunc

  Purpose:  send command to servant for changing the function mode

  Inputs:   cmd				- i2c command struct object pointer
            mode_id	- id of function mode
			
	FUNC_MODE_PASSBY: record witout algorithm 
	FUNC_MODE_NOISECLEAN: noise reduction record function
	FUNC_MODE_PHONE: echo reduction for telephone conversation function
	FUNC_MODE_WAKEUP: echo reduction and wakeup algorithm function

  Outputs:	return			- succeed : 0; 
	`						  failed  : error code < 0
------------------------------------------------------------------------------*/
typedef enum
{
	FUNC_MODE_PASSBY				= 0x0,	//所有功能关闭直接录音
	FUNC_MODE_NOISECLEAN			= 0x1,	//降噪功能
	FUNC_MODE_PHONE					= 0x2,	//通话回声消除功能
	FUNC_MODE_WAKEUP				= 0x3,	//唤醒回声消除功能
} cmd_func_id;
extern int VCChangeFunc(Command_t * command, cmd_func_id func_id);

/*------------------------------------------------------------------------------
  Function: VCGetFunc

  Purpose:  send command to Servant to get the function mode : 

  Inputs:   cmd                                - i2c command struct object pointer

  Outputs:       return                    - failed  : error code < 0
											succeed  :	0：所有功能关闭直接录音
														1：降噪功能
														2：通话回声消除功能
														3：唤醒回声消除功能 
------------------------------------------------------------------------------*/
extern int VCGetFunc(Command_t * command);

/*------------------------------------------------------------------------------
  Function: VCEnableWakeupGpio

  Purpose:  sending a command to servant for wakeup gpio signal config
            in echo reduction and wakeup function

  Inputs:   command				- i2c command struct object pointer
			key_level			- key level is low or high level
									  high level  : 1
									  low level   : 0
		    msecond				- key level length 

  Outputs:  return				- succeed : 0
								  failed  : error code < 0
------------------------------------------------------------------------------*/
extern int VCConfigWakeupGpio(Command_t * command, bool key_level, int msecond);

/*------------------------------------------------------------------------------
  Function: VCConfigWakeupID

  Purpose:  sending a command to servant for wakeup id config
            in echo reduction and wakeup function

  Inputs:   command				- i2c command struct object pointer
			wakeup_id			- id of wakeup word
									  0					: iflytek use 你好语音助理
									  0xea0d0471: kai yue zai xian 凯越在线

  Outputs:  return				- succeed : 0
								  failed  : error code < 0
------------------------------------------------------------------------------*/
extern int VCConfigWakeupID(Command_t * command, int wakeup_id);

/*------------------------------------------------------------------------------
  Function: VCGetWakeupSign

  Purpose:  send command to Servant to get the last wakeup sign

  Inputs:   cmd                 - i2c command struct object pointer

  Outputs: 
			return              - failed : error code < 0
								  succeed : 0：调用函数前没有唤醒发生
											1：调用函数前有唤醒发生
			* pmsecond			- the last wakeup signal happened (*pmsecond) millisecond before.
								0 <=（*pmsecond）<= 30000ms
  函数调用时间间隔建议大于200毫秒。函数调用获取最近一次的唤醒信息，调用后，系统会清除唤醒标志和时间信息。
  当唤醒30秒内没有调用函数读取信号，（*pmsecond）将停止在30000，不再增加，因此这个最大值没有实际使用意义。  
------------------------------------------------------------------------------*/
extern int VCGetWakeupSign(Command_t * command, int * pmsecond);

/*------------------------------------------------------------------------------
  Function: VCEnableNR

  Purpose:  sending a command to servant for turning on/off the noise reduction
            algorithm in noise reduction function

  Inputs:   command				- i2c command struct object pointer
			enable_nr			- enable or disable noise reduction algorithm
									  enable  : 1
									  disable : 0

  Outputs:  return				- succeed : 0
								  failed  : error code < 0
------------------------------------------------------------------------------*/
/*************************************************************************************/
/*   VCEnableNR 仅用于版本二，于版本三中调用会造成异常   */
/*************************************************************************************/
extern int VCEnableNR(Command_t * command, bool enable_nr);

/*------------------------------------------------------------------------------
  Function: VCSetDACVolume

  Purpose:  sending a command to servant to set output dac gain

  Inputs:   command			- i2c command struct object pointer
            cmd_adc_gain_id	- id of dac gain, for WOKING_MODE_NR_ANALOG

  Outputs:	return			- succeed : 0; 
							  failed  : error code < 0
------------------------------------------------------------------------------*/
typedef enum
{
	DAC_GAIN_N23          = 34,	//-23	dB
	DAC_GAIN_N22          = 35,	//-22	dB
	DAC_GAIN_N21          = 36,	//-21	dB
	DAC_GAIN_N20          = 37,	//-20	dB
	DAC_GAIN_N19          = 38,	//-19	dB
	DAC_GAIN_N18          = 39,	//-18	dB
	DAC_GAIN_N17          = 40,	//-17	dB
	DAC_GAIN_N16          = 41,	//-16	dB
	DAC_GAIN_N15          = 42,	//-15	dB
	DAC_GAIN_N14          = 43,	//-14	dB
	DAC_GAIN_N13          = 44,	//-13	dB
	DAC_GAIN_N12          = 45,	//-12	dB
	DAC_GAIN_N11          = 46,	//-11	dB
	DAC_GAIN_N10          = 47,	//-10	dB
	DAC_GAIN_N09          = 48,	//-9	dB
	DAC_GAIN_N08          = 49,	//-8	dB
	DAC_GAIN_N07          = 50,	//-7	dB	default
	DAC_GAIN_N06          = 51,	//-6	dB
	DAC_GAIN_N05          = 52,	//-5	dB
	DAC_GAIN_N04          = 53,	//-4	dB
	DAC_GAIN_N03          = 54,	//-3	dB
	DAC_GAIN_N02          = 55,	//-2	dB
	DAC_GAIN_N01          = 56,	//-1	dB
	DAC_GAIN_000          = 57,	// 0	dB
	DAC_GAIN_001          = 58,	//+1	dB
	DAC_GAIN_002          = 59,	//+2	dB
	DAC_GAIN_003          = 60,	//+3	dB
	DAC_GAIN_004          = 61,	//+4	dB  麦克风模组，顶灯模式
	DAC_GAIN_005          = 62,	//+5	dB
	DAC_GAIN_006_MAX      = 63	//+6	dB
} cmd_dac_gain_id;

extern int VCSetDACVolume(Command_t * command, cmd_dac_gain_id dac_gain_id);
