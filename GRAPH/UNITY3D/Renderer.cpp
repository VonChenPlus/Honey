#include <algorithm>
#include "GRAPH/UNITY3D/Renderer.h"
#include "GRAPH/UNITY3D/RenderState.h"
#include "GRAPH/UNITY3D/GLStateCache.h"
#include "GRAPH/UNITY3D/GLProgram.h"
#include "GRAPH/UNITY3D/RenderCommand.h"
#include "MATH/Rectangle.h"

namespace GRAPH
{
    // helper
    static bool compareRenderCommand(RenderCommand* a, RenderCommand* b)
    {
        return a->getGlobalOrder() < b->getGlobalOrder();
    }

    static bool compare3DCommand(RenderCommand* a, RenderCommand* b)
    {
        return  a->getDepth() > b->getDepth();
    }

    // queue
    RenderQueue::RenderQueue() {

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

    int64 RenderQueue::size() const {
        int64 result(0);
        for(int index = 0; index < QUEUE_GROUP::QUEUE_COUNT; ++index) {
            result += commands_[index].size();
        }

        return result;
    }

    void RenderQueue::sort() {
        // Don't sort _queue0, it already comes sorted
        std::sort(std::begin(commands_[QUEUE_GROUP::TRANSPARENT_3D]), std::end(commands_[QUEUE_GROUP::TRANSPARENT_3D]), compare3DCommand);
        std::sort(std::begin(commands_[QUEUE_GROUP::GLOBALZ_NEG]), std::end(commands_[QUEUE_GROUP::GLOBALZ_NEG]), compareRenderCommand);
        std::sort(std::begin(commands_[QUEUE_GROUP::GLOBALZ_POS]), std::end(commands_[QUEUE_GROUP::GLOBALZ_POS]), compareRenderCommand);
    }

    RenderCommand* RenderQueue::operator[](int64 index) const {
        for(int queIndex = 0; queIndex < QUEUE_GROUP::QUEUE_COUNT; ++queIndex) {
            if(index < static_cast<int64>(commands_[queIndex].size()))
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

    void RenderQueue::realloc(size_t reserveSize) {
        for(int i = 0; i < QUEUE_COUNT; ++i) {
            commands_[i] = std::vector<RenderCommand*>();
            commands_[i].reserve(reserveSize);
        }
    }

    void RenderQueue::saveRenderState() {
        isDepthEnabled_ = glIsEnabled(GL_DEPTH_TEST) != GL_FALSE;
        isCullEnabled_ = glIsEnabled(GL_CULL_FACE) != GL_FALSE;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &isDepthWrite_);
    }

    void RenderQueue::restoreRenderState() {
        if (isCullEnabled_) {
            glEnable(GL_CULL_FACE);
            RenderState::DefaultState().setCullFace(true);
        }
        else {
            glDisable(GL_CULL_FACE);
            RenderState::DefaultState().setCullFace(false);
        }

        if (isDepthEnabled_) {
            glEnable(GL_DEPTH_TEST);
            RenderState::DefaultState().setDepthTest(true);
        }
        else {
            glDisable(GL_DEPTH_TEST);
            RenderState::DefaultState().setDepthTest(false);
        }

        glDepthMask(isDepthWrite_);
        RenderState::DefaultState().setDepthWrite(isDepthEnabled_);
    }

    static const int DEFAULT_RENDER_QUEUE = 0;

    Renderer::Renderer()
        : lastMaterialID_(0)
        , glViewAssigned_(false)
        , isRendering_(false)
        , isDepthTestFor2D_(false) {
        groupCommandManager_ = new (std::nothrow) GroupCommandManager(this);
        commandGroupStack_.push(DEFAULT_RENDER_QUEUE);
        RenderQueue defaultRenderQueue;
        renderGroups_.push_back(defaultRenderQueue);
        batchedCommands_.reserve(BATCH_QUADCOMMAND_RESEVER_SIZE);

        memset(vboArray_, 0, sizeof(VertexBufferObject<V3F_C4B_T2F>) * 2);
        for (auto &object : vboArray_) {
            object.u2.bufferData = new V3F_C4B_T2F[VBO_SIZE];
            object.u2.bufferCapacity = VBO_SIZE;
            object.u2.indexData = new GLushort[INDEX_VBO_SIZE];
            object.u2.indexCapacity = INDEX_VBO_SIZE;
        }

        clearColor_ = Color4F::BLACK;
    }

    Renderer::~Renderer() {
        renderGroups_.clear();
        groupCommandManager_->release();

        for (auto object : vboArray_) {
            delete[] object.u2.bufferData;
            delete[] object.u2.indexData;
            glDeleteBuffers(2, object.u2.objectID);
        }
    }

    void Renderer::initGLView() {
        for (int i = 0; i < VBO_SIZE / 4; i++)
        {
            vboArray_[1].u2.indexData[i * 6 + 0] = (GLushort) (i * 4 + 0);
            vboArray_[1].u2.indexData[i * 6 + 1] = (GLushort) (i * 4 + 1);
            vboArray_[1].u2.indexData[i * 6 + 2] = (GLushort) (i * 4 + 2);
            vboArray_[1].u2.indexData[i * 6 + 3] = (GLushort) (i * 4 + 3);
            vboArray_[1].u2.indexData[i * 6 + 4] = (GLushort) (i * 4 + 2);
            vboArray_[1].u2.indexData[i * 6 + 5] = (GLushort) (i * 4 + 1);
        }

        setupBuffer();
        glViewAssigned_ = true;
    }

    void Renderer::setupBuffer() {
        setupVBO();
    }

    void Renderer::setupVBO() {
        glGenBuffers(2, vboArray_[0].u2.objectID);
        glGenBuffers(2, vboArray_[1].u2.objectID);
        mapBuffers();
    }

    void Renderer::mapBuffers() {
        for (auto object : vboArray_) {
            glBindBuffer(GL_ARRAY_BUFFER, object.u2.objectID[0]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(V3F_C4B_T2F) * VBO_SIZE, object.u2.bufferData, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object.u2.objectID[1]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * INDEX_VBO_SIZE, object.u2.indexData, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
            if(cmd->isSkipBatching() || vboArray_[0].u2.bufferCount + cmd->getVertexCount() > VBO_SIZE || vboArray_[0].u2.indexCount + cmd->getIndexCount() > INDEX_VBO_SIZE) {
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
            if(cmd->isSkipBatching()|| (vboArray_[1].u2.bufferCount + cmd->getQuadCount()) * 4 > VBO_SIZE ) {
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
                glEnable(GL_DEPTH_TEST);
                glDepthMask(true);
                RenderState::DefaultState().setDepthTest(true);
                RenderState::DefaultState().setDepthWrite(true);
            }
            else {
                glDisable(GL_DEPTH_TEST);
                glDepthMask(false);
                RenderState::DefaultState().setDepthTest(false);
                RenderState::DefaultState().setDepthWrite(false);
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
            glEnable(GL_DEPTH_TEST);
            glDepthMask(true);
            RenderState::DefaultState().setDepthTest(true);
            RenderState::DefaultState().setDepthWrite(true);

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
            glEnable(GL_DEPTH_TEST);
            glDepthMask(false);
            RenderState::DefaultState().setDepthTest(true);
            RenderState::DefaultState().setDepthWrite(false);

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
                glEnable(GL_DEPTH_TEST);
                glDepthMask(true);

                RenderState::DefaultState().setDepthTest(true);
                RenderState::DefaultState().setDepthWrite(true);

            }
            else {
                glDisable(GL_DEPTH_TEST);
                glDepthMask(false);

                RenderState::DefaultState().setDepthTest(false);
                RenderState::DefaultState().setDepthWrite(false);

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
        clean();
        isRendering_ = false;
    }

    void Renderer::clean() {
        for (size_t j = 0 ; j < renderGroups_.size(); j++) {
            renderGroups_[j].clear();
        }

        batchedCommands_.clear();
        batchQuadCommands_.clear();
        lastMaterialID_ = 0;

        for (auto &object : vboArray_) {
            object.u2.bufferCount = 0;
            object.u2.indexCount = 0;
        }
    }

    void Renderer::clear() {
        //Enable Depth mask to make sure glClear clear the depth buffer correctly
        glDepthMask(true);
        glClearColor(clearColor_.red, clearColor_.green, clearColor_.blue, clearColor_.alpha);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthMask(false);

        RenderState::DefaultState().setDepthWrite(false);
    }

    void Renderer::setDepthTest(bool enable) {
        if (enable) {
            glClearDepth(1.0f);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            RenderState::DefaultState().setDepthTest(true);
            RenderState::DefaultState().setDepthFunction(RenderState::DEPTH_LEQUAL);
        }
        else {
            glDisable(GL_DEPTH_TEST);
            RenderState::DefaultState().setDepthTest(false);
        }

        isDepthTestFor2D_ = enable;
    }

    void Renderer::fillVerticesAndIndices(const TrianglesCommand* cmd) {
        memcpy(vboArray_[0].u2.bufferData + vboArray_[0].u2.bufferCount, cmd->getVertices(), sizeof(V3F_C4B_T2F) * cmd->getVertexCount());
        const MATH::Matrix4& modelView = cmd->getModelView();

        for(int64 i=0; i< cmd->getVertexCount(); ++i) {
            V3F_C4B_T2F *q = &vboArray_[0].u2.bufferData[i + vboArray_[0].u2.bufferCount];
            MATH::Vector3f *vec1 = (MATH::Vector3f*)&q->vertices;
            modelView.transformPoint(vec1);
        }

        const unsigned short* indices = cmd->getIndices();
        //fill index
        for(int64 i=0; i< cmd->getIndexCount(); ++i) {
            vboArray_[0].u2.indexData[vboArray_[0].u2.indexCount + i] = vboArray_[0].u2.bufferCount + indices[i];
        }

        vboArray_[0].u2.bufferCount += cmd->getVertexCount();
        vboArray_[0].u2.indexCount += cmd->getIndexCount();
    }

    void Renderer::fillQuads(const QuadCommand *cmd) {
        const MATH::Matrix4& modelView = cmd->getModelView();
        const V3F_C4B_T2F* quads =  (V3F_C4B_T2F*)cmd->getQuads();
        for(int64 i=0; i< cmd->getQuadCount() * 4; ++i) {
            vboArray_[1].u2.bufferData[i + vboArray_[1].u2.bufferCount * 4] = quads[i];
            modelView.transformPoint(quads[i].vertices,&(vboArray_[1].u2.bufferData[i + vboArray_[1].u2.bufferCount * 4].vertices));
        }

        vboArray_[1].u2.bufferCount += cmd->getQuadCount();
    }

    void Renderer::drawBatchedTriangles()
    {
        //TODO: we can improve the draw performance by insert material switching command before hand.
        int indexToDraw = 0;
        int startIndex = 0;

        //Upload buffer to VBO
        if(vboArray_[0].u2.bufferCount <= 0 || vboArray_[0].u2.indexCount <= 0 || batchedCommands_.empty()) {
            return;
        }

        glBindBuffer(GL_ARRAY_BUFFER, vboArray_[0].u2.objectID[0]);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vboArray_[0].u2.bufferData[0]) * vboArray_[0].u2.bufferCount , vboArray_[0].u2.bufferData, GL_DYNAMIC_DRAW);

        GLStateCache::EnableVertexAttribs(VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);

        // vertices
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid*) offsetof(V3F_C4B_T2F, vertices));

        // colors
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V3F_C4B_T2F), (GLvoid*) offsetof(V3F_C4B_T2F, colors));

        // tex coords
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid*) offsetof(V3F_C4B_T2F, texCoords));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboArray_[0].u2.objectID[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vboArray_[0].u2.indexData[0]) * vboArray_[0].u2.indexCount, vboArray_[0].u2.indexData, GL_STATIC_DRAW);

        //Start drawing verties in batch
        for(const auto& cmd : batchedCommands_) {
            auto newMaterialID = cmd->getMaterialID();
            if(lastMaterialID_ != newMaterialID || newMaterialID == MATERIAL_ID_DO_NOT_BATCH) {
                //Draw quads
                if(indexToDraw > 0) {
                    glDrawElements(GL_TRIANGLES, (GLsizei) indexToDraw, GL_UNSIGNED_SHORT, (GLvoid*) (startIndex*sizeof(vboArray_[0].u2.indexData[0])) );
                    startIndex += indexToDraw;
                    indexToDraw = 0;
                }

                //Use new material
                cmd->useMaterial();
                lastMaterialID_ = newMaterialID;
            }

            indexToDraw += cmd->getIndexCount();
        }

        //Draw any remaining triangles
        if(indexToDraw > 0) {
            glDrawElements(GL_TRIANGLES, (GLsizei) indexToDraw, GL_UNSIGNED_SHORT, (GLvoid*) (startIndex*sizeof(vboArray_[0].u2.indexData[0])) );
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        batchedCommands_.clear();
        vboArray_[0].u2.bufferCount = 0;
        vboArray_[0].u2.indexCount = 0;
    }

    void Renderer::drawBatchedQuads() {
        //TODO: we can improve the draw performance by insert material switching command before hand.
        int64 indexToDraw = 0;
        int startIndex = 0;

        //Upload buffer to VBO
        if(vboArray_[1].u2.bufferCount <= 0 || batchQuadCommands_.empty()) {
            return;
        }

        glBindBuffer(GL_ARRAY_BUFFER, vboArray_[1].u2.objectID[0]);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vboArray_[1].u2.bufferData[0]) * vboArray_[1].u2.bufferCount * 4 , vboArray_[1].u2.bufferData, GL_DYNAMIC_DRAW);

        GLStateCache::EnableVertexAttribs(VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);

        // vertices
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid*) offsetof(V3F_C4B_T2F, vertices));

        // colors
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V3F_C4B_T2F), (GLvoid*) offsetof(V3F_C4B_T2F, colors));

        // tex coords
        glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V3F_C4B_T2F), (GLvoid*) offsetof(V3F_C4B_T2F, texCoords));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboArray_[1].u2.objectID[1]);


        // FIXME: The logic of this code is confusing, and error prone
        // Needs refactoring

        //Start drawing vertices in batch
        for(const auto& cmd : batchQuadCommands_) {
            bool commandQueued = true;
            auto newMaterialID = cmd->getMaterialID();
            if(lastMaterialID_ != newMaterialID || newMaterialID == MATERIAL_ID_DO_NOT_BATCH) {
                // flush buffer
                if(indexToDraw > 0) {
                    glDrawElements(GL_TRIANGLES, (GLsizei) indexToDraw, GL_UNSIGNED_SHORT, (GLvoid*) (startIndex*sizeof(vboArray_[0].u2.indexData[0])) );
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
            glDrawElements(GL_TRIANGLES, (GLsizei) indexToDraw, GL_UNSIGNED_SHORT, (GLvoid*) (startIndex*sizeof(vboArray_[0].u2.indexData[0])) );
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        batchQuadCommands_.clear();
        vboArray_[1].u2.bufferCount = 0;
    }

    void Renderer::flush() {
        flush2D();
    }

    void Renderer::flush2D() {
        flushQuads();
        flushTriangles();
    }

    void Renderer::flushQuads() {
        if(vboArray_[1].u2.bufferCount > 0) {
            drawBatchedQuads();
            lastMaterialID_ = 0;
        }
    }

    void Renderer::flushTriangles() {
        if(vboArray_[0].u2.indexCount > 0)
        {
            drawBatchedTriangles();
            lastMaterialID_ = 0;
        }
    }

    void Renderer::setClearColor(const Color4F &clearColor) {
        clearColor_ = clearColor;
    }
}
