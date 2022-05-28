LOCAL_PATH := $(call my-dir)

###########################
#
# yasli shared library
#
###########################

include $(CLEAR_VARS)

LOCAL_MODULE := yasli

LOCAL_C_INCLUDES := $(LOCAL_PATH)/..

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

LOCAL_CFLAGS += -DANDROID_NDK

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/*.cpp) \
    	$(wildcard $(LOCAL_PATH)/decorators/*.cpp))

include $(BUILD_SHARED_LIBRARY)

###########################
#
# yasli static library
#
###########################

LOCAL_MODULE := yasli_static

LOCAL_MODULE_FILENAME := libyasli

include $(BUILD_STATIC_LIBRARY)

