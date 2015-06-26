#ifndef INPUTSTATE_H
#define INPUTSTATE_H

#include "BASE/Honey.h"
#include "MATH/Vector.h"
#include "BASE/Mutex.h"

namespace UI
{
    // Default device IDs

    enum
    {
        DEVICE_ID_DEFAULT = 0,  // Old Android
        DEVICE_ID_KEYBOARD = 1,  // PC keyboard, android keyboards
        DEVICE_ID_MOUSE = 2,  // PC mouse only (not touchscreen!)
        DEVICE_ID_PAD_0 = 10,  // Generic joypads
        DEVICE_ID_PAD_1 = 11,  // these should stay as contiguous numbers
        DEVICE_ID_PAD_2 = 12,
        DEVICE_ID_PAD_3 = 13,
        DEVICE_ID_PAD_4 = 14,
        DEVICE_ID_PAD_5 = 15,
        DEVICE_ID_PAD_6 = 16,
        DEVICE_ID_PAD_7 = 17,
        DEVICE_ID_PAD_8 = 18,
        DEVICE_ID_PAD_9 = 19,
        DEVICE_ID_X360_0 = 20,  // XInput joypads
        DEVICE_ID_ACCELEROMETER = 30,
    };
    enum
    {
        PAD_BUTTON_A = 1,
        PAD_BUTTON_B = 2,
        PAD_BUTTON_X = 4,
        PAD_BUTTON_Y = 8,
        PAD_BUTTON_LBUMPER = 16,
        PAD_BUTTON_RBUMPER = 32,
        PAD_BUTTON_START = 64,
        PAD_BUTTON_SELECT = 128,
        PAD_BUTTON_UP = 256,
        PAD_BUTTON_DOWN = 512,
        PAD_BUTTON_LEFT = 1024,
        PAD_BUTTON_RIGHT = 2048,

        PAD_BUTTON_MENU = 4096,
        PAD_BUTTON_BACK = 8192,

        // For Blackberry and Qt
        PAD_BUTTON_JOY_UP = 1<<14,
        PAD_BUTTON_JOY_DOWN = 1<<15,
        PAD_BUTTON_JOY_LEFT = 1<<16,
        PAD_BUTTON_JOY_RIGHT = 1<<17,

        PAD_BUTTON_LEFT_THUMB = 1 << 18,   // Click left thumb stick on X360
        PAD_BUTTON_RIGHT_THUMB = 1 << 19,   // Click right thumb stick on X360

        PAD_BUTTON_LEFT_TRIGGER = 1 << 21,   // Click left thumb stick on X360
        PAD_BUTTON_RIGHT_TRIGGER = 1 << 22,   // Click left thumb stick on X360

        PAD_BUTTON_UNTHROTTLE = 1 << 20, // Click Tab to unthrottle
    };

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
            accelerometer_valid(false) {
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
        MATH::Vector3 acc;

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

#endif // INPUTSTATE_H

