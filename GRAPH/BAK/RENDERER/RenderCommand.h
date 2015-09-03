#ifndef RENDERCOMMAND_H
#define RENDERCOMMAND_H

#include <stdint.h>
#include <vector>
#include <unordered_map>
#include "BASE/HObject.h"
#include "GRAPH/BASE/Types.h"
#include "MATH/Matrix.h"
#include "GRAPH/RENDERER/GLProgramState.h"
#include "GRAPH/RENDERER/TextureAtlas.h"

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
        inline float getGlobalOrder() const { return _globalOrder; }
        inline Type getType() const { return _type; }

        inline bool isTransparent() const { return _isTransparent; }
        inline void setTransparent(bool isTransparent) { _isTransparent = isTransparent; }

        inline bool isSkipBatching() const { return _skipBatching; }
        inline void setSkipBatching(bool value) { _skipBatching = value; }

        inline bool is3D() const { return _is3D; }
        inline void set3D(bool value) { _is3D = value; }

        inline float getDepth() const { return _depth; }

    protected:
        RenderCommand();
        virtual ~RenderCommand();

        Type _type;
        float _globalOrder;
        bool  _isTransparent;
        bool _skipBatching;
        bool _is3D;
        float _depth;
    };

    class GroupCommandManager : public HObject
    {
    public:
        int getGroupID();
        void releaseGroupID(int groupID);

    protected:
        friend class Renderer;
        GroupCommandManager();
        ~GroupCommandManager();
        bool init();
        std::unordered_map<int, bool> _groupMapping;
        std::vector<int> _unusedIDs;
    };

    class GroupCommand : public RenderCommand
    {
    public:
        GroupCommand();
        ~GroupCommand();

        void init(float globalOrder);
        inline int getRenderQueueID() const {return _renderQueueID;}

    protected:
        int _renderQueueID;
    };

    class QuadCommand : public RenderCommand
    {
    public:
        /**Constructor.*/
        QuadCommand();
        /**Destructor.*/
        ~QuadCommand();

        void init(float globalOrder, GLuint textureID, GLProgramState* shader, const BlendFunc& blendType, V3F_C4B_T2F_Quad* quads, ssize_t quadCount,
                  const MATH::Matrix4& mv, uint32_t flags);

        /**Apply the texture, shaders, programs, blend functions to GPU pipeline.*/
        void useMaterial() const;
        /**Get the material id of command.*/
        inline uint32_t getMaterialID() const { return _materialID; }
        /**Get the openGL texture handle.*/
        inline GLuint getTextureID() const { return _textureID; }
        /**Get the pointer of the rendered quads.*/
        inline V3F_C4B_T2F_Quad* getQuads() const { return _quads; }
        /**Get the number of quads for rendering.*/
        inline ssize_t getQuadCount() const { return _quadsCount; }
        /**Get the glprogramstate.*/
        inline GLProgramState* getGLProgramState() const { return _glProgramState; }
        /**Get the blend function.*/
        inline BlendFunc getBlendType() const { return _blendType; }
        /**Get the model view matrix.*/
        inline const MATH::Matrix4& getModelView() const { return _mv; }

    protected:
        /**Generate the material ID by textureID, glProgramState, and blend function.*/
        void generateMaterialID();

        /**Generated material id.*/
        uint32_t _materialID;
        /**OpenGL handle for texture.*/
        GLuint _textureID;
        /**GLprogramstate for the commmand. encapsulate shaders and uniforms.*/
        GLProgramState* _glProgramState;
        /**Blend function when rendering the triangles.*/
        BlendFunc _blendType;
        /**The pointer to the rendered quads.*/
        V3F_C4B_T2F_Quad* _quads;
        /**The number of quads for rendering.*/
        ssize_t _quadsCount;
        /**Model view matrix when rendering the triangles.*/
        MATH::Matrix4 _mv;
    };

    class TrianglesCommand : public RenderCommand
    {
    public:
        /**The structure of Triangles. */
        struct Triangles
        {
            /**Vertex data pointer.*/
            V3F_C4B_T2F* verts;
            /**Index data pointer.*/
            unsigned short* indices;
            /**The number of vertices.*/
            ssize_t vertCount;
            /**The number of indices.*/
            ssize_t indexCount;
        };
        /**Construtor.*/
        TrianglesCommand();
        /**Destructor.*/
        ~TrianglesCommand();

        void init(float globalOrder, GLuint textureID, GLProgramState* glProgramState, BlendFunc blendType, const Triangles& triangles,const MATH::Matrix4& mv, uint32_t flags);
        /**Apply the texture, shaders, programs, blend functions to GPU pipeline.*/
        void useMaterial() const;
        /**Get the material id of command.*/
        inline uint32_t getMaterialID() const { return _materialID; }
        /**Get the openGL texture handle.*/
        inline GLuint getTextureID() const { return _textureID; }
        /**Get a const reference of triangles.*/
        inline const Triangles& getTriangles() const { return _triangles; }
        /**Get the vertex count in the triangles.*/
        inline ssize_t getVertexCount() const { return _triangles.vertCount; }
        /**Get the index count of the triangles.*/
        inline ssize_t getIndexCount() const { return _triangles.indexCount; }
        /**Get the vertex data pointer.*/
        inline const V3F_C4B_T2F* getVertices() const { return _triangles.verts; }
        /**Get the index data pointer.*/
        inline const unsigned short* getIndices() const { return _triangles.indices; }
        /**Get the glprogramstate.*/
        inline GLProgramState* getGLProgramState() const { return _glProgramState; }
        /**Get the blend function.*/
        inline BlendFunc getBlendType() const { return _blendType; }
        /**Get the model view matrix.*/
        inline const MATH::Matrix4& getModelView() const { return _mv; }

    protected:
        /**Generate the material ID by textureID, glProgramState, and blend function.*/
        void generateMaterialID();

        /**Generated material id.*/
        uint32_t _materialID;
        /**OpenGL handle for texture.*/
        GLuint _textureID;
        /**GLprogramstate for the commmand. encapsulate shaders and uniforms.*/
        GLProgramState* _glProgramState;
        /**Blend function when rendering the triangles.*/
        BlendFunc _blendType;
        /**Rendered triangles.*/
        Triangles _triangles;
        /**Model view matrix when rendering the triangles.*/
        MATH::Matrix4 _mv;
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

    protected:
    };

    class BatchCommand : public RenderCommand
    {
    public:
        BatchCommand();
        ~BatchCommand();

        void init(float globalZOrder, GLProgram* shader, BlendFunc blendType, TextureAtlas *textureAtlas, const MATH::Matrix4& modelViewTransform, uint32_t flags);
        void execute();

    protected:
        //TODO: This member variable is not used. It should be removed.
        int32_t _materialID;
        /**Texture ID used for texture atlas rendering.*/
        GLuint _textureID;
        /**Shaders used for rendering.*/
        GLProgram* _shader;
        /**Blend function for rendering.*/
        BlendFunc _blendType;
        /**Texture atlas for rendering.*/
        TextureAtlas *_textureAtlas;

        /**ModelView transform.*/
        MATH::Matrix4 _mv;
    };
}

#endif // RENDERCOMMAND_H
