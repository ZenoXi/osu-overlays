#pragma once

#include <cstdint>

namespace zwnd
{
    class WindowMessage
    {
    public:
        const char* id;
        unsigned char data[32];
    };

    class WindowMoveMessage
    {
    public:
        int x;
        int y;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(int*)(msg.data + 0) = x;
            *(int*)(msg.data + 4) = y;
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            x = *(int*)(msg.data + 0);
            y = *(int*)(msg.data + 4);
            return true;
        }

        static const char* ID() { return "window_move"; }
    };

    class WindowSizeMessage
    {
    public:
        int width;
        int height;
        bool maximized = false;
        bool minimized = false;
        bool restored = false;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(int*)(msg.data + 0) = width;
            *(int*)(msg.data + 4) = height;
            *(bool*)(msg.data + 8) = maximized;
            *(bool*)(msg.data + 9) = minimized;
            *(bool*)(msg.data + 10) = restored;
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            width = *(int*)(msg.data + 0);
            height = *(int*)(msg.data + 4);
            maximized = *(bool*)(msg.data + 8);
            minimized = *(bool*)(msg.data + 9);
            restored = *(bool*)(msg.data + 10);
            return true;
        }

        static const char* ID() { return "window_size"; }
    };

    class WindowCloseMessage
    {
    public:
        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;
            return true;
        }

        static const char* ID() { return "window_close"; }
    };

    class MouseMoveMessage
    {
    public:
        int x;
        int y;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(int*)(msg.data + 0) = x;
            *(int*)(msg.data + 4) = y;
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            x = *(int*)(msg.data + 0);
            y = *(int*)(msg.data + 4);
            return true;
        }

        static const char* ID() { return "mouse_move"; }
    };

    class MouseEnterMessage
    {
    public:
        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;
            return true;
        }

        static const char* ID() { return "mouse_enter"; }
    };

    class MouseLeaveMessage
    {
    public:
        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;
            return true;
        }

        static const char* ID() { return "mouse_leave"; }
    };

    class MouseLeftPressedMessage
    {
    public:
        int x;
        int y;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(int*)(msg.data + 0) = x;
            *(int*)(msg.data + 4) = y;
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            x = *(int*)(msg.data + 0);
            y = *(int*)(msg.data + 4);
            return true;
        }

        static const char* ID() { return "mouse_left_pressed"; }
    };

    class MouseLeftReleasedMessage
    {
    public:
        int x;
        int y;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(int*)(msg.data + 0) = x;
            *(int*)(msg.data + 4) = y;
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            x = *(int*)(msg.data + 0);
            y = *(int*)(msg.data + 4);
            return true;
        }

        static const char* ID() { return "mouse_left_released"; }
    };

    class MouseRightPressedMessage
    {
    public:
        int x;
        int y;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(int*)(msg.data + 0) = x;
            *(int*)(msg.data + 4) = y;
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            x = *(int*)(msg.data + 0);
            y = *(int*)(msg.data + 4);
            return true;
        }

        static const char* ID() { return "mouse_right_pressed"; }
    };

    class MouseRightReleasedMessage
    {
    public:
        int x;
        int y;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(int*)(msg.data + 0) = x;
            *(int*)(msg.data + 4) = y;
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            x = *(int*)(msg.data + 0);
            y = *(int*)(msg.data + 4);
            return true;
        }

        static const char* ID() { return "mouse_right_released"; }
    };

    class NonClientMouseLeftPressedMessage
    {
    public:
        int x;
        int y;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(int*)(msg.data + 0) = x;
            *(int*)(msg.data + 4) = y;
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            x = *(int*)(msg.data + 0);
            y = *(int*)(msg.data + 4);
            return true;
        }

        static const char* ID() { return "mouse_left_pressed"; }
    };

    class NonClientMouseLeftReleasedMessage
    {
    public:
        int x;
        int y;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(int*)(msg.data + 0) = x;
            *(int*)(msg.data + 4) = y;
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            x = *(int*)(msg.data + 0);
            y = *(int*)(msg.data + 4);
            return true;
        }

        static const char* ID() { return "mouse_left_released"; }
    };

    class NonClientMouseRightPressedMessage
    {
    public:
        int x;
        int y;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(int*)(msg.data + 0) = x;
            *(int*)(msg.data + 4) = y;
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            x = *(int*)(msg.data + 0);
            y = *(int*)(msg.data + 4);
            return true;
        }

        static const char* ID() { return "mouse_right_pressed"; }
    };

    class NonClientMouseRightReleasedMessage
    {
    public:
        int x;
        int y;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(int*)(msg.data + 0) = x;
            *(int*)(msg.data + 4) = y;
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            x = *(int*)(msg.data + 0);
            y = *(int*)(msg.data + 4);
            return true;
        }

        static const char* ID() { return "mouse_right_released"; }
    };

    class MouseWheelUpMessage
    {
    public:
        int x;
        int y;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(int*)(msg.data + 0) = x;
            *(int*)(msg.data + 4) = y;
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            x = *(int*)(msg.data + 0);
            y = *(int*)(msg.data + 4);
            return true;
        }

        static const char* ID() { return "mouse_wheel_up"; }
    };

    class MouseWheelDownMessage
    {
    public:
        int x;
        int y;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(int*)(msg.data + 0) = x;
            *(int*)(msg.data + 4) = y;
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            x = *(int*)(msg.data + 0);
            y = *(int*)(msg.data + 4);
            return true;
        }

        static const char* ID() { return "mouse_wheel_down"; }
    };

    class KeyDownMessage
    {
    public:
        uint8_t keyCode;

        // The following values are extracted from the WM_KEYDOWN message
        // See the Microsoft docs for more information

        uint16_t repeatCount;
        uint8_t scanCode;
        bool isExtended;
        bool repeated;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(uint8_t*)(msg.data + 0) = keyCode;
            *(uint16_t*)(msg.data + 1) = repeatCount;
            *(uint8_t*)(msg.data + 3) = scanCode;
            *(uint8_t*)(msg.data + 4) = (isExtended ? 1 : 0);
            *(uint8_t*)(msg.data + 5) = (repeated ? 1 : 0);
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            keyCode = *(uint8_t*)(msg.data + 0);
            repeatCount = *(uint16_t*)(msg.data + 1);
            scanCode = *(uint8_t*)(msg.data + 3);
            isExtended = (*(uint8_t*)(msg.data + 4) == 1 ? true : false);
            repeated = (*(uint8_t*)(msg.data + 5) == 1 ? true : false);
            return true;
        }

        static const char* ID() { return "key_down"; }
    };

    class KeyUpMessage
    {
    public:
        uint8_t keyCode;

        // The following values are extracted from the WM_KEYUP message
        // See the Microsoft docs for more information

        uint8_t scanCode;
        bool isExtended;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(uint8_t*)(msg.data + 0) = keyCode;
            *(uint8_t*)(msg.data + 1) = scanCode;
            *(uint8_t*)(msg.data + 2) = (isExtended ? 1 : 0);
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            keyCode = *(uint8_t*)(msg.data + 0);
            scanCode = *(uint8_t*)(msg.data + 1);
            isExtended = (*(uint8_t*)(msg.data + 2) == 1 ? true : false);
            return true;
        }

        static const char* ID() { return "key_up"; }
    };

    class CharMessage
    {
    public:
        wchar_t character;

        // The following values are extracted from the WM_CHAR message
        // See the Microsoft docs for more information

        uint16_t repeatCount;
        uint8_t scanCode;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(wchar_t*)(msg.data + 0) = character;
            *(uint16_t*)(msg.data + 2) = repeatCount;
            *(uint8_t*)(msg.data + 4) = scanCode;
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            character = *(wchar_t*)(msg.data + 0);
            repeatCount = *(uint16_t*)(msg.data + 2);
            scanCode = *(uint8_t*)(msg.data + 4);
            return true;
        }

        static const char* ID() { return "char"; }
    };

    class MouseInputMessage
    {
    public:
        int deltaX;
        int deltaY;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(int*)(msg.data + 0) = deltaX;
            *(int*)(msg.data + 4) = deltaY;
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            deltaX = *(int*)(msg.data + 0);
            deltaY = *(int*)(msg.data + 4);
            return true;
        }

        static const char* ID() { return "input"; }
    };

    class WindowActivateMessage
    {
    public:
        enum {
            ACTIVATED = 1,
            CLICK_ACTIVATED = 2,
            DEACTIVATED = 0
        };
        int activationType;

        WindowMessage Encode()
        {
            WindowMessage msg;
            msg.id = ID();
            *(int*)(msg.data) = activationType;
            return msg;
        }

        bool Decode(WindowMessage msg)
        {
            if (msg.id != ID())
                return false;

            activationType = *(int*)(msg.data);
            return true;
        }

        static const char* ID() { return "window_activate"; }
    };
}