#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <vector>
#include "GRAPH/BASE/Node.h"
#include "MATH/Size.h"

namespace GRAPH
{
    class Camera;
    class BaseLight;
    class Renderer;
    class EventListenerCustom;
    class EventCustom;

    /** @class Scene
    * @brief Scene is a subclass of Node that is used only as an abstract concept.

    Scene and Node are almost identical with the difference that Scene has its
    anchor point (by default) at the center of the screen.

    For the moment Scene has no other logic than that, but in future releases it might have
    additional logic.

    It is a good practice to use a Scene as the parent of all your nodes.

    Scene will create a default camera for you.
    */
    class Scene : public Node
    {
    public:
        /** Creates a new Scene object.
         *
         * @return An autoreleased Scene object.
         */
        static Scene *create();

        /** Creates a new Scene object with a predefined Size.
         *
         * @param size The predefined size of scene.
         * @return An autoreleased Scene object.
         * @js NA
         */
        static Scene *createWithSize(const MATH::Sizef& size);

        using Node::addChild;

        /** Get all cameras.
         *
         * @return The vector of all cameras, ordered by camera depth.
         * @js NA
         */
        const std::vector<Camera*>& getCameras();

        /** Get the default camera.
         * @js NA
         * @return The default camera of scene.
         */
        Camera* getDefaultCamera() const { return _defaultCamera; }

        /** Get lights.
         * @return The vector of lights.
         * @js NA
         */
        const std::vector<BaseLight*>& getLights() const { return _lights; }

        /** Render the scene.
         * @param renderer The renderer use to render the scene.
         * @js NA
         */
        void render(Renderer* renderer);

        /** override function */
        virtual void removeAllChildren() override;

    public:
        Scene();
        virtual ~Scene();

        bool init() override;
        bool initWithSize(const MATH::Sizef& size);

        void setCameraOrderDirty() { _cameraOrderDirty = true; }

        void onProjectionChanged(EventCustom* event);

    protected:
        friend class Node;
        friend class ProtectedNode;
        friend class SpriteBatchNode;
        friend class Camera;
        friend class BaseLight;
        friend class Renderer;

        std::vector<Camera*> _cameras; //weak ref to Camera
        Camera*              _defaultCamera; //weak ref, default camera created by scene, _cameras[0], Caution that the default camera can not be added to _cameras before onEnter is called
        bool                 _cameraOrderDirty; // order is dirty, need sort
        EventListenerCustom*       _event;

        std::vector<BaseLight *> _lights;

    private:
        DISALLOW_COPY_AND_ASSIGN(Scene)
    };
}

#endif // SCENE_H
