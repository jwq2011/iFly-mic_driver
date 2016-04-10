#include "mic_hal.h"

int dev_file = 0;

int mic_set(struct mic_control_device_t *dev, int32_t mic_command, int32_t value)
{
	unsigned char buf[5];
	unsigned long arg;
	int ret;

	buf[0] = (mic_command|MIC_CMD_CTL_FUNC_MASK) >> 8;
	buf[1] = (mic_command|MIC_CMD_CTL_FUNC_MASK) & 0xF;

	switch(mic_command)
	{
		case GET_MIC_VERSION:
		case GET_MIC_WAITREADY:
		case GET_MIC_CURFUNCTION:
			write(dev_file, buf, 2);
			read(dev_file, buf, 5);
			ret = buf[2];
			break;
		case SET_MIC_FUNCTION:
			ret = ioctl(dev_file, value, arg);
			break;
		case SET_MIC_WOEKMODE:
		case SET_MIC_DACVOLUME:
		case SET_MIC_RESET:
			buf[3] = value >> 8;
			buf[4] = value & 0xF;
			ret = write(dev_file, buf, 5);
			break;
		default:
			break;
	}

	return ret;
}

int mic_fun_switch(struct mic_control_device_t *dev, int32_t mic, int32_t value)
{
	return mic_set(dev, mic, value);
}

static int mic_init_gpm(void)
{
	unsigned char buf[5];
	buf[0] = SET_MIC_RESET;
	write(dev_file, buf, 5);

	return 0;
}

static int mic_device_close(struct hw_device_t* device)
{
	struct mic_control_device_t *ctx = (struct mic_control_device_t *)device;
	if(ctx)
	{
		free(ctx);
	}
	close(dev_file);

	return 0;
}

static int mic_device_open(const struct hw_module_t* module, const char* id,
            struct hw_device_t** device)
{
	struct mic_control_device_t *dev;
	dev = (struct mic_control_device_t *)malloc(sizeof(*dev));
	memset(dev, 0, sizeof(*dev));

	dev->hw_device.tag = HARDWARE_DEVICE_TAG;
	dev->hw_device.version = 0;
	dev->hw_device.module = (struct hw_module_t *)module;
	dev->hw_device.close = mic_device_close;

	dev->fun_switch = mic_fun_switch;

	*device = (hw_device_t *)dev;

	dev_file = open(MIC_DEV_NAME, O_RDWR);
//	LOGI("MIC Stub:open %s %s.", MIC_DEV_NAME, (dev_file < 0)?"fail":"success");

	mic_init_gpm();

	return 0;
}

static struct hw_module_methods_t mic_module_methods={
	open: mic_device_open
};

struct mic_module_t HAL_MODULE_INFO_SYM = {
	hw_module:
	{
		tag:HARDWARE_MODULE_TAG,
		version_major:1,
		version_minor:0,
		id:MIC_HARDWARE_MODULE_ID,
		name:"iFly MIC HAL Stub",
		author:"HCN",
		methods:&mic_module_methods,
	}
};
