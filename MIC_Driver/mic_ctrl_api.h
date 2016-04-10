

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
	{0x36360022, 0x1E351E35},       //Ĭ��ģʽ [NC_mic1][NC_mic2][XX][DAC_CSP],[WK_mic2][WK_mic1][P_mic2][P_mic1] 
	{0x3636003D, 0x1E351E35},       //���ģʽ [NC_mic1][NC_mic2][XX][DAC_CSP],[WK_mic2][WK_mic1][P_mic2][P_mic1]
	{0x0B0B003D, 0x1E0B1E0B},       //����ģʽ [NC_mic1][NC_mic2][XX][DAC_CSP],[WK_mic2][WK_mic1][P_mic2][P_mic1]
	{0x3B3B003D, 0x1E3A1E3A}        //����ģʽ [NC_mic1][NC_mic2][XX][DAC_CSP],[WK_mic2][WK_mic1][P_mic2][P_mic1]
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

// ��λ������ʱ1s
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

  Purpose:  ����Ѷ�ɾ���ģ���Ƕ��棨�����룩�������棨�����ѡ�����������

  Inputs:   command				- i2c command struct object pointer

  Outputs:  return				- 2  ���棨�����룩
								- 3  ���棨�����ѡ�����������
								- 0  δ֪����I2C��ȡʧ��
								  
------------------------------------------------------------------------------*/

extern int VCGetVersion(Command_t * command);

/*------------------------------------------------------------------------------
  Function: VCChangeWorkMode

  Purpose:  send command to Servant for changing the working mode : 
                         1�����ģʽ��       2������ģʽ��       3������ģʽ

  Inputs:   cmd                                - i2c command struct object pointer
           work_mode_id    - id of work mode
                            
         WORK_MODE_PERIPHERAL: ���ģʽ
         WORK_MODE_TOPLIGHT: ����ģʽ
         WORK_MODE_HOST������ģʽ

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
											succeed  :	1�����ģʽ
														2������ģʽ
														3������ģʽ 
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
	FUNC_MODE_PASSBY				= 0x0,	//���й��ܹر�ֱ��¼��
	FUNC_MODE_NOISECLEAN			= 0x1,	//���빦��
	FUNC_MODE_PHONE					= 0x2,	//ͨ��������������
	FUNC_MODE_WAKEUP				= 0x3,	//���ѻ�����������
} cmd_func_id;
extern int VCChangeFunc(Command_t * command, cmd_func_id func_id);

/*------------------------------------------------------------------------------
  Function: VCGetFunc

  Purpose:  send command to Servant to get the function mode : 

  Inputs:   cmd                                - i2c command struct object pointer

  Outputs:       return                    - failed  : error code < 0
											succeed  :	0�����й��ܹر�ֱ��¼��
														1�����빦��
														2��ͨ��������������
														3�����ѻ����������� 
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
									  0					: iflytek use �����������
									  0xea0d0471: kai yue zai xian ��Խ����

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
								  succeed : 0�����ú���ǰû�л��ѷ���
											1�����ú���ǰ�л��ѷ���
			* pmsecond			- the last wakeup signal happened (*pmsecond) millisecond before.
								0 <=��*pmsecond��<= 30000ms
  ��������ʱ�����������200���롣�������û�ȡ���һ�εĻ�����Ϣ�����ú�ϵͳ��������ѱ�־��ʱ����Ϣ��
  ������30����û�е��ú�����ȡ�źţ���*pmsecond����ֹͣ��30000���������ӣ����������ֵû��ʵ��ʹ�����塣  
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
/*   VCEnableNR �����ڰ汾�����ڰ汾���е��û�����쳣   */
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
	DAC_GAIN_004          = 61,	//+4	dB  ��˷�ģ�飬����ģʽ
	DAC_GAIN_005          = 62,	//+5	dB
	DAC_GAIN_006_MAX      = 63	//+6	dB
} cmd_dac_gain_id;
extern int VCSetDACVolume(Command_t * command, cmd_dac_gain_id dac_gain_id);

