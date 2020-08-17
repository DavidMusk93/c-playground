LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_LDLIBS += -lz -llog -lm
LOCAL_CFLAGS += -fPIE -fPIC
LOCAL_CPPFLAGS += -std=c++14
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_MODULE := forward_server
LOCAL_SRC_FILES := connection.cpp server.cpp
include $(BUILD_EXECUTABLE)