#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <vector>
#include "GRAPH/Node.h"
#include "MATH/Size.h"

namespace GRAPH
{
    class Camera;
    class Renderer;

    class Scene : public Node
    {
    public:
        static Scene *create();
        static Scene *createWithSize(const MATH::Sizef& size);

        const std::vector<Camera*>& getCameras();
        Camera* getDefaultCamera() const { return _defaultCamera; }

        void render(Renderer* renderer);

        virtual void removeAllChildren() override;

    public:
        Scene();
        virtual ~Scene();

        bool init() override;
        bool initWithSize(const MATH::Sizef& size);

        void setCameraOrderDirty() { _cameraOrderDirty = true; }

    protected:
        std::vector<Camera*> _cameras; //weak ref to Camera
        Camera*              _defaultCamera; //weak ref, default camera created by scene, _cameras[0], Caution that the default camera can not be added to _cameras before onEnter is called
        bool                 _cameraOrderDirty; // order is dirty, need sort

    private:
        DISALLOW_COPY_AND_ASSIGN(Scene)

        friend class Camera;
    };
}

#endif // SCENE_H
