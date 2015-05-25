#ifndef GLDEBUG_H
#define GLDEBUG_H

namespace GFX
{
    void gl_log_enable();

    #if defined(DEBUG_OPENGL)
    void glCheckzor(const char *file, int line);
    #define GL_CHECK() glCheckzor(__FILE__, __LINE__)
    #else
    #define GL_CHECK()
    #endif
}

#endif // GLDEBUG_H
