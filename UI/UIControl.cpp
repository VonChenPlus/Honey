#include <queue>
#include <set>

#include "UI/ViewGroup.h"
#include "BASE/Mutex.h"
#include "INPUT/InputState.h"
#include "INPUT/KeyCodes.h"
using namespace _INPUT;
#include "MATH/Bounds.h"
using MATH::Bounds;
#include "UTILS/TIME/Time.h"
using UTILS::TIME::time_now_d;
#include "UI/UI.h"

namespace GLOBAL
{
    extern UI::UIState &uiState();
    extern UI::UIState &uiStateSaved();
}

namespace UI
{
    struct DispatchQueueItem
    {
        Event *e;
        EventParams params;
    };
    // Ignore deviceId when checking for matches. Turns out that Ouya for example sends
    // completely broken input where the original keypresses have deviceId = 10 and the repeats
    // have deviceId = 0.
    struct HeldKey
    {
        int key;
        int deviceId;
        double triggerTime;

        // Ignores startTime
        bool operator <(const HeldKey &other) const {
            if (key < other.key) return true;
            return false;
        }
        bool operator ==(const HeldKey &other) const { return key == other.key; }
    };
    static recursive_mutex focusLock;
    static std::vector<int> focusMoves;
    static bool focusMovementEnabled;
    static View *focusedView;
    static recursive_mutex mutex_;
    static std::deque<DispatchQueueItem> g_dispatchQueue;
    static std::set<HeldKey> heldKeys;
    const double repeatDelay = 15 * (1.0 / 60.0f);  // 15 frames like before.
    const double repeatInterval = 5 * (1.0 / 60.0f);  // 5 frames like before.
    // TODO: Figure out where this should really live.
    // Simple simulation of key repeat on platforms and for gamepads where we don't
    // automatically get it.
    static int frameCount;
    static bool focusForced;

    View *GetFocusedView() {
        return focusedView;
    }

    void SetFocusedView(View *view, bool force) {
        if (focusedView) {
            focusedView->focusChanged(FF_LOSTFOCUS);
        }
        focusedView = view;
        if (focusedView) {
            focusedView->focusChanged(FF_GOTFOCUS);
            if (force) {
                focusForced = true;
            }
        }
    }

    void EnableFocusMovement(bool enable) {
        focusMovementEnabled = enable;
        if (!enable) {
            if (focusedView) {
                focusedView->focusChanged(FF_LOSTFOCUS);
            }
            focusedView = 0;
        }
    }

    void MoveFocus(ViewGroup *root, FocusDirection direction) {
        if (!GetFocusedView()) {
            // Nothing was focused when we got in here. Focus the first non-group in the hierarchy.
            root->setFocus();
            return;
        }

        NeighborResult neigh(0, 0);
        neigh = root->findNeighbor(GetFocusedView(), direction, neigh);

        if (neigh.view) {
            neigh.view->setFocus();
            root->subviewFocused(neigh.view);
        }
    }

    bool IsFocusMovementEnabled() {
        return focusMovementEnabled;
    }

    bool KeyEvent(const KeyInput &key, ViewGroup *root) {
        bool retval = false;
        // Ignore repeats for focus moves.
        if ((key.flags & (KEY_FLAG::KEY_DOWN | KEY_FLAG::KEY_IS_REPEAT)) == KEY_DOWN) {
            // We ignore the device ID here. Anything with a DPAD is OK.
            if (key.keyCode >= NKCODE_DPAD_UP && key.keyCode <= NKCODE_DPAD_RIGHT) {
                // Let's only repeat DPAD initially.
                HeldKey hk;
                hk.key = key.keyCode;
                hk.deviceId = key.deviceId;
                hk.triggerTime = time_now_d() + repeatDelay;

                // Check if the key is already held. If it is, ignore it. This is to avoid
                // multiple key repeat mechanisms colliding.
                if (heldKeys.find(hk) != heldKeys.end()) {
                    return false;
                }

                heldKeys.insert(hk);
                lock_guard lock(focusLock);
                focusMoves.push_back(key.keyCode);
                retval = true;
            }
        }
        if (key.flags & KEY_UP) {
            // We ignore the device ID here. Anything with a DPAD is OK.
            if (key.keyCode >= NKCODE_DPAD_UP && key.keyCode <= NKCODE_DPAD_RIGHT) {
                HeldKey hk;
                hk.key = key.keyCode;
                hk.deviceId = key.deviceId;
                hk.triggerTime = 0.0; // irrelevant
                heldKeys.erase(hk);
                retval = true;
            }
        }

        retval = root->key(key);

        // Ignore volume keys and stuff here. Not elegant but need to propagate bools through the view hierarchy as well...
        switch (key.keyCode) {
        case NKCODE_VOLUME_DOWN:
        case NKCODE_VOLUME_UP:
        case NKCODE_VOLUME_MUTE:
            retval = false;
            break;
        }

        return retval;
    }


    bool TouchEvent(const TouchInput &touch, ViewGroup *root) {
        focusForced = false;
        root->touch(touch);
        if ((touch.flags & TOUCH_DOWN) && !focusForced) {
            EnableFocusMovement(false);
        }
        return true;
    }

    bool AxisEvent(const AxisInput &axis, ViewGroup *root) {
        root->axis(axis);
        return true;
    }

    void EventTriggered(Event *e, EventParams params) {
        lock_guard guard(mutex_);

        DispatchQueueItem item;
        item.e = e;
        item.params = params;
        g_dispatchQueue.push_front(item);
    }

    void DispatchEvents() {
        lock_guard guard(mutex_);

        while (!g_dispatchQueue.empty()) {
            DispatchQueueItem item = g_dispatchQueue.back();
            g_dispatchQueue.pop_back();
            if (item.e) {
                item.e->dispatch(item.params);
            }
        }
    }

    void RemoveQueuedEvents(View *v) {
        for (size_t i = 0; i < g_dispatchQueue.size(); i++) {
            if (g_dispatchQueue[i].params.v == v)
                g_dispatchQueue.erase(g_dispatchQueue.begin() + i);
        }
    }

    static void ProcessHeldKeys(ViewGroup *root) {
        double now = time_now_d();

    restart:

        for (std::set<HeldKey>::iterator iter = heldKeys.begin(); iter != heldKeys.end(); ++iter) {
            if (iter->triggerTime < now) {
                KeyInput key;
                key.keyCode = iter->key;
                key.deviceId = iter->deviceId;
                key.flags = KEY_DOWN;
                KeyEvent(key, root);

                lock_guard lock(focusLock);
                focusMoves.push_back(key.keyCode);

                // Cannot modify the current item when looping over a set, so let's do this instead.
                HeldKey hk = *iter;
                heldKeys.erase(hk);
                hk.triggerTime = now + repeatInterval;
                heldKeys.insert(hk);
                goto restart;
            }
        }
    }

    void UpdateViewHierarchy(const InputState &input_state, ViewGroup *root) {
        ProcessHeldKeys(root);
        frameCount++;

        if (!root) {
            throw _NException_Normal("Tried to update a view hierarchy from a zero pointer root");
        }

        if (focusMoves.size()) {
            lock_guard lock(focusLock);
            EnableFocusMovement(true);
            if (!GetFocusedView()) {
                if (root->getDefaultFocusView()) {
                    root->getDefaultFocusView()->setFocus();
                } else {
                    root->setFocus();
                }
                root->subviewFocused(GetFocusedView());
            } else {
                for (size_t i = 0; i < focusMoves.size(); i++) {
                    switch (focusMoves[i]) {
                        case NKCODE_DPAD_LEFT: MoveFocus(root, FOCUS_LEFT); break;
                        case NKCODE_DPAD_RIGHT: MoveFocus(root, FOCUS_RIGHT); break;
                        case NKCODE_DPAD_UP: MoveFocus(root, FOCUS_UP); break;
                        case NKCODE_DPAD_DOWN: MoveFocus(root, FOCUS_DOWN); break;
                    }
                }
            }
            focusMoves.clear();
        }

        root->update(input_state);
        DispatchEvents();
    }

    void LayoutViewHierarchy(const UIContext &dc, ViewGroup *root) {
        if (!root) {
            throw _NException_Normal("Tried to layout a view hierarchy from a zero pointer root");
        }
        const Bounds &rootBounds = dc.getBounds();

        MeasureSpec horiz(EXACTLY, rootBounds.w);
        MeasureSpec vert(EXACTLY, rootBounds.h);

        // Two phases - measure contents, layout.
        root->measure(dc, horiz, vert);
        // Root has a specified size. Set it, then let root layout all its children.
        root->setBounds(rootBounds);
        root->layout();
    }

    void UIDisableBegin()
    {
        GLOBAL::uiStateSaved() = GLOBAL::uiState();
        memset(&GLOBAL::uiState(), 0, sizeof(GLOBAL::uiState()));
    }

    void UIDisableEnd()
    {
        GLOBAL::uiState() = GLOBAL::uiStateSaved();
    }
}
