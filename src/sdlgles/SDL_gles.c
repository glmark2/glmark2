/* This file is part of SDL_gles - SDL addon for OpenGL|ES
 * Copyright (C) 2010 Javier S. Pedro
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA or see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <assert.h>

#include <EGL/egl.h>

#include <SDL.h>
#include <SDL_syswm.h>

#include "SDL_gles.h"

typedef struct SDL_GLES_ContextPriv
{
	SDL_GLES_Context p;

	EGLConfig egl_config;
	EGLContext egl_context;
} SDL_GLES_ContextPriv;

static const char * default_libgl[] = {
	[SDL_GLES_VERSION_1_1] = "/usr/lib/libGLES_CM.so",
	[SDL_GLES_VERSION_2_0] = "/usr/lib/libGLESv2.so"
};

/** SDL GFX display */
static Display *display = NULL;
/** EGLDisplay for the above X11 display */
static EGLDisplay *egl_display = EGL_NO_DISPLAY;
/** The current surface. Recreated by SDL_GLES_SetVideoMode(). */
static EGLSurface egl_surface = EGL_NO_SURFACE;
/** A pointer to the current active context. */
static SDL_GLES_ContextPriv *cur_context = NULL;

/** The desired GLES version, as selected by the SDL_GLES_Init() call. */
static SDL_GLES_Version gl_version = SDL_GLES_VERSION_NONE;
/** A handle to the dynamically loaded GL library. */
static void* gl_handle = NULL;
/** EGL version. */
static EGLint egl_major, egl_minor;

/** Your average countof() macro. */
#define countof(a) (sizeof(a)/sizeof(a[0]))

/** List of EGLConfig attributes we care about;
  * Used for filtering; modified by SDL_GLES_Get/SetAttribute(). */
static EGLint attrib_list[] = {
#define A(number, attrib, default_value) \
	attrib, default_value,
#include "attribs.inc"
#undef A
	EGL_NONE
};
/** A enum which maps A_EGL_* attrib constants to attrib_list positions. */
typedef enum {
#define A(number, attrib, default_value) \
	A_ ## attrib = (number * 2),
#include "attribs.inc"
#undef A
} attrib_enum;
static EGLint context_attrib_list[] = {
	EGL_CONTEXT_CLIENT_VERSION,	1,
	EGL_NONE
};

static const char * get_error_string(int error) {
	switch (error) {
		case EGL_SUCCESS:
			return "EGL_SUCCESS";
		case EGL_NOT_INITIALIZED:
			return "EGL_NOT_INITIALIZED";
		case EGL_BAD_ACCESS:
			return "EGL_BAD_ACCESS";
		case EGL_BAD_ALLOC:
			return "EGL_BAD_ALLOC";
		case EGL_BAD_ATTRIBUTE:
			return "EGL_BAD_ATTRIBUTE";
		case EGL_BAD_CONFIG:
			return "EGL_BAD_CONFIG";
		case EGL_BAD_CONTEXT:
			return "EGL_BAD_CONTEXT";
		case EGL_BAD_CURRENT_SURFACE:
			return "EGL_BAD_CURRENT_SURFACE";
		case EGL_BAD_DISPLAY:
			return "EGL_BAD_DISPLAY";
		case EGL_BAD_MATCH:
			return "EGL_BAD_MATCH";
		case EGL_BAD_NATIVE_PIXMAP:
			return "EGL_BAD_NATIVE_PIXMAP";
		case EGL_BAD_NATIVE_WINDOW:
			return "EGL_BAD_NATIVE_WINDOW";
		case EGL_BAD_PARAMETER:
			return "EGL_BAD_PARAMETER";
		case EGL_BAD_SURFACE:
			return "EGL_BAD_SURFACE";
		case EGL_CONTEXT_LOST:
			return "EGL_CONTEXT_LOST";
		default:
			return "EGL_UNKNOWN_ERROR";
    }
}

static inline void set_egl_attrib(attrib_enum attrib, EGLint value)
{
	const unsigned int i = (unsigned int)attrib + 1;
	assert(i < countof(attrib_list));
	attrib_list[i] = value;
}

static inline EGLint get_egl_attrib(attrib_enum attrib)
{
	const unsigned int i = (unsigned int)attrib + 1;
	assert(i < countof(attrib_list));
	return attrib_list[i];
}

static inline void set_egl_context_attrib(EGLenum attrib, EGLint value)
{
	/* Only one attribute supported here. */
	assert(attrib == EGL_CONTEXT_CLIENT_VERSION);
	context_attrib_list[1] = value;
}

int SDL_GLES_LoadLibrary(const char *path)
{
	/* If path is NULL, try first to use path from SDL_VIDEO_GL_DRIVER,
	 * otherwise use a sane default depending on selected GLES version. */
	if (!path) {
		path = getenv("SDL_VIDEO_GL_DRIVER");
		if (!path) {
			switch (gl_version) {
				case SDL_GLES_VERSION_1_1:
				case SDL_GLES_VERSION_2_0:
					path = default_libgl[gl_version];
				break;
				default:
					SDL_SetError("No GL version specific and SDL_VIDEO_GL_DRIVER set");
					return -1;
			}
		}
	}

	/* Dynamically load the desired GL library */
	gl_handle = dlopen(path, RTLD_LAZY|RTLD_GLOBAL);
	if (!gl_handle) {
		SDL_SetError("Failed to open GL library: %s (%s)", path, dlerror());
		return -2;
	}

	return 0;
}

void* SDL_GLES_GetProcAddress(const char *proc)
{
	if (!gl_handle) return NULL;
	return dlsym(gl_handle, proc);
}

int SDL_GLES_Init(SDL_GLES_Version version)
{
	SDL_SysWMinfo info;
	EGLBoolean res;

	SDL_VERSION(&info.version);
	if (SDL_GetWMInfo(&info) != 1) {
		SDL_SetError("SDL_gles is incompatible with this SDL version");
		return -1;
	}

	/* We use the SDL GFX display (we're using the GFX window too after all) */
	display = info.info.x11.gfxdisplay;

	egl_display = eglGetDisplay((EGLNativeDisplayType)display);
	if (egl_display == EGL_NO_DISPLAY) {
		SDL_SetError("EGL found no available displays");
		return -2;
	}

	res = eglInitialize(egl_display, &egl_major, &egl_minor);
	if (!res) {
		SDL_SetError("EGL failed to initialize: %s",
			get_error_string(eglGetError()));
		return -2;
	}

	/* Configure some context attributes and bind the required API now. */
	EGLenum api_to_bind = EGL_OPENGL_ES_API;
	gl_version = version;
	switch (gl_version) {
		case SDL_GLES_VERSION_1_1:
			/* OpenGL|ES 1.1 */
			api_to_bind = EGL_OPENGL_ES_API;
			/* filter non ES 1.0 renderable configurations */
			set_egl_attrib(A_EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT);
			/* default egl_context_client_version is OK */
			break;
		case SDL_GLES_VERSION_2_0:
			/* OpenGL|ES 2.0 */
			api_to_bind = EGL_OPENGL_ES_API; /* Note: no EGL_OPENGL_ES2_API */
			/* filter non ES 2.0 renderable configurations */
			set_egl_attrib(A_EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT);
			/* and request GL ES 2.0 contexts */
			set_egl_context_attrib(EGL_CONTEXT_CLIENT_VERSION, 2);
			break;
		default:
			SDL_SetError("Unsupported API version");
			return -1;
	}

	res = eglBindAPI(api_to_bind);
	if (!res) {
		SDL_SetError("EGL failed to bind the required API");
		return -2;
	}

	return 0;
}

void SDL_GLES_Quit()
{
	/* Close the loaded GL library (if any) */
	if (gl_handle) {
		dlclose(gl_handle);
		gl_handle = NULL;
	}
	/* Unallocate most stuff we can unallocate. */
	if (egl_display != EGL_NO_DISPLAY) {
		eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
			EGL_NO_CONTEXT);

		if (cur_context) {
			eglDestroyContext(egl_display, cur_context->egl_context);
			free(cur_context);
			cur_context = 0;
		}
		if (egl_surface != EGL_NO_SURFACE) {
			eglDestroySurface(egl_display, egl_surface);
			egl_surface = EGL_NO_SURFACE;
		}

		eglTerminate(egl_display);
		egl_display = EGL_NO_DISPLAY;
	}
}

int SDL_GLES_SetVideoMode()
{
	SDL_SysWMinfo info;
	EGLBoolean res;

	SDL_VERSION(&info.version);
	if (SDL_GetWMInfo(&info) != 1) {
		SDL_SetError("SDL_gles is incompatible with this SDL version");
		return -1;
	}

	/* Destroy previous surface, if any. */
	if (egl_surface != EGL_NO_SURFACE) {
		/* Ensure the surface is not the current one,
		 * thus freeing memory earlier. */
		eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
			 EGL_NO_CONTEXT);
		eglDestroySurface(egl_display, egl_surface);
		egl_surface = EGL_NO_SURFACE;
	}

	/* No current context? Quietly defer surface creation.
	 * Surface will be created on the call to MakeCurrent. */
	if (!cur_context) {
		return 0;
	}

	/* Create the new window surface. */
	egl_surface = eglCreateWindowSurface(egl_display, cur_context->egl_config,
		(EGLNativeWindowType)info.info.x11.window, NULL);
	if (egl_surface == EGL_NO_SURFACE) {
		SDL_SetError("EGL failed to create a window surface: %s",
			get_error_string(eglGetError()));
		return -2;
	}

	/* New surface created. Make it active. */
	assert(cur_context && cur_context->egl_context != EGL_NO_CONTEXT);
	res = eglMakeCurrent(egl_display, egl_surface, egl_surface,
		cur_context->egl_context);

	if (!res) {
		SDL_SetError("EGL failed to change current surface: %s",
			get_error_string(eglGetError()));
		cur_context = NULL;
		return -2;
	}

	return 0;
}

SDL_GLES_Context* SDL_GLES_CreateContext(void)
{
	SDL_GLES_ContextPriv *context = malloc(sizeof(SDL_GLES_ContextPriv));
	if (!context) {
		SDL_Error(SDL_ENOMEM);
		return NULL;
	}

	EGLBoolean res;
	EGLConfig configs[1];
	EGLint num_config;

	res = eglChooseConfig(egl_display, attrib_list, configs, 1, &num_config);
	if (!res || num_config < 1) {
		SDL_SetError("EGL failed to find any valid config with required attributes: %s",
			get_error_string(eglGetError()));
		free(context);
		return NULL;
	}

	context->egl_config = configs[0];
	context->egl_context = eglCreateContext(egl_display, configs[0],
		EGL_NO_CONTEXT, context_attrib_list);
	if (context->egl_context == EGL_NO_CONTEXT) {
		SDL_SetError("EGL failed to create context: %s",
			get_error_string(eglGetError()));
		free(context);
		return NULL;
	}

	return (SDL_GLES_Context*) context;
}

void SDL_GLES_DeleteContext(SDL_GLES_Context* c)
{
	SDL_GLES_ContextPriv *context = (SDL_GLES_ContextPriv*)c;
	if (!context) return;

	if (cur_context == context) {
		/* Deleting the active context */
		SDL_GLES_MakeCurrent(NULL);
	}

	eglDestroyContext(egl_display, context->egl_context);
	free(context);
}

int SDL_GLES_MakeCurrent(SDL_GLES_Context* c)
{
	SDL_GLES_ContextPriv *context = (SDL_GLES_ContextPriv*)c;
	int res;

	cur_context = context;

	/* SDL_GLES_SetVideoMode() will appropiately clear the current context
	 * (and surface), then create a new surface matching the selected context
	 * config and make both the surface and the context the active ones. */
	res = SDL_GLES_SetVideoMode();
	if (res != 0) return res; /* Surface (re-)creation failed. */

	return 0;
}

void SDL_GLES_SwapBuffers()
{
	eglSwapBuffers(egl_display, egl_surface);
}

/** A simple map between SDL_GLES_* attributes and EGL ones.
  * More abstraction layers is always good.
  */
static const attrib_enum attrib_map[] = {
	[SDL_GLES_BUFFER_SIZE]		= A_EGL_BUFFER_SIZE,
	[SDL_GLES_RED_SIZE]			= A_EGL_RED_SIZE,
	[SDL_GLES_GREEN_SIZE]		= A_EGL_GREEN_SIZE,
	[SDL_GLES_BLUE_SIZE]		= A_EGL_BLUE_SIZE,
	[SDL_GLES_ALPHA_SIZE]		= A_EGL_ALPHA_SIZE,
	[SDL_GLES_LUMINANCE_SIZE]	= A_EGL_LUMINANCE_SIZE,
	[SDL_GLES_DEPTH_SIZE]		= A_EGL_DEPTH_SIZE,
	[SDL_GLES_STENCIL_SIZE]		= A_EGL_STENCIL_SIZE,
};

int SDL_GLES_SetAttribute(SDL_GLES_Attr attr, int value)
{
	if (attr >= countof(attrib_map)) return -1;
	attrib_enum list_attr = attrib_map[attr];
	set_egl_attrib(list_attr, value);
	return 0;
}

int SDL_GLES_GetAttribute(SDL_GLES_Attr attr, int *value)
{
	if (attr >= countof(attrib_map)) return -1;
	attrib_enum list_attr = attrib_map[attr];
	if (cur_context) {
		EGLenum egl_attr = attrib_list[list_attr];
		EGLint egl_value = 0;
		EGLBoolean res = eglGetConfigAttrib(egl_display,
			cur_context->egl_config, egl_attr, &egl_value);
		if (res) {
			*value = egl_value;
			return 0;
		} else {
			printf("Failed: %s\n", get_error_string(eglGetError()));
			return -1;
		}
	} else {
		*value = get_egl_attrib(list_attr);
		return 0;
	}
}

