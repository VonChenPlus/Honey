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

        _cameraOrderDirty = true;

        //create default camera
        _defaultCamera = Camera::create();
        addChild(_defaultCamera);
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

    static bool CameraCmp(const Camera* a, const Camera* b) {
        return a->getRenderOrder() < b->getRenderOrder();
    }

    const std::vector<Camera*>& Scene::getCameras() {
        if (_cameraOrderDirty) {
            std::stable_sort(_cameras.begin(), _cameras.end(), CameraCmp);
            _cameraOrderDirty = false;
        }
        return _cameras;
    }

    void Scene::render(Renderer* renderer) {
        auto director = &Director::getInstance();
        Camera* defaultCamera = nullptr;
        const auto& transform = getNodeToParentTransform();

        for (const auto& camera : getCameras()) {
            if (!camera->isVisible())
                continue;

            Camera::_visitingCamera = camera;
            if (Camera::_visitingCamera->getCameraFlag() == CameraFlag::DEFAULT) {
                defaultCamera = Camera::_visitingCamera;
            }

            director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
            director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION, Camera::_visitingCamera->getViewProjectionMatrix());
            // TODO
            //camera->apply();
            //clear background with max depth
            // TODO
            //camera->clearBackground(1.0);
            //visit the scene
            visit(renderer, transform, 0);

            renderer->render();

            director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
        }

        Camera::_visitingCamera = nullptr;
    }

    void Scene::removeAllChildren() {
        if (_defaultCamera)
            _defaultCamera->retain();

        Node::removeAllChildren();

        if (_defaultCamera) {
            addChild(_defaultCamera);
            _defaultCamera->release();
        }
    }
}
