#ifndef _WF_GL_ENV_H
#define _WF_GL_ENV_H


#ifdef EGL_RPI2
#define GLFW_INCLUDE_ES2
#else
#define GLFW_INCLUDE_GLEXT
#define GLFW_INCLUDE_GLCOREARB
#endif

#if defined(__linux)
#if defined(EGL_RPI2)
#include <bcm_host.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#else
#include <epoxy/gl.h>
#include <epoxy/glx.h>
#endif
#endif

#include <GLFW/glfw3.h>
#endif
