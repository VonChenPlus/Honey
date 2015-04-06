#include "GLDebug.h"
#include "GFX/GLCommon.h"
#include "BASE/BasicTypes.h"
#include "UTILS/STRING/String.h"
using UTILS::STRING::StringFromFormat;

namespace GFX
{
    void glCheckzor(const char *file, int line) {
        UNUSED(file); UNUSED(line);
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            throw _NException_(StringFromFormat("GL error on line %i in %s: %i (%04x)", line, file, (int)err, (int)err), NException::GFX);
        }
    }
}
