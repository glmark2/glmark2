LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPP_EXTENSION := .cc
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/libmatrix
LOCAL_MODULE := matrix
LOCAL_CPPFLAGS := -DANDROID -DUSE_GLESv2
LOCAL_SRC_FILES := libmatrix/mat.cc \
                   libmatrix/program.cc

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := png
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/libpng
LOCAL_SRC_FILES := libpng/png.c \
                   libpng/pngerror.c \
                   libpng/pnggccrd.c \
                   libpng/pngget.c \
                   libpng/pngmem.c \
                   libpng/pngpread.c \
                   libpng/pngread.c \
                   libpng/pngrio.c \
                   libpng/pngrtran.c \
                   libpng/pngrutil.c \
                   libpng/pngset.c \
                   libpng/pngtrans.c \
                   libpng/pngvcrd.c \
                   libpng/pngwio.c \
                   libpng/pngwrite.c \
                   libpng/pngwtran.c \
                   libpng/pngwutil.c

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := glmark2
LOCAL_STATIC_LIBRARIES := matrix png
LOCAL_CPPFLAGS := -DANDROID -DUSE_GLESv2 -DGLMARK_DATA_PATH=""
LOCAL_LDLIBS := -landroid -llog -lGLESv2 -lEGL -lz -lm
LOCAL_SRC_FILES := android.cpp \
                   benchmark.cpp \
                   canvas-android.cpp \
                   log.cpp \
                   mesh.cpp \
                   model.cpp \
                   options.cpp \
                   scene-build.cpp \
                   scene-bump.cpp \
                   scene-conditionals.cpp \
                   scene.cpp \
                   scene-default-options.cpp \
                   scene-effect-2d.cpp \
                   scene-function.cpp \
                   scene-grid.cpp \
                   scene-loop.cpp \
                   scene-shading.cpp \
                   scene-texture.cpp \
                   shader-source.cpp \
                   texture.cpp \
                   util.cpp

include $(BUILD_SHARED_LIBRARY)
