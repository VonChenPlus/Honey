#include "GRAPH/BASE/ActionInstant.h"
#include "GRAPH/BASE/Node.h"
#include "GRAPH/BASE/Sprite.h"

namespace GRAPH
{
    //
    // InstantAction
    //
    bool ActionInstant::isDone() const
    {
        return true;
    }

    void ActionInstant::step(float dt) {
        update(1);
    }

    void ActionInstant::update(float time) {
    }

    //
    // Show
    //

    Show* Show::create()
    {
        Show* ret = new (std::nothrow) Show();

        if (ret) {
            ret->autorelease();
        }

        return ret;
    }

    void Show::update(float time) {
        _target->setVisible(true);
    }

    //
    // Hide
    //
    Hide * Hide::create()
    {
        Hide *ret = new (std::nothrow) Hide();

        if (ret) {
            ret->autorelease();
        }

        return ret;
    }

    void Hide::update(float time) {
        _target->setVisible(false);
    }

    //
    // ToggleVisibility
    //
    ToggleVisibility * ToggleVisibility::create()
    {
        ToggleVisibility *ret = new (std::nothrow) ToggleVisibility();

        if (ret)
        {
            ret->autorelease();
        }

        return ret;
    }

    void ToggleVisibility::update(float time)
    {
        _target->setVisible(!_target->isVisible());
    }

    //
    // Remove Self
    //
    RemoveSelf * RemoveSelf::create(bool isNeedCleanUp /*= true*/)
    {
        RemoveSelf *ret = new (std::nothrow) RemoveSelf();

        if (ret && ret->init(isNeedCleanUp)) {
            ret->autorelease();
        }

        return ret;
    }

    bool RemoveSelf::init(bool isNeedCleanUp) {
        _isNeedCleanUp = isNeedCleanUp;
        return true;
    }

    void RemoveSelf::update(float time) {
        _target->removeFromParentAndCleanup(_isNeedCleanUp);
    }

    //
    // FlipX
    //

    FlipX *FlipX::create(bool x)
    {
        FlipX *ret = new (std::nothrow) FlipX();

        if (ret && ret->initWithFlipX(x)) {
            ret->autorelease();
            return ret;
        }

        SAFE_DELETE(ret);
        return nullptr;
    }

    bool FlipX::initWithFlipX(bool x) {
        _flipX = x;
        return true;
    }

    void FlipX::update(float time) {
        static_cast<Sprite*>(_target)->setFlippedX(_flipX);
    }

    //
    // FlipY
    //

    FlipY * FlipY::create(bool y)
    {
        FlipY *ret = new (std::nothrow) FlipY();

        if (ret && ret->initWithFlipY(y)) {
            ret->autorelease();
            return ret;
        }

        SAFE_DELETE(ret);
        return nullptr;
    }

    bool FlipY::initWithFlipY(bool y) {
        _flipY = y;
        return true;
    }

    void FlipY::update(float time) {
        static_cast<Sprite*>(_target)->setFlippedY(_flipY);
    }

    //
    // Place
    //

    Place* Place::create(const MATH::Vector2f& pos)
    {
        Place *ret = new (std::nothrow) Place();

        if (ret && ret->initWithPosition(pos)) {
            ret->autorelease();
            return ret;
        }

        SAFE_DELETE(ret);
        return nullptr;
    }

    bool Place::initWithPosition(const MATH::Vector2f& pos) {
        _position = pos;
        return true;
    }

    void Place::update(float time) {
        _target->setPosition(_position);
    }

    //
    // CallFunc
    //

    CallFunc * CallFunc::create(const std::function<void()> &func)
    {
        CallFunc *ret = new (std::nothrow) CallFunc();

        if (ret && ret->initWithFunction(func) ) {
            ret->autorelease();
            return ret;
        }

        SAFE_DELETE(ret);
        return nullptr;
    }

    bool CallFunc::initWithFunction(const std::function<void()> &func)
    {
        _function = func;
        return true;
    }

    CallFunc::~CallFunc()
    {
        SAFE_RELEASE(_selectorTarget);
    }

    void CallFunc::update(float time) {
        this->execute();
    }

    void CallFunc::execute() {
        if (_callFunc) {
            (_selectorTarget->*_callFunc)();
        } else if( _function ){
            _function();
        }
    }

    //
    // CallFuncN
    //

    CallFuncN * CallFuncN::create(const std::function<void(Node*)> &func)
    {
        auto ret = new (std::nothrow) CallFuncN();

        if (ret && ret->initWithFunction(func) ) {
            ret->autorelease();
            return ret;
        }

        SAFE_DELETE(ret);
        return nullptr;
    }

    void CallFuncN::execute() {
        if (_callFuncN) {
            (_selectorTarget->*_callFuncN)(_target);
        }
        else if (_functionN) {
            _functionN(_target);
        }
    }

    bool CallFuncN::initWithFunction(const std::function<void (Node *)> &func)
    {
        _functionN = func;
        return true;
    }

    void CCCallFuncND::execute()
    {
        if (_callFuncND)
        {
            (_selectorTarget->*_callFuncND)(_target, _data);
        }
    }

    //
    // CallFuncO
    //
    CCCallFuncO::CCCallFuncO() :
    _object(nullptr)
    {
    }

    CCCallFuncO::~CCCallFuncO()
    {
        SAFE_RELEASE(_object);
    }

    void CCCallFuncO::execute()
    {
        if (_callFuncO) {
            (_selectorTarget->*_callFuncO)(_object);
        }
    }

    HObject* CCCallFuncO::getObject() const
    {
        return _object;
    }

    void CCCallFuncO::setObject(HObject* obj)
    {
        if (obj != _object)
        {
            SAFE_RELEASE(_object);
            _object = obj;
            SAFE_RETAIN(_object);
        }
    }
}
