#ifndef INPUTSTATE
#define INPUTSTATE

#include "BASE/BasicTypes.h"
#include "MATH/Vector.h"
using MATH::Vector3;
#include "BASE/Mutex.h"

namespace Input
{
    #ifndef MAX_POINTERS
    #define MAX_POINTERS 10
    #endif
    // Collection of all possible inputs, and automatically computed
    // deltas where applicable.
    struct InputState
    {
        // Lock this whenever you access the data in this struct.
        mutable recursive_mutex lock;
        InputState()
            : pad_buttons(0),
            pad_last_buttons(0),
            pad_buttons_down(0),
            pad_buttons_up(0),
            mouse_valid(false),
            accelerometer_valid(false)
        {
            memset(pointer_down, 0, sizeof(pointer_down));
        }

        // Gamepad style input. For ease of use.
        int pad_buttons; // bitfield
        int pad_last_buttons;
        int pad_buttons_down;	// buttons just pressed this frame
        int pad_buttons_up;	// buttons just pressed last frame
        float pad_lstick_x;
        float pad_lstick_y;
        float pad_rstick_x;
        float pad_rstick_y;
        float pad_ltrigger;
        float pad_rtrigger;

        // Mouse/touch style input
        // There are up to 8 mice / fingers.
        volatile bool mouse_valid;

        int pointer_x[MAX_POINTERS];
        int pointer_y[MAX_POINTERS];
        bool pointer_down[MAX_POINTERS];

        // Accelerometer
        bool accelerometer_valid;
        Vector3 acc;

        DISALLOW_COPY_AND_ASSIGN(InputState)
    };

    enum TOUCH_FLAG
    {
        TOUCH_MOVE = 1 << 0,
        TOUCH_DOWN = 1 << 1,
        TOUCH_UP = 1 << 2,
        TOUCH_CANCEL = 1 << 3,  // Sent by scrollviews to their children when they detect a scroll
        TOUCH_WHEEL = 1 << 4,  // Scrollwheel event. Usually only affects Y but can potentially affect X.
        TOUCH_MOUSE = 1 << 5,  // Identifies that this touch event came from a mouse

        // These are the Android getToolType() codes, shifted by 10.
        TOUCH_TOOL_MASK = 7 << 10,
        TOUCH_TOOL_UNKNOWN = 0 << 10,
        TOUCH_TOOL_FINGER = 1 << 10,
        TOUCH_TOOL_STYLUS = 2 << 10,
        TOUCH_TOOL_MOUSE = 3 << 10,
        TOUCH_TOOL_ERASER = 4 << 10,
    };

    struct TouchInput
    {
        float x;
        float y;
        int id;  // can be relied upon to be 0...MAX_POINTERS
        int flags;
        double timestamp;
    };

    #undef KEY_DOWN
    #undef KEY_UP

    enum KEY_FLAG
    {
        KEY_DOWN = 1 << 0,
        KEY_UP = 1 << 1,
        KEY_HASWHEELDELTA = 1 << 2,
        KEY_IS_REPEAT = 1 << 3,
        KEY_CHAR = 1 << 4,  // Unicode character input. Cannot detect keyups of these so KEY_DOWN and KEY_UP are zero when this is set.
    };

    struct KeyInput
    {
        KeyInput() {}
        KeyInput(int devId, int code, int fl) : deviceId(devId), keyCode(code), flags(fl) {}
        int deviceId;
        int keyCode;  // Android keycodes are the canonical keycodes, everyone else map to them.
        int flags;
    };

    struct AxisInput
    {
        int deviceId;
        int axisId;  // Android axis Ids are the canonical ones.
        float value;
        int flags;
    };
}

#endif // INPUTSTATE

