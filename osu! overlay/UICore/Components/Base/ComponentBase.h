#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <atomic>

#include "Window/DisplayWindow.h"
#include "Window/Graphics.h"
#include "Window/CursorIcon.h"
#include "Window/MouseEventHandler.h"
#include "Helper/EventEmitter.h"
#include "Helper/Time.h"

namespace zcom
{
    enum class Alignment
    {
        START,
        CENTER,
        END
    };

    // Releases the resource
    void SafeRelease(IUnknown** res);

    // Base class for component properties.
    // Used to give components properties which can be used by specific objects without adding the property to 'Base'.
    // Derived classes should implement (if necessary):
    //  - Default constructor
    //  - Copy assignment operator
    // MUST IMPLEMENT:
    //  - static std::string _NAME_();
    class Property
    {
    public:
        virtual ~Property() {};
        bool valid = true;
    };

    class Component;
    // Class that contains all components that handled an event.
    class EventTargets
    {
    public:
        struct Params
        {
            Component* target;
            int x;
            int y;
        };

    private:
        std::vector<Params> _targets;

    public:
        EventTargets()
        {
            // Reserve initial capacity to prevent reallocations in most cases
            _targets.reserve(16);
        }
        EventTargets(EventTargets&& other) noexcept
        {
            _targets = std::move(other._targets);
        }
        EventTargets& operator=(EventTargets&& other) noexcept
        {
            if (this != &other)
            {
                _targets = std::move(other._targets);
            }
            return *this;
        }
        EventTargets(const EventTargets& other) = delete;
        EventTargets& operator=(const EventTargets& other) = delete;

        EventTargets Add(Component* item, int x = std::numeric_limits<int>::min(), int y = std::numeric_limits<int>::min()) &&
        {
            _targets.push_back({ item, x, y });
            return std::move(*this);
        }

        EventTargets& Add(Component* item, int x = std::numeric_limits<int>::min(), int y = std::numeric_limits<int>::min()) &
        {
            _targets.push_back({ item, x, y });
            return *this;
        }

        void Remove(Component* item)
        {
            auto it = std::find_if(_targets.begin(), _targets.end(), [item](Params p) { return p.target == item; });
            if (it != _targets.end())
                _targets.erase(it);
        }

        void RemoveLast()
        {
            if (!_targets.empty())
                _targets.pop_back();
        }

        bool Empty() const
        {
            return _targets.empty();
        }

        size_t Size() const
        {
            return _targets.size();
        }

        bool Contains(Component* item) const
        {
            return std::find_if(_targets.begin(), _targets.end(), [item](Params p) { return p.target == item; }) != _targets.end();
        }

        Component* MainTarget() const
        {
            if (!_targets.empty())
                return _targets.front().target;
            else
                return nullptr;
        }

        std::vector<Params> GetTargets() const
        {
            return _targets;
        }
    };

    class Scene;

    // The base component class
    class Component
    {
    public:
        // Releases the resource and removes the reference
        void SafeFullRelease(IUnknown** res);

        // Adds specified function to queue of pending actions, which get executed
        // in the component thread right before the component update function
        void ExecuteSynchronously(std::function<void()>&& func)
        {
            std::lock_guard<std::mutex> lock(_m_pendingActions);
            _pendingActions.push_back(std::move(func));
        }

        // A shorthand to check whether the given coordinates represent the special invalid position
        // Invalid positions are used for invoking mouse events without a specific position
        bool CoordinatesInvalid(int x, int y)
        {
            return x == std::numeric_limits<int>::min() && y == std::numeric_limits<int>::min();
        }

        // Component creation
        template<class T, typename... Args>
        std::unique_ptr<T> Create(Args&&... args)
        {
            auto uptr = std::unique_ptr<T>(new T(_scene));
            uptr->Init(std::forward<Args>(args)...);
            return uptr;
        }
    protected:
        Scene* _scene = nullptr;
    private:

        ID2D1Bitmap1* _canvas = nullptr;
        bool _redraw = true;

        uint64_t _id = _GenerateId();
        static uint64_t _GenerateId()
        {
            static std::atomic<uint64_t> _ID_COUNTER = 0;
            return _ID_COUNTER.fetch_add(1);
        }

        // Position description
        Alignment _hPosAlign = Alignment::START;
        Alignment _vPosAlign = Alignment::START;
        float _hPosPercentOffset = 0.0f;
        float _vPosPercentOffset = 0.0f;
        int _hPosPixelOffset = 0;
        int _vPosPixelOffset = 0;

        // Size description
        float _hSizeParentPercent = 0.0f;
        float _vSizeParentPercent = 0.0f;
        int _hSize = 0;
        int _vSize = 0;

        // Main
        int _x = 0;
        int _y = 0;
        int _windowX = 0;
        int _windowY = 0;
        int _width = 100;
        int _height = 100;
        float _opacity = 1.0f;
        bool _active = true;
    protected:
        // Child components set this to true, if they
        // want to do custom rendering when inactive
        bool _customInactiveDraw = false;
    private:
        bool _visible = true;
        bool _interactable = true;

        // Rendering
        bool _ignoreAlpha = false;

        // Selection
        bool _selectable = false;
        bool _selected = false;
        int _tabIndex = -1;
        int _zIndex = -1;

        // Border
        bool _borderVisible = false;
        float _borderWidth = 1.0f;
        D2D1_COLOR_F _borderColor = D2D1::ColorF(1.0f, 1.0f, 1.0f);
        D2D1_COLOR_F _selectedBorderColor = D2D1::ColorF(0.0f, 0.5f, 0.8f);

        // Background
        D2D1_COLOR_F _backgroundColor = D2D1::ColorF(0, 0);
        ID2D1Bitmap* _background = nullptr;

        // Cursor
        zwnd::CursorIcon _cursor = zwnd::CursorIcon::ARROW;

        // Hover text
        std::wstring _hoverText = L"";
        Duration _hoverTextDelay = Duration(400, MILLISECONDS);
        TimePoint _hoverStart = ztime::Main();
        bool _hoverWaiting = false;

        // Rounding
        float _cornerRounding = 0.0f;

        // Other properties
        std::unordered_map<std::string, std::unique_ptr<Property>> _properties;

        // Synchronous execution
        std::vector<std::function<void()>> _pendingActions;
        std::mutex _m_pendingActions;

        // Mouse events
        bool _mouseInside = false;
        bool _mouseInsideArea = false;
        bool _mouseLeftClicked = false;
        bool _mouseRightClicked = false;
        int _mousePosX = 0;
        int _mousePosY = 0;

    protected:
        // Pre default handling
        EventEmitter<void, Component*, int, int, int, int> _onMouseMove;
        EventEmitter<void, Component*> _onMouseEnter;
        EventEmitter<void, Component*> _onMouseEnterArea;
        EventEmitter<void, Component*> _onMouseLeave;
        EventEmitter<void, Component*> _onMouseLeaveArea;
        EventEmitter<void, Component*, int, int> _onLeftPressed;
        EventEmitter<void, Component*, int, int> _onRightPressed;
        EventEmitter<void, Component*, int, int> _onLeftReleased;
        EventEmitter<void, Component*, int, int> _onRightReleased;
        EventEmitter<void, Component*, int, int> _onWheelUp;
        EventEmitter<void, Component*, int, int> _onWheelDown;
        EventEmitter<void, Component*, bool> _onSelected;
        EventEmitter<void, Component*> _onDeselected;
        EventEmitter<void, Component*, Graphics> _onDraw;

        // Post default handling
        EventEmitter<void, Component*, std::vector<EventTargets::Params>, int, int, int, int> _postMouseMove;
        EventEmitter<void, Component*, std::vector<EventTargets::Params>, int, int> _postLeftPressed;
        EventEmitter<void, Component*, std::vector<EventTargets::Params>, int, int> _postRightPressed;
        EventEmitter<void, Component*, std::vector<EventTargets::Params>, int, int> _postLeftReleased;
        EventEmitter<void, Component*, std::vector<EventTargets::Params>, int, int> _postRightReleased;
        EventEmitter<void, Component*, std::vector<EventTargets::Params>, int, int> _postWheelUp;
        EventEmitter<void, Component*, std::vector<EventTargets::Params>, int, int> _postWheelDown;
        EventEmitter<void, Component*, Graphics> _postDraw;

        // Layout events
        EventEmitter<void> _onLayoutChanged;

    public:
        Component(Scene* scene) : _scene(scene) {}
        virtual ~Component()
        {
            SafeFullRelease((IUnknown**)&_canvas);
        }
        Component(Component&&) = delete;
        Component& operator=(Component&&) = delete;
        Component(const Component&) = delete;
        Component& operator=(const Component&) = delete;

        // Position description
        Alignment GetHorizontalAlignment() const { return _hPosAlign; }
        Alignment GetVerticalAlignment() const { return _vPosAlign; }
        float GetHorizontalOffsetPercent() const { return _hPosPercentOffset; }
        float GetVerticalOffsetPercent() const { return _vPosPercentOffset; }
        int GetHorizontalOffsetPixels() const { return _hPosPixelOffset; }
        int GetVerticalOffsetPixels() const { return _vPosPixelOffset; }

        void SetHorizontalAlignment(Alignment alignment)
        {
            SetAlignment(alignment, GetVerticalAlignment());
        }
        void SetVerticalAlignment(Alignment alignment)
        {
            SetAlignment(GetHorizontalAlignment(), alignment);
        }
        void SetAlignment(Alignment horizontal, Alignment vertical)
        {
            bool changed = false;
            if (_hPosAlign != horizontal || _vPosAlign != vertical)
                changed = true;

            _hPosAlign = horizontal;
            _vPosAlign = vertical;

            if (changed)
                _onLayoutChanged->InvokeAll();
        }
        void SetHorizontalOffsetPercent(float offset)
        {
            SetOffsetPercent(offset, GetVerticalOffsetPercent());
        }
        void SetVerticalOffsetPercent(float offset)
        {
            SetOffsetPercent(GetHorizontalOffsetPercent(), offset);
        }
        void SetOffsetPercent(float horizontal, float vertical)
        {
            bool changed = false;
            if (_hPosPercentOffset != horizontal || _vPosPercentOffset != vertical)
                changed = true;

            _hPosPercentOffset = horizontal;
            _vPosPercentOffset = vertical;

            if (changed)
                _onLayoutChanged->InvokeAll();
        }
        void SetHorizontalOffsetPixels(int offset)
        {
            SetOffsetPixels(offset, GetVerticalOffsetPixels());
        }
        void SetVerticalOffsetPixels(int offset)
        {
            SetOffsetPixels(GetHorizontalOffsetPixels(), offset);
        }
        void SetOffsetPixels(int horizontal, int vertical)
        {
            if (_hPosPixelOffset == horizontal && _vPosPixelOffset == vertical)
                return;

            _hPosPixelOffset = horizontal;
            _vPosPixelOffset = vertical;
            _onLayoutChanged->InvokeAll();
        }

        // Size description
        float GetParentWidthPercent() const { return _hSizeParentPercent; }
        float GetParentHeightPercent() const { return _vSizeParentPercent; }
        int GetBaseWidth() const { return _hSize; }
        int GetBaseHeight() const { return _vSize; }

        void SetParentWidthPercent(float width)
        {
            SetParentSizePercent(width, GetParentHeightPercent());
        }
        void SetParentHeightPercent(float height)
        {
            SetParentSizePercent(GetParentWidthPercent(), height);
        }
        void SetParentSizePercent(float width, float height)
        {
            if (_hSizeParentPercent == width && _vSizeParentPercent == height)
                return;

            _hSizeParentPercent = width;
            _vSizeParentPercent = height;
            _onLayoutChanged->InvokeAll();
        }
        void SetBaseWidth(int width)
        {
            SetBaseSize(width, GetBaseHeight());
        }
        void SetBaseHeight(int height)
        {
            SetBaseSize(GetBaseWidth(), height);
        }
        void SetBaseSize(int width, int height)
        {
            if (_hSize == width && _vSize == height)
                return;

            _hSize = width;
            _vSize = height;
            _onLayoutChanged->InvokeAll();
        }

        // Common
        int GetX() const { return _x; }
        int GetY() const { return _y; }
        int GetWindowX() const { return _windowX; }
        int GetWindowY() const { return _windowY; }
        int GetWidth() const { return _width; }
        int GetHeight() const { return _height; }
        float GetOpacity() const { return _opacity; }
        bool GetActive() const { return _active; }
        bool GetVisible() const { return _visible; }
        bool GetInteractable() const { return _interactable; }

        void SetX(int x)
        {
            SetPosition(x, _y);
        }
        void SetY(int y)
        {
            SetPosition(_x, y);
        }
        void SetPosition(int x, int y)
        {
            _x = x;
            _y = y;
        }
        void SetWindowX(int x)
        {
            SetWindowPosition(x, _windowY);
        }
        void SetWindowY(int y)
        {
            SetWindowPosition(_windowX, y);
        }
        void SetWindowPosition(int x, int y)
        {
            _windowX = x;
            _windowY = y;
            _OnWindowPosChange(_windowX, _windowY);
        }
        void SetWidth(int width)
        {
            SetSize(width, _height);
        }
        void SetHeight(int height)
        {
            SetSize(_width, height);
        }
        void SetSize(int width, int height)
        {
            if (width == _width && height == _height)
                return;

            if (width < 0) width = 0;
            if (height < 0) height = 0;
            _width = width;
            _height = height;
            SafeFullRelease((IUnknown**)&_canvas);
            _redraw = true;
            //if (_canvas)
            //{
            //    _canvas->Release();
            //    _canvas = nullptr;
            //}
        }
        void SetOpacity(float opacity)
        {
            if (opacity == _opacity)
                return;

            _opacity = opacity;
            _redraw = true;
        }
        void SetActive(bool active)
        {
            if (active == _active)
                return;

            if (!active)
            {
                OnDeselected();
                OnLeftReleased();
                OnRightReleased();
            }
            _active = active;
            _redraw = true;
        }
        void SetVisible(bool visible)
        {
            if (visible == _visible)
                return;

            if (!visible)
            {
                OnDeselected();
                OnLeftReleased();
                OnRightReleased();
            }
            _visible = visible;
            _redraw = true;
            _onLayoutChanged->InvokeAll();
        }
        void SetInteractable(bool interactable)
        {
            if (interactable == _interactable)
                return;

            if (!interactable)
            {
                OnLeftReleased();
                OnRightReleased();
            }
            _interactable = interactable;
        }

        // Rendering
        bool IgnoreAlpha() const { return _ignoreAlpha; }

        void IgnoreAlpha(bool ignore)
        {
            if (ignore == _ignoreAlpha)
                return;

            _ignoreAlpha = ignore;
            if (_canvas)
            {
                SafeFullRelease((IUnknown**)&_canvas);
                _redraw = true;
            }
        }

        // Selection
        bool GetSelectable() const { return _selectable; }
        bool Selected() const { return _selected; }
        int GetZIndex() const { return _zIndex; }
        int GetTabIndex() const { return _tabIndex; }

        void SetSelectable(bool selectable)
        {
            _selectable = selectable;
        }
        void SetZIndex(int index)
        {
            if (index == _zIndex)
                return;

            _zIndex = index;
            _redraw = true;
        }
        void SetTabIndex(int index)
        {
            _tabIndex = index;
        }

        // Border
        bool GetBorderVisibility() const { return _borderVisible; }
        float GetBorderWidth() const { return _borderWidth; }
        D2D1_COLOR_F GetBorderColor() const { return _borderColor; }
        D2D1_COLOR_F GetSelectedBorderColor() const { return _selectedBorderColor; }

        void SetBorderVisibility(bool visible)
        {
            if (visible == _borderVisible)
                return;

            _borderVisible = visible;
            _redraw = true;
        }
        void SetBorderWidth(float width)
        {
            if (width == _borderWidth)
                return;

            _borderWidth = width;
            _redraw = true;
        }
        void SetBorderColor(D2D1_COLOR_F color)
        {
            if (color == _borderColor)
                return;

            _borderColor = color;
            _redraw = true;
        }
        void SetSelectedBorderColor(D2D1_COLOR_F color)
        {
            if (color == _selectedBorderColor)
                return;

            _selectedBorderColor = color;
            _redraw = true;
        }

        // Background
        D2D1_COLOR_F GetBackgroundColor() const { return _backgroundColor; }
        ID2D1Bitmap* GetBackgroundImage() const { return _background; }

        void SetBackgroundColor(D2D1_COLOR_F color)
        {
            if (color == _backgroundColor)
                return;

            _backgroundColor = color;
            _redraw = true;
        }
        void SetBackgroundImage(ID2D1Bitmap* image)
        {
            if (image == _background)
                return;

            _background = image;
            _redraw = true;
        }

        // Cursor
        zwnd::CursorIcon GetDefaultCursor() const { return _cursor; }

        void SetDefaultCursor(zwnd::CursorIcon cursor)
        {
            _cursor = cursor;
        }

    private:
        void _ApplyCursor();
    public:

        // Hover text
        std::wstring GetHoverText() const { return _hoverText; }
        Duration GetHoverTextDelay() const { return _hoverTextDelay; }

        void SetHoverText(std::wstring text)
        {
            _hoverText = text;
        }
        void SetHoverTextDelay(Duration delay)
        {
            _hoverTextDelay = delay;
        }

    private:
        void _ShowHoverText();
    public:

        // Corner rounding
        float GetCornerRounding() const { return _cornerRounding; }

        void SetCornerRounding(float rounding)
        {
            if (rounding == _cornerRounding)
                return;

            _cornerRounding = rounding;
            _redraw = true;
        }

        // Other properties
        template<class _Prop>
        void SetProperty(_Prop prop)
        {
            auto entry = _properties.find(_Prop::_NAME_());
            if (entry != _properties.end())
            {
                // The cast only fails when two properties have same names
                *dynamic_cast<_Prop*>(entry->second.get()) = prop;
            }
            else
            {
                auto propPtr = std::make_unique<_Prop>();
                *propPtr = prop;
                _properties.insert({ _Prop::_NAME_(), std::move(propPtr) });
            }

            // Properties might not change the visuals,
            // but optimising for that scenario is unnecessary
            _redraw = true;
        }

        template<class _Prop>
        _Prop GetProperty()
        {
            auto entry = _properties.find(_Prop::_NAME_());
            if (entry != _properties.end())
            {
                // The cast only fails when two properties have same names
                _Prop prop = *dynamic_cast<_Prop*>(entry->second.get());
                prop.valid = true;
                return prop;
            }
            else
            {
                _Prop prop{};
                prop.valid = false;
                return prop;
            }
        }

        template<class _Prop>
        void RemoveProperty()
        {
            _properties.erase(_Prop::_NAME_());

            // Properties might not change the visuals,
            // but optimising for that scenario is unnecessary
            _redraw = true;
        }

        // Mouse events
        EventTargets OnMouseMove(int x, int y)
        {
            if (!_active)
                return EventTargets();

            int deltaX = x - _mousePosX;
            int deltaY = y - _mousePosY;
            _mousePosX = x;
            _mousePosY = y;
            _onMouseMove->InvokeAll(this, x, y, deltaX, deltaY);
            auto targets = _OnMouseMove(x, y, deltaX, deltaY);
            _postMouseMove->InvokeAll(this, targets.GetTargets(), x, y, deltaX, deltaY);

            if (!targets.Empty())
                OnMouseEnter();
            if (targets.Size() == 1 && targets.MainTarget() == this)
            {
                // Set cursor
                _ApplyCursor();

                // Show hover text
                if (!_hoverText.empty())
                {
                    _hoverStart = ztime::Main();
                    _hoverWaiting = true;
                }
            }
            return targets;
        }
        void OnMouseEnter()
        {
            if (!_active)
                return;
            if (_mouseInside)
                return;

            _mouseInside = true;
            _onMouseEnter->InvokeAll(this);
            _OnMouseEnter();
        }
        void OnMouseLeave()
        {
            if (!_active)
                return;
            if (!_mouseInside)
                return;

            _mouseInside = false;
            _onMouseLeave->InvokeAll(this);
            _OnMouseLeave();
            _hoverWaiting = false;
        }
        void OnMouseEnterArea()
        {
            if (!_active)
                return;
            if (_mouseInsideArea)
                return;

            _mouseInsideArea = true;
            _onMouseEnterArea->InvokeAll(this);
            _OnMouseEnterArea();
        }
        void OnMouseLeaveArea()
        {
            if (!_active)
                return;
            if (!_mouseInsideArea)
                return;

            _mouseInsideArea = false;
            _onMouseLeaveArea->InvokeAll(this);
            _OnMouseLeaveArea();
        }
        EventTargets OnLeftPressed(int x, int y)
        {
            if (!_active)
                return EventTargets();

            // Correct mouse position if it doesn't match click position
            if (_mousePosX != x || _mousePosY != y)
                OnMouseMove(x, y);

            _mouseLeftClicked = true;
            _onLeftPressed->InvokeAll(this, x, y);
            auto targets = _OnLeftPressed(x, y);
            _postLeftPressed->InvokeAll(this, targets.GetTargets(), x, y);
            return targets;
        }
        EventTargets OnLeftReleased(int x = std::numeric_limits<int>::min(), int y = std::numeric_limits<int>::min())
        {
            if (!_active)
                return EventTargets();

            // Correct mouse position if it doesn't match release position
            if (!CoordinatesInvalid(x, y) && (_mousePosX != x || _mousePosY != y))
                OnMouseMove(x, y);

            _mouseLeftClicked = false;
            _onLeftReleased->InvokeAll(this, x, y);
            auto targets = _OnLeftReleased(x, y);
            _postLeftReleased->InvokeAll(this, targets.GetTargets(), x, y);
            return targets;
        }
        EventTargets OnRightPressed(int x, int y)
        {
            if (!_active)
                return EventTargets();

            // Correct mouse position if it doesn't match click position
            if (_mousePosX != x || _mousePosY != y)
                OnMouseMove(x, y);

            _mouseRightClicked = true;
            _onRightPressed->InvokeAll(this, x, y);
            auto targets = _OnRightPressed(x, y);
            _postRightPressed->InvokeAll(this, targets.GetTargets(), x, y);
            return targets;
        }
        EventTargets OnRightReleased(int x = std::numeric_limits<int>::min(), int y = std::numeric_limits<int>::min())
        {
            if (!_active)
                return EventTargets();

            // Correct mouse position if it doesn't match release position
            if (!CoordinatesInvalid(x, y) && (_mousePosX != x || _mousePosY != y))
                OnMouseMove(x, y);

            _mouseRightClicked = false;
            _onRightReleased->InvokeAll(this, x, y);
            auto targets = _OnRightReleased(x, y);
            _postRightReleased->InvokeAll(this, targets.GetTargets(), x, y);
            return targets;
        }
        EventTargets OnWheelUp(int x, int y)
        {
            if (!_active)
                return EventTargets();

            _onWheelUp->InvokeAll(this, x, y);
            auto targets = _OnWheelUp(x, y);
            _postWheelUp->InvokeAll(this, targets.GetTargets(), x, y);
            return targets;
        }
        EventTargets OnWheelDown(int x, int y)
        {
            if (!_active)
                return EventTargets();

            _onWheelDown->InvokeAll(this, x, y);
            auto targets = _OnWheelDown(x, y);
            _postWheelDown->InvokeAll(this, targets.GetTargets(), x, y);
            return targets;
        }
        void OnSelected(bool reverse = false)
        {
            if (!_active)
                return;
            if (_selected)
                return;

            _selected = true;
            _redraw = true;
            _onSelected->InvokeAll(this, reverse);
            _OnSelected(reverse);
        }
        void OnDeselected()
        {
            if (!_active)
                return;
            if (!_selected)
                return;

            _selected = false;
            _redraw = true;
            _onDeselected->InvokeAll(this);
            _OnDeselected();
        }
    protected:
        virtual EventTargets _OnMouseMove(int x, int y, int deltaX, int deltaY) { return EventTargets().Add(this, x, y); }
        virtual void _OnMouseEnter() {}
        virtual void _OnMouseLeave() {}
        virtual void _OnMouseEnterArea() {}
        virtual void _OnMouseLeaveArea() {}
        virtual EventTargets _OnLeftPressed(int x, int y) { return EventTargets().Add(this, x, y); }
        virtual EventTargets _OnRightPressed(int x, int y) { return EventTargets().Add(this, x, y); }
        virtual EventTargets _OnLeftReleased(int x = std::numeric_limits<int>::min(), int y = std::numeric_limits<int>::min()) { return EventTargets().Add(this, x, y); }
        virtual EventTargets _OnRightReleased(int x = std::numeric_limits<int>::min(), int y = std::numeric_limits<int>::min()) { return EventTargets().Add(this, x, y); }
        virtual EventTargets _OnWheelUp(int x, int y) { return EventTargets(); }
        virtual EventTargets _OnWheelDown(int x, int y) { return EventTargets(); }
        virtual void _OnSelected(bool reverse) {}
        virtual void _OnDeselected() {}
    public:
        bool GetMouseInside() const { return _mouseInside; }
        bool GetMouseInsideArea() const { return _mouseInsideArea; }
        bool GetMouseLeftClicked() const { return _mouseLeftClicked; }
        bool GetMouseRightClicked() const { return _mouseRightClicked; }
        int GetMousePosX() const { return _mousePosX; }
        int GetMousePosY() const { return _mousePosY; }

        EventSubscription<void, Component*, int, int, int, int> SubscribeOnMouseMove(std::function<void(Component*, int, int, int, int)> handler)
        {
            return _onMouseMove->Subscribe(handler);
        }
        EventSubscription<void, Component*> SubscribeOnMouseEnter(std::function<void(Component*)> handler)
        {
            return _onMouseEnter->Subscribe(handler);
        }
        EventSubscription<void, Component*> SubscribeOnMouseLeave(std::function<void(Component*)> handler)
        {
            return _onMouseLeave->Subscribe(handler);
        }
        EventSubscription<void, Component*> SubscribeOnMouseEnterArea(std::function<void(Component*)> handler)
        {
            return _onMouseEnterArea->Subscribe(handler);
        }
        EventSubscription<void, Component*> SubscribeOnMouseLeaveArea(std::function<void(Component*)> handler)
        {
            return _onMouseLeaveArea->Subscribe(handler);
        }
        EventSubscription<void, Component*, int, int> SubscribeOnLeftPressed(std::function<void(Component*, int, int)> handler)
        {
            return _onLeftPressed->Subscribe(handler);
        }
        EventSubscription<void, Component*, int, int> SubscribeOnRightPressed(std::function<void(Component*, int, int)> handler)
        {
            return _onRightPressed->Subscribe(handler);
        }
        EventSubscription<void, Component*, int, int> SubscribeOnLeftReleased(std::function<void(Component*, int, int)> handler)
        {
            return _onLeftReleased->Subscribe(handler);
        }
        EventSubscription<void, Component*, int, int> SubscribeOnRightReleased(std::function<void(Component*, int, int)> handler)
        {
            return _onRightReleased->Subscribe(handler);
        }
        EventSubscription<void, Component*, int, int> SubscribeOnWheelUp(std::function<void(Component*, int, int)> handler)
        {
            return _onWheelUp->Subscribe(handler);
        }
        EventSubscription<void, Component*, int, int> SubscribeOnWheelDown(std::function<void(Component*, int, int)> handler)
        {
            return _onWheelDown->Subscribe(handler);
        }
        EventSubscription<void, Component*, bool> SubscribeOnSelected(std::function<void(Component*, bool)> handler)
        {
            return _onSelected->Subscribe(handler);
        }
        EventSubscription<void, Component*> SubscribeOnDeselected(std::function<void(Component*)> handler)
        {
            return _onDeselected->Subscribe(handler);
        }
        EventSubscription<void, Component*, Graphics> SubscribeOnDraw(std::function<void(Component*, Graphics)> handler)
        {
            return _onDraw->Subscribe(handler);
        }

        EventSubscription<void, Component*, std::vector<EventTargets::Params>, int, int, int, int> SubscribePostMouseMove(std::function<void(Component*, std::vector<EventTargets::Params>, int, int, int, int)> handler)
        {
            return _postMouseMove->Subscribe(handler);
        }
        EventSubscription<void, Component*, std::vector<EventTargets::Params>, int, int> SubscribePostLeftPressed(std::function<void(Component*, std::vector<EventTargets::Params>, int, int)> handler)
        {
            return _postLeftPressed->Subscribe(handler);
        }
        EventSubscription<void, Component*, std::vector<EventTargets::Params>, int, int> SubscribePostRightPressed(std::function<void(Component*, std::vector<EventTargets::Params>, int, int)> handler)
        {
            return _postRightPressed->Subscribe(handler);
        }
        EventSubscription<void, Component*, std::vector<EventTargets::Params>, int, int> SubscribePostLeftReleased(std::function<void(Component*, std::vector<EventTargets::Params>, int, int)> handler)
        {
            return _postLeftReleased->Subscribe(handler);
        }
        EventSubscription<void, Component*, std::vector<EventTargets::Params>, int, int> SubscribePostRightReleased(std::function<void(Component*, std::vector<EventTargets::Params>, int, int)> handler)
        {
            return _postRightReleased->Subscribe(handler);
        }
        EventSubscription<void, Component*, std::vector<EventTargets::Params>, int, int> SubscribePostWheelUp(std::function<void(Component*, std::vector<EventTargets::Params>, int, int)> handler)
        {
            return _postWheelUp->Subscribe(handler);
        }
        EventSubscription<void, Component*, std::vector<EventTargets::Params>, int, int> SubscribePostWheelDown(std::function<void(Component*, std::vector<EventTargets::Params>, int, int)> handler)
        {
            return _postWheelDown->Subscribe(handler);
        }
        EventSubscription<void, Component*, Graphics> SubscribePostDraw(std::function<void(Component*, Graphics)> handler)
        {
            return _postDraw->Subscribe(handler);
        }

        // Layout events

        EventSubscription<void> SubscribeOnLayoutChanged(std::function<void()> handler)
        {
            return _onLayoutChanged->Subscribe(handler);
        }
        // Emits the layout change event. Useful when 
        void NotifyLayoutChanged()
        {
            _onLayoutChanged->InvokeAll();
        }

        // //////////////
        // Main functions
        // //////////////

        virtual void Update()
        {
            //if (!_active) return;

            // Execute pending actions
            // A (possibly expensive, but not relevant for now) copy of the pending actions is
            // created to allow the pending action itself call 'ExecutePending'.
            // TODO: This safeguard is not always necessary, so it would make sense to provide
            // the ability to disable this behavior for a component (or maybe opt-in)
            std::unique_lock<std::mutex> lock(_m_pendingActions);
            std::vector<std::function<void()>> pendingActionsCopy = _pendingActions;
            _pendingActions.clear();
            lock.unlock();
            for (auto& action : pendingActionsCopy)
                action();

            // Show hover text
            if (_hoverWaiting && (ztime::Main() - _hoverStart) >= _hoverTextDelay)
            {
                _hoverWaiting = false;
                _ShowHoverText();
            }

            _OnUpdate();
        }

        // If this function returns true, the 'Draw()' function should be called
        // to redraw any visual changes
        virtual bool Redraw()
        {
            return _redraw || !_canvas || _Redraw();
        }

        virtual void InvokeRedraw()
        {
            _redraw = true;
        }

        virtual ID2D1Bitmap* Draw(Graphics g)
        {
            _redraw = false;

            //static int counter = 0;
            //std::cout << counter++ << '\n';

            if (_width == 0 || _height == 0)
                return nullptr;

            if (!_canvas)
            {
                g.target->CreateBitmap(
                    D2D1::SizeU(_width, _height),
                    nullptr,
                    0,
                    D2D1::BitmapProperties1(
                        D2D1_BITMAP_OPTIONS_TARGET,
                        { DXGI_FORMAT_B8G8R8A8_UNORM, _ignoreAlpha ? D2D1_ALPHA_MODE_IGNORE : D2D1_ALPHA_MODE_PREMULTIPLIED }
                    ),
                    &_canvas
                );
                g.refs->push_back({ (IUnknown**)&_canvas, std::string("Base canvas: ") + GetName() });
            }

            // Stash current target
            ID2D1Image* target;
            g.target->GetTarget(&target);

            // Set canvas as target
            g.target->SetTarget(_canvas);
            g.target->Clear();

            if (_visible)
            {
                // Invoke pre draw handlers
                _onDraw->InvokeAll(this, g);

                ID2D1Image* stash = nullptr;
                ID2D1Bitmap1* contentBitmap = nullptr;
                const float rounding = _cornerRounding; // Use const value in this function in case '_cornerRouding' is modified

                if (rounding > 0.0f)
                {
                    // Create separate target for content
                    g.target->CreateBitmap(
                        D2D1::SizeU(_width, _height),
                        nullptr,
                        0,
                        D2D1::BitmapProperties1(
                            D2D1_BITMAP_OPTIONS_TARGET,
                            { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }
                        ),
                        &contentBitmap
                    );
                    g.target->GetTarget(&stash);
                    g.target->SetTarget(contentBitmap);
                }

                // Draw background
                g.target->Clear(GetBackgroundColor());
                if (_background)
                {
                    g.target->DrawBitmap
                    (
                        _background,
                        D2D1::RectF(0, 0, g.target->GetSize().width, g.target->GetSize().height)
                    );
                }

                // Draw component
                _OnDraw(g);

                // Draw border
                if (_borderVisible)
                {
                    ID2D1SolidColorBrush* borderBrush = nullptr;
                    g.target->CreateSolidColorBrush(_borderColor, &borderBrush);
                    if (borderBrush)
                    {
                        float offset = _borderWidth * 0.5f;
                        if (rounding > 0.0f)
                        {
                            D2D1_ROUNDED_RECT roundedrect{};
                            roundedrect.radiusX = _cornerRounding - offset;
                            roundedrect.radiusY = _cornerRounding - offset;
                            roundedrect.rect = D2D1::RectF(offset, offset, _width - offset, _height - offset);
                            g.target->DrawRoundedRectangle(roundedrect, borderBrush, _borderWidth);
                        }
                        else
                        {
                            g.target->DrawRectangle(D2D1::RectF(offset, offset, _width - offset, _height - offset), borderBrush, _borderWidth);
                        }
                        borderBrush->Release();
                    }
                }
                if (_selected)
                {
                    ID2D1SolidColorBrush* borderBrush = nullptr;
                    g.target->CreateSolidColorBrush(_selectedBorderColor, &borderBrush);
                    if (borderBrush)
                    {
                        float offset = _borderWidth * 0.5f;
                        if (rounding > 0.0f)
                        {
                            D2D1_ROUNDED_RECT roundedrect{};
                            roundedrect.radiusX = _cornerRounding - offset;
                            roundedrect.radiusY = _cornerRounding - offset;
                            roundedrect.rect = D2D1::RectF(offset, offset, _width - offset, _height - offset);
                            g.target->DrawRoundedRectangle(roundedrect, borderBrush, _borderWidth);
                        }
                        else
                        {
                            g.target->DrawRectangle(D2D1::RectF(offset, offset, _width - offset, _height - offset), borderBrush, _borderWidth);
                        }
                        borderBrush->Release();
                    }
                }

                if (rounding > 0.0f)
                {
                    // Round corners
                    ID2D1Bitmap1* opacityMask = nullptr;
                    g.target->CreateBitmap(
                        D2D1::SizeU(_width, _height),
                        nullptr,
                        0,
                        D2D1::BitmapProperties1(
                            D2D1_BITMAP_OPTIONS_TARGET,
                            { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }
                        ),
                        &opacityMask
                    );
                    g.target->SetTarget(opacityMask);
                    g.target->Clear();

                    D2D1_ROUNDED_RECT roundedrect{};
                    roundedrect.radiusX = _cornerRounding;
                    roundedrect.radiusY = _cornerRounding;
                    roundedrect.rect.left = 0;
                    roundedrect.rect.top = 0;
                    roundedrect.rect.right = (float)_width;
                    roundedrect.rect.bottom = (float)_height;
                    ID2D1SolidColorBrush* opacityBrush;
                    g.target->CreateSolidColorBrush(D2D1::ColorF(0), &opacityBrush);
                    g.target->FillRoundedRectangle(roundedrect, opacityBrush);
                    opacityBrush->Release();

                    g.target->SetTarget(stash);
                    stash->Release();

                    ID2D1BitmapBrush* bitmapBrush;
                    g.target->CreateBitmapBrush(
                        contentBitmap,
                        D2D1::BitmapBrushProperties(
                            D2D1_EXTEND_MODE_CLAMP,
                            D2D1_EXTEND_MODE_CLAMP,
                            D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
                        ),
                        &bitmapBrush
                    );

                    g.target->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
                    g.target->FillOpacityMask(opacityMask, bitmapBrush, D2D1_OPACITY_MASK_CONTENT_GRAPHICS);
                    g.target->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                    g.target->Flush();
                    bitmapBrush->Release();
                    opacityMask->Release();
                    contentBitmap->Release();
                }

                // If inactive, gray out the canvas
                if (!_active && !_customInactiveDraw)
                {
                    ID2D1Bitmap1* grayscaleBitmap = nullptr;
                    g.target->CreateBitmap(
                        D2D1::SizeU(_width, _height),
                        nullptr,
                        0,
                        D2D1::BitmapProperties1(
                            D2D1_BITMAP_OPTIONS_TARGET,
                            { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }
                        ),
                        &grayscaleBitmap
                    );

                    ID2D1Effect* grayscaleEffect;
                    g.target->CreateEffect(CLSID_D2D1Grayscale, &grayscaleEffect);
                    grayscaleEffect->SetInput(0, _canvas);
                    ID2D1Effect* brightnessEffect;
                    g.target->CreateEffect(CLSID_D2D1Brightness, &brightnessEffect);
                    brightnessEffect->SetInputEffect(0, grayscaleEffect);
                    brightnessEffect->SetValue(D2D1_BRIGHTNESS_PROP_WHITE_POINT, D2D1::Vector2F(1.0f, 0.6f));
                    brightnessEffect->SetValue(D2D1_BRIGHTNESS_PROP_BLACK_POINT, D2D1::Vector2F(1.0f, 0.6f));

                    g.target->GetTarget(&stash);
                    g.target->SetTarget(grayscaleBitmap);
                    g.target->Clear();
                    g.target->DrawImage(brightnessEffect);
                    g.target->SetTarget(stash);
                    g.target->Clear();
                    g.target->DrawBitmap(grayscaleBitmap);

                    stash->Release();
                    brightnessEffect->Release();
                    grayscaleEffect->Release();
                    grayscaleBitmap->Release();
                }

                // Invoke post draw handlers
                _postDraw->InvokeAll(this, g);
            }
            else if (_Redraw())
            {
                _OnDraw(g);
                g.target->Clear();
            }

            // Unstash target
            g.target->SetTarget(target);
            target->Release();

            return _canvas;
        }

        virtual ID2D1Bitmap* ContentImage()
        {
            return _canvas;
        }

        virtual void Resize(int width, int height)
        {
            if (width != _width || height != _height)
            {
                SetSize(width, height);
                _OnResize(_width, _height);
            }
        }

        // ////////////////////
        // Additional functions
        // ////////////////////

        virtual std::list<Component*> GetChildren()
        {
            return std::list<Component*>();
        }

        virtual std::list<Component*> GetAllChildren()
        {
            return std::list<Component*>();
        }

        virtual Component* IterateTab(bool reverse = false)
        {
            if (!Selected())
                return this;
            else
                return nullptr;
        }

    protected:
        virtual void _OnUpdate() {}
        virtual bool _Redraw() { return false; }
        virtual void _OnDraw(Graphics g) {}
        virtual void _OnResize(int width, int height) {}
        virtual void _OnWindowPosChange(int x, int y) {}

    public:
        virtual const char* GetName() const { return "base"; }
    };

}

//#include "Scene.h"
//
//template<class T, typename... Args>
//std::unique_ptr<T> zcom::Base::Create(Args&&... args)
//{
//    return _scene->Create(std::forward<Args>(args)...);
//}