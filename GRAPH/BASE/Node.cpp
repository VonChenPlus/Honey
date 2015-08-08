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
    // children (lazy allocs)
    // lazy alloc
    , _localZOrder(0)
    , _globalZOrder(0)
    , _parent(nullptr)
    // "whole screen" objects. like Scenes and Layers, should set _ignoreAnchorPointForPosition to true
    , _tag(Node::INVALID_TAG)
    , _name("")
    , _hashOfName(0)
    // userData is always inited as nil
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
        // set default scheduler and actionManager
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
        // User object has to be released before others, since userObject may have a weak reference of this node
        // It may invoke `node->stopAllAction();` while `_actionManager` is null if the next line is after `SAFE_RELEASE_NULL(_actionManager)`.
        SAFE_RELEASE_NULL(_userObject);

        // attributes
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
        // actions
        this->stopAllActions();
        this->unscheduleAllCallbacks();

        // timers
        for( const auto &child: _children)
            child->cleanup();
    }

    std::string Node::getDescription() const
    {
        return UTILS::STRING::StringFromFormat("<Node | Tag = %d", _tag);
    }

    // MARK: getters / setters

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

    /// rotation getter
    float Node::getRotation() const
    {
        return _rotationZ_X;
    }

    /// rotation setter
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

        // rotation Z is decomposed in 2 to simulate Skew for Flash animations
        _rotationZ_Y = _rotationZ_X = rotation.z;

        updateRotationQuat();
    }

    MATH::Vector3f Node::getRotation3D() const
    {
        return MATH::Vector3f(_rotationX,_rotationY,_rotationZ_X);
    }

    void Node::updateRotationQuat()
    {
        // convert Euler angle to quaternion
        // when _rotationZ_X == _rotationZ_Y, _rotationQuat = RotationZ_X * RotationY * RotationX
        // when _rotationZ_X != _rotationZ_Y, _rotationQuat = RotationY * RotationX
        float halfRadx = MATH_DEGREES_TO_RADIANS(_rotationX / 2.f), halfRady = MATH_DEGREES_TO_RADIANS(_rotationY / 2.f), halfRadz = _rotationZ_X == _rotationZ_Y ? -MATH_DEGREES_TO_RADIANS(_rotationZ_X / 2.f) : 0;
        float coshalfRadx = cosf(halfRadx), sinhalfRadx = sinf(halfRadx), coshalfRady = cosf(halfRady), sinhalfRady = sinf(halfRady), coshalfRadz = cosf(halfRadz), sinhalfRadz = sinf(halfRadz);
        _rotationQuat.x = sinhalfRadx * coshalfRady * coshalfRadz - coshalfRadx * sinhalfRady * sinhalfRadz;
        _rotationQuat.y = coshalfRadx * sinhalfRady * coshalfRadz + sinhalfRadx * coshalfRady * sinhalfRadz;
        _rotationQuat.z = coshalfRadx * coshalfRady * sinhalfRadz - sinhalfRadx * sinhalfRady * coshalfRadz;
        _rotationQuat.w = coshalfRadx * coshalfRady * coshalfRadz + sinhalfRadx * sinhalfRady * sinhalfRadz;
    }

    void Node::updateRotation3D()
    {
        //convert quaternion to Euler angle
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

    /// scale getter
    float Node::getScale(void) const
    {
        return _scaleX;
    }

    /// scale setter
    void Node::setScale(float scale)
    {
        if (_scaleX == scale && _scaleY == scale && _scaleZ == scale)
            return;

        _scaleX = _scaleY = _scaleZ = scale;
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }

    /// scaleX getter
    float Node::getScaleX() const
    {
        return _scaleX;
    }

    /// scale setter
    void Node::setScale(float scaleX,float scaleY)
    {
        if (_scaleX == scaleX && _scaleY == scaleY)
            return;

        _scaleX = scaleX;
        _scaleY = scaleY;
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }

    /// scaleX setter
    void Node::setScaleX(float scaleX)
    {
        if (_scaleX == scaleX)
            return;

        _scaleX = scaleX;
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }

    /// scaleY getter
    float Node::getScaleY() const
    {
        return _scaleY;
    }

    /// scaleY setter
    void Node::setScaleZ(float scaleZ)
    {
        if (_scaleZ == scaleZ)
            return;

        _scaleZ = scaleZ;
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }

    /// scaleY getter
    float Node::getScaleZ() const
    {
        return _scaleZ;
    }

    /// scaleY setter
    void Node::setScaleY(float scaleY)
    {
        if (_scaleY == scaleY)
            return;

        _scaleY = scaleY;
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }


    /// position getter
    const MATH::Vector2f& Node::getPosition() const
    {
        return _position;
    }

    /// position setter
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

    /// position getter
    const MATH::Vector2f& Node::getNormalizedPosition() const
    {
        return _normalizedPosition;
    }

    /// position setter
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

    /// isVisible getter
    bool Node::isVisible() const
    {
        return _visible;
    }

    /// isVisible setter
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

    /// anchorPoint getter
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

    /// contentSize getter
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

    // isRunning getter
    bool Node::isRunning() const
    {
        return _running;
    }

    /// parent setter
    void Node::setParent(Node * parent)
    {
        _parent = parent;
        _transformUpdated = _transformDirty = _inverseDirty = true;
    }

    /// isRelativeAnchorPoint getter
    bool Node::isIgnoreAnchorPointForPosition() const
    {
        return _ignoreAnchorPointForPosition;
    }
    /// isRelativeAnchorPoint setter
    void Node::ignoreAnchorPointForPosition(bool newValue)
    {
        if (newValue != _ignoreAnchorPointForPosition)
        {
            _ignoreAnchorPointForPosition = newValue;
            _transformUpdated = _transformDirty = _inverseDirty = true;
        }
    }

    /// tag getter
    int Node::getTag() const
    {
        return _tag;
    }

    /// tag setter
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

    /// userData setter
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

    // MARK: Children logic

    // lazy allocs
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
            // Different strings may have the same hash code, but can use it to compare first for speed
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

        // Starts with '//'?
        bool searchRecursively = false;
        if (length > 2 && name[0] == '/' && name[1] == '/')
        {
            searchRecursively = true;
            subStrStartPos = 2;
            subStrlength -= 2;
        }

        // End with '/..'?
        bool searchFromParent = false;
        if (length > 3 &&
            name[length-3] == '/' &&
            name[length-2] == '.' &&
            name[length-1] == '.')
        {
            searchFromParent = true;
            subStrlength -= 3;
        }

        // Remove '//', '/..' if exist
        std::string newName = name.substr(subStrStartPos, subStrlength);

        if (searchFromParent)
        {
            newName.insert(0, "[[:alnum:]]+/");
        }


        if (searchRecursively)
        {
            // name is '//xxx'
            doEnumerateRecursive(this, newName, callback);
        }
        else
        {
            // name is xxx
            doEnumerate(newName, callback);
        }
    }

    bool Node::doEnumerateRecursive(const Node* node, const std::string &name, std::function<bool (Node *)> callback) const
    {
        bool ret =false;

        if (node->doEnumerate(name, callback))
        {
            // search itself
            ret = true;
        }
        else
        {
            // search its children
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
        // name may be xxx/yyy, should find its parent
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
                    // terminate enumeration if callback return true
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

    /* "add" logic MUST only be on this method
    * If a class want's to extend the 'addChild' behavior it only needs
    * to override this method
    */
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
            // prevent onEnterTransitionDidFinish to be called twice when a node is added in onEnter
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

    /* "remove" logic MUST only be on this method
    * If a class want's to extend the 'removeChild' behavior it only needs
    * to override this method
    */
    void Node::removeChild(Node* child, bool cleanup /* = true */)
    {
        // explicit nil handling
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
        // not using detachChild improves speed here
        for (const auto& child : _children)
        {
            // IMPORTANT:
            //  -1st do onExit
            //  -2nd cleanup
            if(_running)
            {
                child->onExitTransitionDidStart();
                child->onExit();
            }

            if (cleanup)
            {
                child->cleanup();
            }
            // set parent nil at the end
            child->setParent(nullptr);
        }

        _children.clear();
    }

    void Node::detachChild(Node *child, ssize_t childIndex, bool doCleanup)
    {
        // IMPORTANT:
        //  -1st do onExit
        //  -2nd cleanup
        if (_running)
        {
            child->onExitTransitionDidStart();
            child->onExit();
        }

        // If you don't do cleanup, the child's actions will not get removed and the
        // its scheduledSelectors_ dict will not get released!
        if (doCleanup)
        {
            child->cleanup();
        }

        // set parent nil at the end
        child->setParent(nullptr);

        _children.erase(childIndex);
    }


    // helper used by reorderChild & add
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

    // MARK: draw / visit

    void Node::draw()
    {
        auto renderer = _director->getRenderer();
        draw(renderer, _modelViewTransform, true);
    }

    void Node::draw(Renderer* renderer, const MATH::Matrix4 &transform, uint32_t flags)
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

        //remove this two line given that isVisitableByVisitingCamera should not affect the calculation of transform given that we are visiting scene
        //without involving view and projection matrix.

    //    if (!isVisitableByVisitingCamera())
    //        return parentFlags;

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
        // quick return if not visible. children won't be drawn.
        if (!_visible)
        {
            return;
        }

        uint32_t flags = processParentFlags(parentTransform, parentFlags);

        // IMPORTANT:
        // To ease the migration to v3.0, we still support the MATH::Matrix4 stack,
        // but it is deprecated and your code should not rely on it
        _director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        _director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, _modelViewTransform);

        bool visibleByCamera = isVisitableByVisitingCamera();

        int i = 0;

        if(!_children.empty())
        {
            sortAllChildren();
            // draw children zOrder < 0
            for( ; i < _children.size(); i++ )
            {
                auto node = _children.at(i);

                if (node && node->_localZOrder < 0)
                    node->visit(renderer, _modelViewTransform, flags);
                else
                    break;
            }
            // self draw
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

        // FIX ME: Why need to set _orderOfArrival to 0??
        // Please refer to https://github.com/cocos2d/cocos2d-x/pull/6920
        // reset for next frame
        // _orderOfArrival = 0;
    }

    MATH::Matrix4 Node::transform(const MATH::Matrix4& parentTransform)
    {
        return parentTransform * this->getNodeToParentTransform();
    }

    // MARK: events

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

    // MARK: actions

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

    // MARK: Callbacks

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

    void Node::scheduleUpdateWithPriorityLua(int nHandler, int priority)
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
        // explicit null handling
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

    // override me
    void Node::update(float fDelta)
    {
    }

    // MARK: coordinates

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
            // Translate values
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

            // caculate real position
            if (! needsSkewMatrix && !_anchorPointInPoints.isZero())
            {
                x += -anchorPoint.x;
                y += -anchorPoint.y;
            }

            // Build Transform Matrix = translation * rotation * scale
            MATH::Matrix4 translation;
            //move to anchor point first, then rotate
            MATH::Matrix4::createTranslation(x + anchorPoint.x, y + anchorPoint.y, z, &translation);

            MATH::Matrix4::createRotation(_rotationQuat, &_transform);

            if (_rotationZ_X != _rotationZ_Y)
            {
                // Rotation values
                // Change rotation code to handle X and Y
                // If we skew with the exact same value for both x and y then we're simply just rotating
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
            //move by (-anchorPoint.x, -anchorPoint.y, 0) after rotation
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

            // FIXME:: Try to inline skew
            // If skew is needed, apply skew and then anchor point
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

                // adjust anchor point
                if (!_anchorPointInPoints.isZero())
                {
                    // FIXME:: Argh, MATH::Matrix4 needs a "translate" method.
                    // FIXME:: Although this is faster than multiplying a vec4 * MATH::Matrix4
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

    // convenience methods which take a Touch instead of MATH::Vector2f
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
        // Recursively iterate over children
        for( const auto &child: _children)
            child->updateTransform();
    }

    // MARK: Opacity and Color

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
        _displayedColor.r = _realColor.r * parentColor.r/255.0;
        _displayedColor.g = _realColor.g * parentColor.g/255.0;
        _displayedColor.b = _realColor.b * parentColor.b/255.0;
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

    bool isScreenPointInRect(const MATH::Vector2f &pt, const Camera* camera, const MATH::Matrix4& w2l, const MATH::Rectf& rect, MATH::Vector3f *p)
    {
        if (nullptr == camera || rect.size.width <= 0 || rect.size.height <= 0)
        {
            return false;
        }

        // first, convert pt to near/far plane, get Pn and Pf
        MATH::Vector3f Pn(pt.x, pt.y, -1), Pf(pt.x, pt.y, 1);
        Pn = camera->unprojectGL(Pn);
        Pf = camera->unprojectGL(Pf);

        //  then convert Pn and Pf to node space
        w2l.transformPoint(&Pn);
        w2l.transformPoint(&Pf);

        // Pn and Pf define a line Q(t) = D + t * E which D = Pn
        auto E = Pf - Pn;

        // second, get three points which define content plane
        //  these points define a plane P(u, w) = A + uB + wC
        MATH::Vector3f A = MATH::Vector3f(rect.origin.x, rect.origin.y, 0);
        MATH::Vector3f B(rect.size.width, 0, 0);
        MATH::Vector3f C(0, rect.size.height, 0);
        B = B - A;
        C = C - A;

        //  the line Q(t) intercept with plane P(u, w)
        //  calculate the intercept point P = Q(t)
        //      (BxC).A - (BxC).D
        //  t = -----------------
        //          (BxC).E
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

    // MARK: Camera
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
}
