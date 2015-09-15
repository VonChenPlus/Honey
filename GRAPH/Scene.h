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

        void render(Renderer* renderer);

        virtual void removeAllChildren() override;

    public:
        Scene();
        virtual ~Scene();

        bool init() override;
        bool initWithSize(const MATH::Sizef& size);

    private:
        DISALLOW_COPY_AND_ASSIGN(Scene)

        friend class Camera;
    };
}

#endif // SCENE_H
