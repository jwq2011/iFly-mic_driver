#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <jni.h>
#include "mic_hal.h"

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

static jboolean mic_init(JNIEnv *env, jclass clazz)
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

static const JNINativeMethod methods[] = {
	{ "_init", "()Z", (void *) mic_init },
	{ "_set", "(I)Z", (void *) mic_Set },
};

int register_mic_hal_jni(JNIEnv *env)
{
	static const char* const kClassName = "mobile/android/mic/hal/service/MicHalService";

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
