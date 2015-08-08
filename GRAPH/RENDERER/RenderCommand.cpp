#include "GRAPH/RENDERER/RenderCommand.h"
#include "GRAPH/BASE/Node.h"
#include "GRAPH/BASE/Camera.h"
#include "GRAPH/BASE/Director.h"
#include "GRAPH/RENDERER/Renderer.h"
#include "GRAPH/RENDERER/GLProgram.h"
#include "GRAPH/RENDERER/GLStateCache.h"
#include "UTILS/HASH/Hash.h"

namespace GRAPH
{
    RenderCommand::RenderCommand()
        : _type(RenderCommand::Type::UNKNOWN_COMMAND)
        , _globalOrder(0)
        , _isTransparent(true)
        , _skipBatching(false)
        , _is3D(false)
        , _depth(0) {
    }

    RenderCommand::~RenderCommand() {
    }

    void RenderCommand::init(float globalZOrder, const MATH::Matrix4 &transform, uint32_t flags) {
        _globalOrder = globalZOrder;
        if (flags & Node::FLAGS_RENDER_AS_3D) {
            if (Camera::getVisitingCamera())
                _depth = Camera::getVisitingCamera()->getDepthInView(transform);

            set3D(true);
        }
        else {
            set3D(false);
            _depth = 0;
        }
    }

    GroupCommandManager::GroupCommandManager() {

    }

    GroupCommandManager::~GroupCommandManager(){

    }

    bool GroupCommandManager::init() {
        //0 is the default render group
        _groupMapping[0] = true;
        return true;
    }

    int GroupCommandManager::getGroupID() {
        //Reuse old id
        if (!_unusedIDs.empty()) {
            int groupID = *_unusedIDs.rbegin();
            _unusedIDs.pop_back();
            _groupMapping[groupID] = true;
            return groupID;
        }

        //Create new ID
        // int newID = _groupMapping.size();
        int newID = Director::getInstance()->getRenderer()->createRenderQueue();
        _groupMapping[newID] = true;

        return newID;
    }

    void GroupCommandManager::releaseGroupID(int groupID) {
        _groupMapping[groupID] = false;
        _unusedIDs.push_back(groupID);
    }

    GroupCommand::GroupCommand() {
        _type = RenderCommand::Type::GROUP_COMMAND;
        _renderQueueID = Director::getInstance()->getRenderer()->getGroupCommandManager()->getGroupID();
    }

    void GroupCommand::init(float globalOrder) {
        _globalOrder = globalOrder;
        auto manager = Director::getInstance()->getRenderer()->getGroupCommandManager();
        manager->releaseGroupID(_renderQueueID);
        _renderQueueID = manager->getGroupID();
    }

    GroupCommand::~GroupCommand() {
        Director::getInstance()->getRenderer()->getGroupCommandManager()->releaseGroupID(_renderQueueID);
    }

    QuadCommand::QuadCommand()
        :_materialID(0)
        ,_textureID(0)
        ,_glProgramState(nullptr)
        ,_blendType(BlendFunc::DISABLE)
        ,_quads(nullptr)
        ,_quadsCount(0)
    {
        _type = RenderCommand::Type::QUAD_COMMAND;
    }

    void QuadCommand::init(float globalOrder, GLuint textureID, GLProgramState* shader, const BlendFunc& blendType, V3F_C4B_T2F_Quad* quads, ssize_t quadCount,
                           const MATH::Matrix4& mv, uint32_t flags)
    {
        RenderCommand::init(globalOrder, mv, flags);

        _quadsCount = quadCount;
        _quads = quads;

        _mv = mv;

        if( _textureID != textureID || _blendType.src != blendType.src || _blendType.dst != blendType.dst || _glProgramState != shader) {

            _textureID = textureID;
            _blendType = blendType;
            _glProgramState = shader;

            generateMaterialID();
        }
    }

    QuadCommand::~QuadCommand()
    {
    }

    void QuadCommand::generateMaterialID()
    {
        _skipBatching = false;

        if(_glProgramState->getUniformCount() == 0)
        {
            int glProgram = (int)_glProgramState->getGLProgram()->getProgram();
            int intArray[4] = { glProgram, (int)_textureID, (int)_blendType.src, (int)_blendType.dst};

            _materialID = XXH32((const void*)intArray, sizeof(intArray), 0);
        }
        else
        {
            _materialID = Renderer::MATERIAL_ID_DO_NOT_BATCH;
            _skipBatching = true;
        }
    }

    void QuadCommand::useMaterial() const
    {
        //Set texture
        bindTexture2D(_textureID);

        //set blend mode
        blendFunc(_blendType.src, _blendType.dst);

        _glProgramState->applyGLProgram(_mv);
        _glProgramState->applyUniforms();
    }

    TrianglesCommand::TrianglesCommand()
        :_materialID(0)
        ,_textureID(0)
        ,_glProgramState(nullptr)
        ,_blendType(BlendFunc::DISABLE)
    {
        _type = RenderCommand::Type::TRIANGLES_COMMAND;
    }

    void TrianglesCommand::init(float globalOrder, GLuint textureID, GLProgramState* glProgramState, BlendFunc blendType, const Triangles& triangles,const MATH::Matrix4& mv, uint32_t flags)
    {
        RenderCommand::init(globalOrder, mv, flags);

        _triangles = triangles;
        if(_triangles.indexCount % 3 != 0)
        {
            ssize_t count = _triangles.indexCount;
            _triangles.indexCount = count / 3 * 3;
        }
        _mv = mv;

        if( _textureID != textureID || _blendType.src != blendType.src || _blendType.dst != blendType.dst || _glProgramState != glProgramState) {

            _textureID = textureID;
            _blendType = blendType;
            _glProgramState = glProgramState;

            generateMaterialID();
        }
    }

    TrianglesCommand::~TrianglesCommand()
    {
    }

    void TrianglesCommand::generateMaterialID()
    {

        if(_glProgramState->getUniformCount() > 0)
        {
            _materialID = Renderer::MATERIAL_ID_DO_NOT_BATCH;
        }
        else
        {
            int glProgram = (int)_glProgramState->getGLProgram()->getProgram();
            int intArray[4] = { glProgram, (int)_textureID, (int)_blendType.src, (int)_blendType.dst};

            _materialID = UTILS::HASH::Fletcher((const void*)intArray, sizeof(intArray));
        }
    }

    void TrianglesCommand::useMaterial() const
    {
        //Set texture
        bindTexture2D(_textureID);

        //set blend mode
        blendFunc(_blendType.src, _blendType.dst);

        _glProgramState->apply(_mv);
    }

}
