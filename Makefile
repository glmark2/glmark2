INSTALL = install -D
INSTALL_DATA = $(INSTALL) -m644

PKG_DEPS = sdl gl glu
PKG_LIBS = $(shell pkg-config --libs $(PKG_DEPS))
PKG_CFLAGS = $(shell pkg-config --cflags $(PKG_DEPS))

ALL_CFLAGS = -Wall -g -O2 $(CFLAGS) $(PKG_CFLAGS) 
ALL_DEFINES = -DGLMARK_DATA_PATH=\"$(GLMARK_DATA_PATH)\" $(DEFINES)

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

.cpp.o:
	$(CXX) $(ALL_CFLAGS) $(ALL_DEFINES) -c $< -o $@

glmark: $(OBJS)
	$(CXX) -o glmark $(LDFLAGS) $(PKG_LIBS) $(OBJS)

install: glmark
	$(INSTALL) glmark $(DESTDIR)/usr/bin/glmark

clean:
	-rm glmark $(OBJS)
