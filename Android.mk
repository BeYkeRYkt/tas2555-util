ifneq ($(TARGET_SIMULATOR), true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := main.c

LOCAL_C_INCLUDES := external/ti_audio/

LOCAL_CFLAGS := -O2 -g -W -Wall -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

LOCAL_MODULE := ti_audio
LOCAL_MODULE_TAGS := debug
LOCAL_SYSTEM_SHARED_LIBRARIES := libc libm

include $(BUILD_EXECUTABLE)

endif
