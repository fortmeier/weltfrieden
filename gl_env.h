#ifndef _WF_GL_ENV_H
#define _WF_GL_ENV_H


#if defined(EGL_RPI2)

#include <bcm_host.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#else

#define GLFW_INCLUDE_GLEXT
#define GLFW_INCLUDE_GLCOREARB

#if defined(__linux)
#include <epoxy/gl.h>
#include <epoxy/glx.h>
#endif

#include <GLFW/glfw3.h>

#endif


#endif
