#include "GRAPH/UNITY3D/RenderCommand.h"
#include "GRAPH/UNITY3D/Renderer.h"
#include "GRAPH/UNITY3D/GLProgram.h"
#include "GRAPH/UNITY3D/GLStateCache.h"
#include "GRAPH/UNITY3D/Texture2D.h"
#include "UTILS/HASH/HashUtils.h"

namespace GRAPH
{
    RenderCommand::RenderCommand()
        : commandType_(RenderCommand::Type::UNKNOWN_COMMAND)
        , globalOrder_(0)
        , isTransparent_(true)
        , skipBatching_(false)
        , depth_(0) {
    }

    RenderCommand::~RenderCommand() {
    }

    void RenderCommand::init(float globalZOrder, const MATH::Matrix4 &, uint32_t) {
        globalOrder_ = globalZOrder;
        depth_ = 0;
    }

    GroupCommandManager::GroupCommandManager(Renderer *renderer) {
        renderer_ = renderer;
    }

    GroupCommandManager::~GroupCommandManager(){
        renderer_->release();
    }

    bool GroupCommandManager::init() {
        //0 is the default render group
        groupMapping_[0] = true;
        return true;
    }

    int GroupCommandManager::getGroupID() {
        //Reuse old id
        if (!unusedIDs_.empty()) {
            int groupID = *unusedIDs_.rbegin();
            unusedIDs_.pop_back();
            groupMapping_[groupID] = true;
            return groupID;
        }

        //Create new ID
        int newID = renderer_->createRenderQueue();
        groupMapping_[newID] = true;

        return newID;
    }

    void GroupCommandManager::releaseGroupID(int groupID) {
        groupMapping_[groupID] = false;
        unusedIDs_.push_back(groupID);
    }

    GroupCommand::GroupCommand(Renderer *renderer) {
        renderer_ = renderer;
        commandType_ = RenderCommand::Type::GROUP_COMMAND;
        renderQueueID_ = renderer_->getGroupCommandManager()->getGroupID();
    }

    void GroupCommand::init(float globalOrder) {
        globalOrder_ = globalOrder;
        renderer_->getGroupCommandManager()->releaseGroupID(renderQueueID_);
        renderQueueID_ = renderer_->getGroupCommandManager()->getGroupID();
    }

    GroupCommand::~GroupCommand() {
        renderer_->getGroupCommandManager()->releaseGroupID(renderQueueID_);
    }

    QuadCommand::QuadCommand()
        :materialID_(0)
        ,textureID_(0)
        ,glProgramState_(nullptr)
        ,blendType_(BlendFunc::DISABLE)
        ,quads_(nullptr)
        ,quadsCount_(0) {
        commandType_ = RenderCommand::Type::QUAD_COMMAND;
    }

    void QuadCommand::init(float globalOrder, GLuint textureID, GLProgramState* shader, const BlendFunc& blendType, V3F_C4B_T2F_Quad* quads, int64 quadCount,
                           const MATH::Matrix4& mv, uint32_t flags) {
        RenderCommand::init(globalOrder, mv, flags);

        quadsCount_ = quadCount;
        quads_ = quads;

        matrix4_ = mv;

        if( textureID_ != textureID || blendType_.src != blendType.src || blendType_.dst != blendType.dst || glProgramState_ != shader) {
            textureID_ = textureID;
            blendType_ = blendType;
            glProgramState_ = shader;
            generateMaterialID();
        }
    }

    QuadCommand::~QuadCommand() {
    }

    void QuadCommand::generateMaterialID() {
        skipBatching_ = false;

        if(glProgramState_->getUniformCount() == 0) {
            int glProgram = (int)glProgramState_->getGLProgram()->getProgram();
            int intArray[4] = { glProgram, (int)textureID_, (int)blendType_.src, (int)blendType_.dst};

            materialID_ = UTILS::HASH::Fletcher((const uint8*)intArray, sizeof(intArray));
        }
        else {
            materialID_ = Renderer::MATERIAL_ID_DO_NOT_BATCH;
            skipBatching_ = true;
        }
    }

    void QuadCommand::useMaterial() const {
        //Set texture
        GLStateCache::BindTexture2D(textureID_);

        //set blend mode
        GLStateCache::BlendFunc(blendType_.src, blendType_.dst);

        glProgramState_->applyGLProgram(matrix4_);
        glProgramState_->applyUniforms();
    }

    TrianglesCommand::TrianglesCommand()
        :materialID_(0)
        ,textureID_(0)
        ,glProgramState_(nullptr)
        ,blendType_(BlendFunc::DISABLE) {
        commandType_ = RenderCommand::Type::TRIANGLES_COMMAND;
    }

    void TrianglesCommand::init(float globalOrder, GLuint textureID, GLProgramState* glProgramState, BlendFunc blendType, const Triangles& triangles,const MATH::Matrix4& mv, uint32_t flags) {
        RenderCommand::init(globalOrder, mv, flags);

        _triangles = triangles;
        if(_triangles.indexCount % 3 != 0) {
            int64 count = _triangles.indexCount;
            _triangles.indexCount = count / 3 * 3;
        }
        matrix4_ = mv;

        if( textureID_ != textureID || blendType_.src != blendType.src || blendType_.dst != blendType.dst || glProgramState_ != glProgramState) {
            textureID_ = textureID;
            blendType_ = blendType;
            glProgramState_ = glProgramState;
            generateMaterialID();
        }
    }

    TrianglesCommand::~TrianglesCommand() {
    }

    void TrianglesCommand::generateMaterialID() {
        if(glProgramState_->getUniformCount() > 0) {
            materialID_ = Renderer::MATERIAL_ID_DO_NOT_BATCH;
        }
        else {
            int glProgram = (int)glProgramState_->getGLProgram()->getProgram();
            int intArray[4] = { glProgram, (int)textureID_, (int)blendType_.src, (int)blendType_.dst};

            materialID_ = UTILS::HASH::Fletcher((const uint8*)intArray, sizeof(intArray));
        }
    }

    void TrianglesCommand::useMaterial() const {
        //Set texture
        GLStateCache::BindTexture2D(textureID_);
        //set blend mode
        GLStateCache::BlendFunc(blendType_.src, blendType_.dst);
        glProgramState_->apply(matrix4_);
    }

    CustomCommand::CustomCommand()
        : func(nullptr) {
        commandType_ = RenderCommand::Type::CUSTOM_COMMAND;
    }

    void CustomCommand::init(float depth, const MATH::Matrix4 &modelViewTransform, uint32_t flags) {
        RenderCommand::init(depth, modelViewTransform, flags);
    }

    void CustomCommand::init(float globalOrder) {
        globalOrder_ = globalOrder;
    }

    CustomCommand::~CustomCommand() {
    }

    void CustomCommand::execute() {
        if(func) {
            func();
        }
    }

    BatchCommand::BatchCommand()
        : textureID_(0)
        , blendType_(BlendFunc::DISABLE)
        , textureAtlas_(nullptr) {
        commandType_ = RenderCommand::Type::BATCH_COMMAND;
        shader_ = nullptr;
    }

    void BatchCommand::init(float globalOrder, GLProgram* shader, BlendFunc blendType, TextureAtlas *textureAtlas, const MATH::Matrix4& modelViewTransform, uint32_t flags) {
        RenderCommand::init(globalOrder, modelViewTransform, flags);
        textureID_ = textureAtlas->getTexture()->getName();
        blendType_ = blendType;
        shader_ = shader;
        textureAtlas_ = textureAtlas;
        matrix4_ = modelViewTransform;
    }

    BatchCommand::~BatchCommand() {
    }

    void BatchCommand::execute() {
        shader_->use();
        shader_->setUniformsForBuiltins(matrix4_);
        GLStateCache::BindTexture2D(textureID_);
        GLStateCache::BlendFunc(blendType_.src, blendType_.dst);

        // Draw
        textureAtlas_->drawQuads();
    }
}
