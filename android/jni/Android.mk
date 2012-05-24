LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := libglmark2-matrix
LOCAL_CFLAGS := -DUSE_GLESv2 -Werror -Wall -Wextra -Wnon-virtual-dtor \
                -Wno-error=unused-parameter
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libmatrix/*.cc))
LOCAL_SHARED_LIBRARIES := libdl libstlport

include external/stlport/libstlport.mk

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := libglmark2-png
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libpng/*.c))
LOCAL_C_INCLUDES := external/zlib

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := libglmark2-ideas
LOCAL_CFLAGS := -DGLMARK_DATA_PATH="" -DUSE_GLESv2 -Werror -Wall -Wextra\
                -Wnon-virtual-dtor -Wno-error=unused-parameter
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src \
                    $(LOCAL_PATH)/src/libmatrix
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/scene-ideas/*.cc))
LOCAL_SHARED_LIBRARIES := libdl libstlport

include external/stlport/libstlport.mk

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libglmark2-android
LOCAL_STATIC_LIBRARIES := libglmark2-matrix libglmark2-png libglmark2-ideas
LOCAL_CFLAGS := -DGLMARK_DATA_PATH="" -DGLMARK_VERSION="\"2012.05\"" \
                -DUSE_GLESv2 -Werror -Wall -Wextra -Wnon-virtual-dtor \
                -Wno-error=unused-parameter
LOCAL_SHARED_LIBRARIES := liblog libz libEGL libGLESv2 libandroid libdl libstlport
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src \
                    $(LOCAL_PATH)/src/libmatrix \
                    $(LOCAL_PATH)/src/scene-ideas \
                    $(LOCAL_PATH)/src/libpng \
                    external/zlib
LOCAL_SRC_FILES := $(filter-out src/canvas% src/main.cpp, \
                     $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/*.cpp))) \
                   src/canvas-android.cpp
LOCAL_PRELINK_MODULE := false

include external/stlport/libstlport.mk

include $(BUILD_SHARED_LIBRARY)
