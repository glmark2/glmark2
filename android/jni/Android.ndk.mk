LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := libglmark2-matrix
LOCAL_CFLAGS := -DGLMARK2_USE_GLESv2 -Werror -Wall -Wextra -Wnon-virtual-dtor \
                -Wno-error=unused-parameter
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src \
                    $(LOCAL_PATH)/src/glad/include
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libmatrix/*.cc))

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := libglmark2-png
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libpng/*.c))

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := libglmark2-jpeg
LOCAL_CFLAGS := -Werror -Wall -Wextra -Wno-error=attributes \
                -Wno-error=unused-parameter -Wno-error=unused-function -Wno-error=unused-variable
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src/libjpeg-turbo/
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jcapimin.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jcapistd.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jccoefct.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jccolor.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jcdctmgr.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jchuff.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jcinit.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jcmainct.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jcmarker.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jcmaster.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jcomapi.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jcparam.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jcphuff.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jcprepct.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jcsample.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jdapimin.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jdapistd.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jdatadst.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jdatasrc.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jdcoefct.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jdcolor.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jddctmgr.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jdhuff.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jdinput.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jdmainct.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jdmarker.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jdmaster.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jdmerge.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jdphuff.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jdpostct.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jdsample.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jerror.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jfdctflt.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jfdctfst.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jfdctint.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jidctflt.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jidctfst.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jidctint.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jidctred.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jmemmgr.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jmemnobs.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jquant1.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jquant2.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jutils.c)) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/libjpeg-turbo/jsimd_none.c))
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_CPP_EXTENSION := .cc
LOCAL_MODULE := libglmark2-ideas
LOCAL_CFLAGS := -DGLMARK_DATA_PATH="" -DGLMARK2_USE_GLESv2 -Werror -Wall -Wextra\
                -Wnon-virtual-dtor -Wno-error=unused-parameter
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src \
                    $(LOCAL_PATH)/src/libmatrix \
                    $(LOCAL_PATH)/src/glad/include
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/scene-ideas/*.cc))
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libglad-egl
LOCAL_CFLAGS := -Werror -Wall
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src/glad/include
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/glad/src/egl.c))
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libglad-glesv2
LOCAL_CFLAGS := -Werror -Wall
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src/glad/include
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/glad/src/gles2.c))
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libglmark2-android
LOCAL_STATIC_LIBRARIES := libglmark2-matrix libglmark2-png libglmark2-ideas libglmark2-jpeg libglad-egl libglad-glesv2
LOCAL_CFLAGS := -DGLMARK_DATA_PATH="" -DGLMARK_VERSION="\"2021.02\"" \
                -DGLMARK2_USE_GLESv2 -Werror -Wall -Wextra -Wnon-virtual-dtor \
                -Wno-error=unused-parameter
LOCAL_LDLIBS := -landroid -llog -lz
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src \
                    $(LOCAL_PATH)/src/libmatrix \
                    $(LOCAL_PATH)/src/scene-ideas \
                    $(LOCAL_PATH)/src/scene-terrain \
                    $(LOCAL_PATH)/src/libjpeg-turbo \
                    $(LOCAL_PATH)/src/libpng \
                    $(LOCAL_PATH)/src/glad/include
LOCAL_SRC_FILES := $(filter-out src/canvas% src/gl-state% src/native-state% src/main.cpp, \
                     $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/*.cpp))) \
                   $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/scene-terrain/*.cpp)) \
                   src/canvas-android.cpp

include $(BUILD_SHARED_LIBRARY)
