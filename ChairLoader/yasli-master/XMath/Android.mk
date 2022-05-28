LOCAL_PATH := $(call my-dir)

###########################
#
# XMath shared library
#
###########################

include $(CLEAR_VARS)

LOCAL_MODULE := XMath

LOCAL_C_INCLUDES := $(LOCAL_PATH)/..
LOCAL_CFLAGS := -DXMATH_IN_ENGLISH

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

LOCAL_SRC_FILES := XMath.cpp Recti.cpp Rectf.cpp Range.cpp Colors.cpp ExceptionStub.cpp

include $(BUILD_SHARED_LIBRARY)

###########################
#
# XMath static library
#
###########################

LOCAL_MODULE := XMath_static

LOCAL_MODULE_FILENAME := libXMath

include $(BUILD_STATIC_LIBRARY)

