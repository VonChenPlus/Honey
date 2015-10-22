#ifndef EVENTDISPATCHER_H
#define EVENTDISPATCHER_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include "BASE/HObject.h"
#include "GRAPH/EventListener.h"

namespace GRAPH
{
    class Event;
    class EventTouch;
    class EventCustom;
    class EventListenerCustom;

    class EventDispatcher : public HObject
    {
    public:
        void addEventListenerWithSceneGraphPriority(EventListener* listener, HObject* target);
        void addEventListenerWithFixedPriority(EventListener* listener, int fixedPriority);
        EventListenerCustom* addCustomEventListener(const std::string &eventName, const std::function<void(EventCustom*)>& callback);

        void removeEventListener(EventListener* listener);
        void removeEventListenersForType(EventListener::Type listenerType);
        void removeEventListenersForTarget(HObject* target, bool recursive = false);
        void removeCustomEventListeners(const std::string& customEventName);
        void removeAllEventListeners();

        void pauseEventListenersForTarget(HObject* target, bool recursive = false);
        void resumeEventListenersForTarget(HObject* target, bool recursive = false);

        void setPriority(EventListener* listener, int fixedPriority);
        void setEnabled(bool isEnabled);
        bool isEnabled() const;

        void dispatchEvent(Event* event);
        void dispatchCustomEvent(const std::string &eventName, void *optionalUserData = nullptr);

        EventDispatcher();
        ~EventDispatcher();

    protected:
        void setDirtyForNode(HObject* target);

        class EventListenerVector
        {
        public:
            EventListenerVector();
            ~EventListenerVector();
            uint64 size() const;
            bool empty() const;

            void push_back(EventListener* item);
            void clearSceneGraphListeners();
            void clearFixedListeners();
            void clear();

            inline std::vector<EventListener*>* getFixedPriorityListeners() const { return _fixedListeners; }
            inline std::vector<EventListener*>* getSceneGraphPriorityListeners() const { return _sceneGraphListeners; }
            inline uint64 getGt0Index() const { return _gt0Index; }
            inline void setGt0Index(uint64 index) { _gt0Index = index; }
        private:
            std::vector<EventListener*>* _fixedListeners;
            std::vector<EventListener*>* _sceneGraphListeners;
            uint64 _gt0Index;
        };

        void addEventListener(EventListener* listener);
        void forceAddEventListener(EventListener* listener);

        EventListenerVector* getListeners(const EventListener::ListenerID& listenerID);

        void updateDirtyFlagForSceneGraph();

        void removeEventListenersForListenerID(const EventListener::ListenerID& listenerID);

        void sortEventListeners(const EventListener::ListenerID& listenerID);
        void sortEventListenersOfSceneGraphPriority(const EventListener::ListenerID& listenerID, HObject* rootTarget);
        void sortEventListenersOfFixedPriority(const EventListener::ListenerID& listenerID);

        void updateListeners(Event* event);

        void dispatchTouchEvent(EventTouch* event);
        void dispatchEventToListeners(EventListenerVector* listeners, const std::function<bool(EventListener*)>& onEvent);
        void dispatchTouchEventToListeners(EventListenerVector* listeners, const std::function<bool(EventListener*)>& onEvent);

        void associateNodeAndEventListener(HObject* target, EventListener* listener);
        void dissociateNodeAndEventListener(HObject* target, EventListener* listener);

        enum class DirtyFlag
        {
            NONE = 0,
            FIXED_PRIORITY = 1 << 0,
            SCENE_GRAPH_PRIORITY = 1 << 1,
            ALL = FIXED_PRIORITY | SCENE_GRAPH_PRIORITY
        };

        void setDirty(const EventListener::ListenerID& listenerID, DirtyFlag flag);

        void visitTarget(HObject* target, bool isRootTarget);

    private:
        std::unordered_map<EventListener::ListenerID, EventListenerVector*> listenerMap_;
        std::unordered_map<EventListener::ListenerID, DirtyFlag> priorityDirtyFlagMap_;
        std::unordered_map<HObject*, std::vector<EventListener*>*> nodeListenersMap_;
        std::unordered_map<HObject*, int> nodePriorityMap_;
        std::unordered_map<float, std::vector<HObject*>> globalZOrderNodeMap_;
        std::vector<EventListener*> toAddedListeners_;
        std::set<HObject*> dirtyNodes_;
        int inDispatch_;
        bool isEnabled_;
        int nodePriorityIndex_;
        std::set<std::string> internalCustomListenerIDs_;

        friend class Node;
    };
}

#endif // EVENTDISPATCHER_H
