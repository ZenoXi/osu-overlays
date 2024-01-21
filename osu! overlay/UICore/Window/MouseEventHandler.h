#pragma once

class MouseEventHandler
{
    //friend class DisplayWindow;

protected:
    bool _mouseInside = false;
    bool _mouseLeftClicked = false;
    bool _mouseRightClicked = false;
    int _mousePosX = 0;
    int _mousePosY = 0;

    virtual bool _OnMouseMove(int x, int y) { return false; }
    virtual void _OnMouseLeave() {}
    virtual void _OnMouseEnter() {}
    virtual bool _OnLeftPressed(int x, int y) { return false; }
    virtual bool _OnLeftReleased(int x, int y) { return false; }
    virtual bool _OnRightPressed(int x, int y) { return false; }
    virtual bool _OnRightReleased(int x, int y) { return false; }
    virtual bool _OnWheelUp(int x, int y) { return false; }
    virtual bool _OnWheelDown(int x, int y) { return false; }

public:
    bool OnMouseMove(int x, int y)
    {
        _mousePosX = x;
        _mousePosY = y;
        return _OnMouseMove(x, y);
    }
    void OnMouseLeave()
    {
        _mouseInside = false;
        _OnMouseLeave();
    }
    void OnMouseEnter()
    {
        _mouseInside = true;
        _OnMouseEnter();
    }
    bool OnLeftPressed(int x, int y)
    {
        _mouseLeftClicked = true;
        return _OnLeftPressed(x, y);
    }
    bool OnLeftReleased(int x, int y)
    {
        _mouseLeftClicked = false;
        return _OnLeftReleased(x, y);
    }
    bool OnRightPressed(int x, int y)
    {
        _mouseRightClicked = true;
        return _OnRightPressed(x, y);
    }
    bool OnRightReleased(int x, int y)
    {
        _mouseRightClicked = false;
        return _OnRightReleased(x, y);
    }
    bool OnWheelUp(int x, int y)
    {
        return _OnWheelUp(x, y);
    }
    bool OnWheelDown(int x, int y)
    {
        return _OnWheelDown(x, y);
    }

    bool MouseInside() { return _mouseInside; }
    bool MouseLeftClicked() { return _mouseLeftClicked; }
    bool MouseRightClicked() { return _mouseRightClicked; }
    int MousePosX() { return _mousePosX; }
    int MousePosY() { return _mousePosY; }
};