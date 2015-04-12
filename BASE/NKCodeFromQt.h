#include "BASE/ConstMap.h"
#include <map>
#include "INPUT/KeyCodes.h"
using namespace _INPUT;

// TODO: Add any missing keys 
static const std::map<int, int> KeyMapRawQttoNative = ConstMap<int, int>
	(Qt::Key_P, NKCODE_P)
	(Qt::Key_O, NKCODE_O)
	(Qt::Key_I, NKCODE_I)
	(Qt::Key_U, NKCODE_U)
	(Qt::Key_Y, NKCODE_Y)
	(Qt::Key_T, NKCODE_T)
	(Qt::Key_R, NKCODE_R)
	(Qt::Key_E, NKCODE_E)
	(Qt::Key_W, NKCODE_W)
	(Qt::Key_Q, NKCODE_Q)
	(Qt::Key_L, NKCODE_L)
	(Qt::Key_K, NKCODE_K)
	(Qt::Key_J, NKCODE_J)
	(Qt::Key_H, NKCODE_H)
	(Qt::Key_G, NKCODE_G)
	(Qt::Key_F, NKCODE_F)
	(Qt::Key_D, NKCODE_D)
	(Qt::Key_S, NKCODE_S)
	(Qt::Key_A, NKCODE_A)
	(Qt::Key_M, NKCODE_M)
	(Qt::Key_N, NKCODE_N)
	(Qt::Key_B, NKCODE_B)
	(Qt::Key_V, NKCODE_V)
	(Qt::Key_C, NKCODE_C)
	(Qt::Key_X, NKCODE_X)
	(Qt::Key_Z, NKCODE_Z)
	(Qt::Key_P + 0x20, NKCODE_P)
	(Qt::Key_O + 0x20, NKCODE_O)
	(Qt::Key_I + 0x20, NKCODE_I)
	(Qt::Key_U + 0x20, NKCODE_U)
	(Qt::Key_Y + 0x20, NKCODE_Y)
	(Qt::Key_T + 0x20, NKCODE_T)
	(Qt::Key_R + 0x20, NKCODE_R)
	(Qt::Key_E + 0x20, NKCODE_E)
	(Qt::Key_W + 0x20, NKCODE_W)
	(Qt::Key_Q + 0x20, NKCODE_Q)
	(Qt::Key_L + 0x20, NKCODE_L)
	(Qt::Key_K + 0x20, NKCODE_K)
	(Qt::Key_J + 0x20, NKCODE_J)
	(Qt::Key_H + 0x20, NKCODE_H)
	(Qt::Key_G + 0x20, NKCODE_G)
	(Qt::Key_F + 0x20, NKCODE_F)
	(Qt::Key_D + 0x20, NKCODE_D)
	(Qt::Key_S + 0x20, NKCODE_S)
	(Qt::Key_A + 0x20, NKCODE_A)
	(Qt::Key_M + 0x20, NKCODE_M)
	(Qt::Key_N + 0x20, NKCODE_N)
	(Qt::Key_B + 0x20, NKCODE_B)
	(Qt::Key_V + 0x20, NKCODE_V)
	(Qt::Key_C + 0x20, NKCODE_C)
	(Qt::Key_X + 0x20, NKCODE_X)
	(Qt::Key_Z + 0x20, NKCODE_Z)
	(Qt::Key_Comma, NKCODE_COMMA)
	(Qt::Key_Period, NKCODE_PERIOD)
	(Qt::Key_Alt, NKCODE_ALT_LEFT)
	(Qt::Key_Shift, NKCODE_SHIFT_LEFT)
	(Qt::Key_Tab, NKCODE_TAB)
	(Qt::Key_Space, NKCODE_SPACE)
	(Qt::Key_Return, NKCODE_ENTER)
	(Qt::Key_Minus, NKCODE_MINUS)
	(Qt::Key_Minus, NKCODE_PLUS)
	(Qt::Key_Equal, NKCODE_EQUALS)
	(Qt::Key_BracketLeft, NKCODE_LEFT_BRACKET)
	(Qt::Key_BracketRight, NKCODE_RIGHT_BRACKET)
	(Qt::Key_Backslash, NKCODE_BACKSLASH)
	(Qt::Key_Semicolon, NKCODE_SEMICOLON)
	(Qt::Key_Apostrophe, NKCODE_APOSTROPHE)
	(Qt::Key_Slash, NKCODE_SLASH)
	(Qt::Key_At, NKCODE_AT)
	(Qt::Key_PageUp, NKCODE_PAGE_UP)
	(Qt::Key_PageDown, NKCODE_PAGE_DOWN)
	(Qt::Key_Escape, NKCODE_ESCAPE)
	(Qt::Key_Delete, NKCODE_FORWARD_DEL)
	(Qt::Key_Control, NKCODE_CTRL_LEFT)
	(Qt::Key_CapsLock, NKCODE_CAPS_LOCK)
	(Qt::Key_Home, NKCODE_MOVE_HOME)
	(Qt::Key_End, NKCODE_MOVE_END)
	(Qt::Key_Insert, NKCODE_INSERT)
	(Qt::Key_Period, NKCODE_NUMPAD_DOT)
	(Qt::Key_Enter, NKCODE_NUMPAD_ENTER)
	(Qt::Key_1, NKCODE_1)
	(Qt::Key_2, NKCODE_2)
	(Qt::Key_3, NKCODE_3)
	(Qt::Key_4, NKCODE_4)
	(Qt::Key_5, NKCODE_5)
	(Qt::Key_6, NKCODE_6)
	(Qt::Key_7, NKCODE_7)
	(Qt::Key_8, NKCODE_8)
	(Qt::Key_9, NKCODE_9)
	(Qt::Key_0, NKCODE_0)
	(Qt::Key_Left, NKCODE_DPAD_LEFT)
	(Qt::Key_Up, NKCODE_DPAD_UP)
	(Qt::Key_Right, NKCODE_DPAD_RIGHT)
	(Qt::Key_Down, NKCODE_DPAD_DOWN)
	(Qt::Key_Back, NKCODE_BACK);

