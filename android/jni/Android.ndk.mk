LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := libglmark2-matrix
LOCAL_CFLAGS := -DUSE_GLESv2 -Werror -Wall -Wextra -Wnon-virtual-dtor
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libmatrix/*.cc))

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := libglmark2-png
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libpng/*.c))

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libglmark2-android
LOCAL_STATIC_LIBRARIES := libglmark2-matrix libglmark2-png
LOCAL_CFLAGS := -DGLMARK_DATA_PATH="" -DGLMARK_VERSION="\"2012.01\"" \
                -DUSE_GLESv2 -Werror -Wall -Wextra -Wnon-virtual-dtor
LOCAL_LDLIBS := -landroid -llog -lGLESv2 -lEGL -lz
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src \
                    $(LOCAL_PATH)/src/libmatrix \
                    $(LOCAL_PATH)/src/libpng
LOCAL_SRC_FILES := $(filter-out src/canvas% src/main.cpp, \
                     $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/*.cpp))) \
                   src/canvas-android.cpp

include $(BUILD_SHARED_LIBRARY)
