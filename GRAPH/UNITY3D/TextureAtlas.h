#ifndef TEXTUREATLAS_H
#define TEXTUREATLAS_H

#include "GRAPH/Types.h"
#include "GRAPH/UNITY3D/Unity3D.h"

namespace GRAPH
{
    class Unity3DTexture;

    class TextureAtlas : public HObject
    {
    public:
        static TextureAtlas* create(const std::string& file , uint64 capacity);
        static TextureAtlas* createWithTexture(Unity3DTexture *texture, uint64 capacity);

        TextureAtlas();
        virtual ~TextureAtlas();

        bool initWithFile(const std::string& file, uint64 capacity);
        bool initWithTexture(Unity3DTexture *texture, uint64 capacity);

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

        Unity3DTexture* getTexture() const;
        void setTexture(Unity3DTexture* texture);

        V3F_C4B_T2F_Quad* getQuads();
        void setQuads(V3F_C4B_T2F_Quad* quads);

    private:
        void renderCommand();
        void setupIndices();
        void setupBuffer();

    protected:
        bool    dirty_;
        Unity3DTexture* texture_;
        VertexBufferObject<V3F_C4B_T2F_Quad> vbo_;
        Unity3DBuffer *u3dVertexBuffer_;
        Unity3DBuffer *u3dIndexBuffer_;
        Unity3DVertexFormat *u3dVertexFormat_;
        Unity3DContext *u3dContext_;
    };
}

#endif // TEXTUREATLAS_H
