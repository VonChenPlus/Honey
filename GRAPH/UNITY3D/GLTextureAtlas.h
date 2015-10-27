#ifndef GLTEXTUREATLAS_H
#define GLTEXTUREATLAS_H

#include "BASE/HObject.h"
#include "GRAPH/UNITY3D/Unity3DGL.h"

namespace GRAPH
{
    class GLTexture;

    class GLTextureAtlas : public HObject
    {
    public:
        static GLTextureAtlas* create(const std::string& file , uint64 capacity);
        static GLTextureAtlas* createWithTexture(GLTexture *texture, uint64 capacity);

        GLTextureAtlas();
        virtual ~GLTextureAtlas();

        bool initWithFile(const std::string& file, uint64 capacity);
        bool initWithTexture(GLTexture *texture, uint64 capacity);

        void updateQuad(V3F_C4B_T2F_Quad* quad, uint64 index);
        void insertQuad(V3F_C4B_T2F_Quad* quad, uint64 index);
        void insertQuads(V3F_C4B_T2F_Quad* quads, uint64 index, uint64 amount);
        void insertQuadFromIndex(uint64 fromIndex, uint64 newIndex);

        void removeQuadAtIndex(uint64 index);
        void removeQuadsAtIndex(uint64 index, uint64 amount);
        void removeAllQuads();

        bool resizeCapacity(uint64 capacity);

        void increaseTotalQuadsWith(uint64 amount);
        void moveQuadsFromIndex(uint64 oldIndex, uint64 amount, uint64 newIndex);
        void moveQuadsFromIndex(uint64 index, uint64 newIndex);
        void fillWithEmptyQuadsFromIndex(uint64 index, uint64 amount);

        void drawNumberOfQuads(uint64 n);
        void drawNumberOfQuads(uint64 numberOfQuads, uint64 start);
        void drawQuads();

        inline bool isDirty(void) { return dirty_; }
        inline void setDirty(bool bDirty) { dirty_ = bDirty; }

        uint64 getTotalQuads() const;
        uint64 getCapacity() const;

        GLTexture* getTexture() const;
        void setTexture(GLTexture* texture);

        V3F_C4B_T2F_Quad* getQuads();
        void setQuads(V3F_C4B_T2F_Quad* quads);

    private:
        void renderCommand();
        void mapBuffers();
        void setupIndices();
        void setupVBO();

    protected:
        VertexBufferObject<V3F_C4B_T2F_Quad> vbo_;
        Unity3DGLBuffer *glBuffer_;
        Unity3DGLVertexFormat *glVertexFormat_;
        bool    dirty_;
        GLTexture* texture_;
    };
}

#endif // GLTEXTUREATLAS_H
