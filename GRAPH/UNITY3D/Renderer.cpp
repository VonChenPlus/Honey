#include <algorithm>
#include "GRAPH/UNITY3D/Renderer.h"
#include "GRAPH/UNITY3D/RenderCommand.h"

namespace GRAPH
{
    static bool CompareRenderCommand(RenderCommand* a, RenderCommand* b) {
        return a->getGlobalOrder() < b->getGlobalOrder();
    }

    static bool Compare3DCommand(RenderCommand* a, RenderCommand* b) {
        return  a->getDepth() > b->getDepth();
    }

    RenderQueue::RenderQueue()
        : depthState_(Unity3DCreator::CreateDepthState()) {
    }

    void RenderQueue::push_back(RenderCommand* command) {
        float z = command->getGlobalOrder();
        if(z < 0) {
            commands_[QUEUE_GROUP::GLOBALZ_NEG].push_back(command);
        }
        else if(z > 0) {
            commands_[QUEUE_GROUP::GLOBALZ_POS].push_back(command);
        }
        else {
            commands_[QUEUE_GROUP::GLOBALZ_ZERO].push_back(command);
        }
    }

    uint64 RenderQueue::size() const {
        uint64 result(0);
        for(int index = 0; index < QUEUE_GROUP::QUEUE_COUNT; ++index) {
            result += commands_[index].size();
        }

        return result;
    }

    void RenderQueue::sort() {
        // Don't sort _queue0, it already comes sorted
        std::sort(std::begin(commands_[QUEUE_GROUP::TRANSPARENT_3D]), std::end(commands_[QUEUE_GROUP::TRANSPARENT_3D]), Compare3DCommand);
        std::sort(std::begin(commands_[QUEUE_GROUP::GLOBALZ_NEG]), std::end(commands_[QUEUE_GROUP::GLOBALZ_NEG]), CompareRenderCommand);
        std::sort(std::begin(commands_[QUEUE_GROUP::GLOBALZ_POS]), std::end(commands_[QUEUE_GROUP::GLOBALZ_POS]), CompareRenderCommand);
    }

    RenderCommand* RenderQueue::operator[](uint64 index) const {
        for(int queIndex = 0; queIndex < QUEUE_GROUP::QUEUE_COUNT; ++queIndex) {
            if(index < static_cast<uint64>(commands_[queIndex].size()))
                return commands_[queIndex][index];
            else {
                index -= commands_[queIndex].size();
            }
        }

        return nullptr;
    }

    void RenderQueue::clear() {
        for(int i = 0; i < QUEUE_COUNT; ++i) {
            commands_[i].clear();
        }
    }

    void RenderQueue::realloc(uint64 reserveSize) {
        for(int i = 0; i < QUEUE_COUNT; ++i) {
            commands_[i] = std::vector<RenderCommand*>();
            commands_[i].reserve(reserveSize);
        }
    }

    void RenderQueue::saveRenderState() {
        depthState_->loadDefault();
    }

    void RenderQueue::restoreRenderState() {
        depthState_->apply();
    }

    static const int DEFAULT_RENDER_QUEUE = 0;

    Renderer::Renderer()
        : lastMaterialID_(0)
        , glViewAssigned_(false)
        , isRendering_(false)
        , isDepthTestFor2D_(false)
        , u3dContext_(Unity3DCreator::CreateContext())
        , depthState_(Unity3DCreator::CreateDepthState()){
        groupCommandManager_ = new (std::nothrow) GroupCommandManager(this);
        commandGroupStack_.push(DEFAULT_RENDER_QUEUE);
        RenderQueue defaultRenderQueue;
        renderGroups_.push_back(defaultRenderQueue);
        batchedCommands_.reserve(BATCH_QUADCOMMAND_RESEVER_SIZE);

        memset(vboArray_, 0, sizeof(VertexBufferObject<V3F_C4B_T2F>) * 2);
        for (auto &object : vboArray_) {
            object.u2.bufferData = new V3F_C4B_T2F[VBO_SIZE];
            object.u2.bufferCapacity = VBO_SIZE;
            object.u2.indexData = new uint16[INDEX_VBO_SIZE];
            object.u2.indexCapacity = INDEX_VBO_SIZE;
        }

        clearColor_ = Color4F::BLACK;
    }

    Renderer::~Renderer() {
        renderGroups_.clear();
        groupCommandManager_->release();
        SAFE_DELETE_PTRARRAY(u3dVertexBuffer_, 2);
        SAFE_DELETE_PTRARRAY(u3dIndexBuffer_, 2);
        SAFE_DELETE_PTRARRAY(u3dVertexFormat_, 2);
        SAFE_RELEASE(u3dContext_);
        SAFE_RELEASE(depthState_);
    }

    void Renderer::initGLView() {
        for (int i = 0; i < VBO_SIZE / 4; i++) {
            vboArray_[QUADS].u2.indexData[i * 6 + 0] = (uint16) (i * 4 + 0);
            vboArray_[QUADS].u2.indexData[i * 6 + 1] = (uint16) (i * 4 + 1);
            vboArray_[QUADS].u2.indexData[i * 6 + 2] = (uint16) (i * 4 + 2);
            vboArray_[QUADS].u2.indexData[i * 6 + 3] = (uint16) (i * 4 + 3);
            vboArray_[QUADS].u2.indexData[i * 6 + 4] = (uint16) (i * 4 + 2);
            vboArray_[QUADS].u2.indexData[i * 6 + 5] = (uint16) (i * 4 + 1);
        }

        setupBuffer();
        glViewAssigned_ = true;
    }

    void Renderer::setupBuffer() {
        for (int index = 0; index < 2; ++index) {
            u3dVertexBuffer_[index] = Unity3DCreator::CreateBuffer(VERTEXDATA | DYNAMIC);
            u3dIndexBuffer_[index] = Unity3DCreator::CreateBuffer(INDEXDATA);

            u3dVertexBuffer_[index]->setData((const uint8 *) vboArray_[index].u2.bufferData, sizeof(V3F_C4B_T2F) * vboArray_[index].u2.bufferCapacity);
            u3dIndexBuffer_[index]->setData((const uint8 *) vboArray_[index].u2.indexData, sizeof(uint16) * vboArray_[index].u2.indexCapacity);

            std::vector<U3DVertexComponent> vertexFormat = {
                U3DVertexComponent(SEM_POSITION, FLOATx3, sizeof(V3F_C4B_T2F), offsetof(V3F_C4B_T2F, vertices)),
                U3DVertexComponent(SEM_COLOR0, UNORM8x4, sizeof(V3F_C4B_T2F), offsetof(V3F_C4B_T2F, colors)),
                U3DVertexComponent(SEM_TEXCOORD0, FLOATx2, sizeof(V3F_C4B_T2F), offsetof(V3F_C4B_T2F, texCoords)) };
            u3dVertexFormat_[index] = Unity3DCreator::CreateVertexFormat(vertexFormat);
        }
    }

    void Renderer::addCommand(RenderCommand* command) {
        int renderQueue =commandGroupStack_.top();
        addCommand(command, renderQueue);
    }

    void Renderer::addCommand(RenderCommand* command, int renderQueue) {
        renderGroups_[renderQueue].push_back(command);
    }

    void Renderer::pushGroup(int renderQueueID) {
        commandGroupStack_.push(renderQueueID);
    }

    void Renderer::popGroup() {
        commandGroupStack_.pop();
    }

    int Renderer::createRenderQueue() {
        RenderQueue newRenderQueue;
        renderGroups_.push_back(newRenderQueue);
        return (int)renderGroups_.size() - 1;
    }

    void Renderer::processRenderCommand(RenderCommand* command) {
        auto commandType = command->getType();
        if( RenderCommand::Type::TRIANGLES_COMMAND == commandType) {
            //Draw if we have batched other commands which are not triangle command
            flushQuads();

            //Process triangle command
            auto cmd = static_cast<TrianglesCommand*>(command);

            //Draw batched Triangles if necessary
            if (cmd->isSkipBatching() || vboArray_[TRIANGLES].u2.bufferCount + cmd->getVertexCount() > VBO_SIZE || vboArray_[TRIANGLES].u2.indexCount + cmd->getIndexCount() > INDEX_VBO_SIZE) {
                //Draw batched Triangles if VBO is full
                drawBatchedTriangles();
            }

            //Batch Triangles
            batchedCommands_.push_back(cmd);

            fillVerticesAndIndices(cmd);

            if(cmd->isSkipBatching()) {
                drawBatchedTriangles();
            }

        }
        else if ( RenderCommand::Type::QUAD_COMMAND == commandType ) {
            //Draw if we have batched other commands which are not quad command
            flushTriangles();

            //Process quad command
            auto cmd = static_cast<QuadCommand*>(command);

            //Draw batched quads if necessary
            if (cmd->isSkipBatching() || (vboArray_[QUADS].u2.bufferCount + cmd->getQuadCount()) * 4 > VBO_SIZE) {
                //Draw batched quads if VBO is full
                drawBatchedQuads();
            }

            //Batch Quads
            batchQuadCommands_.push_back(cmd);

            fillQuads(cmd);

            if(cmd->isSkipBatching()) {
                drawBatchedQuads();
            }
        }
        else if(RenderCommand::Type::GROUP_COMMAND == commandType) {
            flush();
            int renderQueueID = ((GroupCommand*) command)->getRenderQueueID();
            visitRenderQueue(renderGroups_[renderQueueID]);
        }
        else if(RenderCommand::Type::CUSTOM_COMMAND == commandType) {
            flush();
            auto cmd = static_cast<CustomCommand*>(command);
            cmd->execute();
        }
        else if(RenderCommand::Type::BATCH_COMMAND == commandType) {
            flush();
            auto cmd = static_cast<BatchCommand*>(command);
            cmd->execute();
        }
    }

    void Renderer::visitRenderQueue(RenderQueue& queue) {
        queue.saveRenderState();

        //
        //Process Global-Z < 0 Objects
        //
        const auto& zNegQueue = queue.getSubQueue(RenderQueue::QUEUE_GROUP::GLOBALZ_NEG);
        if (zNegQueue.size() > 0) {
            if(isDepthTestFor2D_) {
                depthState_->setDepthTest(true);
                depthState_->setDepthWrite(true);
                depthState_->apply();
            }
            else {
                depthState_->setDepthTest(false);
                depthState_->setDepthWrite(false);
                depthState_->apply();
            }
            for (auto it = zNegQueue.cbegin(); it != zNegQueue.cend(); ++it) {
                processRenderCommand(*it);
            }
            flush();
        }

        //
        //Process Opaque Object
        //
        const auto& opaqueQueue = queue.getSubQueue(RenderQueue::QUEUE_GROUP::OPAQUE_3D);
        if (opaqueQueue.size() > 0) {
            //Clear depth to achieve layered rendering
            depthState_->setDepthTest(true);
            depthState_->setDepthWrite(true);
            depthState_->apply();

            for (auto it = opaqueQueue.cbegin(); it != opaqueQueue.cend(); ++it) {
                processRenderCommand(*it);
            }
            flush();
        }

        //
        //Process 3D Transparent object
        //
        const auto& transQueue = queue.getSubQueue(RenderQueue::QUEUE_GROUP::TRANSPARENT_3D);
        if (transQueue.size() > 0) {
            depthState_->setDepthTest(true);
            depthState_->setDepthWrite(false);
            depthState_->apply();

            for (auto it = transQueue.cbegin(); it != transQueue.cend(); ++it) {
                processRenderCommand(*it);
            }
            flush();
        }

        //
        //Process Global-Z = 0 Queue
        //
        const auto& zZeroQueue = queue.getSubQueue(RenderQueue::QUEUE_GROUP::GLOBALZ_ZERO);
        if (zZeroQueue.size() > 0) {
            if(isDepthTestFor2D_) {
                depthState_->setDepthTest(true);
                depthState_->setDepthWrite(true);
                depthState_->apply();
            }
            else {
                depthState_->setDepthTest(false);
                depthState_->setDepthWrite(false);
                depthState_->apply();
            }
            for (auto it = zZeroQueue.cbegin(); it != zZeroQueue.cend(); ++it) {
                processRenderCommand(*it);
            }
            flush();
        }

        //
        //Process Global-Z > 0 Queue
        //
        const auto& zPosQueue = queue.getSubQueue(RenderQueue::QUEUE_GROUP::GLOBALZ_POS);
        if (zPosQueue.size() > 0) {
            for (auto it = zPosQueue.cbegin(); it != zPosQueue.cend(); ++it) {
                processRenderCommand(*it);
            }
            flush();
        }

        queue.restoreRenderState();
    }

    void Renderer::render() {
        isRendering_ = true;

        if (glViewAssigned_) {
            for (auto &renderqueue : renderGroups_) {
                renderqueue.sort();
            }
            visitRenderQueue(renderGroups_[0]);
        }
        
        for (uint64 j = 0; j < renderGroups_.size(); j++) {
            renderGroups_[j].clear();
        }

        batchedCommands_.clear();
        batchQuadCommands_.clear();
        lastMaterialID_ = 0;

        for (auto &object : vboArray_) {
            object.u2.bufferCount = 0;
            object.u2.indexCount = 0;
        }

        isRendering_ = false;
    }

    void Renderer::clear() {
        u3dContext_->clear(U3DClear::COLOR, clearColor_, 1.0, 0);
        depthState_->setDepthTest(false);
        depthState_->apply();
    }

    void Renderer::setDepthTest(bool enable) {
        if (enable) {
            u3dContext_->clear(U3DClear::DEPTH, 0, 1.0f, 0.0);
            depthState_->setDepthTest(true);
            depthState_->setDepthComp(LESS_EQUAL);
            depthState_->apply();
        }
        else {
            depthState_->setDepthTest(false);
            depthState_->apply();
        }

        isDepthTestFor2D_ = enable;
    }

    void Renderer::fillVerticesAndIndices(const TrianglesCommand* cmd) {
        memcpy(vboArray_[TRIANGLES].u2.bufferData + vboArray_[TRIANGLES].u2.bufferCount, cmd->getVertices(), sizeof(V3F_C4B_T2F) * cmd->getVertexCount());
        const MATH::Matrix4& modelView = cmd->getModelView();

        for(uint64 i=0; i< cmd->getVertexCount(); ++i) {
            V3F_C4B_T2F *q = &vboArray_[TRIANGLES].u2.bufferData[i + vboArray_[TRIANGLES].u2.bufferCount];
            MATH::Vector3f *vec1 = (MATH::Vector3f*)&q->vertices;
            modelView.transformPoint(vec1);
        }

        const unsigned short* indices = cmd->getIndices();
        //fill index
        for(uint64 i=0; i< cmd->getIndexCount(); ++i) {
            vboArray_[TRIANGLES].u2.indexData[vboArray_[TRIANGLES].u2.indexCount + i] = vboArray_[TRIANGLES].u2.bufferCount + indices[i];
        }

        vboArray_[TRIANGLES].u2.bufferCount += cmd->getVertexCount();
        vboArray_[TRIANGLES].u2.indexCount += cmd->getIndexCount();
    }

    void Renderer::fillQuads(const QuadCommand *cmd) {
        const MATH::Matrix4& modelView = cmd->getModelView();
        const V3F_C4B_T2F* quads =  (V3F_C4B_T2F*)cmd->getQuads();
        for(uint64 i=0; i< cmd->getQuadCount() * 4; ++i) {
            vboArray_[QUADS].u2.bufferData[i + vboArray_[QUADS].u2.bufferCount * 4] = quads[i];
            modelView.transformPoint(quads[i].vertices, &(vboArray_[QUADS].u2.bufferData[i + vboArray_[QUADS].u2.bufferCount * 4].vertices));
        }

        vboArray_[QUADS].u2.bufferCount += cmd->getQuadCount();
    }

    void Renderer::drawBatchedTriangles()
    {
        //TODO: we can improve the draw performance by insert material switching command before hand.
        int indexToDraw = 0;
        int startIndex = 0;

        //Upload buffer to VBO
        if (vboArray_[TRIANGLES].u2.bufferCount <= 0 || vboArray_[TRIANGLES].u2.indexCount <= 0 || batchedCommands_.empty()) {
            return;
        }

        u3dVertexBuffer_[TRIANGLES]->bind();
        u3dVertexBuffer_[TRIANGLES]->setData((const uint8 *) vboArray_[TRIANGLES].u2.bufferData, sizeof(V3F_C4B_T2F) * vboArray_[TRIANGLES].u2.bufferCount);

        u3dIndexBuffer_[TRIANGLES]->bind();
        u3dIndexBuffer_[TRIANGLES]->setData((const uint8 *) vboArray_[TRIANGLES].u2.indexData, sizeof(uint16) * vboArray_[TRIANGLES].u2.indexCount);

        // Start drawing verties in batch
        for(const auto& cmd : batchedCommands_) {
            auto newMaterialID = cmd->getMaterialID();
            if(lastMaterialID_ != newMaterialID || newMaterialID == MATERIAL_ID_DO_NOT_BATCH) {
                // Draw quads
                if(indexToDraw > 0) {
                    u3dContext_->drawIndexed(PRIM_TRIANGLES, u3dVertexFormat_[TRIANGLES], u3dVertexBuffer_[TRIANGLES], u3dIndexBuffer_[TRIANGLES], (void *) (startIndex*sizeof(uint16)), indexToDraw);
                    startIndex += indexToDraw;
                    indexToDraw = 0;
                }

                // Use new material
                cmd->useMaterial();
                lastMaterialID_ = newMaterialID;
            }

            indexToDraw += cmd->getIndexCount();
        }

        //Draw any remaining triangles
        if(indexToDraw > 0) {
            u3dContext_->drawIndexed(PRIM_TRIANGLES, u3dVertexFormat_[TRIANGLES], u3dVertexBuffer_[TRIANGLES], u3dIndexBuffer_[TRIANGLES], (void *) (startIndex*sizeof(uint16)), indexToDraw);
        }

        batchedCommands_.clear();
        vboArray_[TRIANGLES].u2.bufferCount = 0;
        vboArray_[TRIANGLES].u2.indexCount = 0;
    }

    void Renderer::drawBatchedQuads() {
        //TODO: we can improve the draw performance by insert material switching command before hand.
        uint64 indexToDraw = 0;
        int startIndex = 0;

        //Upload buffer to VBO
        if (vboArray_[QUADS].u2.bufferCount <= 0 || batchQuadCommands_.empty()) {
            return;
        }

        u3dVertexBuffer_[QUADS]->bind();
        u3dVertexBuffer_[QUADS]->setData((const uint8 *) vboArray_[QUADS].u2.bufferData, sizeof(V3F_C4B_T2F) * vboArray_[QUADS].u2.bufferCount * 4);

        //Start drawing vertices in batch
        for(const auto& cmd : batchQuadCommands_) {
            bool commandQueued = true;
            auto newMaterialID = cmd->getMaterialID();
            if(lastMaterialID_ != newMaterialID || newMaterialID == MATERIAL_ID_DO_NOT_BATCH) {
                // flush buffer
                if(indexToDraw > 0) {
                    u3dContext_->drawIndexed(PRIM_TRIANGLES, u3dVertexFormat_[QUADS], u3dVertexBuffer_[QUADS], u3dIndexBuffer_[QUADS], (void *) (startIndex*sizeof(uint16)), indexToDraw);
                    startIndex += indexToDraw;
                    indexToDraw = 0;
                }

                //Use new material
                lastMaterialID_ = newMaterialID;

                cmd->useMaterial();
            }

            if (commandQueued) {
                indexToDraw += cmd->getQuadCount() * 6;
            }
        }

        //Draw any remaining quad
        if(indexToDraw > 0) {
            u3dContext_->drawIndexed(PRIM_TRIANGLES, u3dVertexFormat_[QUADS], u3dVertexBuffer_[QUADS], u3dIndexBuffer_[QUADS], (void *) (startIndex*sizeof(uint16)), indexToDraw);
        }

        batchQuadCommands_.clear();
        vboArray_[QUADS].u2.bufferCount = 0;
    }

    void Renderer::flush() {
        flush2D();
    }

    void Renderer::flush2D() {
        flushQuads();
        flushTriangles();
    }

    void Renderer::flushQuads() {
        if (vboArray_[QUADS].u2.bufferCount > 0) {
            drawBatchedQuads();
            lastMaterialID_ = 0;
        }
    }

    void Renderer::flushTriangles() {
        if (vboArray_[TRIANGLES].u2.indexCount > 0) {
            drawBatchedTriangles();
            lastMaterialID_ = 0;
        }
    }

    void Renderer::setClearColor(const Color4F &clearColor) {
        clearColor_ = clearColor;
    }
}
