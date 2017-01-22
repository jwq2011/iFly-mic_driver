# Android.mk
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE:= libmiccontrol_jni

LOCAL_MODULE_PATH := ~/workspace/Git_Save_share/my_code_collect/iFly-mic_driver/MIC_HAL_JNI
#LOCAL_MODULE_PATH := ./

LOCAL_SRC_FILES:= MicHalService.cpp vc_ctrl_api.c vc_i2c_command.c vc_i2c_io_interface.c
   

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

include $(CLEAR_VARS)

LOCAL_MODULE    := nr-ctrl
LOCAL_MODULE_PATH := ~/workspace/Git_Save_share/my_code_collect/iFly-mic_driver/MIC_HAL_JNI
LOCAL_SRC_FILES := main.c vc_ctrl_api.c vc_i2c_command.c vc_i2c_io_interface.c
include $(BUILD_EXECUTABLE)
