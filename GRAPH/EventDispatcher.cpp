#include "GRAPH/EventDispatcher.h"
#include <algorithm>
#include "GRAPH/Director.h"
#include "GRAPH/Node.h"
#include "GRAPH/Scene.h"

namespace GRAPH
{
    #define DUMP_LISTENER_ITEM_PRIORITY_INFO 0

    class DispatchGuard
    {
    public:
        DispatchGuard(int& count):
                _count(count) {
            ++_count;
        }

        ~DispatchGuard() {
            --_count;
        }

    private:
        int& _count;
    };

    static EventListener::ListenerID __getListenerID(Event* event) {
        EventListener::ListenerID ret;
        switch (event->getType()) {
            case Event::Type::ACCELERATION:
                ret = EventListenerAcceleration::LISTENER_ID;
                break;
            case Event::Type::CUSTOM: {
                    auto customEvent = static_cast<EventCustom*>(event);
                    ret = customEvent->getEventName();
                }
                break;
            case Event::Type::KEYBOARD:
                ret = EventListenerKeyboard::LISTENER_ID;
                break;
            case Event::Type::MOUSE:
                ret = EventListenerMouse::LISTENER_ID;
                break;
            case Event::Type::FOCUS:
                ret = EventListenerFocus::LISTENER_ID;
                break;
            case Event::Type::TOUCH:
                // Touch listener is very special, it contains two kinds of listeners, EventListenerTouchOneByOne and EventListenerTouchAllAtOnce.
                // return UNKNOWN instead.
                break;
            default:
                break;
        }

        return ret;
    }

    EventDispatcher::EventListenerVector::EventListenerVector()
        : _fixedListeners(nullptr)
        , _sceneGraphListeners(nullptr)
        , _gt0Index(0) {

    }

    EventDispatcher::EventListenerVector::~EventListenerVector() {
        SAFE_DELETE(_sceneGraphListeners);
        SAFE_DELETE(_fixedListeners);
    }

    uint64 EventDispatcher::EventListenerVector::size() const {
        uint64 ret = 0;
        if (_sceneGraphListeners)
            ret += _sceneGraphListeners->size();
        if (_fixedListeners)
            ret += _fixedListeners->size();

        return ret;
    }

    bool EventDispatcher::EventListenerVector::empty() const {
        return (_sceneGraphListeners == nullptr || _sceneGraphListeners->empty())
            && (_fixedListeners == nullptr || _fixedListeners->empty());
    }

    void EventDispatcher::EventListenerVector::push_back(EventListener* listener) {
        if (listener->getFixedPriority() == 0) {
            if (_sceneGraphListeners == nullptr) {
                _sceneGraphListeners = new std::vector<EventListener*>();
                _sceneGraphListeners->reserve(100);
            }

            _sceneGraphListeners->push_back(listener);
        }
        else {
            if (_fixedListeners == nullptr) {
                _fixedListeners = new std::vector<EventListener*>();
                _fixedListeners->reserve(100);
            }

            _fixedListeners->push_back(listener);
        }
    }

    void EventDispatcher::EventListenerVector::clearSceneGraphListeners() {
        if (_sceneGraphListeners) {
            _sceneGraphListeners->clear();
            delete _sceneGraphListeners;
            _sceneGraphListeners = nullptr;
        }
    }

    void EventDispatcher::EventListenerVector::clearFixedListeners() {
        if (_fixedListeners) {
            _fixedListeners->clear();
            delete _fixedListeners;
            _fixedListeners = nullptr;
        }
    }

    void EventDispatcher::EventListenerVector::clear() {
        clearSceneGraphListeners();
        clearFixedListeners();
    }

    // The application will come to foreground.
    // This message is posted in cocos/platform/android/jni/Java_org_cocos2dx_lib_Cocos2dxRenderer.cpp.
    #define EVENT_COME_TO_FOREGROUND    "event_come_to_foreground"

    // The renderer[android:GLSurfaceView.Renderer  WP8:Cocos2dRenderer] was recreated.
    // This message is used for reloading resources before renderer is recreated on Android/WP8.
    // This message is posted in cocos/platform/android/javaactivity.cpp and cocos\platform\wp8-xaml\cpp\Cocos2dRenderer.cpp.
    #define EVENT_RENDERER_RECREATED    "event_renderer_recreated"

    // The application will come to background.
    // This message is used for doing something before coming to background, such as save RenderTexture.
    // This message is posted in cocos/platform/android/jni/Java_org_cocos2dx_lib_Cocos2dxRenderer.cpp and cocos\platform\wp8-xaml\cpp\Cocos2dRenderer.cpp.
    #define EVENT_COME_TO_BACKGROUND    "event_come_to_background"

    EventDispatcher::EventDispatcher()
        : _inDispatch(0)
        , _isEnabled(false)
        , _nodePriorityIndex(0) {
        _toAddedListeners.reserve(50);

        // Therefore, internal listeners would not be cleaned when removeAllEventListeners is invoked.
        _internalCustomListenerIDs.insert(EVENT_COME_TO_FOREGROUND);
        _internalCustomListenerIDs.insert(EVENT_COME_TO_BACKGROUND);
        _internalCustomListenerIDs.insert(EVENT_RENDERER_RECREATED);
    }

    EventDispatcher::~EventDispatcher() {
        // Clear internal custom listener IDs from set,
        // so removeAllEventListeners would clean internal custom listeners.
        _internalCustomListenerIDs.clear();
        removeAllEventListeners();
    }

    void EventDispatcher::visitTarget(HObject* node, bool isRootNode) {
        dynamic_cast<Node *>(node)->sortAllChildren();

        int i = 0;
        auto& children = dynamic_cast<Node *>(node)->getChildren();

        auto childrenCount = children.size();

        if(childrenCount > 0) {
            HObject* child = nullptr;
            for( ; i < childrenCount; i++ ) {
                child = children.at(i);

                if ( child && dynamic_cast<Node *>(child)->getLocalZOrder() < 0 )
                    visitTarget(child, false);
                else
                    break;
            }

            if (_nodeListenersMap.find(node) != _nodeListenersMap.end()) {
                _globalZOrderNodeMap[dynamic_cast<Node *>(node)->getGlobalZOrder()].push_back(node);
            }

            for( ; i < childrenCount; i++ ) {
                child = children.at(i);
                if (child)
                    visitTarget(child, false);
            }
        }
        else {
            if (_nodeListenersMap.find(node) != _nodeListenersMap.end()) {
                _globalZOrderNodeMap[dynamic_cast<Node *>(node)->getGlobalZOrder()].push_back(node);
            }
        }

        if (isRootNode) {
            std::vector<float> globalZOrders;
            globalZOrders.reserve(_globalZOrderNodeMap.size());

            for (const auto& e : _globalZOrderNodeMap) {
                globalZOrders.push_back(e.first);
            }

            std::sort(globalZOrders.begin(), globalZOrders.end(), [](const float a, const float b){
                return a < b;
            });

            for (const auto& globalZ : globalZOrders) {
                for (const auto& n : _globalZOrderNodeMap[globalZ]) {
                    _nodePriorityMap[n] = ++_nodePriorityIndex;
                }
            }

            _globalZOrderNodeMap.clear();
        }
    }

    void EventDispatcher::pauseEventListenersForTarget(HObject* target, bool recursive/* = false */) {
        auto listenerIter = _nodeListenersMap.find(target);
        if (listenerIter != _nodeListenersMap.end()) {
            auto listeners = listenerIter->second;
            for (auto& l : *listeners)
            {
                l->setPaused(true);
            }
        }

        for (auto& listener : _toAddedListeners) {
            if (listener->getAssociatedNode() == target) {
                listener->setPaused(true);
            }
        }

        if (recursive) {
            const auto& children = dynamic_cast<Node *>(target)->getChildren();
            for (const auto& child : children) {
                pauseEventListenersForTarget(child, true);
            }
        }
    }

    void EventDispatcher::resumeEventListenersForTarget(HObject* target, bool recursive/* = false */) {
        auto listenerIter = _nodeListenersMap.find(target);
        if (listenerIter != _nodeListenersMap.end()) {
            auto listeners = listenerIter->second;
            for (auto& l : *listeners) {
                l->setPaused(false);
            }
        }

        for (auto& listener : _toAddedListeners) {
            if (listener->getAssociatedNode() == target)
            {
                listener->setPaused(false);
            }
        }

        setDirtyForNode(target);

        if (recursive) {
            const auto& children = dynamic_cast<Node *>(target)->getChildren();
            for (const auto& child : children) {
                resumeEventListenersForTarget(child, true);
            }
        }
    }

    void EventDispatcher::removeEventListenersForTarget(HObject* target, bool recursive/* = false */) {
        // Ensure the node is removed from these immediately also.
        // Don't want any dangling pointers or the possibility of dealing with deleted objects..
        _nodePriorityMap.erase(target);
        _dirtyNodes.erase(target);

        auto listenerIter = _nodeListenersMap.find(target);
        if (listenerIter != _nodeListenersMap.end()) {
            auto listeners = listenerIter->second;
            auto listenersCopy = *listeners;
            for (auto& l : listenersCopy) {
                removeEventListener(l);
            }
        }

        for (auto iter = _toAddedListeners.begin(); iter != _toAddedListeners.end(); ) {
            EventListener * listener = *iter;

            if (listener->getAssociatedNode() == target) {
                listener->setAssociatedNode(nullptr);
                listener->setRegistered(false);
                listener->release();
                iter = _toAddedListeners.erase(iter);
            }
            else {
                ++iter;
            }
        }

        if (recursive) {
            const auto& children = dynamic_cast<Node *>(target)->getChildren();
            for (const auto& child : children) {
                removeEventListenersForTarget(child, true);
            }
        }
    }

    void EventDispatcher::associateNodeAndEventListener(HObject* node, EventListener* listener) {
        std::vector<EventListener*>* listeners = nullptr;
        auto found = _nodeListenersMap.find(node);
        if (found != _nodeListenersMap.end()) {
            listeners = found->second;
        }
        else {
            listeners = new std::vector<EventListener*>();
            _nodeListenersMap.insert(std::make_pair(node, listeners));
        }

        listeners->push_back(listener);
    }

    void EventDispatcher::dissociateNodeAndEventListener(HObject* node, EventListener* listener) {
        std::vector<EventListener*>* listeners = nullptr;
        auto found = _nodeListenersMap.find(node);
        if (found != _nodeListenersMap.end()) {
            listeners = found->second;
            auto iter = std::find(listeners->begin(), listeners->end(), listener);
            if (iter != listeners->end()) {
                listeners->erase(iter);
            }

            if (listeners->empty()) {
                _nodeListenersMap.erase(found);
                delete listeners;
            }
        }
    }

    void EventDispatcher::addEventListener(EventListener* listener) {
        if (_inDispatch == 0) {
            forceAddEventListener(listener);
        }
        else {
            _toAddedListeners.push_back(listener);
        }

        listener->retain();
    }

    void EventDispatcher::forceAddEventListener(EventListener* listener) {
        EventListenerVector* listeners = nullptr;
        EventListener::ListenerID listenerID = listener->getListenerID();
        auto itr = _listenerMap.find(listenerID);
        if (itr == _listenerMap.end()) {
            listeners = new (std::nothrow) EventListenerVector();
            _listenerMap.insert(std::make_pair(listenerID, listeners));
        }
        else {
            listeners = itr->second;
        }

        listeners->push_back(listener);

        if (listener->getFixedPriority() == 0) {
            setDirty(listenerID, DirtyFlag::SCENE_GRAPH_PRIORITY);

            auto node = listener->getAssociatedNode();

            associateNodeAndEventListener(node, listener);

            if (dynamic_cast<Node *>(node)->isRunning()) {
                resumeEventListenersForTarget(node);
            }
        }
        else {
            setDirty(listenerID, DirtyFlag::FIXED_PRIORITY);
        }
    }

    void EventDispatcher::addEventListenerWithSceneGraphPriority(EventListener* listener, HObject* node) {
        if (!listener->checkAvailable())
            return;

        listener->setAssociatedNode(node);
        listener->setFixedPriority(0);
        listener->setRegistered(true);

        addEventListener(listener);
    }

    void EventDispatcher::addEventListenerWithFixedPriority(EventListener* listener, int fixedPriority) {
        if (!listener->checkAvailable())
            return;

        listener->setAssociatedNode(nullptr);
        listener->setFixedPriority(fixedPriority);
        listener->setRegistered(true);
        listener->setPaused(false);

        addEventListener(listener);
    }

    EventListenerCustom* EventDispatcher::addCustomEventListener(const std::string &eventName, const std::function<void(EventCustom*)>& callback) {
        EventListenerCustom *listener = EventListenerCustom::create(eventName, callback);
        addEventListenerWithFixedPriority(listener, 1);
        return listener;
    }

    void EventDispatcher::removeEventListener(EventListener* listener) {
        if (listener == nullptr)
            return;

        bool isFound = false;

        auto removeListenerInVector = [&](std::vector<EventListener*>* listeners){
            if (listeners == nullptr)
                return;

            for (auto iter = listeners->begin(); iter != listeners->end(); ++iter) {
                auto l = *iter;
                if (l == listener) {
                    SAFE_RETAIN(l);
                    l->setRegistered(false);
                    if (l->getAssociatedNode() != nullptr) {
                        dissociateNodeAndEventListener(l->getAssociatedNode(), l);
                        l->setAssociatedNode(nullptr);  // nullptr out the node pointer so we don't have any dangling pointers to destroyed nodes.
                    }

                    if (_inDispatch == 0) {
                        listeners->erase(iter);
                        SAFE_RELEASE(l);
                    }

                    isFound = true;
                    break;
                }
            }
        };

        for (auto iter = _listenerMap.begin(); iter != _listenerMap.end();) {
            auto listeners = iter->second;
            auto fixedPriorityListeners = listeners->getFixedPriorityListeners();
            auto sceneGraphPriorityListeners = listeners->getSceneGraphPriorityListeners();

            removeListenerInVector(sceneGraphPriorityListeners);
            if (isFound) {
                setDirty(listener->getListenerID(), DirtyFlag::SCENE_GRAPH_PRIORITY);
            }
            else {
                removeListenerInVector(fixedPriorityListeners);
                if (isFound) {
                    setDirty(listener->getListenerID(), DirtyFlag::FIXED_PRIORITY);
                }
            }

            if (iter->second->empty()) {
                _priorityDirtyFlagMap.erase(listener->getListenerID());
                auto list = iter->second;
                iter = _listenerMap.erase(iter);
                SAFE_DELETE(list);
            }
            else {
                ++iter;
            }

            if (isFound)
                break;
        }

        if (isFound) {
            SAFE_RELEASE(listener);
        }
        else {
            for(auto iter = _toAddedListeners.begin(); iter != _toAddedListeners.end(); ++iter) {
                if (*iter == listener) {
                    listener->setRegistered(false);
                    listener->release();
                    _toAddedListeners.erase(iter);
                    break;
                }
            }
        }
    }

    void EventDispatcher::setPriority(EventListener* listener, int fixedPriority) {
        if (listener == nullptr)
            return;

        for (auto iter = _listenerMap.begin(); iter != _listenerMap.end(); ++iter) {
            auto fixedPriorityListeners = iter->second->getFixedPriorityListeners();
            if (fixedPriorityListeners) {
                auto found = std::find(fixedPriorityListeners->begin(), fixedPriorityListeners->end(), listener);
                if (found != fixedPriorityListeners->end()) {
                    if (listener->getFixedPriority() != fixedPriority) {
                        listener->setFixedPriority(fixedPriority);
                        setDirty(listener->getListenerID(), DirtyFlag::FIXED_PRIORITY);
                    }
                    return;
                }
            }
        }
    }

    void EventDispatcher::dispatchEventToListeners(EventListenerVector* listeners, const std::function<bool(EventListener*)>& onEvent) {
        bool shouldStopPropagation = false;
        auto fixedPriorityListeners = listeners->getFixedPriorityListeners();
        auto sceneGraphPriorityListeners = listeners->getSceneGraphPriorityListeners();

        uint64 i = 0;
        if (fixedPriorityListeners) {
            if (!fixedPriorityListeners->empty()) {
                for (; i < listeners->getGt0Index(); ++i) {
                    auto l = fixedPriorityListeners->at(i);
                    if (l->isEnabled() && !l->isPaused() && l->isRegistered() && onEvent(l)) {
                        shouldStopPropagation = true;
                        break;
                    }
                }
            }
        }

        if (sceneGraphPriorityListeners) {
            if (!shouldStopPropagation) {
                for (auto& l : *sceneGraphPriorityListeners) {
                    if (l->isEnabled() && !l->isPaused() && l->isRegistered() && onEvent(l)) {
                        shouldStopPropagation = true;
                        break;
                    }
                }
            }
        }

        if (fixedPriorityListeners) {
            if (!shouldStopPropagation) {
                uint64 size = fixedPriorityListeners->size();
                for (; i < size; ++i)
                {
                    auto l = fixedPriorityListeners->at(i);

                    if (l->isEnabled() && !l->isPaused() && l->isRegistered() && onEvent(l)) {
                        shouldStopPropagation = true;
                        break;
                    }
                }
            }
        }
    }

    void EventDispatcher::dispatchTouchEventToListeners(EventListenerVector* listeners, const std::function<bool(EventListener*)>& onEvent) {
        bool shouldStopPropagation = false;
        auto fixedPriorityListeners = listeners->getFixedPriorityListeners();
        auto sceneGraphPriorityListeners = listeners->getSceneGraphPriorityListeners();

        uint64 i = 0;
        // priority < 0
        if (fixedPriorityListeners) {
            if (!fixedPriorityListeners->empty()) {
                for (; i < listeners->getGt0Index(); ++i) {
                    auto l = fixedPriorityListeners->at(i);
                    if (l->isEnabled() && !l->isPaused() && l->isRegistered() && onEvent(l)) {
                        shouldStopPropagation = true;
                        break;
                    }
                }
            }
        }

        if (sceneGraphPriorityListeners) {
            if (!shouldStopPropagation) {
                // priority == 0, scene graph priority

                // first, get all enabled, unPaused and registered listeners
                std::vector<EventListener*> sceneListeners;
                for (auto& l : *sceneGraphPriorityListeners) {
                    if (l->isEnabled() && !l->isPaused() && l->isRegistered()) {
                        sceneListeners.push_back(l);
                    }
                }

                for (auto& l : sceneListeners) {
                    if (onEvent(l)) {
                        shouldStopPropagation = true;
                        break;
                    }
                }
            }
        }

        if (fixedPriorityListeners) {
            if (!shouldStopPropagation) {
                // priority > 0
                uint64 size = fixedPriorityListeners->size();
                for (; i < size; ++i) {
                    auto l = fixedPriorityListeners->at(i);

                    if (l->isEnabled() && !l->isPaused() && l->isRegistered() && onEvent(l)) {
                        shouldStopPropagation = true;
                        break;
                    }
                }
            }
        }
    }

    void EventDispatcher::dispatchEvent(Event* event) {
        if (!_isEnabled)
            return;

        updateDirtyFlagForSceneGraph();

        DispatchGuard guard(_inDispatch);

        if (event->getType() == Event::Type::TOUCH) {
            dispatchTouchEvent(static_cast<EventTouch*>(event));
            return;
        }

        auto listenerID = __getListenerID(event);

        sortEventListeners(listenerID);

        auto pfnDispatchEventToListeners = &EventDispatcher::dispatchEventToListeners;
        if (event->getType() == Event::Type::MOUSE) {
            pfnDispatchEventToListeners = &EventDispatcher::dispatchTouchEventToListeners;
        }
        auto iter = _listenerMap.find(listenerID);
        if (iter != _listenerMap.end()) {
            auto listeners = iter->second;

            auto onEvent = [&event](EventListener* listener) -> bool{
                event->setCurrentTarget(listener->getAssociatedNode());
                listener->_onEvent(event);
                return event->isStopped();
            };

            (this->*pfnDispatchEventToListeners)(listeners, onEvent);
        }

        updateListeners(event);
    }

    void EventDispatcher::dispatchCustomEvent(const std::string &eventName, void *optionalUserData) {
        EventCustom ev(eventName);
        ev.setUserData(optionalUserData);
        dispatchEvent(&ev);
    }

    void EventDispatcher::dispatchTouchEvent(EventTouch* event) {
        sortEventListeners(EventListenerTouchOneByOne::LISTENER_ID);
        sortEventListeners(EventListenerTouchAllAtOnce::LISTENER_ID);

        auto oneByOneListeners = getListeners(EventListenerTouchOneByOne::LISTENER_ID);
        auto allAtOnceListeners = getListeners(EventListenerTouchAllAtOnce::LISTENER_ID);

        // If there aren't any touch listeners, return directly.
        if (nullptr == oneByOneListeners && nullptr == allAtOnceListeners)
            return;

        bool isNeedsMutableSet = (oneByOneListeners && allAtOnceListeners);

        const std::vector<Touch*>& originalTouches = event->getTouches();
        std::vector<Touch*> mutableTouches(originalTouches.size());
        std::copy(originalTouches.begin(), originalTouches.end(), mutableTouches.begin());

        //
        // process the target handlers 1st
        //
        if (oneByOneListeners) {
            auto mutableTouchesIter = mutableTouches.begin();
            auto touchesIter = originalTouches.begin();

            for (; touchesIter != originalTouches.end(); ++touchesIter) {
                bool isSwallowed = false;

                auto onTouchEvent = [&](EventListener* l) -> bool { // Return true to break
                    EventListenerTouchOneByOne* listener = static_cast<EventListenerTouchOneByOne*>(l);

                    // Skip if the listener was removed.
                    if (!listener->registered_)
                        return false;

                    event->setCurrentTarget(listener->node_);

                    bool isClaimed = false;
                    std::vector<Touch*>::iterator removedIter;

                    EventTouch::EventCode eventCode = event->getEventCode();

                    if (eventCode == EventTouch::EventCode::BEGAN) {
                        if (listener->onTouchBegan) {
                            isClaimed = listener->onTouchBegan(*touchesIter, event);
                            if (isClaimed && listener->registered_)
                            {
                                listener->claimedTouches_.push_back(*touchesIter);
                            }
                        }
                    }
                    else if (listener->claimedTouches_.size() > 0
                             && ((removedIter = std::find(listener->claimedTouches_.begin(), listener->claimedTouches_.end(), *touchesIter)) != listener->claimedTouches_.end())) {
                        isClaimed = true;

                        switch (eventCode) {
                            case EventTouch::EventCode::MOVED:
                                if (listener->onTouchMoved)
                                {
                                    listener->onTouchMoved(*touchesIter, event);
                                }
                                break;
                            case EventTouch::EventCode::ENDED:
                                if (listener->onTouchEnded) {
                                    listener->onTouchEnded(*touchesIter, event);
                                }
                                if (listener->registered_) {
                                    listener->claimedTouches_.erase(removedIter);
                                }
                                break;
                            case EventTouch::EventCode::CANCELLED:
                                if (listener->onTouchCancelled) {
                                    listener->onTouchCancelled(*touchesIter, event);
                                }
                                if (listener->registered_) {
                                    listener->claimedTouches_.erase(removedIter);
                                }
                                break;
                            default:
                                break;
                        }
                    }

                    // If the event was stopped, return directly.
                    if (event->isStopped()) {
                        updateListeners(event);
                        return true;
                    }

                    if (isClaimed && listener->registered_ && listener->needSwallow_) {
                        if (isNeedsMutableSet) {
                            mutableTouchesIter = mutableTouches.erase(mutableTouchesIter);
                            isSwallowed = true;
                        }
                        return true;
                    }

                    return false;
                };

                dispatchTouchEventToListeners(oneByOneListeners, onTouchEvent);
                if (event->isStopped()) {
                    return;
                }

                if (!isSwallowed)
                    ++mutableTouchesIter;
            }
        }

        if (allAtOnceListeners && mutableTouches.size() > 0) {

            auto onTouchesEvent = [&](EventListener* l) -> bool{
                EventListenerTouchAllAtOnce* listener = static_cast<EventListenerTouchAllAtOnce*>(l);
                // Skip if the listener was removed.
                if (!listener->registered_)
                    return false;

                event->setCurrentTarget(listener->node_);

                switch (event->getEventCode()) {
                    case EventTouch::EventCode::BEGAN:
                        if (listener->onTouchesBegan) {
                            listener->onTouchesBegan(mutableTouches, event);
                        }
                        break;
                    case EventTouch::EventCode::MOVED:
                        if (listener->onTouchesMoved) {
                            listener->onTouchesMoved(mutableTouches, event);
                        }
                        break;
                    case EventTouch::EventCode::ENDED:
                        if (listener->onTouchesEnded) {
                            listener->onTouchesEnded(mutableTouches, event);
                        }
                        break;
                    case EventTouch::EventCode::CANCELLED:
                        if (listener->onTouchesCancelled) {
                            listener->onTouchesCancelled(mutableTouches, event);
                        }
                        break;
                    default:
                        break;
                }

                if (event->isStopped()) {
                    updateListeners(event);
                    return true;
                }

                return false;
            };

            dispatchTouchEventToListeners(allAtOnceListeners, onTouchesEvent);
            if (event->isStopped()) {
                return;
            }
        }

        updateListeners(event);
    }

    void EventDispatcher::updateListeners(Event* event) {
        if (_inDispatch > 1)
            return;

        auto onUpdateListeners = [this](const EventListener::ListenerID& listenerID) {
            auto listenersIter = _listenerMap.find(listenerID);
            if (listenersIter == _listenerMap.end())
                return;

            auto listeners = listenersIter->second;

            auto fixedPriorityListeners = listeners->getFixedPriorityListeners();
            auto sceneGraphPriorityListeners = listeners->getSceneGraphPriorityListeners();

            if (sceneGraphPriorityListeners) {
                for (auto iter = sceneGraphPriorityListeners->begin(); iter != sceneGraphPriorityListeners->end();) {
                    auto l = *iter;
                    if (!l->isRegistered()) {
                        iter = sceneGraphPriorityListeners->erase(iter);
                        l->release();
                    }
                    else {
                        ++iter;
                    }
                }
            }

            if (fixedPriorityListeners) {
                for (auto iter = fixedPriorityListeners->begin(); iter != fixedPriorityListeners->end();) {
                    auto l = *iter;
                    if (!l->isRegistered()) {
                        iter = fixedPriorityListeners->erase(iter);
                        l->release();
                    }
                    else {
                        ++iter;
                    }
                }
            }

            if (sceneGraphPriorityListeners && sceneGraphPriorityListeners->empty()) {
                listeners->clearSceneGraphListeners();
            }

            if (fixedPriorityListeners && fixedPriorityListeners->empty()) {
                listeners->clearFixedListeners();
            }
        };

        if (event->getType() == Event::Type::TOUCH) {
            onUpdateListeners(EventListenerTouchOneByOne::LISTENER_ID);
            onUpdateListeners(EventListenerTouchAllAtOnce::LISTENER_ID);
        }
        else {
            onUpdateListeners(__getListenerID(event));
        }

        for (auto iter = _listenerMap.begin(); iter != _listenerMap.end();) {
            if (iter->second->empty()) {
                _priorityDirtyFlagMap.erase(iter->first);
                delete iter->second;
                iter = _listenerMap.erase(iter);
            }
            else {
                ++iter;
            }
        }

        if (!_toAddedListeners.empty()) {
            for (auto& listener : _toAddedListeners) {
                forceAddEventListener(listener);
            }
            _toAddedListeners.clear();
        }
    }

    void EventDispatcher::updateDirtyFlagForSceneGraph() {
        if (!_dirtyNodes.empty()) {
            for (auto& node : _dirtyNodes) {
                auto iter = _nodeListenersMap.find(node);
                if (iter != _nodeListenersMap.end()) {
                    for (auto& l : *iter->second) {
                        setDirty(l->getListenerID(), DirtyFlag::SCENE_GRAPH_PRIORITY);
                    }
                }
            }

            _dirtyNodes.clear();
        }
    }

    void EventDispatcher::sortEventListeners(const EventListener::ListenerID& listenerID) {
        DirtyFlag dirtyFlag = DirtyFlag::NONE;

        auto dirtyIter = _priorityDirtyFlagMap.find(listenerID);
        if (dirtyIter != _priorityDirtyFlagMap.end()) {
            dirtyFlag = dirtyIter->second;
        }

        if (dirtyFlag != DirtyFlag::NONE) {
            // Clear the dirty flag first, if `rootNode` is nullptr, then set its dirty flag of scene graph priority
            dirtyIter->second = DirtyFlag::NONE;

            if ((int)dirtyFlag & (int)DirtyFlag::FIXED_PRIORITY) {
                sortEventListenersOfFixedPriority(listenerID);
            }

            if ((int)dirtyFlag & (int)DirtyFlag::SCENE_GRAPH_PRIORITY) {
                auto rootNode = Director::getInstance().getRunningScene();
                if (rootNode) {
                    sortEventListenersOfSceneGraphPriority(listenerID, rootNode);
                }
                else {
                    dirtyIter->second = DirtyFlag::SCENE_GRAPH_PRIORITY;
                }
            }
        }
    }

    void EventDispatcher::sortEventListenersOfSceneGraphPriority(const EventListener::ListenerID& listenerID, HObject* rootNode) {
        auto listeners = getListeners(listenerID);

        if (listeners == nullptr)
            return;
        auto sceneGraphListeners = listeners->getSceneGraphPriorityListeners();

        if (sceneGraphListeners == nullptr)
            return;

        // Reset priority index
        _nodePriorityIndex = 0;
        _nodePriorityMap.clear();

        visitTarget(rootNode, true);

        // After sort: priority < 0, > 0
        std::sort(sceneGraphListeners->begin(), sceneGraphListeners->end(), [this](const EventListener* l1, const EventListener* l2) {
            return _nodePriorityMap[l1->getAssociatedNode()] > _nodePriorityMap[l2->getAssociatedNode()];
        });
    }

    void EventDispatcher::sortEventListenersOfFixedPriority(const EventListener::ListenerID& listenerID) {
        auto listeners = getListeners(listenerID);

        if (listeners == nullptr)
            return;

        auto fixedListeners = listeners->getFixedPriorityListeners();
        if (fixedListeners == nullptr)
            return;

        // After sort: priority < 0, > 0
        std::sort(fixedListeners->begin(), fixedListeners->end(), [](const EventListener* l1, const EventListener* l2) {
            return l1->getFixedPriority() < l2->getFixedPriority();
        });

        int index = 0;
        for (auto& listener : *fixedListeners) {
            if (listener->getFixedPriority() >= 0)
                break;
            ++index;
        }

        listeners->setGt0Index(index);
    }

    EventDispatcher::EventListenerVector* EventDispatcher::getListeners(const EventListener::ListenerID& listenerID) {
        auto iter = _listenerMap.find(listenerID);
        if (iter != _listenerMap.end()) {
            return iter->second;
        }

        return nullptr;
    }

    void EventDispatcher::removeEventListenersForListenerID(const EventListener::ListenerID& listenerID) {
        auto listenerItemIter = _listenerMap.find(listenerID);
        if (listenerItemIter != _listenerMap.end()) {
            auto listeners = listenerItemIter->second;
            auto fixedPriorityListeners = listeners->getFixedPriorityListeners();
            auto sceneGraphPriorityListeners = listeners->getSceneGraphPriorityListeners();

            auto removeAllListenersInVector = [&](std::vector<EventListener*>* listenerVector){
                if (listenerVector == nullptr)
                    return;

                for (auto iter = listenerVector->begin(); iter != listenerVector->end();) {
                    auto l = *iter;
                    l->setRegistered(false);
                    if (l->getAssociatedNode() != nullptr) {
                        dissociateNodeAndEventListener(l->getAssociatedNode(), l);
                        l->setAssociatedNode(nullptr);  // nullptr out the node pointer so we don't have any dangling pointers to destroyed nodes.
                    }

                    if (_inDispatch == 0) {
                        iter = listenerVector->erase(iter);
                        SAFE_RELEASE(l);
                    }
                    else {
                        ++iter;
                    }
                }
            };

            removeAllListenersInVector(sceneGraphPriorityListeners);
            removeAllListenersInVector(fixedPriorityListeners);

            // Remove the dirty flag according the 'listenerID'.
            // No need to check whether the dispatcher is dispatching event.
            _priorityDirtyFlagMap.erase(listenerID);

            if (!_inDispatch) {
                listeners->clear();
                delete listeners;
                _listenerMap.erase(listenerItemIter);
            }
        }

        for (auto iter = _toAddedListeners.begin(); iter != _toAddedListeners.end();) {
            if ((*iter)->getListenerID() == listenerID) {
                (*iter)->setRegistered(false);
                (*iter)->release();
                iter = _toAddedListeners.erase(iter);
            }
            else {
                ++iter;
            }
        }
    }

    void EventDispatcher::removeEventListenersForType(EventListener::Type listenerType) {
        if (listenerType == EventListener::Type::TOUCH_ONE_BY_ONE)
        {
            removeEventListenersForListenerID(EventListenerTouchOneByOne::LISTENER_ID);
        }
        else if (listenerType == EventListener::Type::TOUCH_ALL_AT_ONCE) {
            removeEventListenersForListenerID(EventListenerTouchAllAtOnce::LISTENER_ID);
        }
        else if (listenerType == EventListener::Type::MOUSE) {
            removeEventListenersForListenerID(EventListenerMouse::LISTENER_ID);
        }
        else if (listenerType == EventListener::Type::ACCELERATION) {
            removeEventListenersForListenerID(EventListenerAcceleration::LISTENER_ID);
        }
        else if (listenerType == EventListener::Type::KEYBOARD) {
            removeEventListenersForListenerID(EventListenerKeyboard::LISTENER_ID);
        }
    }

    void EventDispatcher::removeCustomEventListeners(const std::string& customEventName) {
        removeEventListenersForListenerID(customEventName);
    }

    void EventDispatcher::removeAllEventListeners() {
        bool cleanMap = true;
        std::vector<EventListener::ListenerID> types(_listenerMap.size());

        for (const auto& e : _listenerMap) {
            if (_internalCustomListenerIDs.find(e.first) != _internalCustomListenerIDs.end()) {
                cleanMap = false;
            }
            else {
                types.push_back(e.first);
            }
        }

        for (const auto& type : types) {
            removeEventListenersForListenerID(type);
        }

        if (!_inDispatch && cleanMap) {
            _listenerMap.clear();
        }
    }

    void EventDispatcher::setEnabled(bool isEnabled) {
        _isEnabled = isEnabled;
    }

    bool EventDispatcher::isEnabled() const {
        return _isEnabled;
    }

    void EventDispatcher::setDirtyForNode(HObject* node) {
        // Mark the node dirty only when there is an eventlistener associated with it.
        if (_nodeListenersMap.find(node) != _nodeListenersMap.end()) {
            _dirtyNodes.insert(node);
        }

        // Also set the dirty flag for node's children
        const auto& children = dynamic_cast<Node *>(node)->getChildren();
        for (const auto& child : children) {
            setDirtyForNode(child);
        }
    }

    void EventDispatcher::setDirty(const EventListener::ListenerID& listenerID, DirtyFlag flag) {
        auto iter = _priorityDirtyFlagMap.find(listenerID);
        if (iter == _priorityDirtyFlagMap.end()) {
            _priorityDirtyFlagMap.insert(std::make_pair(listenerID, flag));
        }
        else {
            int ret = (int)flag | (int)iter->second;
            iter->second = (DirtyFlag) ret;
        }
    }
}
