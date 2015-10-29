#ifndef NODE_H
#define NODE_H

#include <functional>
#include "BASE/HObject.h"
#include "GRAPH/UNITY3D/GLCommon.h"
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
    class Unity3DGLShaderSet;
    class GLShaderState;
    class Component;
    class ComponentContainer;
    class Camera;

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
        virtual int getLocalZOrder() const { return localZOrder_; }

        virtual void setGlobalZOrder(float globalZOrder);
        virtual float getGlobalZOrder() const { return globalZOrder_; }

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
        virtual HObjectVector<Node*>& getChildren() { return children_; }
        virtual const HObjectVector<Node*>& getChildren() const { return children_; }
        virtual uint64 getChildrenCount() const;
        virtual void setParent(Node* parent);
        virtual Node* getParent() { return parent_; }
        virtual const Node* getParent() const { return parent_; }
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

        virtual void* getUserData() { return userData_; }
        virtual const void* getUserData() const { return userData_; }
        virtual void setUserData(void *userData);
        virtual HObject* getUserObject() { return userObject_; }
        virtual const HObject* getUserObject() const { return userObject_; }
        virtual void setUserObject(HObject *userObject);

        Unity3DGLShaderSet* getU3DShader() const;
        virtual void setGLShader(Unity3DGLShaderSet *u3dShader);

        GLShaderState *getGLShaderState() const;
        virtual void setGLShaderState(GLShaderState *glShaderState);

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

        virtual Scene* getScene() const;

        virtual MATH::Rectf getBoundingBox() const;

        virtual void setEventDispatcher(EventDispatcher* dispatcher);
        virtual EventDispatcher* getEventDispatcher() const { return eventDispatcher_; }

        virtual void setActionManager(ActionManager* actionManager);
        virtual ActionManager* getActionManager() { return actionManager_; }
        virtual const ActionManager* getActionManager() const { return actionManager_; }

        virtual Action* runAction(Action* action);
        void stopAllActions();
        void stopAction(Action* action);
        void stopActionByTag(int tag);
        void stopAllActionsByTag(int tag);
        Action* getActionByTag(int tag);
        uint64 getNumberOfRunningActions() const;

        virtual void setScheduler(Scheduler* scheduler);
        virtual Scheduler* getScheduler() { return scheduler_; }
        virtual const Scheduler* getScheduler() const { return scheduler_; }

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

        void setOnEnterCallback(const std::function<void()>& callback) { onEnterCallback = callback; }
        const std::function<void()>& getOnEnterCallback() const { return onEnterCallback; }
        void setOnExitCallback(const std::function<void()>& callback) { onExitCallback = callback; }
        const std::function<void()>& getOnExitCallback() const { return onExitCallback; }
        void setonEnterTransitionDidFinishCallback(const std::function<void()>& callback) { onEnterTransitionDidFinishCallback = callback; }
        const std::function<void()>& getonEnterTransitionDidFinishCallback() const { return onEnterTransitionDidFinishCallback; }
        void setonExitTransitionDidStartCallback(const std::function<void()>& callback) { onExitTransitionDidStartCallback = callback; }
        const std::function<void()>& getonExitTransitionDidStartCallback() const { return onExitTransitionDidStartCallback; }

        unsigned short getCameraMask() const { return cameraMask_; }
        virtual void setCameraMask(unsigned short mask, bool applyChildren = true);

    public:
        Node();
        virtual ~Node();

        virtual bool init();

    protected:
        void childrenAlloc(void);
        void insertChild(Node* child, int z);
        void detachChild(Node *child, uint64 index, bool doCleanup);

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

        float rotationX_;
        float rotationY_;

        float rotationZ_X_;
        float rotationZ_Y_;

        MATH::Quaternion rotationQuat_;

        float scaleX_;
        float scaleY_;
        float scaleZ_;

        MATH::Vector2f position_;
        float positionZ_;
        MATH::Vector2f normalizedPosition_;
        bool usingNormalizedPosition_;
        bool normalizedPositionDirty_;

        float skewX_;
        float skewY_;

        MATH::Vector2f anchorPointInPoints_;
        MATH::Vector2f anchorPoint_;

        MATH::Sizef contentSize_;
        bool contentSizeDirty_;

        MATH::Matrix4 modelViewTransform_;

        mutable MATH::Matrix4 transform_;
        mutable bool transformDirty_;
        mutable MATH::Matrix4 inverse_;
        mutable bool inverseDirty_;
        mutable MATH::Matrix4 additionalTransform_;
        bool useAdditionalTransform_;
        bool transformUpdated_;

        int localZOrder_;
        float globalZOrder_;

        HObjectVector<Node*> children_;
        Node *parent_;
        Director* director_;
        int tag_;

        std::string name_;
        uint64 hashOfName_;

        void *userData_;
        HObject *userObject_;

        GLShaderState *glShaderState_;

        int orderOfArrival_;

        Scheduler *scheduler_;

        ActionManager *actionManager_;

        EventDispatcher* eventDispatcher_;

        bool running_;

        bool visible_;

        bool ignoreAnchorPointForPosition_;

        bool reorderChildDirty_;
        bool isTransitionFinished_;

        ComponentContainer *componentContainer_;

        GLubyte		displayedOpacity_;
        GLubyte     realOpacity_;
        Color3B	    displayedColor_;
        Color3B     realColor_;
        bool		cascadeColorEnabled_;
        bool        cascadeOpacityEnabled_;

        static int s_globalOrderOfArrival;

        unsigned short cameraMask_;

        std::function<void()> onEnterCallback;
        std::function<void()> onExitCallback;
        std::function<void()> onEnterTransitionDidFinishCallback;
        std::function<void()> onExitTransitionDidStartCallback;

        static bool nodeComparisonLess(Node* n1, Node* n2);
        static bool isScreenPointInRect(const MATH::Vector2f &pt, const Camera* camera, const MATH::Matrix4& w2l,
                                        const MATH::Rectf& rect, MATH::Vector3f *p);

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

        HObjectVector<Node*> protectedChildren_;
        bool reorderProtectedChildDirty_;

    private:
        DISALLOW_COPY_AND_ASSIGN(ProtectedNode)
    };

    inline bool NodeComparisonLess(Node* n1, Node* n2) {
        return( n1->getLocalZOrder() < n2->getLocalZOrder() ||
               ( n1->getLocalZOrder() == n2->getLocalZOrder() && n1->getOrderOfArrival() < n2->getOrderOfArrival() )
               );
    }
}

#endif // NODE_H
