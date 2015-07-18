#include "GLDebug.h"
#include "GRAPH/GFX/GLCommon.h"
#include "BASE/Honey.h"
#include "UTILS/STRING/StringUtils.h"
using UTILS::STRING::StringFromFormat;

namespace GFX
{
    void glCheckzor(const char *file, int line) {
        UNUSED(file); UNUSED(line);
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            throw _HException_(StringFromFormat("GL error on line %i in %s: %i (%04x)", line, file, (int)err, (int)err), HException::GFX);
        }
    }
}
