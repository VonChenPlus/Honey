#include "GRAPH/UNITY3D/GLStateCache.h"

namespace GRAPH
{
    static const int MAX_ATTRIBUTES = 16;
    static const int MAX_ACTIVE_TEXTURE = 16;

    namespace
    {
        static uint32_t s_attributeFlags = 0;  // 32 attributes max

        static GLuint    s_currentShaderProgram = -1;
        static GLuint    s_currentBoundTexture[MAX_ACTIVE_TEXTURE] =  {(GLuint)-1,(GLuint)-1,(GLuint)-1,(GLuint)-1, (GLuint)-1,(GLuint)-1,(GLuint)-1,(GLuint)-1, (GLuint)-1,(GLuint)-1,(GLuint)-1,(GLuint)-1, (GLuint)-1,(GLuint)-1,(GLuint)-1,(GLuint)-1, };
        static GLenum    s_blendingSource = -1;
        static GLenum    s_blendingDest = -1;
        static int       s_GLServerState = 0;
        static GLenum    s_activeTexture = -1;
    }

    void GLStateCache::Invalidate(void) {
        s_attributeFlags = 0;

        s_currentShaderProgram = -1;
        for( int i=0; i < MAX_ACTIVE_TEXTURE; i++ ) {
            s_currentBoundTexture[i] = -1;
        }

        s_blendingSource = -1;
        s_blendingDest = -1;
        s_GLServerState = 0;
    }

    void GLStateCache::BindTexture2D(GLuint textureId) {
        BindTexture2DN(0, textureId);
    }

    void GLStateCache::BindTexture2DN(GLuint textureUnit, GLuint textureId) {
        if (s_currentBoundTexture[textureUnit] != textureId) {
            s_currentBoundTexture[textureUnit] = textureId;
            ActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, textureId);
        }
    }

    void GLStateCache::BindTextureN(GLuint textureUnit, GLuint textureId, GLuint textureType/* = GL_TEXTURE_2D*/) {
        if (s_currentBoundTexture[textureUnit] != textureId) {
            s_currentBoundTexture[textureUnit] = textureId;
            ActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(textureType, textureId);
        }
    }


    void GLStateCache::DeleteTexture(GLuint textureId) {
        for (uint64 i = 0; i < MAX_ACTIVE_TEXTURE; ++i) {
            if (s_currentBoundTexture[i] == textureId)
            {
                s_currentBoundTexture[i] = -1;
            }
        }
        glDeleteTextures(1, &textureId);
    }

    void GLStateCache::ActiveTexture(GLenum texture) {
        if(s_activeTexture != texture) {
            s_activeTexture = texture;
            glActiveTexture(s_activeTexture);
        }
    }

    void GLStateCache::EnableVertexAttribs(uint32_t flags) {
        for(int i=0; i < MAX_ATTRIBUTES; i++) {
            unsigned int bit = 1 << i;
            //FIXME:Cache is disabled, try to enable cache as before
            bool enabled = (flags & bit) != 0;
            bool enabledBefore = (s_attributeFlags & bit) != 0;
            if(enabled != enabledBefore) {
                if( enabled )
                    glEnableVertexAttribArray(i);
                else
                    glDisableVertexAttribArray(i);
            }
        }
        s_attributeFlags = flags;
    }
}
