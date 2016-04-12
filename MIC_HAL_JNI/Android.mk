# Android.mk
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE:= mic_hal_jni

LOCAL_MODULE_PATH := /home/jiawq/workspace/SourceSave/git_save/iFly-mic_driver/MIC_HAL_JNI

LOCAL_SRC_FILES:= MicHalService.cpp
   

LOCAL_SHARED_LIBRARIES := \
	libandroid_runtime \
	libcutils \
	libhardware \
	libhardware_legacy \
	libnativehelper \
#        libsystem_server \
	libutils \
	libui \
#        libsurfaceflinger_client
       
  
LOCAL_C_INCLUDES += \
	$(JNI_H_INCLUDE) \
        hardware/mic_hal

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)

