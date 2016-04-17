#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <jni.h>
#include "MicHalService.h"
#include "vc_i2c_io_interface.h"
#include "vc_ctrl_api.h"

#define IIC_DEV "/proc/mic_proc/set_mic"            //device i2c address
#if 0
struct mic_control_device_t *mic_hal_device = NULL;

static jboolean mic_Set(JNIEnv *env, jobject thiz, jint mic, jint value)
{
//	LOGI("Mic HAL JNI: mic_Set() is invoked.");
	if(mic_hal_device == NULL)
	{
//		LOGI("Mic HAL JNI: mic_hal_device was not fetched correctly.");
		return -1;
	}
	else
	{
		return mic_hal_device->fun_switch(mic_hal_device, mic, value);
	}
}

static inline int mic_control_open(const struct hw_module_t* module,
        struct mic_control_device_t** device)
{
	return module->methods->open(module, MIC_HARDWARE_MODULE_ID,
		(struct hw_device_t **)device );
}

static jint mic_init(JNIEnv *env, jclass clazz)
{
	mic_module_t *module;

//	LOGE(MIC_HARDWARE_MODULE_ID);

	if(hw_get_module(MIC_HARDWARE_MODULE_ID, (const struct hw_module_t **)module) == 0)
	{
//		LOGI("MicService JNI: MIC Stub found.");
		if(mic_control_open(&module->hw_module, &mic_hal_device) == 0)
		{
//			LOGI("MicService JNI: Got Stub operations.");
//			return 0;
		}
	}

//	LOGE("MicService JNI: Get Stub operations failed.");
	return -1;
}
#endif
jint JNICALL Java_com_iflytek_autofly_platformservice_skypine_audio_MicControl_Open(JNIEnv * env, jclass c)
{
	const char * filename = IIC_DEV;
	int rv = VCI2CInit(filename);
	return rv;
}

jint JNICALL Java_com_iflytek_autofly_platformservice_skypine_audio_MicControl_Close(JNIEnv * env, jclass c)
{
	VCI2CTerminate();
	return 0;
}

jint JNICALL Java_com_iflytek_autofly_platformservice_skypine_audio_MicControl_ChangeFuncMode(JNIEnv * env, jclass c, jint mode)
{
	int ret;
	Command_t cmd ={0};
	if (mode <0 || mode > 3){
		return -1;
	}
	ret = VCChangeFunc(&cmd, (cmd_func_id)mode);
	return ret;
}

jint JNICALL Java_com_iflytek_autofly_platformservice_skypine_audio_MicControl_GetWakeupSign(JNIEnv * env, jclass c)
{
	int ret;
	int millisecond;
	Command_t cmd ={0};
	ret = VCGetWakeupSign(&cmd, &millisecond);
	return ret;
}

jint JNICALL Java_com_iflytek_autofly_platformservice_skypine_audio_MicControl_GetFuncMode(JNIEnv * env, jclass c)
{
	int ret;
	Command_t cmd ={0};
	ret = VCGetFunc(&cmd);
	return ret;
}

jint JNICALL Java_com_iflytek_autofly_platformservice_skypine_audio_MicControl_ChangeEnvironmentMode(JNIEnv * env, jclass c, jint mode)
{
	int ret;
	Command_t cmd ={0};
	if (mode <1 || mode>3){
		return -1;
	}
	ret = VCChangeWorkMode(&cmd, (cmd_work_mode_id)mode);
	return ret;
}

jint JNICALL Java_com_iflytek_autofly_platformservice_skypine_audio_MicControl_GetEnvironmentMode(JNIEnv * env, jclass c)
{
	int ret;
	Command_t cmd ={0};
	ret = VCGetWorkMode(&cmd);
	return ret;
}


static const JNINativeMethod methods[] = {
//	{ "_init", "()I", (void *) mic_init },
//	{ "_set", "(II)Z", (void *) mic_Set },

	{"Open", "()I", (void *) Java_com_iflytek_autofly_platformservice_skypine_audio_MicControl_Open},
	{"Close", "()I", (void *) Java_com_iflytek_autofly_platformservice_skypine_audio_MicControl_Close},
	{"ChangeFuncMode", "(I)I", (void *) Java_com_iflytek_autofly_platformservice_skypine_audio_MicControl_ChangeFuncMode},
	{"GetWakeupSign", "()I", (void *) Java_com_iflytek_autofly_platformservice_skypine_audio_MicControl_GetWakeupSign},
	{"GetFuncMode", "()I", (void *) Java_com_iflytek_autofly_platformservice_skypine_audio_MicControl_GetFuncMode},
	{"GetEnvironmentMode", "()I", (void *) Java_com_iflytek_autofly_platformservice_skypine_audio_MicControl_GetEnvironmentMode},
	{"ChangeEnvironmentMode", "(I)I", (void *) Java_com_iflytek_autofly_platformservice_skypine_audio_MicControl_ChangeEnvironmentMode},
};

int register_mic_hal_jni(JNIEnv *env)
{
	static const char* const kClassName = "com/android/mic/hal/service/MicHalService";

	jclass clazz;

	clazz = env->FindClass(kClassName);
	if (clazz == NULL)
	{
//		LOGE("Can't find class %s\n", kClassName);
		return -1;
	}
	if (env->RegisterNatives(clazz, methods,
	        sizeof(methods) / sizeof(methods[0])) != JNI_OK)
	{
//		LOGE("Failed registering methods for %s\n", kClassName);
		return -1;
	}

	return 0;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	JNIEnv* env = NULL;
	jint result = -1;

	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
	{
//		LOGE("GetEnv failed!");
		return result;
	}

	register_mic_hal_jni(env);

	return JNI_VERSION_1_4;
}
