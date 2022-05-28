LOCAL_PATH := $(call my-dir)

###########################
#
# ww shared library
#
###########################

include $(CLEAR_VARS)

LOCAL_MODULE := ww

LOCAL_C_INCLUDES := $(LOCAL_PATH)/..

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

LOCAL_SRC_FILES := Color_ww.cpp SliderDecorator.cpp

include $(BUILD_SHARED_LIBRARY)

###########################
#
# ww static library
#
###########################

LOCAL_MODULE := ww_static

LOCAL_MODULE_FILENAME := libww

include $(BUILD_STATIC_LIBRARY)

