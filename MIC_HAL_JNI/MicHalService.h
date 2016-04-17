#include <hardware/hardware.h>
#include <fcntl.h>
#include <cutils/log.h>

struct mic_module_t{
	struct hw_module_t hw_module;
};

struct mic_control_device_t{
	struct hw_device_t hw_device;

	int (*fun_switch)(struct mic_control_device_t *dev, int32_t mic, int32_t value);
};


#define MIC_HARDWARE_MODULE_ID	"mic_hal"
//#define MIC_DEV_NAME			"/dev/mic-ifly-0"

#define MIC_CMD_CTL_FUNC_MASK		0x80
#define GET_MIC_VERSION				0
#define GET_MIC_WAITREADY			(GET_MIC_VERSION + 1)
#define GET_MIC_CURFUNCTION			(GET_MIC_WAITREADY + 1)
#define SET_MIC_FUNCTION			(GET_MIC_CURFUNCTION + 1)
#define SET_MIC_WOEKMODE			(SET_MIC_FUNCTION + 1)
#define SET_MIC_DACVOLUME			(SET_MIC_WOEKMODE + 1)
#define SET_MIC_RESET				(SET_MIC_DACVOLUME + 1)
