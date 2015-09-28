#ifndef RENDERCOMMAND_H
#define RENDERCOMMAND_H

#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <functional>
#include "BASE/HObject.h"
#include "GRAPH/Types.h"
#include "MATH/Matrix.h"

namespace GRAPH
{
    class RenderCommand
    {
    public:
        enum class Type
        {
            /** Reserved type.*/
            UNKNOWN_COMMAND,
            /** Quad command, used for draw quad.*/
            QUAD_COMMAND,
            /**Custom command, used for calling callback for rendering.*/
            CUSTOM_COMMAND,
            /**Batch command, used for draw batches in texture atlas.*/
            BATCH_COMMAND,
            /**Group command, which can group command in a tree hierarchy.*/
            GROUP_COMMAND,
            /**Primitive command, used to draw primitives such as lines, points and triangles.*/
            PRIMITIVE_COMMAND,
            /**Triangles command, used to draw triangles.*/
            TRIANGLES_COMMAND
        };

        void init(float globalZOrder, const MATH::Matrix4& modelViewTransform, uint32_t flags);
        inline float getGlobalOrder() const { return globalOrder_; }
        inline Type getType() const { return commandType_; }

        inline bool isTransparent() const { return isTransparent_; }
        inline void setTransparent(bool isTransparent) { isTransparent_ = isTransparent; }

        inline bool isSkipBatching() const { return skipBatching_; }
        inline void setSkipBatching(bool value) { skipBatching_ = value; }

        inline float getDepth() const { return depth_; }

    protected:
        RenderCommand();
        virtual ~RenderCommand();

        Type commandType_;
        float globalOrder_;
        bool  isTransparent_;
        bool skipBatching_;
        float depth_;
    };

    class Renderer;
    class GroupCommandManager : public HObject
    {
        friend class Renderer;
    public:
        int getGroupID();
        void releaseGroupID(int groupID);

    protected:
        GroupCommandManager(Renderer *renderer);
        ~GroupCommandManager();
        bool init();

    private:
        std::unordered_map<int, bool> groupMapping_;
        std::vector<int> unusedIDs_;
        Renderer *renderer_;
    };

    class GroupCommand : public RenderCommand
    {
    public:
        GroupCommand(Renderer *renderer);
        ~GroupCommand();

        void init(float globalOrder);
        inline int getRenderQueueID() const {return renderQueueID_;}

    protected:
        int renderQueueID_;
        Renderer *renderer_;
    };

    class GLProgramState;

    class QuadCommand : public RenderCommand
    {
    public:
        QuadCommand();
        ~QuadCommand();

        void init(float globalOrder, GLuint textureID, GLProgramState* shader, const BlendFunc& blendType, V3F_C4B_T2F_Quad* quads, int64 quadCount,
                  const MATH::Matrix4& mv, uint32_t flags);

        void useMaterial() const;
        inline uint32_t getMaterialID() const { return materialID_; }
        inline GLuint getTextureID() const { return textureID_; }
        inline V3F_C4B_T2F_Quad* getQuads() const { return quads_; }
        inline int64 getQuadCount() const { return quadsCount_; }
        inline GLProgramState* getGLProgramState() const { return glProgramState_; }
        inline BlendFunc getBlendType() const { return blendType_; }
        inline const MATH::Matrix4& getModelView() const { return matrix4_; }

    protected:
        void generateMaterialID();

        uint32_t materialID_;
        GLuint textureID_;
        GLProgramState* glProgramState_;
        BlendFunc blendType_;
        V3F_C4B_T2F_Quad* quads_;
        int64 quadsCount_;
        MATH::Matrix4 matrix4_;
    };

    class TrianglesCommand : public RenderCommand
    {
    public:
        struct Triangles
        {
            V3F_C4B_T2F* verts;
            unsigned short* indices;
            int64 vertCount;
            int64 indexCount;
        };

        TrianglesCommand();
        ~TrianglesCommand();

        void init(float globalOrder, GLuint textureID, GLProgramState* glProgramState, BlendFunc blendType, const Triangles& triangles,const MATH::Matrix4& mv, uint32_t flags);
        void useMaterial() const;
        inline uint32_t getMaterialID() const { return materialID_; }
        inline GLuint getTextureID() const { return textureID_; }
        inline const Triangles& getTriangles() const { return _triangles; }
        inline int64 getVertexCount() const { return _triangles.vertCount; }
        inline int64 getIndexCount() const { return _triangles.indexCount; }
        inline const V3F_C4B_T2F* getVertices() const { return _triangles.verts; }
        inline const unsigned short* getIndices() const { return _triangles.indices; }
        inline GLProgramState* getGLProgramState() const { return glProgramState_; }
        inline BlendFunc getBlendType() const { return blendType_; }
        inline const MATH::Matrix4& getModelView() const { return matrix4_; }

    protected:
        void generateMaterialID();

        uint32_t materialID_;
        GLuint textureID_;
        GLProgramState* glProgramState_;
        BlendFunc blendType_;
        Triangles _triangles;
        MATH::Matrix4 matrix4_;
    };

    class CustomCommand : public RenderCommand
    {
    public:
        CustomCommand();
        ~CustomCommand();

    public:
        void init(float globalZOrder, const MATH::Matrix4& modelViewTransform, uint32_t flags);
        void init(float globalZOrder);

        void execute();
        std::function<void()> func;
    };

    class TextureAtlas;
    class GLProgram;

    class BatchCommand : public RenderCommand
    {
    public:
        BatchCommand();
        ~BatchCommand();

        void init(float globalZOrder, GLProgram* shader, BlendFunc blendType, TextureAtlas *textureAtlas, const MATH::Matrix4& modelViewTransform, uint32_t flags);
        void execute();

    protected:
        GLuint textureID_;
        GLProgram* shader_;
        BlendFunc blendType_;
        TextureAtlas *textureAtlas_;
        MATH::Matrix4 matrix4_;
    };
}

#endif // RENDERCOMMAND_H
