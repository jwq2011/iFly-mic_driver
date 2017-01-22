#include "vc_ctrl_api.h"
#include "vc_i2c_command.h"

// 调用示例


int  VCI2CInit(const char * devname);

void  VCI2CTerminate(void);

int main(int argc, char *argv[])
{
	int32_t ret = -1;
	int rv;
	Command_t cmd ={0};

	if(argc < 2)
	{
		printf("Please enter as | ntr-ctrl /dev/name \n");
		return 0;
	}
	rv = VCI2CInit(argv[1]);
	printf("VCI2CInit rv = %d\n", rv);
	if(rv < 0)
		return -1;
	
//	VCI2CSleep(2500); // 模块启动
	
	// 功能切换
	// 上电后必须先切换到录音功能。一般不运行识别和唤醒时，都使用模块的录音功能。
	ret = VCChangeFunc(&cmd, FUNC_MODE_PASSBY);
	printf("VCChangeFunc(&cmd, FUNC_MODE_PASSBY) ret = %d\n", ret);
	// 启动识别前切换到降噪功能
	ret = VCChangeFunc(&cmd, FUNC_MODE_NOISECLEAN);
	// 启动唤醒时切换到唤醒回声消除功能
	ret = VCChangeFunc(&cmd, FUNC_MODE_WAKEUP);
	printf("VCChangeFunc(&cmd, FUNC_MODE_WAKEUP) ret = %d\n", ret);
	// 打电话时切换到电话回声消除功能。通常你不需要此功能。
	// 注意你的电话系统自身集成了回声消除功能。不需要使用我们的电话回声消除。
	// 打电话时，只要把模块切换到录音功能即可。
	ret = VCChangeFunc(&cmd, FUNC_MODE_PHONE);
	
	VCI2CTerminate();

	return 0;
}
