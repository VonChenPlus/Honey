#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <stack>
#include "GRAPH/RENDERER/GLCommon.h"
#include "GRAPH/RENDERER/RenderCommand.h"
#include "BASE/Honey.h"
#include "GRAPH/Types.h"
#include "MATH/Size.h"

namespace GRAPH
{
    class QuadCommand;
    class TrianglesCommand;

    class RenderQueue {
    public:
        enum QUEUE_GROUP
        {
            /**Objects with globalZ smaller than 0.*/
            GLOBALZ_NEG = 0,
            /**Opaque 3D objects with 0 globalZ.*/
            OPAQUE_3D = 1,
            /**Transparent 3D objects with 0 globalZ.*/
            TRANSPARENT_3D = 2,
            /**2D objects with 0 globalZ.*/
            GLOBALZ_ZERO = 3,
            /**Objects with globalZ bigger than 0.*/
            GLOBALZ_POS = 4,
            QUEUE_COUNT = 5,
        };

    public:
        RenderQueue();
        void push_back(RenderCommand* command);
        ssize_t size() const;
        void sort();
        RenderCommand* operator[](ssize_t index) const;
        void clear();
        void realloc(size_t reserveSize);
        inline std::vector<RenderCommand*>& getSubQueue(QUEUE_GROUP group) { return commands_[group]; }
        inline ssize_t getSubQueueSize(QUEUE_GROUP group) const { return commands_[group].size();}

        void saveRenderState();
        void restoreRenderState();

    protected:
        std::vector<RenderCommand*> commands_[QUEUE_COUNT];

        bool isCullEnabled_;
        bool isDepthEnabled_;
        GLboolean isDepthWrite_;
    };

    struct RenderStackElement
    {
        int renderQueueID;
        ssize_t currentIndex;
    };

    class GroupCommandManager;

    class Renderer : public HObject
    {
    public:
        static const int VBO_SIZE = 65536;
        static const int INDEX_VBO_SIZE = VBO_SIZE * 6 / 4;
        static const int BATCH_QUADCOMMAND_RESEVER_SIZE = 64;
        static const int MATERIAL_ID_DO_NOT_BATCH = 0;

        Renderer();
        ~Renderer();

        void initGLView();

        void addCommand(RenderCommand* command);
        void addCommand(RenderCommand* command, int renderQueue);

        void pushGroup(int renderQueueID);
        void popGroup();

        int createRenderQueue();

        void render();

        void clean();
        void clear();

        void setClearColor(const Color4F& clearColor);
        void setDepthTest(bool enable);

        inline GroupCommandManager* getGroupCommandManager() const { return groupCommandManager_; }

    protected:
        void setupBuffer();
        void setupVBO();
        void mapBuffers();
        void drawBatchedTriangles();
        void drawBatchedQuads();

        void flush();
        void flush2D();
        void flushQuads();
        void flushTriangles();

        void processRenderCommand(RenderCommand* command);
        void visitRenderQueue(RenderQueue& queue);

        void fillVerticesAndIndices(const TrianglesCommand* cmd);
        void fillQuads(const QuadCommand* cmd);

    private:
        Color4F clearColor_;
        std::stack<int> commandGroupStack_;
        GroupCommandManager* groupCommandManager_;
        std::vector<RenderQueue> renderGroups_;
        std::vector<TrianglesCommand*> batchedCommands_;
        std::vector<QuadCommand*> batchQuadCommands_;

        VertexBufferObject<V3F_C4B_T2F> vboArray_[2]; //0: vertex  1: indices

        bool glViewAssigned_;
        bool isRendering_;
        bool isDepthTestFor2D_;
        uint32_t lastMaterialID_;
    };
}

#endif // RENDERER_H
