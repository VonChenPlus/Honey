#include <algorithm>
#include <string>
#include <regex>
#include "GRAPH/Node.h"
#include "GRAPH/Scene.h"
#include "GRAPH/Director.h"
#include "GRAPH/Action.h"
#include "GRAPH/Scheduler.h"
#include "GRAPH/EventDispatcher.h"
#include "GRAPH/Component.h"
#include "GRAPH/Camera.h"
#include "GRAPH/UNITY3D/ShaderState.h"

namespace GRAPH
{
    bool Node::nodeComparisonLess(Node* n1, Node* n2) {
        return( n1->getLocalZOrder() < n2->getLocalZOrder() ||
               ( n1->getLocalZOrder() == n2->getLocalZOrder() && n1->getOrderOfArrival() < n2->getOrderOfArrival() )
               );
    }

    bool Node::isScreenPointInRect(const MATH::Vector2f &pt, const Camera* camera, const MATH::Matrix4& w2l,
                                            const MATH::Rectf& rect, MATH::Vector3f *p) {
        if (rect.size.width <= 0 || rect.size.height <= 0) {
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
        MATH::Vector3f B(rect.origin.x + rect.size.width, rect.origin.y, 0);
        MATH::Vector3f C(rect.origin.x, rect.origin.y + rect.size.height, 0);
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

    void CGAffineToGL(const MATH::AffineTransform& t, float *m) {
        // | m[0] m[4] m[8]  m[12] |     | m11 m21 m31 m41 |     | a c 0 tx |
        // | m[1] m[5] m[9]  m[13] |     | m12 m22 m32 m42 |     | b d 0 ty |
        // | m[2] m[6] m[10] m[14] | <=> | m13 m23 m33 m43 | <=> | 0 0 1  0 |
        // | m[3] m[7] m[11] m[15] |     | m14 m24 m34 m44 |     | 0 0 0  1 |

        m[2] = m[3] = m[6] = m[7] = m[8] = m[9] = m[11] = m[14] = 0.0f;
        m[10] = m[15] = 1.0f;
        m[0] = t.a; m[4] = t.c; m[12] = t.tx;
        m[1] = t.b; m[5] = t.d; m[13] = t.ty;
    }

    void GLToCGAffine(const float *m, MATH::AffineTransform *t) {
        t->a = m[0]; t->c = m[4]; t->tx = m[12];
        t->b = m[1]; t->d = m[5]; t->ty = m[13];
    }

    // FIXME:: Yes, nodes might have a sort problem once every 15 days if the game runs at 60 FPS and each frame sprites are reordered.
    int Node::s_globalOrderOfArrival = 1;

    // MARK: Constructor, Destructor, Init

    Node::Node(void)
        : rotationX_(0.0f)
        , rotationY_(0.0f)
        , rotationZ_X_(0.0f)
        , rotationZ_Y_(0.0f)
        , scaleX_(1.0f)
        , scaleY_(1.0f)
        , scaleZ_(1.0f)
        , positionZ_(0.0f)
        , usingNormalizedPosition_(false)
        , normalizedPositionDirty_(false)
        , skewX_(0.0f)
        , skewY_(0.0f)
        , contentSize_(MATH::SizefZERO)
        , contentSizeDirty_(true)
        , transformDirty_(true)
        , inverseDirty_(true)
        , useAdditionalTransform_(false)
        , transformUpdated_(true)
        , localZOrder_(0)
        , globalZOrder_(0)
        , parent_(nullptr)
        , tag_(Node::INVALID_TAG)
        , name_("")
        , hashOfName_(0)
        , userData_(nullptr)
        , userObject_(nullptr)
        , shaderState_(nullptr)
        , orderOfArrival_(0)
        , running_(false)
        , visible_(true)
        , ignoreAnchorPointForPosition_(false)
        , reorderChildDirty_(false)
        , isTransitionFinished_(false)
        , displayedOpacity_(255)
        , realOpacity_(255)
        , displayedColor_(Color3B::WHITE)
        , realColor_(Color3B::WHITE)
        , cascadeColorEnabled_(false)
        , cascadeOpacityEnabled_(false)
        , cameraMask_(1) {
        director_ = &Director::getInstance();
        actionManager_ = director_->getActionManager();
        actionManager_->retain();
        scheduler_ = director_->getScheduler();
        scheduler_->retain();
        eventDispatcher_ = director_->getEventDispatcher();
        eventDispatcher_->retain();
        transform_ = inverse_ = additionalTransform_ = MATH::Matrix4::IDENTITY;
    }

    Node * Node::create() {
        Node * ret = new (std::nothrow) Node();
        if (ret && ret->init()) {
            ret->autorelease();
        }
        else {
            SAFE_DELETE(ret);
        }
        return ret;
    }

    Node::~Node() {
        SAFE_RELEASE_NULL(userObject_);
        SAFE_RELEASE_NULL(shaderState_);

        for (auto& child : children_) {
            child->parent_ = nullptr;
        }

        stopAllActions();
        unscheduleAllCallbacks();
        SAFE_RELEASE_NULL(actionManager_);
        SAFE_RELEASE_NULL(scheduler_);

        eventDispatcher_->removeEventListenersForTarget(this);

        SAFE_RELEASE(eventDispatcher_);
    }

    bool Node::init() {
        return true;
    }

    void Node::cleanup() {
        this->stopAllActions();
        this->unscheduleAllCallbacks();

        for( const auto &child: children_)
            child->cleanup();
    }

    float Node::getSkewX() const {
        return skewX_;
    }

    void Node::setSkewX(float skewX) {
        if (skewX_ == skewX)
            return;

        skewX_ = skewX;
        transformUpdated_ = transformDirty_ = inverseDirty_ = true;
    }

    float Node::getSkewY() const {
        return skewY_;
    }

    void Node::setSkewY(float skewY) {
        if (skewY_ == skewY)
            return;

        skewY_ = skewY;
        transformUpdated_ = transformDirty_ = inverseDirty_ = true;
    }

    void Node::setLocalZOrder(int z) {
        if (localZOrder_ == z)
            return;

        localZOrder_ = z;
        if (parent_) {
            parent_->reorderChild(this, z);
        }

        eventDispatcher_->setDirtyForNode(this);
    }

    void Node::setGlobalZOrder(float globalZOrder) {
        if (globalZOrder_ != globalZOrder) {
            globalZOrder_ = globalZOrder;
            eventDispatcher_->setDirtyForNode(this);
        }
    }

    float Node::getRotation() const {
        return rotationZ_X_;
    }

    void Node::setRotation(float rotation) {
        if (rotationZ_X_ == rotation)
            return;

        rotationZ_X_ = rotationZ_Y_ = rotation;
        transformUpdated_ = transformDirty_ = inverseDirty_ = true;
        updateRotationQuat();
    }

    float Node::getRotationSkewX() const {
        return rotationZ_X_;
    }

    void Node::setRotation3D(const MATH::Vector3f& rotation) {
        if (rotationX_ == rotation.x &&
            rotationY_ == rotation.y &&
            rotationZ_X_ == rotation.z)
            return;

        transformUpdated_ = transformDirty_ = inverseDirty_ = true;

        rotationX_ = rotation.x;
        rotationY_ = rotation.y;

        rotationZ_Y_ = rotationZ_X_ = rotation.z;

        updateRotationQuat();
    }

    MATH::Vector3f Node::getRotation3D() const {
        return MATH::Vector3f(rotationX_,rotationY_,rotationZ_X_);
    }

    void Node::updateRotationQuat() {
        float halfRadx = MATH_DEGREES_TO_RADIANS(rotationX_ / 2.f), halfRady = MATH_DEGREES_TO_RADIANS(rotationY_ / 2.f), halfRadz = rotationZ_X_ == rotationZ_Y_ ? -MATH_DEGREES_TO_RADIANS(rotationZ_X_ / 2.f) : 0;
        float coshalfRadx = cosf(halfRadx), sinhalfRadx = sinf(halfRadx), coshalfRady = cosf(halfRady), sinhalfRady = sinf(halfRady), coshalfRadz = cosf(halfRadz), sinhalfRadz = sinf(halfRadz);
        rotationQuat_.x = sinhalfRadx * coshalfRady * coshalfRadz - coshalfRadx * sinhalfRady * sinhalfRadz;
        rotationQuat_.y = coshalfRadx * sinhalfRady * coshalfRadz + sinhalfRadx * coshalfRady * sinhalfRadz;
        rotationQuat_.z = coshalfRadx * coshalfRady * sinhalfRadz - sinhalfRadx * sinhalfRady * coshalfRadz;
        rotationQuat_.w = coshalfRadx * coshalfRady * coshalfRadz + sinhalfRadx * sinhalfRady * sinhalfRadz;
    }

    void Node::updateRotation3D() {
        float x = rotationQuat_.x, y = rotationQuat_.y, z = rotationQuat_.z, w = rotationQuat_.w;
        rotationX_ = atan2f(2.f * (w * x + y * z), 1.f - 2.f * (x * x + y * y));
        rotationY_ = asinf(2.f * (w * y - z * x));
        rotationZ_X_ = atan2f(2.f * (w * z + x * y), 1.f - 2.f * (y * y + z * z));

        rotationX_ = MATH_RADIANS_TO_DEGREES(rotationX_);
        rotationY_ = MATH_RADIANS_TO_DEGREES(rotationY_);
        rotationZ_X_ = rotationZ_Y_ = -MATH_RADIANS_TO_DEGREES(rotationZ_X_);
    }

    void Node::setRotationQuat(const MATH::Quaternion& quat) {
        rotationQuat_ = quat;
        updateRotation3D();
        transformUpdated_ = transformDirty_ = inverseDirty_ = true;
    }

    MATH::Quaternion Node::getRotationQuat() const {
        return rotationQuat_;
    }

    void Node::setRotationSkewX(float rotationX) {
        if (rotationZ_X_ == rotationX)
            return;

        rotationZ_X_ = rotationX;
        transformUpdated_ = transformDirty_ = inverseDirty_ = true;

        updateRotationQuat();
    }

    float Node::getRotationSkewY() const {
        return rotationZ_Y_;
    }

    void Node::setRotationSkewY(float rotationY) {
        if (rotationZ_Y_ == rotationY)
            return;

        rotationZ_Y_ = rotationY;
        transformUpdated_ = transformDirty_ = inverseDirty_ = true;

        updateRotationQuat();
    }

    float Node::getScale(void) const {
        return scaleX_;
    }

    void Node::setScale(float scale) {
        if (scaleX_ == scale && scaleY_ == scale && scaleZ_ == scale)
            return;

        scaleX_ = scaleY_ = scaleZ_ = scale;
        transformUpdated_ = transformDirty_ = inverseDirty_ = true;
    }

    float Node::getScaleX() const {
        return scaleX_;
    }

    void Node::setScale(float scaleX,float scaleY) {
        if (scaleX_ == scaleX && scaleY_ == scaleY)
            return;

        scaleX_ = scaleX;
        scaleY_ = scaleY;
        transformUpdated_ = transformDirty_ = inverseDirty_ = true;
    }

    void Node::setScaleX(float scaleX) {
        if (scaleX_ == scaleX)
            return;

        scaleX_ = scaleX;
        transformUpdated_ = transformDirty_ = inverseDirty_ = true;
    }

    float Node::getScaleY() const {
        return scaleY_;
    }

    void Node::setScaleZ(float scaleZ) {
        if (scaleZ_ == scaleZ)
            return;

        scaleZ_ = scaleZ;
        transformUpdated_ = transformDirty_ = inverseDirty_ = true;
    }

    float Node::getScaleZ() const {
        return scaleZ_;
    }

    void Node::setScaleY(float scaleY) {
        if (scaleY_ == scaleY)
            return;

        scaleY_ = scaleY;
        transformUpdated_ = transformDirty_ = inverseDirty_ = true;
    }

    const MATH::Vector2f& Node::getPosition() const {
        return position_;
    }

    void Node::setPosition(const MATH::Vector2f& position) {
        setPosition(position.x, position.y);
    }

    void Node::getPosition(float* x, float* y) const {
        *x = position_.x;
        *y = position_.y;
    }

    void Node::setPosition(float x, float y) {
        if (position_.x == x && position_.y == y)
            return;

        position_.x = x;
        position_.y = y;

        transformUpdated_ = transformDirty_ = inverseDirty_ = true;
        usingNormalizedPosition_ = false;
    }

    void Node::setPosition3D(const MATH::Vector3f& position) {
        setPositionZ(position.z);
        setPosition(position.x, position.y);
    }

    MATH::Vector3f Node::getPosition3D() const {
        return MATH::Vector3f(position_.x, position_.y, positionZ_);
    }

    float Node::getPositionX() const {
        return position_.x;
    }

    void Node::setPositionX(float x) {
        setPosition(x, position_.y);
    }

    float Node::getPositionY() const {
        return  position_.y;
    }

    void Node::setPositionY(float y) {
        setPosition(position_.x, y);
    }

    float Node::getPositionZ() const {
        return positionZ_;
    }

    void Node::setPositionZ(float positionZ) {
        if (positionZ_ == positionZ)
            return;

        transformUpdated_ = transformDirty_ = inverseDirty_ = true;

        positionZ_ = positionZ;
    }

    const MATH::Vector2f& Node::getNormalizedPosition() const {
        return normalizedPosition_;
    }

    void Node::setNormalizedPosition(const MATH::Vector2f& position) {
        if (normalizedPosition_.equals(position))
            return;

        normalizedPosition_ = position;
        usingNormalizedPosition_ = true;
        normalizedPositionDirty_ = true;
        transformUpdated_ = transformDirty_ = inverseDirty_ = true;
    }

    uint64 Node::getChildrenCount() const {
        return children_.size();
    }

    bool Node::isVisible() const {
        return visible_;
    }

    void Node::setVisible(bool visible) {
        if(visible != visible_) {
            visible_ = visible;
            if(visible_)
                transformUpdated_ = transformDirty_ = inverseDirty_ = true;
        }
    }

    const MATH::Vector2f& Node::getAnchorPointInPoints() const {
        return anchorPointInPoints_;
    }

    const MATH::Vector2f& Node::getAnchorPoint() const {
        return anchorPoint_;
    }

    void Node::setAnchorPoint(const MATH::Vector2f& point) {
        if (! point.equals(anchorPoint_)) {
            anchorPoint_ = point;
            anchorPointInPoints_.set(contentSize_.width * anchorPoint_.x, contentSize_.height * anchorPoint_.y);
            transformUpdated_ = transformDirty_ = inverseDirty_ = true;
        }
    }

    const MATH::Sizef& Node::getContentSize() const {
        return contentSize_;
    }

    void Node::setContentSize(const MATH::Sizef & size) {
        if (! size.equals(contentSize_)) {
            contentSize_ = size;

            anchorPointInPoints_.set(contentSize_.width * anchorPoint_.x, contentSize_.height * anchorPoint_.y);
            transformUpdated_ = transformDirty_ = inverseDirty_ = contentSizeDirty_ = true;
        }
    }

    bool Node::isRunning() const {
        return running_;
    }

    void Node::setParent(Node * parent) {
        parent_ = parent;
        transformUpdated_ = transformDirty_ = inverseDirty_ = true;
    }

    bool Node::isIgnoreAnchorPointForPosition() const {
        return ignoreAnchorPointForPosition_;
    }

    void Node::ignoreAnchorPointForPosition(bool newValue) {
        if (newValue != ignoreAnchorPointForPosition_) {
            ignoreAnchorPointForPosition_ = newValue;
            transformUpdated_ = transformDirty_ = inverseDirty_ = true;
        }
    }

    int Node::getTag() const {
        return tag_;
    }

    void Node::setTag(int tag) {
        tag_ = tag ;
    }

    std::string Node::getName() const {
        return name_;
    }

    void Node::setName(const std::string& name) {
        name_ = name;
        std::hash<std::string> h;
        hashOfName_ = h(name);
    }

    void Node::setUserData(void *userData) {
        userData_ = userData;
    }

    int Node::getOrderOfArrival() const {
        return orderOfArrival_;
    }

    void Node::setOrderOfArrival(int orderOfArrival) {
        orderOfArrival_ = orderOfArrival;
    }

    void Node::setUserObject(HObject* userObject) {
        SAFE_RETAIN(userObject);
        SAFE_RELEASE(userObject_);
        userObject_ = userObject;
    }

    ShaderState* Node::getU3DShaderState() const {
        return shaderState_;
    }

    void Node::setU3DShaderState(ShaderState* shaderState) {
        if (shaderState != shaderState_) {
            SAFE_RELEASE(shaderState_);
            shaderState_ = shaderState;
            SAFE_RETAIN(shaderState_);
        }
    }

    void Node::setU3DShader(Unity3DShaderSet* u3dShader) {
        if (shaderState_ == nullptr || (shaderState_ && shaderState_->getU3DShader() != u3dShader)) {
            SAFE_RELEASE(shaderState_);
            shaderState_ = ShaderState::getOrCreateWithShader(u3dShader);
            shaderState_->retain();
        }
    }

    Unity3DShaderSet * Node::getU3DShader() const {
        return shaderState_ ? shaderState_->getU3DShader() : nullptr;
    }

    MATH::Rectf Node::getBoundingBox() const {
        MATH::Rectf rect(0, 0, contentSize_.width, contentSize_.height);
        return RectApplyAffineTransform(rect, getNodeToParentAffineTransform());
    }

    void Node::childrenAlloc() {
        children_.reserve(4);
    }

    Node* Node::getChildByTag(int tag) const {
        for (const auto child : children_) {
            if(child && child->tag_ == tag)
                return child;
        }
        return nullptr;
    }

    Node* Node::getChildByName(const std::string& name) const {
        std::hash<std::string> h;
        uint64 hash = h(name);

        for (const auto& child : children_) {
            if(child->hashOfName_ == hash && child->name_.compare(name) == 0)
                return child;
        }
        return nullptr;
    }

    void Node::enumerateChildren(const std::string &name, std::function<bool (Node *)> callback) const {
        uint64 length = name.length();

        uint64 subStrStartPos = 0;  // sub string start index
        uint64 subStrlength = length; // sub string length

        bool searchRecursively = false;
        if (length > 2 && name[0] == '/' && name[1] == '/') {
            searchRecursively = true;
            subStrStartPos = 2;
            subStrlength -= 2;
        }

        bool searchFromParent = false;
        if (length > 3 &&
            name[length-3] == '/' &&
            name[length-2] == '.' &&
            name[length-1] == '.') {
            searchFromParent = true;
            subStrlength -= 3;
        }

        std::string newName = name.substr(subStrStartPos, subStrlength);

        if (searchFromParent) {
            newName.insert(0, "[[:alnum:]]+/");
        }

        if (searchRecursively) {
            doEnumerateRecursive(this, newName, callback);
        }
        else {
            doEnumerate(newName, callback);
        }
    }

    bool Node::doEnumerateRecursive(const Node* node, const std::string &name, std::function<bool (Node *)> callback) const {
        bool ret =false;

        if (node->doEnumerate(name, callback)) {
            ret = true;
        }
        else {
            for (const auto& child : node->getChildren()) {
                if (doEnumerateRecursive(child, name, callback)) {
                    ret = true;
                    break;
                }
            }
        }

        return ret;
    }

    bool Node::doEnumerate(std::string name, std::function<bool (Node *)> callback) const {
        uint64 pos = name.find('/');
        std::string searchName = name;
        bool needRecursive = false;
        if (pos != name.npos) {
            searchName = name.substr(0, pos);
            name.erase(0, pos+1);
            needRecursive = true;
        }

        bool ret = false;
        for (const auto& child : children_) {
            if (std::regex_match(child->name_, std::regex(searchName))) {
                if (!needRecursive) {
                    if (callback(child)) {
                        ret = true;
                        break;
                    }
                }
                else {
                    ret = child->doEnumerate(name, callback);
                    if (ret)
                        break;
                }
            }
        }

        return ret;
    }

    void Node::addChild(Node *child, int localZOrder, int tag) {
        addChildHelper(child, localZOrder, tag, "", true);
    }

    void Node::addChild(Node* child, int localZOrder, const std::string &name) {
        addChildHelper(child, localZOrder, INVALID_TAG, name, false);
    }

    void Node::addChildHelper(Node* child, int localZOrder, int tag, const std::string &name, bool setTag) {
        if (children_.empty()) {
            this->childrenAlloc();
        }

        this->insertChild(child, localZOrder);

        if (setTag)
            child->setTag(tag);
        else
            child->setName(name);

        child->setParent(this);
        child->setOrderOfArrival(s_globalOrderOfArrival++);

        if( running_ ) {
            child->onEnter();
            if (isTransitionFinished_) {
                child->onEnterTransitionDidFinish();
            }
        }

        if (cascadeColorEnabled_) {
            updateCascadeColor();
        }

        if (cascadeOpacityEnabled_) {
            updateCascadeOpacity();
        }
    }

    void Node::addChild(Node *child, int zOrder) {
        this->addChild(child, zOrder, child->name_);
    }

    void Node::addChild(Node *child) {
        this->addChild(child, child->localZOrder_, child->name_);
    }

    void Node::removeFromParent() {
        this->removeFromParentAndCleanup(true);
    }

    void Node::removeFromParentAndCleanup(bool cleanup) {
        if (parent_ != nullptr) {
            parent_->removeChild(this,cleanup);
        }
    }

    void Node::removeChild(Node* child, bool cleanup /* = true */) {
        if (children_.empty()) {
            return;
        }

        int64 index = children_.getIndex(child);
        if( index != -1 )
            this->detachChild( child, index, cleanup );
    }

    void Node::removeChildByTag(int tag, bool cleanup/* = true */) {
        Node *child = this->getChildByTag(tag);

        if (child != nullptr) {
            this->removeChild(child, cleanup);
        }
    }

    void Node::removeChildByName(const std::string &name, bool cleanup) {
        Node *child = this->getChildByName(name);

        if (child != nullptr) {
            this->removeChild(child, cleanup);
        }
    }

    void Node::removeAllChildren() {
        this->removeAllChildrenWithCleanup(true);
    }

    void Node::removeAllChildrenWithCleanup(bool cleanup) {
        for (const auto& child : children_) {
            if(running_) {
                child->onExitTransitionDidStart();
                child->onExit();
            }

            if (cleanup) {
                child->cleanup();
            }

            child->setParent(nullptr);
        }

        children_.clear();
    }

    void Node::detachChild(Node *child, uint64 childIndex, bool doCleanup) {
        if (running_) {
            child->onExitTransitionDidStart();
            child->onExit();
        }

        if (doCleanup) {
            child->cleanup();
        }

        child->setParent(nullptr);

        children_.erase(childIndex);
    }

    void Node::insertChild(Node* child, int z) {
        transformUpdated_ = true;
        reorderChildDirty_ = true;
        children_.pushBack(child);
        child->localZOrder_ = z;
    }

    void Node::reorderChild(Node *child, int zOrder) {
        reorderChildDirty_ = true;
        child->setOrderOfArrival(s_globalOrderOfArrival++);
        child->localZOrder_ = zOrder;
    }

    void Node::sortAllChildren() {
        if (reorderChildDirty_) {
            std::sort(std::begin(children_), std::end(children_), nodeComparisonLess);
            reorderChildDirty_ = false;
        }
    }

    void Node::draw() {
        auto renderer = director_->getRenderer();
        draw(renderer, modelViewTransform_, true);
    }

    void Node::draw(Renderer*, const MATH::Matrix4 &, uint32_t) {
    }

    void Node::visit() {
        auto renderer = director_->getRenderer();
        auto& parentTransform = director_->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        visit(renderer, parentTransform, true);
    }

    Scene* Node::getScene() const {
        if (!parent_)
            return nullptr;

        auto sceneNode = parent_;
        while (sceneNode->parent_) {
            sceneNode = sceneNode->parent_;
        }

        return dynamic_cast<Scene*>(sceneNode);
    }

    uint32_t Node::processParentFlags(const MATH::Matrix4& parentTransform, uint32_t parentFlags) {
        if(usingNormalizedPosition_) {
            if ((parentFlags & FLAGS_CONTENT_SIZE_DIRTY) || normalizedPositionDirty_)
            {
                auto& s = parent_->getContentSize();
                position_.x = normalizedPosition_.x * s.width;
                position_.y = normalizedPosition_.y * s.height;
                transformUpdated_ = transformDirty_ = inverseDirty_ = true;
                normalizedPositionDirty_ = false;
            }
        }

        uint32_t flags = parentFlags;
        flags |= (transformUpdated_ ? FLAGS_TRANSFORM_DIRTY : 0);
        flags |= (contentSizeDirty_ ? FLAGS_CONTENT_SIZE_DIRTY : 0);


        if(flags & FLAGS_DIRTY_MASK)
            modelViewTransform_ = this->transform(parentTransform);

        transformUpdated_ = false;
        contentSizeDirty_ = false;

        return flags;
    }

    void Node::visit(Renderer* renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags) {
        if (!visible_) {
            return;
        }

        uint32_t flags = processParentFlags(parentTransform, parentFlags);

        director_->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        director_->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, modelViewTransform_);

        uint64 index = 0;

        if(!children_.empty()) {
            sortAllChildren();
            for( ; index < children_.size(); index++ ) {
                auto node = children_.at(index);

                if (node && node->localZOrder_ < 0)
                    node->visit(renderer, modelViewTransform_, flags);
                else
                    break;
            }

            this->draw(renderer, modelViewTransform_, flags);

            for(auto it=children_.cbegin()+index; it != children_.cend(); ++it)
                (*it)->visit(renderer, modelViewTransform_, flags);
        }
        else {
            this->draw(renderer, modelViewTransform_, flags);
        }

        director_->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    }

    MATH::Matrix4 Node::transform(const MATH::Matrix4& parentTransform) {
        return parentTransform * this->getNodeToParentTransform();
    }

    void Node::onEnter() {
        if (onEnterCallback)
            onEnterCallback();

        isTransitionFinished_ = false;

        for( const auto &child: children_)
            child->onEnter();

        this->resume();

        running_ = true;
    }

    void Node::onEnterTransitionDidFinish() {
        if (onEnterTransitionDidFinishCallback)
            onEnterTransitionDidFinishCallback();

        isTransitionFinished_ = true;
        for( const auto &child: children_)
            child->onEnterTransitionDidFinish();
    }

    void Node::onExitTransitionDidStart() {
        if (onExitTransitionDidStartCallback)
            onExitTransitionDidStartCallback();

        for( const auto &child: children_)
            child->onExitTransitionDidStart();
    }

    void Node::onExit() {
        if (onExitCallback)
            onExitCallback();

        this->pause();

        running_ = false;

        for( const auto &child: children_)
            child->onExit();
    }

    void Node::setEventDispatcher(EventDispatcher* dispatcher) {
        if (dispatcher != eventDispatcher_) {
            eventDispatcher_->removeEventListenersForTarget(this);
            SAFE_RETAIN(dispatcher);
            SAFE_RELEASE(eventDispatcher_);
            eventDispatcher_ = dispatcher;
        }
    }

    void Node::setActionManager(ActionManager* actionManager) {
        if( actionManager != actionManager_ ) {
            this->stopAllActions();
            SAFE_RETAIN(actionManager);
            SAFE_RELEASE(actionManager_);
            actionManager_ = actionManager;
        }
    }

    Action * Node::runAction(Action* action) {
        actionManager_->addAction(action, this, !running_);
        return action;
    }

    void Node::stopAllActions() {
        actionManager_->removeAllActionsFromTarget(this);
    }

    void Node::stopAction(Action* action) {
        actionManager_->removeAction(action);
    }

    void Node::stopActionByTag(int tag) {
        actionManager_->removeActionByTag(tag, this);
    }

    void Node::stopAllActionsByTag(int tag) {
        actionManager_->removeAllActionsByTag(tag, this);
    }

    Action * Node::getActionByTag(int tag) {
        return actionManager_->getActionByTag(tag, this);
    }

    uint64 Node::getNumberOfRunningActions() const {
        return actionManager_->getNumberOfRunningActionsInTarget(this);
    }

    void Node::setScheduler(Scheduler* scheduler) {
        if( scheduler != scheduler_ )
        {
            this->unscheduleAllCallbacks();
            SAFE_RETAIN(scheduler);
            SAFE_RELEASE(scheduler_);
            scheduler_ = scheduler;
        }
    }

    bool Node::isScheduled(SelectorF selector) {
        return scheduler_->isScheduled(selector, this);
    }

    bool Node::isScheduled(const std::string &key) {
        return scheduler_->isScheduled(key, this);
    }

    void Node::scheduleUpdate() {
        scheduleUpdateWithPriority(0);
    }

    void Node::scheduleUpdateWithPriority(int priority) {
        scheduler_->scheduleUpdate(this, priority, !running_);
    }

    void Node::scheduleUpdateWithPriorityLua(int, int priority) {
        unscheduleUpdate();

        scheduler_->scheduleUpdate(this, priority, !running_);
    }

    void Node::unscheduleUpdate() {
        scheduler_->unscheduleUpdate(this);
    }

    void Node::schedule(SelectorF selector) {
        this->schedule(selector, 0.0f, -1, 0.0f);
    }

    void Node::schedule(SelectorF selector, float interval) {
        this->schedule(selector, interval, -1, 0.0f);
    }

    void Node::schedule(SelectorF selector, float interval, unsigned int repeat, float delay) {
        scheduler_->schedule(selector, this, !running_, interval , repeat, delay);
    }

    void Node::schedule(const std::function<void(float)> &callback, const std::string &key) {
        scheduler_->schedule(callback, this, key, !running_, 0);
    }

    void Node::schedule(const std::function<void(float)> &callback, float interval, const std::string &key) {
        scheduler_->schedule(callback, this, key, !running_, interval);
    }

    void Node::schedule(const std::function<void(float)>& callback, float interval, unsigned int repeat, float delay, const std::string &key) {
        scheduler_->schedule(callback, this, key, !running_, interval, repeat, delay);
    }

    void Node::scheduleOnce(SelectorF selector, float delay) {
        this->schedule(selector, 0.0f, 0, delay);
    }

    void Node::scheduleOnce(const std::function<void(float)> &callback, float delay, const std::string &key) {
        scheduler_->schedule(callback, this, key, !running_, 0, 0, delay);
    }

    void Node::unschedule(SelectorF selector) {
        if (selector == nullptr)
            return;

        scheduler_->unschedule(selector, this);
    }

    void Node::unschedule(const std::string &key) {
        scheduler_->unschedule(key, this);
    }

    void Node::unscheduleAllCallbacks() {
        scheduler_->unscheduleAllForTarget(this);
    }

    void Node::resume() {
        scheduler_->resumeTarget(this);
        actionManager_->resumeTarget(this);
        eventDispatcher_->resumeEventListenersForTarget(this);
    }

    void Node::pause() {
        scheduler_->pauseTarget(this);
        actionManager_->pauseTarget(this);
        eventDispatcher_->pauseEventListenersForTarget(this);
    }

    void Node::update(float) {
    }

    MATH::AffineTransform Node::getNodeToParentAffineTransform() const {
        MATH::AffineTransform ret;
        GLToCGAffine(getNodeToParentTransform().m, &ret);

        return ret;
    }

    const MATH::Matrix4& Node::getNodeToParentTransform() const {
        if (transformDirty_) {
            float x = position_.x;
            float y = position_.y;
            float z = positionZ_;

            if (ignoreAnchorPointForPosition_) {
                x += anchorPointInPoints_.x;
                y += anchorPointInPoints_.y;
            }

            bool needsSkewMatrix = ( skewX_ || skewY_ );
            MATH::Vector2f anchorPoint(anchorPointInPoints_.x * scaleX_, anchorPointInPoints_.y * scaleY_);

            if (! needsSkewMatrix && !anchorPointInPoints_.isZero()) {
                x += -anchorPoint.x;
                y += -anchorPoint.y;
            }

            MATH::Matrix4 translation;
            MATH::Matrix4::createTranslation(x + anchorPoint.x, y + anchorPoint.y, z, &translation);

            MATH::Matrix4::createRotation(rotationQuat_, &transform_);

            if (rotationZ_X_ != rotationZ_Y_) {
                float radiansX = -MATH_DEGREES_TO_RADIANS(rotationZ_X_);
                float radiansY = -MATH_DEGREES_TO_RADIANS(rotationZ_Y_);
                float cx = cosf(radiansX);
                float sx = sinf(radiansX);
                float cy = cosf(radiansY);
                float sy = sinf(radiansY);

                float m0 = transform_.m[0], m1 = transform_.m[1], m4 = transform_.m[4], m5 = transform_.m[5], m8 = transform_.m[8], m9 = transform_.m[9];
                transform_.m[0] = cy * m0 - sx * m1, transform_.m[4] = cy * m4 - sx * m5, transform_.m[8] = cy * m8 - sx * m9;
                transform_.m[1] = sy * m0 + cx * m1, transform_.m[5] = sy * m4 + cx * m5, transform_.m[9] = sy * m8 + cx * m9;
            }
            transform_ = translation * transform_;
            transform_.translate(-anchorPoint.x, -anchorPoint.y, 0);


            if (scaleX_ != 1.f) {
                transform_.m[0] *= scaleX_, transform_.m[1] *= scaleX_, transform_.m[2] *= scaleX_;
            }
            if (scaleY_ != 1.f) {
                transform_.m[4] *= scaleY_, transform_.m[5] *= scaleY_, transform_.m[6] *= scaleY_;
            }
            if (scaleZ_ != 1.f) {
                transform_.m[8] *= scaleZ_, transform_.m[9] *= scaleZ_, transform_.m[10] *= scaleZ_;
            }

            if (needsSkewMatrix) {
                float skewMatArray[16] = {
                    1, (float)tanf(MATH_DEGREES_TO_RADIANS(skewY_)), 0, 0,
                    (float)tanf(MATH_DEGREES_TO_RADIANS(skewX_)), 1, 0, 0,
                    0,  0,  1, 0,
                    0,  0,  0, 1
                };
                MATH::Matrix4 skewMatrix(skewMatArray);

                transform_ = transform_ * skewMatrix;

                if (!anchorPointInPoints_.isZero()) {
                    transform_.m[12] += transform_.m[0] * -anchorPointInPoints_.x + transform_.m[4] * -anchorPointInPoints_.y;
                    transform_.m[13] += transform_.m[1] * -anchorPointInPoints_.x + transform_.m[5] * -anchorPointInPoints_.y;
                }
            }

            if (useAdditionalTransform_) {
                transform_ = transform_ * additionalTransform_;
            }

            transformDirty_ = false;
        }

        return transform_;
    }

    void Node::setNodeToParentTransform(const MATH::Matrix4& transform) {
        transform_ = transform;
        transformDirty_ = false;
        transformUpdated_ = true;
    }

    void Node::setAdditionalTransform(const MATH::AffineTransform& additionalTransform) {
        MATH::Matrix4 tmp;
        CGAffineToGL(additionalTransform, tmp.m);
        setAdditionalTransform(&tmp);
    }

    void Node::setAdditionalTransform(MATH::Matrix4* additionalTransform) {
        if (additionalTransform == nullptr) {
            useAdditionalTransform_ = false;
        }
        else {
            additionalTransform_ = *additionalTransform;
            useAdditionalTransform_ = true;
        }
        transformUpdated_ = transformDirty_ = inverseDirty_ = true;
    }

    Component* Node::getComponent(const std::string& name) {
        if (componentContainer_)
            return componentContainer_->get(name);

        return nullptr;
    }

    bool Node::addComponent(Component *component) {
        // lazy alloc
        if (!componentContainer_)
            componentContainer_ = new (std::nothrow) ComponentContainer(this);

        return componentContainer_->add(component);
    }

    bool Node::removeComponent(const std::string& name) {
        if (componentContainer_)
            return componentContainer_->remove(name);

        return false;
    }

    bool Node::removeComponent(Component *component) {
        if (componentContainer_) {
            return componentContainer_->remove(component);
        }

        return false;
    }

    void Node::removeAllComponents() {
        if (componentContainer_)
            componentContainer_->removeAll();
    }

    MATH::AffineTransform Node::getParentToNodeAffineTransform() const {
        MATH::AffineTransform ret;

        GLToCGAffine(getParentToNodeTransform().m,&ret);
        return ret;
    }

    const MATH::Matrix4& Node::getParentToNodeTransform() const {
        if ( inverseDirty_ ) {
            inverse_ = getNodeToParentTransform().getInversed();
            inverseDirty_ = false;
        }

        return inverse_;
    }


    MATH::AffineTransform Node::getNodeToWorldAffineTransform() const {
        MATH::AffineTransform t(this->getNodeToParentAffineTransform());

        for (Node *p = parent_; p != nullptr; p = p->getParent())
            t = AffineTransformConcat(t, p->getNodeToParentAffineTransform());

        return t;
    }

    MATH::Matrix4 Node::getNodeToWorldTransform() const {
        MATH::Matrix4 t(this->getNodeToParentTransform());

        for (Node *p = parent_; p != nullptr; p = p->getParent()) {
            t = p->getNodeToParentTransform() * t;
        }

        return t;
    }

    MATH::AffineTransform Node::getWorldToNodeAffineTransform() const {
        return AffineTransformInvert(this->getNodeToWorldAffineTransform());
    }

    MATH::Matrix4 Node::getWorldToNodeTransform() const {
        return getNodeToWorldTransform().getInversed();
    }


    MATH::Vector2f Node::convertToNodeSpace(const MATH::Vector2f& worldPoint) const {
        MATH::Matrix4 tmp = getWorldToNodeTransform();
        MATH::Vector3f vec3(worldPoint.x, worldPoint.y, 0);
        MATH::Vector3f ret;
        tmp.transformPoint(vec3,&ret);
        return MATH::Vector2f(ret.x, ret.y);
    }

    MATH::Vector2f Node::convertToWorldSpace(const MATH::Vector2f& nodePoint) const {
        MATH::Matrix4 tmp = getNodeToWorldTransform();
        MATH::Vector3f vec3(nodePoint.x, nodePoint.y, 0);
        MATH::Vector3f ret;
        tmp.transformPoint(vec3,&ret);
        return MATH::Vector2f(ret.x, ret.y);

    }

    MATH::Vector2f Node::convertToNodeSpaceAR(const MATH::Vector2f& worldPoint) const {
        MATH::Vector2f nodePoint(convertToNodeSpace(worldPoint));
        return nodePoint - anchorPointInPoints_;
    }

    MATH::Vector2f Node::convertToWorldSpaceAR(const MATH::Vector2f& nodePoint) const {
        return convertToWorldSpace(nodePoint + anchorPointInPoints_);
    }

    MATH::Vector2f Node::convertToWindowSpace(const MATH::Vector2f& nodePoint) const {
        MATH::Vector2f worldPoint(this->convertToWorldSpace(nodePoint));
        return director_->convertToUI(worldPoint);
    }

    void Node::updateTransform() {
        for( const auto &child: children_)
            child->updateTransform();
    }

    uint8 Node::getOpacity(void) const {
        return realOpacity_;
    }

    uint8 Node::getDisplayedOpacity() const {
        return displayedOpacity_;
    }

    void Node::setOpacity(uint8 opacity) {
        displayedOpacity_ = realOpacity_ = opacity;

        updateCascadeOpacity();
    }

    void Node::updateDisplayedOpacity(uint8 parentOpacity) {
        displayedOpacity_ = realOpacity_ * parentOpacity/255.0;
        updateColor();

        if (cascadeOpacityEnabled_) {
            for(const auto& child : children_) {
                child->updateDisplayedOpacity(displayedOpacity_);
            }
        }
    }

    bool Node::isCascadeOpacityEnabled(void) const {
        return cascadeOpacityEnabled_;
    }

    void Node::setCascadeOpacityEnabled(bool cascadeOpacityEnabled) {
        if (cascadeOpacityEnabled_ == cascadeOpacityEnabled) {
            return;
        }

        cascadeOpacityEnabled_ = cascadeOpacityEnabled;

        if (cascadeOpacityEnabled) {
            updateCascadeOpacity();
        }
        else {
            disableCascadeOpacity();
        }
    }

    void Node::updateCascadeOpacity() {
        uint8 parentOpacity = 255;

        if (parent_ != nullptr && parent_->isCascadeOpacityEnabled()) {
            parentOpacity = parent_->getDisplayedOpacity();
        }

        updateDisplayedOpacity(parentOpacity);
    }

    void Node::disableCascadeOpacity() {
        displayedOpacity_ = realOpacity_;

        for(const auto& child : children_) {
            child->updateDisplayedOpacity(255);
        }
    }

    const Color3B& Node::getColor(void) const {
        return realColor_;
    }

    const Color3B& Node::getDisplayedColor() const {
        return displayedColor_;
    }

    void Node::setColor(const Color3B& color) {
        displayedColor_ = realColor_ = color;

        updateCascadeColor();
    }

    void Node::updateDisplayedColor(const Color3B& parentColor) {
        displayedColor_.red = realColor_.red * parentColor.red/255.0;
        displayedColor_.green = realColor_.green * parentColor.green/255.0;
        displayedColor_.blue = realColor_.blue * parentColor.blue/255.0;
        updateColor();

        if (cascadeColorEnabled_) {
            for(const auto &child : children_) {
                child->updateDisplayedColor(displayedColor_);
            }
        }
    }

    bool Node::isCascadeColorEnabled(void) const {
        return cascadeColorEnabled_;
    }

    void Node::setCascadeColorEnabled(bool cascadeColorEnabled) {
        if (cascadeColorEnabled_ == cascadeColorEnabled) {
            return;
        }

        cascadeColorEnabled_ = cascadeColorEnabled;

        if (cascadeColorEnabled_) {
            updateCascadeColor();
        }
        else {
            disableCascadeColor();
        }
    }

    void Node::updateCascadeColor() {
        Color3B parentColor = Color3B::WHITE;
        if (parent_ && parent_->isCascadeColorEnabled()) {
            parentColor = parent_->getDisplayedColor();
        }

        updateDisplayedColor(parentColor);
    }

    void Node::disableCascadeColor() {
        for(const auto& child : children_) {
            child->updateDisplayedColor(Color3B::WHITE);
        }
    }

    void Node::setCameraMask(unsigned short mask, bool applyChildren) {
        cameraMask_ = mask;
        if (applyChildren) {
            for (const auto& child : children_) {
                child->setCameraMask(mask, applyChildren);
            }
        }
    }

    ProtectedNode::ProtectedNode() : reorderProtectedChildDirty_(false) {
    }

    ProtectedNode::~ProtectedNode() {
    }

    ProtectedNode * ProtectedNode::create(void) {
        ProtectedNode * ret = new (std::nothrow) ProtectedNode();
        if (ret && ret->init()) {
            ret->autorelease();
        }
        else {
            SAFE_DELETE(ret);
        }
        return ret;
    }

    void ProtectedNode::cleanup() {
        Node::cleanup();
        for( const auto &child: protectedChildren_)
            child->cleanup();
    }

    void ProtectedNode::addProtectedChild(Node *child) {
        addProtectedChild(child, child->getLocalZOrder(), child->getTag());
    }

    void ProtectedNode::addProtectedChild(Node *child, int localZOrder) {
        addProtectedChild(child, localZOrder, child->getTag());
    }

    void ProtectedNode::addProtectedChild(Node *child, int zOrder, int tag) {
        if (protectedChildren_.empty()) {
            protectedChildren_.reserve(4);
        }

        this->insertProtectedChild(child, zOrder);

        child->setTag(tag);

        child->setParent(this);
        child->setOrderOfArrival(s_globalOrderOfArrival++);

        if( running_ ) {
            child->onEnter();
            if (isTransitionFinished_) {
                child->onEnterTransitionDidFinish();
            }
        }

        if (cascadeColorEnabled_) {
            updateCascadeColor();
        }

        if (cascadeOpacityEnabled_) {
            updateCascadeOpacity();
        }
    }

    Node* ProtectedNode::getProtectedChildByTag(int tag) {
        for (auto& child : protectedChildren_) {
            if(child && child->getTag() == tag)
                return child;
        }
        return nullptr;
    }

    void ProtectedNode::removeProtectedChild(Node *child, bool cleanup) {
        if (protectedChildren_.empty()) {
            return;
        }

        int64 index = protectedChildren_.getIndex(child);
        if( index != -1 ) {
            if (running_) {
                child->onExitTransitionDidStart();
                child->onExit();
            }

            if (cleanup) {
                child->cleanup();
            }

            child->setParent(nullptr);

            protectedChildren_.erase(index);
        }
    }

    void ProtectedNode::removeAllProtectedChildren() {
        removeAllProtectedChildrenWithCleanup(true);
    }

    void ProtectedNode::removeAllProtectedChildrenWithCleanup(bool cleanup) {
        for (auto& child : protectedChildren_) {
            if(running_) {
                child->onExitTransitionDidStart();
                child->onExit();
            }

            if (cleanup) {
                child->cleanup();
            }

            child->setParent(nullptr);
        }

        protectedChildren_.clear();
    }

    void ProtectedNode::removeProtectedChildByTag(int tag, bool cleanup) {
        Node *child = this->getProtectedChildByTag(tag);

        if (child != nullptr) {
            this->removeProtectedChild(child, cleanup);
        }
    }

    void ProtectedNode::insertProtectedChild(Node *child, int z) {
        reorderProtectedChildDirty_ = true;
        protectedChildren_.pushBack(child);
        child->setLocalZOrder(z);
    }

    void ProtectedNode::sortAllProtectedChildren() {
        if( reorderProtectedChildDirty_ ) {
            std::sort( std::begin(protectedChildren_), std::end(protectedChildren_), nodeComparisonLess );
            reorderProtectedChildDirty_ = false;
        }
    }

    void ProtectedNode::reorderProtectedChild(Node *child, int localZOrder) {
        reorderProtectedChildDirty_ = true;
        child->setOrderOfArrival(s_globalOrderOfArrival++);
        child->setLocalZOrder(localZOrder);
    }

    void ProtectedNode::visit(Renderer* renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags) {
        if (!visible_) {
            return;
        }

        uint32_t flags = processParentFlags(parentTransform, parentFlags);

        Director* director = &Director::getInstance();
        director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
        director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, modelViewTransform_);

        uint64 i = 0;      // used by _children
        uint64 j = 0;      // used by _protectedChildren

        sortAllChildren();
        sortAllProtectedChildren();

        for( ; i < children_.size(); i++ ) {
            auto node = children_.at(i);

            if ( node && node->getLocalZOrder() < 0 )
                node->visit(renderer, modelViewTransform_, flags);
            else
                break;
        }

        for( ; j < protectedChildren_.size(); j++ ) {
            auto node = protectedChildren_.at(j);

            if ( node && node->getLocalZOrder() < 0 )
                node->visit(renderer, modelViewTransform_, flags);
            else
                break;
        }

        this->draw(renderer, modelViewTransform_, flags);

        for(auto it=protectedChildren_.cbegin()+j; it != protectedChildren_.cend(); ++it)
            (*it)->visit(renderer, modelViewTransform_, flags);

        for(auto it=children_.cbegin()+i; it != children_.cend(); ++it)
            (*it)->visit(renderer, modelViewTransform_, flags);

        director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    }

    void ProtectedNode::onEnter() {
        Node::onEnter();
        for( const auto &child: protectedChildren_)
            child->onEnter();
    }

    void ProtectedNode::onEnterTransitionDidFinish() {
        Node::onEnterTransitionDidFinish();
        for( const auto &child: protectedChildren_)
            child->onEnterTransitionDidFinish();
    }

    void ProtectedNode::onExitTransitionDidStart() {
        Node::onExitTransitionDidStart();
        for( const auto &child: protectedChildren_)
            child->onExitTransitionDidStart();
    }

    void ProtectedNode::onExit() {
        Node::onExit();
        for( const auto &child: protectedChildren_)
            child->onExit();
    }

    void ProtectedNode::updateDisplayedOpacity(uint8 parentOpacity) {
        displayedOpacity_ = realOpacity_ * parentOpacity/255.0;
        updateColor();

        if (cascadeOpacityEnabled_) {
            for(auto child : children_){
                child->updateDisplayedOpacity(displayedOpacity_);
            }
        }

        for(auto child : protectedChildren_){
            child->updateDisplayedOpacity(displayedOpacity_);
        }
    }

    void ProtectedNode::updateDisplayedColor(const Color3B& parentColor) {
        displayedColor_.red = realColor_.red * parentColor.red/255.0;
        displayedColor_.green = realColor_.green * parentColor.green/255.0;
        displayedColor_.blue = realColor_.blue * parentColor.blue/255.0;
        updateColor();

        if (cascadeColorEnabled_) {
            for(const auto &child : children_){
                child->updateDisplayedColor(displayedColor_);
            }
        }
        for(const auto &child : protectedChildren_){
            child->updateDisplayedColor(displayedColor_);
        }
    }

    void ProtectedNode::disableCascadeColor() {
        for(auto child : children_){
            child->updateDisplayedColor(Color3B::WHITE);
        }
        for(auto child : protectedChildren_){
            child->updateDisplayedColor(Color3B::WHITE);
        }
    }

    void ProtectedNode::disableCascadeOpacity() {
        displayedOpacity_ = realOpacity_;

        for(auto child : children_){
            child->updateDisplayedOpacity(255);
        }

        for(auto child : protectedChildren_){
            child->updateDisplayedOpacity(255);
        }
    }

    void ProtectedNode::setCameraMask(unsigned short mask, bool applyChildren) {
        Node::setCameraMask(mask, applyChildren);
        if (applyChildren) {
            for (auto& iter: protectedChildren_) {
                iter->setCameraMask(mask);
            }
        }

    }
}
