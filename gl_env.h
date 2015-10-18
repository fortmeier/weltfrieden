#ifndef _WF_GL_ENV_H
#define _WF_GL_ENV_H

#define GLFW_INCLUDE_GLEXT
#define GLFW_INCLUDE_GLCOREARB

#if defined(__linux)
#include <epoxy/gl.h>
#include <epoxy/glx.h>
#endif

#include <GLFW/glfw3.h>
#endif
