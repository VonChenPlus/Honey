#include <algorithm>
#include "GRAPH/Director.h"
#include "GRAPH/Scene.h"
#include "GRAPH/Camera.h"
#include "GRAPH/RENDERER/Renderer.h"
#include "MATH/Vector.h"

namespace GRAPH
{
    Scene::Scene() {
        _ignoreAnchorPointForPosition = true;
        setAnchorPoint(MATH::Vector2f(0.5f, 0.5f));
        defaultCamera_ = Director::getInstance().getCamera();
        defaultCamera_->retain();
        addChild(defaultCamera_);
    }

    Scene::~Scene() {
    }

    bool Scene::init() {
        auto size = Director::getInstance().getWinSize();
        return initWithSize(size);
    }

    bool Scene::initWithSize(const MATH::Sizef& size) {
        setContentSize(size);
        return true;
    }

    Scene* Scene::create() {
        Scene *ret = new (std::nothrow) Scene();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        else {
            SAFE_DELETE(ret);
            return nullptr;
        }
    }

    Scene* Scene::createWithSize(const MATH::Sizef& size) {
        Scene *ret = new (std::nothrow) Scene();
        if (ret && ret->initWithSize(size)) {
            ret->autorelease();
            return ret;
        }
        else {
            SAFE_DELETE(ret);
            return nullptr;
        }
    }

    void Scene::render(Renderer* renderer) {
        auto &director = Director::getInstance();
        const auto& transform = getNodeToParentTransform();

        director.pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
        director.loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION, defaultCamera_->getViewProjectionMatrix());
        
        defaultCamera_->apply();
        
        //visit the scene
        visit(renderer, transform, 0);

        renderer->render();

        director.popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
    }

    void Scene::removeAllChildren() {
        if (defaultCamera_)
            defaultCamera_->retain();

        Node::removeAllChildren();

        if (defaultCamera_) {
            addChild(defaultCamera_);
            defaultCamera_->release();
        }
    }
}
