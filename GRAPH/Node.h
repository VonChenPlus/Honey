#ifndef NODE_H
#define NODE_H

#include <functional>
#include "BASE/HObject.h"
#include "GRAPH/RENDERER/GLCommon.h"
#include "GRAPH/Color.h"
#include "MATH/Vector.h"
#include "MATH/Quaternion.h"
#include "MATH/Matrix.h"
#include "MATH/Rectangle.h"
#include "MATH/AffineTransform.h"

namespace GRAPH
{
    class Action;
    class Scheduler;
    class ActionManager;
    class EventDispatcher;
    class Scene;
    class Renderer;
    class Director;
    class GLProgram;
    class GLProgramState;
    class Node;
    class Component;
    class ComponentContainer;

    enum
    {
        kNodeOnEnter,
        kNodeOnExit,
        kNodeOnEnterTransitionDidFinish,
        kNodeOnExitTransitionDidStart,
        kNodeOnCleanup
    };

    class Node : public HObject
    {
    public:
        static const int INVALID_TAG = -1;

        enum {
            FLAGS_TRANSFORM_DIRTY = (1 << 0),
            FLAGS_CONTENT_SIZE_DIRTY = (1 << 1),
            FLAGS_RENDER_AS_3D = (1 << 3),

            FLAGS_DIRTY_MASK = (FLAGS_TRANSFORM_DIRTY | FLAGS_CONTENT_SIZE_DIRTY),
        };

        static Node * create();

        virtual void setLocalZOrder(int localZOrder);
        virtual int getLocalZOrder() const { return _localZOrder; }

        virtual void setGlobalZOrder(float globalZOrder);
        virtual float getGlobalZOrder() const { return _globalZOrder; }

        virtual void setScaleX(float scaleX);
        virtual float getScaleX() const;

        virtual void setScaleY(float scaleY);
        virtual float getScaleY() const;

        virtual void setScaleZ(float scaleZ);
        virtual float getScaleZ() const;

        virtual void setScale(float scale);
        virtual float getScale() const;
        virtual void setScale(float scaleX, float scaleY);

        virtual void setPosition(const MATH::Vector2f &position);
        virtual void setNormalizedPosition(const MATH::Vector2f &position);
        virtual const MATH::Vector2f& getPosition() const;
        virtual const MATH::Vector2f& getNormalizedPosition() const;
        virtual void setPosition(float x, float y);
        virtual void getPosition(float* x, float* y) const;
        virtual void  setPositionX(float x);
        virtual float getPositionX(void) const;
        virtual void  setPositionY(float y);
        virtual float getPositionY(void) const;
        virtual void setPosition3D(const MATH::Vector3f& position);
        virtual MATH::Vector3f getPosition3D() const;
        virtual void setPositionZ(float positionZ);
        virtual float getPositionZ() const;

        virtual void setSkewX(float skewX);
        virtual float getSkewX() const;
        virtual void setSkewY(float skewY);
        virtual float getSkewY() const;

        virtual void setAnchorPoint(const MATH::Vector2f& anchorPoint);
        virtual const MATH::Vector2f& getAnchorPoint() const;
        virtual const MATH::Vector2f& getAnchorPointInPoints() const;

        virtual void setContentSize(const MATH::Sizef& contentSize);
        virtual const MATH::Sizef& getContentSize() const;

        virtual void setVisible(bool visible);
        virtual bool isVisible() const;

        virtual void setRotation(float rotation);
        virtual float getRotation() const;

        virtual void setRotation3D(const MATH::Vector3f& rotation);
        virtual MATH::Vector3f getRotation3D() const;

        virtual void setRotationQuat(const MATH::Quaternion& quat);
        virtual MATH::Quaternion getRotationQuat() const;

        virtual void setRotationSkewX(float rotationX);
        virtual float getRotationSkewX() const;
        virtual void setRotationSkewY(float rotationY);
        virtual float getRotationSkewY() const;

        void setOrderOfArrival(int orderOfArrival);
        int getOrderOfArrival() const;

        virtual void ignoreAnchorPointForPosition(bool ignore);
        virtual bool isIgnoreAnchorPointForPosition() const;

        virtual void addChild(Node * child);
        virtual void addChild(Node * child, int localZOrder);
        virtual void addChild(Node* child, int localZOrder, int tag);
        virtual void addChild(Node* child, int localZOrder, const std::string &name);
        virtual Node * getChildByTag(int tag) const;
        virtual Node* getChildByName(const std::string& name) const;
        template <typename T>
        inline T getChildByName(const std::string& name) const { return static_cast<T>(getChildByName(name)); }
        virtual void enumerateChildren(const std::string &name, std::function<bool(Node* node)> callback) const;
        virtual HObjectVector<Node*>& getChildren() { return _children; }
        virtual const HObjectVector<Node*>& getChildren() const { return _children; }
        virtual ssize_t getChildrenCount() const;
        virtual void setParent(Node* parent);
        virtual Node* getParent() { return _parent; }
        virtual const Node* getParent() const { return _parent; }
        virtual void removeFromParent();
        virtual void removeFromParentAndCleanup(bool cleanup);
        virtual void removeChild(Node* child, bool cleanup = true);
        virtual void removeChildByTag(int tag, bool cleanup = true);
        virtual void removeChildByName(const std::string &name, bool cleanup = true);
        virtual void removeAllChildren();
        virtual void removeAllChildrenWithCleanup(bool cleanup);
        virtual void reorderChild(Node * child, int localZOrder);
        virtual void sortAllChildren();

         virtual int getTag() const;
         virtual void setTag(int tag);

        virtual std::string getName() const;
        virtual void setName(const std::string& name);

        virtual void* getUserData() { return _userData; }
        virtual const void* getUserData() const { return _userData; }
        virtual void setUserData(void *userData);
        virtual HObject* getUserObject() { return _userObject; }
        virtual const HObject* getUserObject() const { return _userObject; }
        virtual void setUserObject(HObject *userObject);

        GLProgram* getGLProgram() const;
        virtual void setGLProgram(GLProgram *glprogram);

        GLProgramState *getGLProgramState() const;
        virtual void setGLProgramState(GLProgramState *glProgramState);

        virtual bool isRunning() const;

        void scheduleUpdateWithPriorityLua(int handler, int priority);

        virtual void onEnter();
        virtual void onEnterTransitionDidFinish();
        virtual void onExit();
        virtual void onExitTransitionDidStart();

        virtual void cleanup();

        virtual void draw(Renderer *renderer, const MATH::Matrix4& transform, uint32_t flags);
        virtual void draw() final;

        virtual void visit(Renderer *renderer, const MATH::Matrix4& parentTransform, uint32_t parentFlags);
        virtual void visit() final;

        virtual MATH::Rectf getBoundingBox() const;

        virtual void setEventDispatcher(EventDispatcher* dispatcher);
        virtual EventDispatcher* getEventDispatcher() const { return _eventDispatcher; }

        virtual void setActionManager(ActionManager* actionManager);
        virtual ActionManager* getActionManager() { return _actionManager; }
        virtual const ActionManager* getActionManager() const { return _actionManager; }

        virtual Action* runAction(Action* action);
        void stopAllActions();
        void stopAction(Action* action);
        void stopActionByTag(int tag);
        void stopAllActionsByTag(int tag);
        Action* getActionByTag(int tag);
        ssize_t getNumberOfRunningActions() const;

        virtual void setScheduler(Scheduler* scheduler);
        virtual Scheduler* getScheduler() { return _scheduler; }
        virtual const Scheduler* getScheduler() const { return _scheduler; }

        bool isScheduled(SelectorF selector);
        bool isScheduled(const std::string &key);
        void scheduleUpdate(void);
        void scheduleUpdateWithPriority(int priority);
        void unscheduleUpdate(void);
        void schedule(SelectorF selector, float interval, unsigned int repeat, float delay);
        void schedule(SelectorF selector, float interval);
        void scheduleOnce(SelectorF selector, float delay);
        void scheduleOnce(const std::function<void(float)>& callback, float delay, const std::string &key);
        void schedule(SelectorF selector);
        void schedule(const std::function<void(float)>& callback, const std::string &key);
        void schedule(const std::function<void(float)>& callback, float interval, const std::string &key);
        void schedule(const std::function<void(float)>& callback, float interval, unsigned int repeat, float delay, const std::string &key);
        void unschedule(SelectorF selector);
        void unschedule(const std::string &key);
        void unscheduleAllCallbacks();

        virtual void resume(void);
        virtual void pause(void);
        virtual void update(float delta);

        virtual void updateTransform();
        virtual const MATH::Matrix4& getNodeToParentTransform() const;
        virtual MATH::AffineTransform getNodeToParentAffineTransform() const;
        virtual void setNodeToParentTransform(const MATH::Matrix4& transform);
        virtual const MATH::Matrix4& getParentToNodeTransform() const;
        virtual MATH::AffineTransform getParentToNodeAffineTransform() const;
        virtual MATH::Matrix4 getNodeToWorldTransform() const;
        virtual MATH::AffineTransform getNodeToWorldAffineTransform() const;
        virtual MATH::Matrix4 getWorldToNodeTransform() const;
        virtual MATH::AffineTransform getWorldToNodeAffineTransform() const;

        MATH::Vector2f convertToNodeSpace(const MATH::Vector2f& worldPoint) const;
        MATH::Vector2f convertToWorldSpace(const MATH::Vector2f& nodePoint) const;
        MATH::Vector2f convertToNodeSpaceAR(const MATH::Vector2f& worldPoint) const;
        MATH::Vector2f convertToWorldSpaceAR(const MATH::Vector2f& nodePoint) const;

        void setAdditionalTransform(MATH::Matrix4* additionalTransform);
        void setAdditionalTransform(const MATH::AffineTransform& additionalTransform);

        Component* getComponent(const std::string& name);
        virtual bool addComponent(Component *component);
        virtual bool removeComponent(const std::string& name);
        virtual bool removeComponent(Component *component);
        virtual void removeAllComponents();

        // overrides
        virtual GLubyte getOpacity() const;
        virtual GLubyte getDisplayedOpacity() const;
        virtual void setOpacity(GLubyte opacity);
        virtual void updateDisplayedOpacity(GLubyte parentOpacity);
        virtual bool isCascadeOpacityEnabled() const;
        virtual void setCascadeOpacityEnabled(bool cascadeOpacityEnabled);

        virtual const Color3B& getColor() const;
        virtual const Color3B& getDisplayedColor() const;
        virtual void setColor(const Color3B& color);
        virtual void updateDisplayedColor(const Color3B& parentColor);
        virtual bool isCascadeColorEnabled() const;
        virtual void setCascadeColorEnabled(bool cascadeColorEnabled);

        virtual void setOpacityModifyRGB(bool) {}
        virtual bool isOpacityModifyRGB() const { return false; }

        void setOnEnterCallback(const std::function<void()>& callback) { _onEnterCallback = callback; }
        const std::function<void()>& getOnEnterCallback() const { return _onEnterCallback; }
        void setOnExitCallback(const std::function<void()>& callback) { _onExitCallback = callback; }
        const std::function<void()>& getOnExitCallback() const { return _onExitCallback; }
        void setonEnterTransitionDidFinishCallback(const std::function<void()>& callback) { _onEnterTransitionDidFinishCallback = callback; }
        const std::function<void()>& getonEnterTransitionDidFinishCallback() const { return _onEnterTransitionDidFinishCallback; }
        void setonExitTransitionDidStartCallback(const std::function<void()>& callback) { _onExitTransitionDidStartCallback = callback; }
        const std::function<void()>& getonExitTransitionDidStartCallback() const { return _onExitTransitionDidStartCallback; }

        unsigned short getCameraMask() const { return _cameraMask; }
        virtual void setCameraMask(unsigned short mask, bool applyChildren = true);

    public:
        Node();
        virtual ~Node();

        virtual bool init();

    protected:
        void childrenAlloc(void);
        void insertChild(Node* child, int z);
        void detachChild(Node *child, ssize_t index, bool doCleanup);

        MATH::Vector2f convertToWindowSpace(const MATH::Vector2f& nodePoint) const;

        MATH::Matrix4 transform(const MATH::Matrix4 &parentTransform);
        uint32_t processParentFlags(const MATH::Matrix4& parentTransform, uint32_t parentFlags);

        virtual void updateCascadeOpacity();
        virtual void disableCascadeOpacity();
        virtual void updateCascadeColor();
        virtual void disableCascadeColor();
        virtual void updateColor() {}

        bool doEnumerate(std::string name, std::function<bool (Node *)> callback) const;
        bool doEnumerateRecursive(const Node* node, const std::string &name, std::function<bool (Node *)> callback) const;

        void updateRotationQuat();
        void updateRotation3D();

    private:
        void addChildHelper(Node* child, int localZOrder, int tag, const std::string &name, bool setTag);

    protected:

        float _rotationX;
        float _rotationY;

        float _rotationZ_X;
        float _rotationZ_Y;

        MATH::Quaternion _rotationQuat;

        float _scaleX;
        float _scaleY;
        float _scaleZ;

        MATH::Vector2f _position;
        float _positionZ;
        MATH::Vector2f _normalizedPosition;
        bool _usingNormalizedPosition;
        bool _normalizedPositionDirty;

        float _skewX;
        float _skewY;

        MATH::Vector2f _anchorPointInPoints;
        MATH::Vector2f _anchorPoint;

        MATH::Sizef _contentSize;
        bool _contentSizeDirty;

        MATH::Matrix4 _modelViewTransform;

        mutable MATH::Matrix4 _transform;
        mutable bool _transformDirty;
        mutable MATH::Matrix4 _inverse;
        mutable bool _inverseDirty;
        mutable MATH::Matrix4 _additionalTransform;
        bool _useAdditionalTransform;
        bool _transformUpdated;

        int _localZOrder;
        float _globalZOrder;

        HObjectVector<Node*> _children;
        Node *_parent;
        Director* _director;
        int _tag;

        std::string _name;
        size_t _hashOfName;

        void *_userData;
        HObject *_userObject;

        GLProgramState *_glProgramState;

        int _orderOfArrival;

        Scheduler *_scheduler;

        ActionManager *_actionManager;

        EventDispatcher* _eventDispatcher;

        bool _running;

        bool _visible;

        bool _ignoreAnchorPointForPosition;

        bool _reorderChildDirty;
        bool _isTransitionFinished;

        ComponentContainer *_componentContainer;

        GLubyte		_displayedOpacity;
        GLubyte     _realOpacity;
        Color3B	    _displayedColor;
        Color3B     _realColor;
        bool		_cascadeColorEnabled;
        bool        _cascadeOpacityEnabled;

        static int s_globalOrderOfArrival;

        unsigned short _cameraMask;

        std::function<void()> _onEnterCallback;
        std::function<void()> _onExitCallback;
        std::function<void()> _onEnterTransitionDidFinishCallback;
        std::function<void()> _onExitTransitionDidStartCallback;

    private:
        DISALLOW_COPY_AND_ASSIGN(Node)
    };

    class ProtectedNode : public Node
    {
    public:
        static ProtectedNode * create(void);

        virtual void addProtectedChild(Node * child);
        virtual void addProtectedChild(Node * child, int localZOrder);
        virtual void addProtectedChild(Node* child, int localZOrder, int tag);
        virtual Node * getProtectedChildByTag(int tag);

        virtual void removeProtectedChild(Node* child, bool cleanup = true);
        virtual void removeProtectedChildByTag(int tag, bool cleanup = true);
        virtual void removeAllProtectedChildren();
        virtual void removeAllProtectedChildrenWithCleanup(bool cleanup);

        virtual void reorderProtectedChild(Node * child, int localZOrder);

        virtual void sortAllProtectedChildren();

        virtual void visit(Renderer *renderer, const MATH::Matrix4 &parentTransform, uint32_t parentFlags) override;

        virtual void cleanup() override;

        virtual void onEnter() override;
        virtual void onEnterTransitionDidFinish() override;
        virtual void onExit() override;
        virtual void onExitTransitionDidStart() override;

        virtual void updateDisplayedOpacity(GLubyte parentOpacity) override;
        virtual void updateDisplayedColor(const Color3B& parentColor) override;
        virtual void disableCascadeColor() override;
        virtual void disableCascadeOpacity()override;
        virtual void setCameraMask(unsigned short mask, bool applyChildren = true) override;

    public:
        ProtectedNode();
        virtual ~ProtectedNode();

    protected:
        void insertProtectedChild(Node* child, int z);

        HObjectVector<Node*> _protectedChildren;
        bool _reorderProtectedChildDirty;

    private:
        DISALLOW_COPY_AND_ASSIGN(ProtectedNode)
    };
}

#endif // NODE_H
