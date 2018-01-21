ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=glmake2 application
endef

SRC_ROOT=$(PROJECT_ROOT)/../src

NAME=glmark2

EXTRA_INCVPATH+=$(SRC_ROOT)
EXTRA_INCVPATH+=$(SRC_ROOT)/libmatrix

EXTRA_SRCVPATH+=$(SRC_ROOT)
EXTRA_SRCVPATH+=$(SRC_ROOT)/libmatrix
EXTRA_SRCVPATH+=$(SRC_ROOT)/scene-terrain
EXTRA_SRCVPATH+=$(SRC_ROOT)/scene-ideas

CCFLAGS+=\
	-DHAVE_STDLIB_H=1 \
	-DHAVE_MEMSET=1 \
	-DHAVE_SQRT=1 \
	-DHAVE_LIBPNG=1 \
	-DHAVE_EGL=1 \
	-DGLMARK2_USE_GLESv2=1 \
	-DGLMARK2_USE_EGL=1 \
	-DGLMARK2_USE_QNX=1 \
	-DGLMARK_DATA_PATH=\"data\" \
	-DGLMARK_VERSION=\"2017.07\" \
	-DUSE_EXCEPTIONS \

SRCS=\
	$(SRC_ROOT)/main.cpp \
	$(SRC_ROOT)/canvas-generic.cpp \
	$(SRC_ROOT)/native-state-qnx.cpp \
	$(SRC_ROOT)/gl-state-egl.cpp \
	$(SRC_ROOT)/libmatrix/log.cc \
	$(SRC_ROOT)/libmatrix/mat.cc  \
	$(SRC_ROOT)/libmatrix/program.cc \
	$(SRC_ROOT)/libmatrix/shader-source.cc \
	$(SRC_ROOT)/libmatrix/util.cc \
	$(SRC_ROOT)/benchmark-collection.cpp \
	$(SRC_ROOT)/benchmark.cpp \
	$(SRC_ROOT)/gl-headers.cpp \
	$(SRC_ROOT)/gl-visual-config.cpp \
	$(SRC_ROOT)/image-reader.cpp \
	$(SRC_ROOT)/main-loop.cpp \
	$(SRC_ROOT)/mesh.cpp \
	$(SRC_ROOT)/model.cpp \
	$(SRC_ROOT)/options.cpp \
	$(SRC_ROOT)/scene-buffer.cpp \
	$(SRC_ROOT)/scene-build.cpp \
	$(SRC_ROOT)/scene-bump.cpp \
	$(SRC_ROOT)/scene-clear.cpp \
	$(SRC_ROOT)/scene-conditionals.cpp \
	$(SRC_ROOT)/scene-default-options.cpp \
	$(SRC_ROOT)/scene-desktop.cpp \
	$(SRC_ROOT)/scene-effect-2d.cpp \
	$(SRC_ROOT)/scene-function.cpp \
	$(SRC_ROOT)/scene-grid.cpp \
	$(SRC_ROOT)/scene-ideas.cpp \
	$(SRC_ROOT)/scene-jellyfish.cpp \
	$(SRC_ROOT)/scene-loop.cpp \
	$(SRC_ROOT)/scene-pulsar.cpp \
	$(SRC_ROOT)/scene-refract.cpp \
	$(SRC_ROOT)/scene-shading.cpp \
	$(SRC_ROOT)/scene-shadow.cpp \
	$(SRC_ROOT)/scene-terrain.cpp \
	$(SRC_ROOT)/scene-texture.cpp \
	$(SRC_ROOT)/scene.cpp \
	$(SRC_ROOT)/text-renderer.cpp \
	$(SRC_ROOT)/texture.cpp \
	$(SRC_ROOT)/scene-ideas/a.cc \
	$(SRC_ROOT)/scene-ideas/d.cc \
	$(SRC_ROOT)/scene-ideas/e.cc \
	$(SRC_ROOT)/scene-ideas/i.cc \
	$(SRC_ROOT)/scene-ideas/lamp.cc \
	$(SRC_ROOT)/scene-ideas/logo.cc \
	$(SRC_ROOT)/scene-ideas/m.cc \
	$(SRC_ROOT)/scene-ideas/n.cc \
	$(SRC_ROOT)/scene-ideas/o.cc \
	$(SRC_ROOT)/scene-ideas/s.cc \
	$(SRC_ROOT)/scene-ideas/splines.cc \
	$(SRC_ROOT)/scene-ideas/t.cc \
	$(SRC_ROOT)/scene-ideas/table.cc \
	$(SRC_ROOT)/scene-terrain/base-renderer.cpp \
	$(SRC_ROOT)/scene-terrain/blur-renderer.cpp \
	$(SRC_ROOT)/scene-terrain/copy-renderer.cpp \
	$(SRC_ROOT)/scene-terrain/luminance-renderer.cpp \
	$(SRC_ROOT)/scene-terrain/normal-from-height-renderer.cpp \
	$(SRC_ROOT)/scene-terrain/overlay-renderer.cpp \
	$(SRC_ROOT)/scene-terrain/renderer-chain.cpp \
	$(SRC_ROOT)/scene-terrain/simplex-noise-renderer.cpp \
	$(SRC_ROOT)/scene-terrain/terrain-renderer.cpp \
	$(SRC_ROOT)/scene-terrain/texture-renderer.cpp

include $(MKFILES_ROOT)/qtargets.mk

LIBS += EGL GLESv2 screen m jpeg png

POST_INSTALL+= $(CP_HOST) -r $(PROJECT_ROOT)/../data/* $(INSTALL_ROOT_nto)/usr/share/glmark2/data/
