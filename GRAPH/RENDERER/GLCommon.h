#ifndef GLCOMMON_H
#define GLCOMMON_H

#ifdef IOS
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#elif defined(USING_GLES2)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "EGL/egl.h"
// At least Nokia platforms need the three below
#include <KHR/khrplatform.h>
typedef char GLchar;
#define GL_BGRA_EXT 0x80E1
#else // OpenGL
#include "EXTERNALS/glew/GL/glew.h"
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif

#endif // GLCOMMON_H

