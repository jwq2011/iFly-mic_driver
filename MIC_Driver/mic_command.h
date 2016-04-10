


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



