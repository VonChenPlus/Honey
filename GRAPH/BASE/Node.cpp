#include "GRAPH/BASE/Node.h"

#include <algorithm>
#include <string>
#include <regex>
#include "GRAPH/BASE/Director.h"
#include "GRAPH/BASE/ActionManager.h"
#include "GRAPH/BASE/Scheduler.h"
#include "GRAPH/BASE/EventDispatcher.h"
#include "GRAPH/RENDERER/GLProgramState.h"
#include "UTILS/STRING/StringUtils.h"
#include "GRAPH/BASE/Camera.h"
#include "GRAPH/BASE/Macros.h"
#include "GRAPH/BASE/ActionManager.h"
#include "GRAPH/BASE/Component.h"

namespace GRAPH
{
    #define RENDER_IN_SUBPIXEL

    bool nodeComparisonLess(Node* n1, Node* n2)
    {
        return( n1->getLocalZOrder() < n2->getLocalZOrder() ||
               ( n1->getLocalZOrder() == n2->getLocalZOrder() && n1->getOrderOfArrival() < n2->getOrderOfArrival() )
               );
    }

    void CGAffineToGL(const MATH::AffineTransform& t, GLfloat *m)
    {
        // | m[0] m[4] m[8]  m[12] |     | m11 m21 m31 m41 |     | a c 0 tx |
        // | m[1] m[5] m[9]  m[13] |     | m12 m22 m32 m42 |     | b d 0 ty |
        // | m[2] m[6] m[10] m[14] | <=> | m13 m23 m33 m43 | <=> | 0 0 1  0 |
        // | m[3] m[7] m[11] m[15] |     | m14 m24 m34 m44 |     | 0 0 0  1 |

        m[2] = m[3] = m[6] = m[7] = m[8] = m[9] = m[11] = m[14] = 0.0f;
        m[10] = m[15] = 1.0f;
        m[0] = t.a; m[4] = t.c; m[12] = t.tx;
        m[1] = t.b; m[5] = t.d; m[13] = t.ty;
    }

    void GLToCGAffine(const GLfloat *m, MATH::AffineTransform *t)
    {
        t->a = m[0]; t->c = m[4]; t->tx = m[12];
        t->b = m[1]; t->d = m[5]; t->ty = m[13];
    }

    // FIXME:: Yes, nodes might have a sort problem once every 15 days if the game runs at 60 FPS and each frame sprites are reordered.
    int Node::s_globalOrderOfArrival = 1;

    // MARK: Constructor, Destructor, Init

    Node::Node(void)
    : _rotationX(0.0f)
    , _rotationY(0.0f)
    , _rotationZ_X(0.0f)
    , _rotationZ_Y(0.0f)
    , _scaleX(1.0f)
    , _scaleY(1.0f)
    , _scaleZ(1.0f)
    , _positionZ(0.0f)
    , _usingNormalizedPosition(false)
    , _normalizedPositionDirty(false)
    , _skewX(0.0f)
    , _skewY(0.0f)
    , _contentSize(MATH::SizefZERO)
    , _contentSizeDirty(true)
    , _transformDirty(true)
    , _inverseDirty(true)
    , _useAdditionalTransform(false)
    , _transformUpdated(true)
    , _localZOrder(0)
    , _globalZOrder(0)
    , _parent(nullptr)
    , _tag(Node::INVALID_TAG)
    , _name("")
    , _hashOfName(0)
    , _userData(nullptr)
    , _userObject(nullptr)
    , _glProgramState(nullptr)
    , _orderOfArrival(0)
    , _running(false)
    , _visible(true)
    , _ignoreAnchorPointForPosition(false)
    , _reorderChildDirty(false)
    , _isTransitionFinished(false)
    , _displayedOpacity(255)
    , _realOpacity(255)
    , _displayedColor(Color3B::WHITE)
    , _realColor(Color3B::WHITE)
    , _cascadeColorEnabled(false)
    , _cascadeOpacityEnabled(false)
    , _cameraMask(1)
    {
        _director = Director::getInstance();
        _actionManager = _director->getActionManager();
        _actionManager->retain();
        _scheduler = _director->getScheduler();
        _scheduler->retain();
        _eventDispatcher = _director->getEventDispatcher();
        _eventDispatcher->retain();
        _transform = _inverse = _additionalTransform = MATH::Matrix4::IDENTITY;
    }

    Node * Node::create()
    {
        Node * ret = new (std::nothrow) Node();
        if (ret && ret->init())
        {
            ret->autorelease();
        }
        else
        {
            SAFE_DELETE(ret);
        }
        return ret;
    }

    Node::~Node()
    {
        SAFE_RELEASE_NULL(_userObject);
        SAFE_RELEASE_NULL(_glProgramState);

        for (auto& child : _children)
        {
            child->_parent = nullptr;
        }

        stopAllActions();
        unscheduleAllCallbacks();
        SAFE_RELEASE_NULL(_actionManager);
        SAFE_RELEASE_NULL(_scheduler);

        _eventDispatcher->removeEventListenersForTarget(this);

        SAFE_RELEASE(_eventDispatcher);
    }

    bool Node::init()
    {
        return true;
    }

    void Node::cleanup()
    {
        this->stopAllActions();
        this->unscheduleAllCallbacks();

        for( const auto &child: _children)
            child->cleanup();
    }

    float Node::getSkewX() const
    {
        return _skewX;
    }

    void Node::setSkewX(float skewX)
    {
        if (_skewX == skewX)
            return;

        _skewX = skewX;
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }

    float Node::getSkewY() const
    {
        return _skewY;
    }

    void Node::setSkewY(float skewY)
    {
        if (_skewY == skewY)
            return;

        _skewY = skewY;
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }

    void Node::setLocalZOrder(int z)
    {
        if (_localZOrder == z)
            return;

        _localZOrder = z;
        if (_parent)
        {
            _parent->reorderChild(this, z);
        }

        _eventDispatcher->setDirtyForNode(this);
    }

    void Node::setGlobalZOrder(float globalZOrder)
    {
        if (_globalZOrder != globalZOrder)
        {
            _globalZOrder = globalZOrder;
            _eventDispatcher->setDirtyForNode(this);
        }
    }

    float Node::getRotation() const
    {
        return _rotationZ_X;
    }

    void Node::setRotation(float rotation)
    {
        if (_rotationZ_X == rotation)
            return;

        _rotationZ_X = _rotationZ_Y = rotation;
        _transformUpdated = _transformDirty = _inverseDirty = true;
        updateRotationQuat();
    }

    float Node::getRotationSkewX() const
    {
        return _rotationZ_X;
    }

    void Node::setRotation3D(const MATH::Vector3f& rotation)
    {
        if (_rotationX == rotation.x &&
            _rotationY == rotation.y &&
            _rotationZ_X == rotation.z)
            return;

        _transformUpdated = _transformDirty = _inverseDirty = true;

        _rotationX = rotation.x;
        _rotationY = rotation.y;

        _rotationZ_Y = _rotationZ_X = rotation.z;

        updateRotationQuat();
    }

    MATH::Vector3f Node::getRotation3D() const
    {
        return MATH::Vector3f(_rotationX,_rotationY,_rotationZ_X);
    }

    void Node::updateRotationQuat()
    {
        float halfRadx = MATH_DEGREES_TO_RADIANS(_rotationX / 2.f), halfRady = MATH_DEGREES_TO_RADIANS(_rotationY / 2.f), halfRadz = _rotationZ_X == _rotationZ_Y ? -MATH_DEGREES_TO_RADIANS(_rotationZ_X / 2.f) : 0;
        float coshalfRadx = cosf(halfRadx), sinhalfRadx = sinf(halfRadx), coshalfRady = cosf(halfRady), sinhalfRady = sinf(halfRady), coshalfRadz = cosf(halfRadz), sinhalfRadz = sinf(halfRadz);
        _rotationQuat.x = sinhalfRadx * coshalfRady * coshalfRadz - coshalfRadx * sinhalfRady * sinhalfRadz;
        _rotationQuat.y = coshalfRadx * sinhalfRady * coshalfRadz + sinhalfRadx * coshalfRady * sinhalfRadz;
        _rotationQuat.z = coshalfRadx * coshalfRady * sinhalfRadz - sinhalfRadx * sinhalfRady * coshalfRadz;
        _rotationQuat.w = coshalfRadx * coshalfRady * coshalfRadz + sinhalfRadx * sinhalfRady * sinhalfRadz;
    }

    void Node::updateRotation3D()
    {
        float x = _rotationQuat.x, y = _rotationQuat.y, z = _rotationQuat.z, w = _rotationQuat.w;
        _rotationX = atan2f(2.f * (w * x + y * z), 1.f - 2.f * (x * x + y * y));
        _rotationY = asinf(2.f * (w * y - z * x));
        _rotationZ_X = atan2f(2.f * (w * z + x * y), 1.f - 2.f * (y * y + z * z));

        _rotationX = MATH_RADIANS_TO_DEGREES(_rotationX);
        _rotationY = MATH_RADIANS_TO_DEGREES(_rotationY);
        _rotationZ_X = _rotationZ_Y = -MATH_RADIANS_TO_DEGREES(_rotationZ_X);
    }

    void Node::setRotationQuat(const MATH::Quaternion& quat)
    {
        _rotationQuat = quat;
        updateRotation3D();
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }

    MATH::Quaternion Node::getRotationQuat() const
    {
        return _rotationQuat;
    }

    void Node::setRotationSkewX(float rotationX)
    {
        if (_rotationZ_X == rotationX)
            return;

        _rotationZ_X = rotationX;
        _transformUpdated = _transformDirty = _inverseDirty = true;

        updateRotationQuat();
    }

    float Node::getRotationSkewY() const
    {
        return _rotationZ_Y;
    }

    void Node::setRotationSkewY(float rotationY)
    {
        if (_rotationZ_Y == rotationY)
            return;

        _rotationZ_Y = rotationY;
        _transformUpdated = _transformDirty = _inverseDirty = true;

        updateRotationQuat();
    }

    float Node::getScale(void) const
    {
        return _scaleX;
    }

    void Node::setScale(float scale)
    {
        if (_scaleX == scale && _scaleY == scale && _scaleZ == scale)
            return;

        _scaleX = _scaleY = _scaleZ = scale;
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }

    float Node::getScaleX() const
    {
        return _scaleX;
    }

    void Node::setScale(float scaleX,float scaleY)
    {
        if (_scaleX == scaleX && _scaleY == scaleY)
            return;

        _scaleX = scaleX;
        _scaleY = scaleY;
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }

    void Node::setScaleX(float scaleX)
    {
        if (_scaleX == scaleX)
            return;

        _scaleX = scaleX;
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }

    float Node::getScaleY() const
    {
        return _scaleY;
    }

    void Node::setScaleZ(float scaleZ)
    {
        if (_scaleZ == scaleZ)
            return;

        _scaleZ = scaleZ;
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }

    float Node::getScaleZ() const
    {
        return _scaleZ;
    }

    void Node::setScaleY(float scaleY)
    {
        if (_scaleY == scaleY)
            return;

        _scaleY = scaleY;
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }

    const MATH::Vector2f& Node::getPosition() const
    {
        return _position;
    }

    void Node::setPosition(const MATH::Vector2f& position)
    {
        setPosition(position.x, position.y);
    }

    void Node::getPosition(float* x, float* y) const
    {
        *x = _position.x;
        *y = _position.y;
    }

    void Node::setPosition(float x, float y)
    {
        if (_position.x == x && _position.y == y)
            return;

        _position.x = x;
        _position.y = y;

        _transformUpdated = _transformDirty = _inverseDirty = true;
        _usingNormalizedPosition = false;
    }

    void Node::setPosition3D(const MATH::Vector3f& position)
    {
        setPositionZ(position.z);
        setPosition(position.x, position.y);
    }

    MATH::Vector3f Node::getPosition3D() const
    {
        return MATH::Vector3f(_position.x, _position.y, _positionZ);
    }

    float Node::getPositionX() const
    {
        return _position.x;
    }

    void Node::setPositionX(float x)
    {
        setPosition(x, _position.y);
    }

    float Node::getPositionY() const
    {
        return  _position.y;
    }

    void Node::setPositionY(float y)
    {
        setPosition(_position.x, y);
    }

    float Node::getPositionZ() const
    {
        return _positionZ;
    }

    void Node::setPositionZ(float positionZ)
    {
        if (_positionZ == positionZ)
            return;

        _transformUpdated = _transformDirty = _inverseDirty = true;

        _positionZ = positionZ;
    }

    const MATH::Vector2f& Node::getNormalizedPosition() const
    {
        return _normalizedPosition;
    }

    void Node::setNormalizedPosition(const MATH::Vector2f& position)
    {
        if (_normalizedPosition.equals(position))
            return;

        _normalizedPosition = position;
        _usingNormalizedPosition = true;
        _normalizedPositionDirty = true;
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }

    ssize_t Node::getChildrenCount() const
    {
        return _children.size();
    }

    bool Node::isVisible() const
    {
        return _visible;
    }

    void Node::setVisible(bool visible)
    {
        if(visible != _visible)
        {
            _visible = visible;
            if(_visible)
                _transformUpdated = _transformDirty = _inverseDirty = true;
        }
    }

    const MATH::Vector2f& Node::getAnchorPointInPoints() const
    {
        return _anchorPointInPoints;
    }

    const MATH::Vector2f& Node::getAnchorPoint() const
    {
        return _anchorPoint;
    }

    void Node::setAnchorPoint(const MATH::Vector2f& point)
    {
        if (! point.equals(_anchorPoint))
        {
            _anchorPoint = point;
            _anchorPointInPoints.set(_contentSize.width * _anchorPoint.x, _contentSize.height * _anchorPoint.y);
            _transformUpdated = _transformDirty = _inverseDirty = true;
        }
    }

    const MATH::Sizef& Node::getContentSize() const
    {
        return _contentSize;
    }

    void Node::setContentSize(const MATH::Sizef & size)
    {
        if (! size.equals(_contentSize))
        {
            _contentSize = size;

            _anchorPointInPoints.set(_contentSize.width * _anchorPoint.x, _contentSize.height * _anchorPoint.y);
            _transformUpdated = _transformDirty = _inverseDirty = _contentSizeDirty = true;
        }
    }

    bool Node::isRunning() const
    {
        return _running;
    }

    void Node::setParent(Node * parent)
    {
        _parent = parent;
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }

    bool Node::isIgnoreAnchorPointForPosition() const
    {
        return _ignoreAnchorPointForPosition;
    }

    void Node::ignoreAnchorPointForPosition(bool newValue)
    {
        if (newValue != _ignoreAnchorPointForPosition)
        {
            _ignoreAnchorPointForPosition = newValue;
            _transformUpdated = _transformDirty = _inverseDirty = true;
        }
    }

    int Node::getTag() const
    {
        return _tag;
    }

    void Node::setTag(int tag)
    {
        _tag = tag ;
    }

    std::string Node::getName() const
    {
        return _name;
    }

    void Node::setName(const std::string& name)
    {
        _name = name;
        std::hash<std::string> h;
        _hashOfName = h(name);
    }

    void Node::setUserData(void *userData)
    {
        _userData = userData;
    }

    int Node::getOrderOfArrival() const
    {
        return _orderOfArrival;
    }

    void Node::setOrderOfArrival(int orderOfArrival)
    {
        _orderOfArrival = orderOfArrival;
    }

    void Node::setUserObject(HObject* userObject)
    {
        SAFE_RETAIN(userObject);
        SAFE_RELEASE(_userObject);
        _userObject = userObject;
    }

    GLProgramState* Node::getGLProgramState() const
    {
        return _glProgramState;
    }

    void Node::setGLProgramState(GLProgramState* glProgramState)
    {
        if (glProgramState != _glProgramState)
        {
            SAFE_RELEASE(_glProgramState);
            _glProgramState = glProgramState;
            SAFE_RETAIN(_glProgramState);

            if (_glProgramState)
                _glProgramState->setNodeBinding(this);
        }
    }

    void Node::setGLProgram(GLProgram* glProgram)
    {
        if (_glProgramState == nullptr || (_glProgramState && _glProgramState->getGLProgram() != glProgram))
        {
            SAFE_RELEASE(_glProgramState);
            _glProgramState = GLProgramState::getOrCreateWithGLProgram(glProgram);
            _glProgramState->retain();

            _glProgramState->setNodeBinding(this);
        }
    }

    GLProgram * Node::getGLProgram() const
    {
        return _glProgramState ? _glProgramState->getGLProgram() : nullptr;
    }

    Scene* Node::getScene() const
    {
        if (!_parent)
            return nullptr;

        auto sceneNode = _parent;
        while (sceneNode->_parent)
        {
            sceneNode = sceneNode->_parent;
        }

        return dynamic_cast<Scene*>(sceneNode);
    }

    MATH::Rectf Node::getBoundingBox() const
    {
        MATH::Rectf rect(0, 0, _contentSize.width, _contentSize.height);
        return RectApplyAffineTransform(rect, getNodeToParentAffineTransform());
    }

    void Node::childrenAlloc()
    {
        _children.reserve(4);
    }

    Node* Node::getChildByTag(int tag) const
    {
        for (const auto child : _children)
        {
            if(child && child->_tag == tag)
                return child;
        }
        return nullptr;
    }

    Node* Node::getChildByName(const std::string& name) const
    {
        std::hash<std::string> h;
        size_t hash = h(name);

        for (const auto& child : _children)
        {
            if(child->_hashOfName == hash && child->_name.compare(name) == 0)
                return child;
        }
        return nullptr;
    }

    void Node::enumerateChildren(const std::string &name, std::function<bool (Node *)> callback) const
    {
        size_t length = name.length();

        size_t subStrStartPos = 0;  // sub string start index
        size_t subStrlength = length; // sub string length

        bool searchRecursively = false;
        if (length > 2 && name[0] == '/' && name[1] == '/')
        {
            searchRecursively = true;
            subStrStartPos = 2;
            subStrlength -= 2;
        }

        bool searchFromParent = false;
        if (length > 3 &&
            name[length-3] == '/' &&
            name[length-2] == '.' &&
            name[length-1] == '.')
        {
            searchFromParent = true;
            subStrlength -= 3;
        }

        std::string newName = name.substr(subStrStartPos, subStrlength);

        if (searchFromParent)
        {
            newName.insert(0, "[[:alnum:]]+/");
        }


        if (searchRecursively)
        {
            doEnumerateRecursive(this, newName, callback);
        }
        else
        {
            doEnumerate(newName, callback);
        }
    }

    bool Node::doEnumerateRecursive(const Node* node, const std::string &name, std::function<bool (Node *)> callback) const
    {
        bool ret =false;

        if (node->doEnumerate(name, callback))
        {
            ret = true;
        }
        else
        {
            for (const auto& child : node->getChildren())
            {
                if (doEnumerateRecursive(child, name, callback))
                {
                    ret = true;
                    break;
                }
            }
        }

        return ret;
    }

    bool Node::doEnumerate(std::string name, std::function<bool (Node *)> callback) const
    {
        size_t pos = name.find('/');
        std::string searchName = name;
        bool needRecursive = false;
        if (pos != name.npos)
        {
            searchName = name.substr(0, pos);
            name.erase(0, pos+1);
            needRecursive = true;
        }

        bool ret = false;
        for (const auto& child : _children)
        {
            if (std::regex_match(child->_name, std::regex(searchName)))
            {
                if (!needRecursive)
                {
                    if (callback(child))
                    {
                        ret = true;
                        break;
                    }
                }
                else
                {
                    ret = child->doEnumerate(name, callback);
                    if (ret)
                        break;
                }
            }
        }

        return ret;
    }

    void Node::addChild(Node *child, int localZOrder, int tag)
    {
        addChildHelper(child, localZOrder, tag, "", true);
    }

    void Node::addChild(Node* child, int localZOrder, const std::string &name)
    {
        addChildHelper(child, localZOrder, INVALID_TAG, name, false);
    }

    void Node::addChildHelper(Node* child, int localZOrder, int tag, const std::string &name, bool setTag)
    {
        if (_children.empty())
        {
            this->childrenAlloc();
        }

        this->insertChild(child, localZOrder);

        if (setTag)
            child->setTag(tag);
        else
            child->setName(name);

        child->setParent(this);
        child->setOrderOfArrival(s_globalOrderOfArrival++);

        if( _running )
        {
            child->onEnter();
            if (_isTransitionFinished)
            {
                child->onEnterTransitionDidFinish();
            }
        }

        if (_cascadeColorEnabled)
        {
            updateCascadeColor();
        }

        if (_cascadeOpacityEnabled)
        {
            updateCascadeOpacity();
        }
    }

    void Node::addChild(Node *child, int zOrder)
    {
        this->addChild(child, zOrder, child->_name);
    }

    void Node::addChild(Node *child)
    {
        this->addChild(child, child->_localZOrder, child->_name);
    }

    void Node::removeFromParent()
    {
        this->removeFromParentAndCleanup(true);
    }

    void Node::removeFromParentAndCleanup(bool cleanup)
    {
        if (_parent != nullptr)
        {
            _parent->removeChild(this,cleanup);
        }
    }

    void Node::removeChild(Node* child, bool cleanup /* = true */)
    {
        if (_children.empty())
        {
            return;
        }

        ssize_t index = _children.getIndex(child);
        if( index != -1 )
            this->detachChild( child, index, cleanup );
    }

    void Node::removeChildByTag(int tag, bool cleanup/* = true */)
    {
        Node *child = this->getChildByTag(tag);

        if (child != nullptr)
        {
            this->removeChild(child, cleanup);
        }
    }

    void Node::removeChildByName(const std::string &name, bool cleanup)
    {
        Node *child = this->getChildByName(name);

        if (child != nullptr)
        {
            this->removeChild(child, cleanup);
        }
    }

    void Node::removeAllChildren()
    {
        this->removeAllChildrenWithCleanup(true);
    }

    void Node::removeAllChildrenWithCleanup(bool cleanup)
    {
        for (const auto& child : _children)
        {
            if(_running)
            {
                child->onExitTransitionDidStart();
                child->onExit();
            }

            if (cleanup)
            {
                child->cleanup();
            }

            child->setParent(nullptr);
        }

        _children.clear();
    }

    void Node::detachChild(Node *child, ssize_t childIndex, bool doCleanup)
    {
        if (_running)
        {
            child->onExitTransitionDidStart();
            child->onExit();
        }

        if (doCleanup)
        {
            child->cleanup();
        }

        child->setParent(nullptr);

        _children.erase(childIndex);
    }

    void Node::insertChild(Node* child, int z)
    {
        _transformUpdated = true;
        _reorderChildDirty = true;
        _children.pushBack(child);
        child->_localZOrder = z;
    }

    void Node::reorderChild(Node *child, int zOrder)
    {
        _reorderChildDirty = true;
        child->setOrderOfArrival(s_globalOrderOfArrival++);
        child->_localZOrder = zOrder;
    }

    void Node::sortAllChildren()
    {
        if (_reorderChildDirty)
        {
            std::sort(std::begin(_children), std::end(_children), nodeComparisonLess);
            _reorderChildDirty = false;
        }
    }

    void Node::draw()
    {
        auto renderer = _director->getRenderer();
        draw(renderer, _modelViewTransform, true);
    }

    void Node::draw(Renderer*, const MATH::Matrix4 &, uint32_t)
    {
    }

    void Node::visit()
    {
        auto renderer = _director->getRenderer();
        auto& parentTransform = _director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        visit(renderer, parentTransform, true);
    }

    uint32_t Node::processParentFlags(const MATH::Matrix4& parentTransform, uint32_t parentFlags)
    {
        if(_usingNormalizedPosition)
        {
            if ((parentFlags & FLAGS_CONTENT_SIZE_DIRTY) || _normalizedPositionDirty)
            {
                auto& s = _parent->getContentSize();
                _position.x = _normalizedPosition.x * s.width;
                _position.y = _normalizedPosition.y * s.height;
                _transformUpdated = _transformDirty = _inverseDirty = true;
                _normalizedPositionDirty = false;
            }
        }

        uint32_t flags = parentFlags;
        flags |= (_transformUpdated ? FLAGS_TRANSFORM_DIRTY : 0);
        flags |= (_contentSizeDirty ? FLAGS_CONTENT_SIZE_DIRTY : 0);


        if(flags & FLAGS_DIRTY_MASK)
            _modelViewTransform = this->transform(parentTransform);

        _transformUpdated = false;
        _contentSizeDirty = false;

        return flags;
    }

    bool Node::isVisitableByVisitingCamera() const
    {
        auto camera = Camera::getVisitingCamera();
        bool visibleByCamera = camera ? ((unsigned short)camera->getCameraFlag() & _cameraMask) != 0 : true;
        return visibleByCamera;
    }

    void Node::visit(Renderer* renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags)
    {
        if (!_visible)
        {
            return;
        }

        uint32_t flags = processParentFlags(parentTransform, parentFlags);

        _director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        _director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, _modelViewTransform);

        bool visibleByCamera = isVisitableByVisitingCamera();

        int i = 0;

        if(!_children.empty())
        {
            sortAllChildren();
            for( ; i < _children.size(); i++ )
            {
                auto node = _children.at(i);

                if (node && node->_localZOrder < 0)
                    node->visit(renderer, _modelViewTransform, flags);
                else
                    break;
            }

            if (visibleByCamera)
                this->draw(renderer, _modelViewTransform, flags);

            for(auto it=_children.cbegin()+i; it != _children.cend(); ++it)
                (*it)->visit(renderer, _modelViewTransform, flags);
        }
        else if (visibleByCamera)
        {
            this->draw(renderer, _modelViewTransform, flags);
        }

        _director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    }

    MATH::Matrix4 Node::transform(const MATH::Matrix4& parentTransform)
    {
        return parentTransform * this->getNodeToParentTransform();
    }

    void Node::onEnter()
    {
        if (_onEnterCallback)
            _onEnterCallback();

        _isTransitionFinished = false;

        for( const auto &child: _children)
            child->onEnter();

        this->resume();

        _running = true;
    }

    void Node::onEnterTransitionDidFinish()
    {
        if (_onEnterTransitionDidFinishCallback)
            _onEnterTransitionDidFinishCallback();

        _isTransitionFinished = true;
        for( const auto &child: _children)
            child->onEnterTransitionDidFinish();
    }

    void Node::onExitTransitionDidStart()
    {
        if (_onExitTransitionDidStartCallback)
            _onExitTransitionDidStartCallback();

        for( const auto &child: _children)
            child->onExitTransitionDidStart();
    }

    void Node::onExit()
    {
        if (_onExitCallback)
            _onExitCallback();

        this->pause();

        _running = false;

        for( const auto &child: _children)
            child->onExit();
    }

    void Node::setEventDispatcher(EventDispatcher* dispatcher)
    {
        if (dispatcher != _eventDispatcher)
        {
            _eventDispatcher->removeEventListenersForTarget(this);
            SAFE_RETAIN(dispatcher);
            SAFE_RELEASE(_eventDispatcher);
            _eventDispatcher = dispatcher;
        }
    }

    void Node::setActionManager(ActionManager* actionManager)
    {
        if( actionManager != _actionManager )
        {
            this->stopAllActions();
            SAFE_RETAIN(actionManager);
            SAFE_RELEASE(_actionManager);
            _actionManager = actionManager;
        }
    }

    Action * Node::runAction(Action* action)
    {
        _actionManager->addAction(action, this, !_running);
        return action;
    }

    void Node::stopAllActions()
    {
        _actionManager->removeAllActionsFromTarget(this);
    }

    void Node::stopAction(Action* action)
    {
        _actionManager->removeAction(action);
    }

    void Node::stopActionByTag(int tag)
    {
        _actionManager->removeActionByTag(tag, this);
    }

    void Node::stopAllActionsByTag(int tag)
    {
        _actionManager->removeAllActionsByTag(tag, this);
    }

    Action * Node::getActionByTag(int tag)
    {
        return _actionManager->getActionByTag(tag, this);
    }

    ssize_t Node::getNumberOfRunningActions() const
    {
        return _actionManager->getNumberOfRunningActionsInTarget(this);
    }

    void Node::setScheduler(Scheduler* scheduler)
    {
        if( scheduler != _scheduler )
        {
            this->unscheduleAllCallbacks();
            SAFE_RETAIN(scheduler);
            SAFE_RELEASE(_scheduler);
            _scheduler = scheduler;
        }
    }

    bool Node::isScheduled(SEL_SCHEDULE selector)
    {
        return _scheduler->isScheduled(selector, this);
    }

    bool Node::isScheduled(const std::string &key)
    {
        return _scheduler->isScheduled(key, this);
    }

    void Node::scheduleUpdate()
    {
        scheduleUpdateWithPriority(0);
    }

    void Node::scheduleUpdateWithPriority(int priority)
    {
        _scheduler->scheduleUpdate(this, priority, !_running);
    }

    void Node::scheduleUpdateWithPriorityLua(int, int priority)
    {
        unscheduleUpdate();

        _scheduler->scheduleUpdate(this, priority, !_running);
    }

    void Node::unscheduleUpdate()
    {
        _scheduler->unscheduleUpdate(this);
    }

    void Node::schedule(SEL_SCHEDULE selector)
    {
        this->schedule(selector, 0.0f, REPEAT_FOREVER, 0.0f);
    }

    void Node::schedule(SEL_SCHEDULE selector, float interval)
    {
        this->schedule(selector, interval, REPEAT_FOREVER, 0.0f);
    }

    void Node::schedule(SEL_SCHEDULE selector, float interval, unsigned int repeat, float delay)
    {
        _scheduler->schedule(selector, this, interval , repeat, delay, !_running);
    }

    void Node::schedule(const std::function<void(float)> &callback, const std::string &key)
    {
        _scheduler->schedule(callback, this, 0, !_running, key);
    }

    void Node::schedule(const std::function<void(float)> &callback, float interval, const std::string &key)
    {
        _scheduler->schedule(callback, this, interval, !_running, key);
    }

    void Node::schedule(const std::function<void(float)>& callback, float interval, unsigned int repeat, float delay, const std::string &key)
    {
        _scheduler->schedule(callback, this, interval, repeat, delay, !_running, key);
    }

    void Node::scheduleOnce(SEL_SCHEDULE selector, float delay)
    {
        this->schedule(selector, 0.0f, 0, delay);
    }

    void Node::scheduleOnce(const std::function<void(float)> &callback, float delay, const std::string &key)
    {
        _scheduler->schedule(callback, this, 0, 0, delay, !_running, key);
    }

    void Node::unschedule(SEL_SCHEDULE selector)
    {
        if (selector == nullptr)
            return;

        _scheduler->unschedule(selector, this);
    }

    void Node::unschedule(const std::string &key)
    {
        _scheduler->unschedule(key, this);
    }

    void Node::unscheduleAllCallbacks()
    {
        _scheduler->unscheduleAllForTarget(this);
    }

    void Node::resume()
    {
        _scheduler->resumeTarget(this);
        _actionManager->resumeTarget(this);
        _eventDispatcher->resumeEventListenersForTarget(this);
    }

    void Node::pause()
    {
        _scheduler->pauseTarget(this);
        _actionManager->pauseTarget(this);
        _eventDispatcher->pauseEventListenersForTarget(this);
    }

    void Node::update(float)
    {
    }

    MATH::AffineTransform Node::getNodeToParentAffineTransform() const
    {
        MATH::AffineTransform ret;
        GLToCGAffine(getNodeToParentTransform().m, &ret);

        return ret;
    }

    const MATH::Matrix4& Node::getNodeToParentTransform() const
    {
        if (_transformDirty)
        {
            float x = _position.x;
            float y = _position.y;
            float z = _positionZ;

            if (_ignoreAnchorPointForPosition)
            {
                x += _anchorPointInPoints.x;
                y += _anchorPointInPoints.y;
            }

            bool needsSkewMatrix = ( _skewX || _skewY );


            MATH::Vector2f anchorPoint(_anchorPointInPoints.x * _scaleX, _anchorPointInPoints.y * _scaleY);

            if (! needsSkewMatrix && !_anchorPointInPoints.isZero())
            {
                x += -anchorPoint.x;
                y += -anchorPoint.y;
            }

            MATH::Matrix4 translation;
            MATH::Matrix4::createTranslation(x + anchorPoint.x, y + anchorPoint.y, z, &translation);

            MATH::Matrix4::createRotation(_rotationQuat, &_transform);

            if (_rotationZ_X != _rotationZ_Y)
            {
                float radiansX = -MATH_DEGREES_TO_RADIANS(_rotationZ_X);
                float radiansY = -MATH_DEGREES_TO_RADIANS(_rotationZ_Y);
                float cx = cosf(radiansX);
                float sx = sinf(radiansX);
                float cy = cosf(radiansY);
                float sy = sinf(radiansY);

                float m0 = _transform.m[0], m1 = _transform.m[1], m4 = _transform.m[4], m5 = _transform.m[5], m8 = _transform.m[8], m9 = _transform.m[9];
                _transform.m[0] = cy * m0 - sx * m1, _transform.m[4] = cy * m4 - sx * m5, _transform.m[8] = cy * m8 - sx * m9;
                _transform.m[1] = sy * m0 + cx * m1, _transform.m[5] = sy * m4 + cx * m5, _transform.m[9] = sy * m8 + cx * m9;
            }
            _transform = translation * _transform;
            _transform.translate(-anchorPoint.x, -anchorPoint.y, 0);


            if (_scaleX != 1.f)
            {
                _transform.m[0] *= _scaleX, _transform.m[1] *= _scaleX, _transform.m[2] *= _scaleX;
            }
            if (_scaleY != 1.f)
            {
                _transform.m[4] *= _scaleY, _transform.m[5] *= _scaleY, _transform.m[6] *= _scaleY;
            }
            if (_scaleZ != 1.f)
            {
                _transform.m[8] *= _scaleZ, _transform.m[9] *= _scaleZ, _transform.m[10] *= _scaleZ;
            }

            if (needsSkewMatrix)
            {
                float skewMatArray[16] =
                {
                    1, (float)tanf(MATH_DEGREES_TO_RADIANS(_skewY)), 0, 0,
                    (float)tanf(MATH_DEGREES_TO_RADIANS(_skewX)), 1, 0, 0,
                    0,  0,  1, 0,
                    0,  0,  0, 1
                };
                MATH::Matrix4 skewMatrix(skewMatArray);

                _transform = _transform * skewMatrix;

                if (!_anchorPointInPoints.isZero())
                {
                    _transform.m[12] += _transform.m[0] * -_anchorPointInPoints.x + _transform.m[4] * -_anchorPointInPoints.y;
                    _transform.m[13] += _transform.m[1] * -_anchorPointInPoints.x + _transform.m[5] * -_anchorPointInPoints.y;
                }
            }

            if (_useAdditionalTransform)
            {
                _transform = _transform * _additionalTransform;
            }

            _transformDirty = false;
        }

        return _transform;
    }

    void Node::setNodeToParentTransform(const MATH::Matrix4& transform)
    {
        _transform = transform;
        _transformDirty = false;
        _transformUpdated = true;
    }

    void Node::setAdditionalTransform(const MATH::AffineTransform& additionalTransform)
    {
        MATH::Matrix4 tmp;
        CGAffineToGL(additionalTransform, tmp.m);
        setAdditionalTransform(&tmp);
    }

    void Node::setAdditionalTransform(MATH::Matrix4* additionalTransform)
    {
        if (additionalTransform == nullptr)
        {
            _useAdditionalTransform = false;
        }
        else
        {
            _additionalTransform = *additionalTransform;
            _useAdditionalTransform = true;
        }
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }

    Component* Node::getComponent(const std::string& name)
    {
        if (_componentContainer)
            return _componentContainer->get(name);

        return nullptr;
    }

    bool Node::addComponent(Component *component)
    {
        // lazy alloc
        if (!_componentContainer)
            _componentContainer = new (std::nothrow) ComponentContainer(this);

        return _componentContainer->add(component);
    }

    bool Node::removeComponent(const std::string& name)
    {
        if (_componentContainer)
            return _componentContainer->remove(name);

        return false;
    }

    bool Node::removeComponent(Component *component)
    {
        if (_componentContainer)
        {
            return _componentContainer->remove(component);
        }

        return false;
    }

    void Node::removeAllComponents()
    {
        if (_componentContainer)
            _componentContainer->removeAll();
    }

    MATH::AffineTransform Node::getParentToNodeAffineTransform() const
    {
        MATH::AffineTransform ret;

        GLToCGAffine(getParentToNodeTransform().m,&ret);
        return ret;
    }

    const MATH::Matrix4& Node::getParentToNodeTransform() const
    {
        if ( _inverseDirty )
        {
            _inverse = getNodeToParentTransform().getInversed();
            _inverseDirty = false;
        }

        return _inverse;
    }


    MATH::AffineTransform Node::getNodeToWorldAffineTransform() const
    {
        MATH::AffineTransform t(this->getNodeToParentAffineTransform());

        for (Node *p = _parent; p != nullptr; p = p->getParent())
            t = AffineTransformConcat(t, p->getNodeToParentAffineTransform());

        return t;
    }

    MATH::Matrix4 Node::getNodeToWorldTransform() const
    {
        MATH::Matrix4 t(this->getNodeToParentTransform());

        for (Node *p = _parent; p != nullptr; p = p->getParent())
        {
            t = p->getNodeToParentTransform() * t;
        }

        return t;
    }

    MATH::AffineTransform Node::getWorldToNodeAffineTransform() const
    {
        return AffineTransformInvert(this->getNodeToWorldAffineTransform());
    }

    MATH::Matrix4 Node::getWorldToNodeTransform() const
    {
        return getNodeToWorldTransform().getInversed();
    }


    MATH::Vector2f Node::convertToNodeSpace(const MATH::Vector2f& worldPoint) const
    {
        MATH::Matrix4 tmp = getWorldToNodeTransform();
        MATH::Vector3f vec3(worldPoint.x, worldPoint.y, 0);
        MATH::Vector3f ret;
        tmp.transformPoint(vec3,&ret);
        return MATH::Vector2f(ret.x, ret.y);
    }

    MATH::Vector2f Node::convertToWorldSpace(const MATH::Vector2f& nodePoint) const
    {
        MATH::Matrix4 tmp = getNodeToWorldTransform();
        MATH::Vector3f vec3(nodePoint.x, nodePoint.y, 0);
        MATH::Vector3f ret;
        tmp.transformPoint(vec3,&ret);
        return MATH::Vector2f(ret.x, ret.y);

    }

    MATH::Vector2f Node::convertToNodeSpaceAR(const MATH::Vector2f& worldPoint) const
    {
        MATH::Vector2f nodePoint(convertToNodeSpace(worldPoint));
        return nodePoint - _anchorPointInPoints;
    }

    MATH::Vector2f Node::convertToWorldSpaceAR(const MATH::Vector2f& nodePoint) const
    {
        return convertToWorldSpace(nodePoint + _anchorPointInPoints);
    }

    MATH::Vector2f Node::convertToWindowSpace(const MATH::Vector2f& nodePoint) const
    {
        MATH::Vector2f worldPoint(this->convertToWorldSpace(nodePoint));
        return _director->convertToUI(worldPoint);
    }

    MATH::Vector2f Node::convertTouchToNodeSpace(Touch *touch) const
    {
        return this->convertToNodeSpace(touch->getLocation());
    }

    MATH::Vector2f Node::convertTouchToNodeSpaceAR(Touch *touch) const
    {
        MATH::Vector2f point = touch->getLocation();
        return this->convertToNodeSpaceAR(point);
    }

    void Node::updateTransform()
    {
        for( const auto &child: _children)
            child->updateTransform();
    }

    GLubyte Node::getOpacity(void) const
    {
        return _realOpacity;
    }

    GLubyte Node::getDisplayedOpacity() const
    {
        return _displayedOpacity;
    }

    void Node::setOpacity(GLubyte opacity)
    {
        _displayedOpacity = _realOpacity = opacity;

        updateCascadeOpacity();
    }

    void Node::updateDisplayedOpacity(GLubyte parentOpacity)
    {
        _displayedOpacity = _realOpacity * parentOpacity/255.0;
        updateColor();

        if (_cascadeOpacityEnabled)
        {
            for(const auto& child : _children)
            {
                child->updateDisplayedOpacity(_displayedOpacity);
            }
        }
    }

    bool Node::isCascadeOpacityEnabled(void) const
    {
        return _cascadeOpacityEnabled;
    }

    void Node::setCascadeOpacityEnabled(bool cascadeOpacityEnabled)
    {
        if (_cascadeOpacityEnabled == cascadeOpacityEnabled)
        {
            return;
        }

        _cascadeOpacityEnabled = cascadeOpacityEnabled;

        if (cascadeOpacityEnabled)
        {
            updateCascadeOpacity();
        }
        else
        {
            disableCascadeOpacity();
        }
    }

    void Node::updateCascadeOpacity()
    {
        GLubyte parentOpacity = 255;

        if (_parent != nullptr && _parent->isCascadeOpacityEnabled())
        {
            parentOpacity = _parent->getDisplayedOpacity();
        }

        updateDisplayedOpacity(parentOpacity);
    }

    void Node::disableCascadeOpacity()
    {
        _displayedOpacity = _realOpacity;

        for(const auto& child : _children)
        {
            child->updateDisplayedOpacity(255);
        }
    }

    const Color3B& Node::getColor(void) const
    {
        return _realColor;
    }

    const Color3B& Node::getDisplayedColor() const
    {
        return _displayedColor;
    }

    void Node::setColor(const Color3B& color)
    {
        _displayedColor = _realColor = color;

        updateCascadeColor();
    }

    void Node::updateDisplayedColor(const Color3B& parentColor)
    {
        _displayedColor.red = _realColor.red * parentColor.red/255.0;
        _displayedColor.green = _realColor.green * parentColor.green/255.0;
        _displayedColor.blue = _realColor.blue * parentColor.blue/255.0;
        updateColor();

        if (_cascadeColorEnabled)
        {
            for(const auto &child : _children)
            {
                child->updateDisplayedColor(_displayedColor);
            }
        }
    }

    bool Node::isCascadeColorEnabled(void) const
    {
        return _cascadeColorEnabled;
    }

    void Node::setCascadeColorEnabled(bool cascadeColorEnabled)
    {
        if (_cascadeColorEnabled == cascadeColorEnabled)
        {
            return;
        }

        _cascadeColorEnabled = cascadeColorEnabled;

        if (_cascadeColorEnabled)
        {
            updateCascadeColor();
        }
        else
        {
            disableCascadeColor();
        }
    }

    void Node::updateCascadeColor()
    {
        Color3B parentColor = Color3B::WHITE;
        if (_parent && _parent->isCascadeColorEnabled())
        {
            parentColor = _parent->getDisplayedColor();
        }

        updateDisplayedColor(parentColor);
    }

    void Node::disableCascadeColor()
    {
        for(const auto& child : _children)
        {
            child->updateDisplayedColor(Color3B::WHITE);
        }
    }

    void Node::setCameraMask(unsigned short mask, bool applyChildren)
    {
        _cameraMask = mask;
        if (applyChildren)
        {
            for (const auto& child : _children)
            {
                child->setCameraMask(mask, applyChildren);
            }
        }
    }

    ProtectedNode::ProtectedNode() : _reorderProtectedChildDirty(false)
    {
    }

    ProtectedNode::~ProtectedNode()
    {
    }

    ProtectedNode * ProtectedNode::create(void)
    {
        ProtectedNode * ret = new (std::nothrow) ProtectedNode();
        if (ret && ret->init())
        {
            ret->autorelease();
        }
        else
        {
            SAFE_DELETE(ret);
        }
        return ret;
    }

    void ProtectedNode::cleanup()
    {
        Node::cleanup();
        for( const auto &child: _protectedChildren)
            child->cleanup();
    }

    void ProtectedNode::addProtectedChild(Node *child)
    {
        addProtectedChild(child, child->getLocalZOrder(), child->getTag());
    }

    void ProtectedNode::addProtectedChild(Node *child, int localZOrder)
    {
        addProtectedChild(child, localZOrder, child->getTag());
    }

    void ProtectedNode::addProtectedChild(Node *child, int zOrder, int tag)
    {
        if (_protectedChildren.empty())
        {
            _protectedChildren.reserve(4);
        }

        this->insertProtectedChild(child, zOrder);

        child->setTag(tag);

        child->setParent(this);
        child->setOrderOfArrival(s_globalOrderOfArrival++);

        if( _running )
        {
            child->onEnter();
            if (_isTransitionFinished) {
                child->onEnterTransitionDidFinish();
            }
        }

        if (_cascadeColorEnabled)
        {
            updateCascadeColor();
        }

        if (_cascadeOpacityEnabled)
        {
            updateCascadeOpacity();
        }
    }

    Node* ProtectedNode::getProtectedChildByTag(int tag)
    {
        for (auto& child : _protectedChildren)
        {
            if(child && child->getTag() == tag)
                return child;
        }
        return nullptr;
    }

    void ProtectedNode::removeProtectedChild(Node *child, bool cleanup)
    {
        if (_protectedChildren.empty())
        {
            return;
        }

        ssize_t index = _protectedChildren.getIndex(child);
        if( index != -1 )
        {
            if (_running)
            {
                child->onExitTransitionDidStart();
                child->onExit();
            }

            if (cleanup)
            {
                child->cleanup();
            }

            child->setParent(nullptr);

            _protectedChildren.erase(index);
        }
    }

    void ProtectedNode::removeAllProtectedChildren()
    {
        removeAllProtectedChildrenWithCleanup(true);
    }

    void ProtectedNode::removeAllProtectedChildrenWithCleanup(bool cleanup)
    {
        for (auto& child : _protectedChildren)
        {
            if(_running)
            {
                child->onExitTransitionDidStart();
                child->onExit();
            }

            if (cleanup)
            {
                child->cleanup();
            }

            child->setParent(nullptr);
        }

        _protectedChildren.clear();
    }

    void ProtectedNode::removeProtectedChildByTag(int tag, bool cleanup)
    {
        Node *child = this->getProtectedChildByTag(tag);

        if (child != nullptr)
        {
            this->removeProtectedChild(child, cleanup);
        }
    }

    void ProtectedNode::insertProtectedChild(Node *child, int z)
    {
        _reorderProtectedChildDirty = true;
        _protectedChildren.pushBack(child);
        child->setLocalZOrder(z);
    }

    void ProtectedNode::sortAllProtectedChildren()
    {
        if( _reorderProtectedChildDirty ) {
            std::sort( std::begin(_protectedChildren), std::end(_protectedChildren), nodeComparisonLess );
            _reorderProtectedChildDirty = false;
        }
    }

    void ProtectedNode::reorderProtectedChild(Node *child, int localZOrder)
    {
        _reorderProtectedChildDirty = true;
        child->setOrderOfArrival(s_globalOrderOfArrival++);
        child->setLocalZOrder(localZOrder);
    }

    void ProtectedNode::visit(Renderer* renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags)
    {
        if (!_visible)
        {
            return;
        }

        uint32_t flags = processParentFlags(parentTransform, parentFlags);

        Director* director = Director::getInstance();
        director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, _modelViewTransform);

        int i = 0;      // used by _children
        int j = 0;      // used by _protectedChildren

        sortAllChildren();
        sortAllProtectedChildren();

        for( ; i < _children.size(); i++ )
        {
            auto node = _children.at(i);

            if ( node && node->getLocalZOrder() < 0 )
                node->visit(renderer, _modelViewTransform, flags);
            else
                break;
        }

        for( ; j < _protectedChildren.size(); j++ )
        {
            auto node = _protectedChildren.at(j);

            if ( node && node->getLocalZOrder() < 0 )
                node->visit(renderer, _modelViewTransform, flags);
            else
                break;
        }

        if (isVisitableByVisitingCamera())
            this->draw(renderer, _modelViewTransform, flags);

        for(auto it=_protectedChildren.cbegin()+j; it != _protectedChildren.cend(); ++it)
            (*it)->visit(renderer, _modelViewTransform, flags);

        for(auto it=_children.cbegin()+i; it != _children.cend(); ++it)
            (*it)->visit(renderer, _modelViewTransform, flags);

        director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    }

    void ProtectedNode::onEnter()
    {
        Node::onEnter();
        for( const auto &child: _protectedChildren)
            child->onEnter();
    }

    void ProtectedNode::onEnterTransitionDidFinish()
    {
        Node::onEnterTransitionDidFinish();
        for( const auto &child: _protectedChildren)
            child->onEnterTransitionDidFinish();
    }

    void ProtectedNode::onExitTransitionDidStart()
    {
        Node::onExitTransitionDidStart();
        for( const auto &child: _protectedChildren)
            child->onExitTransitionDidStart();
    }

    void ProtectedNode::onExit()
    {
        Node::onExit();
        for( const auto &child: _protectedChildren)
            child->onExit();
    }

    void ProtectedNode::updateDisplayedOpacity(GLubyte parentOpacity)
    {
        _displayedOpacity = _realOpacity * parentOpacity/255.0;
        updateColor();

        if (_cascadeOpacityEnabled)
        {
            for(auto child : _children){
                child->updateDisplayedOpacity(_displayedOpacity);
            }
        }

        for(auto child : _protectedChildren){
            child->updateDisplayedOpacity(_displayedOpacity);
        }
    }

    void ProtectedNode::updateDisplayedColor(const Color3B& parentColor)
    {
        _displayedColor.red = _realColor.red * parentColor.red/255.0;
        _displayedColor.green = _realColor.green * parentColor.green/255.0;
        _displayedColor.blue = _realColor.blue * parentColor.blue/255.0;
        updateColor();

        if (_cascadeColorEnabled)
        {
            for(const auto &child : _children){
                child->updateDisplayedColor(_displayedColor);
            }
        }
        for(const auto &child : _protectedChildren){
            child->updateDisplayedColor(_displayedColor);
        }
    }

    void ProtectedNode::disableCascadeColor()
    {
        for(auto child : _children){
            child->updateDisplayedColor(Color3B::WHITE);
        }
        for(auto child : _protectedChildren){
            child->updateDisplayedColor(Color3B::WHITE);
        }
    }

    void ProtectedNode::disableCascadeOpacity()
    {
        _displayedOpacity = _realOpacity;

        for(auto child : _children){
            child->updateDisplayedOpacity(255);
        }

        for(auto child : _protectedChildren){
            child->updateDisplayedOpacity(255);
        }
    }

    void ProtectedNode::setCameraMask(unsigned short mask, bool applyChildren)
    {
        Node::setCameraMask(mask, applyChildren);
        if (applyChildren)
        {
            for (auto& iter: _protectedChildren)
            {
                iter->setCameraMask(mask);
            }
        }

    }

    bool isScreenPointInRect(const MATH::Vector2f &pt, const Camera* camera, const MATH::Matrix4& w2l, const MATH::Rectf& rect, MATH::Vector3f *p)
    {
        if (nullptr == camera || rect.size.width <= 0 || rect.size.height <= 0)
        {
            return false;
        }

        MATH::Vector3f Pn(pt.x, pt.y, -1), Pf(pt.x, pt.y, 1);
        Pn = camera->unprojectGL(Pn);
        Pf = camera->unprojectGL(Pf);

        w2l.transformPoint(&Pn);
        w2l.transformPoint(&Pf);

        auto E = Pf - Pn;

        MATH::Vector3f A = MATH::Vector3f(rect.origin.x, rect.origin.y, 0);
        MATH::Vector3f B(rect.size.width, 0, 0);
        MATH::Vector3f C(0, rect.size.height, 0);
        B = B - A;
        C = C - A;

        MATH::Vector3f BxC;
        MATH::Vector3f::cross(B, C, &BxC);
        auto BxCdotE = BxC.dot(E);
        if (BxCdotE == 0) {
            return false;
        }
        auto t = (BxC.dot(A) - BxC.dot(Pn)) / BxCdotE;
        MATH::Vector3f P = Pn + t * E;
        if (p) {
            *p = P;
        }
        return rect.contains(MATH::Vector2f(P.x, P.y));
    }
}
