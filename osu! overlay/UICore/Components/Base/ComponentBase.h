#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <atomic>

#include "UICore/Components/ComHelper.h"
#include "UICore/Window/Graphics.h"
#include "UICore/Window/CursorIcon.h"
#include "UICore/Window/MouseEventHandler.h"
#include "UICore/Model/Color.h"
#include "UICore/Helper/EventEmitter.h"
#include "UICore/Helper/Value.h"
#include "UICore/Helper/Time.h"
#include "UICore/Helper/CollectionsHelper.h"
#include "UICore/Helper/ValueProxy.h"

// Component boilerplate. All components except Canvas should have this exact class setup
#define DEFINE_COMPONENT(component_name, parent) \
public: \
const char* GetName() const override { return Name(); } \
static const char* Name() { return #component_name; } \
protected: \
friend class Scene; \
friend class Component; \
component_name(Scene* scene) : parent(scene) {} \
public: \
component_name(component_name&&) = delete; \
component_name& operator=(component_name&&) = delete; \
component_name(const component_name&) = delete; \
component_name& operator=(const component_name&) = delete; \
private:

#define DEFAULT_DESTRUCTOR(component_name) \
public: \
~component_name() {} \
private:

#define DEFAULT_INIT \
protected: \
void Init() {} \
private:

namespace zcom
{
    enum class Alignment
    {
        START,
        CENTER,
        END
    };
    constexpr std::vector<std::pair<int64_t, std::wstring>> AlignmentValueProxySelectionValues()
    {
        return {
            { (int64_t)Alignment::START, L"Start" },
            { (int64_t)Alignment::CENTER, L"Center" },
            { (int64_t)Alignment::END, L"End" }
        };
    }

    struct ContentBitmap
    {
        Rect rect;
        ID2D1Bitmap1* bitmap;
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
    class EventContext
    {
    public:
        struct Params
        {
            Component* target;
            std::optional<Point> point;
        };

        bool cursorIconSet = false;
        bool hoverTextHandled = false;

    private:
        std::vector<Params> _targets;

    public:
        EventContext()
        {
            // Reserve initial capacity to prevent reallocations in most cases
            _targets.reserve(16);
        }
        EventContext(EventContext&& other) noexcept
        {
            _targets = std::move(other._targets);
            cursorIconSet = other.cursorIconSet;
            hoverTextHandled = other.hoverTextHandled;
        }
        EventContext& operator=(EventContext&& other) noexcept
        {
            if (this != &other)
            {
                _targets = std::move(other._targets);
                cursorIconSet = other.cursorIconSet;
                hoverTextHandled = other.hoverTextHandled;
            }
            return *this;
        }
        EventContext(const EventContext& other) = delete;
        EventContext& operator=(const EventContext& other) = delete;

        EventContext Add(Component* item, std::optional<Point> point) &&
        {
            _targets.push_back({ item, point });
            return std::move(*this);
        }
        EventContext& Add(Component* item, std::optional<Point> point) &
        {
            _targets.push_back({ item, point });
            return *this;
        }
        EventContext Add(Component* item) &&
        {
            _targets.push_back({ item, std::nullopt });
            return std::move(*this);
        }
        EventContext& Add(Component* item) &
        {
            _targets.push_back({ item, std::nullopt });
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
        void ExecuteSynchronously(std::function<void()>&& func, Duration delay = Duration(0))
        {
            std::lock_guard<std::mutex> lock(_m_pendingActions);
            _pendingActions.push_back({ std::move(func), ztime::Main() + delay });
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
    public:
        Scene* GetScene() const { return _scene; }

    protected:
        std::optional<Bitmap> _canvas = std::nullopt;
    private:
        bool _redraw = true;

        uint64_t _id = _GenerateId();
        static uint64_t _GenerateId()
        {
            static std::atomic<uint64_t> _ID_COUNTER = 1;
            return _ID_COUNTER.fetch_add(1);
        }
    public:
        uint64_t GetId() const { return _id; }

    private:
        template<typename T>
        void _SetValueWithLayoutChange(T& currentValue, const T& newValue)
        {
            currentValue = newValue;
            _onLayoutChanged->InvokeAll();
        }
        template<typename T>
        void _SetValueWithRedraw(T& currentValue, const T& newValue)
        {
            currentValue = newValue;
            _redraw = true;
        }
    public:
        Value<Size> size = Value<Size>({ 0, 0 }, [=](Size& cur, const Size& new_) { _SetValueWithLayoutChange(cur, new_); });
        Value<SizeF> parentSize = Value<SizeF>({ 0.0f, 0.0f }, [=](SizeF& cur, const SizeF& new_) { _SetValueWithLayoutChange(cur, new_); });
        Value<Point> position = Value<Point>({ 0, 0 }, [=](Point& cur, const Point& new_) { _SetValueWithLayoutChange(cur, new_); });
        Value<PointF> parentPosition = Value<PointF>({ 0.0f, 0.0f }, [=](PointF& cur, const PointF& new_) { _SetValueWithLayoutChange(cur, new_); });
        Value<Alignment> xAlign = Value<Alignment>(Alignment::START, [=](Alignment& cur, const Alignment& new_) { _SetValueWithLayoutChange(cur, new_); });
        Value<Alignment> yAlign = Value<Alignment>(Alignment::START, [=](Alignment& cur, const Alignment& new_) { _SetValueWithLayoutChange(cur, new_); });

        Value<float> opacity = Value<float>(1.0f, [=](float& cur, const float& new_) { _SetValueWithRedraw(cur, new_); });
        Value<bool> disabled = Value<bool>(false, [=](bool& currentValue, const bool& disabled) {
            if (disabled)
            {
                OnDeselected();
                OnLeftReleased();
                OnRightReleased();
                OnMouseLeaveArea();
                OnMouseLeave();
            }
            _SetValueWithRedraw(currentValue, disabled);
        });
        Value<bool> visible = Value<bool>(true, [=](bool& currentValue, const bool& visible) {
            if (!visible)
            {
                OnDeselected();
                OnLeftReleased();
                OnRightReleased();
                OnMouseLeaveArea();
                OnMouseLeave();
            }
            currentValue = visible;
            _redraw = true;
            _onLayoutChanged->InvokeAll();
        });
        Value<bool> interactable = Value<bool>(true, [=](bool& currentValue, const bool& interactable) {
            if (!interactable)
            {
                OnLeftReleased();
                OnRightReleased();
                OnMouseLeaveArea();
                OnMouseLeave();
            }
            currentValue = interactable;
        });
        Value<int> zIndex = Value<int>(-1, [=](int& cur, const int& new_) { _SetValueWithRedraw(cur, new_); });
        Value<bool> eatScrollEvents = false;
        Value<bool> ignoreAlpha = Value<bool>(false, [=](bool& currentValue, const bool& ignoreAlpha) {
            currentValue = ignoreAlpha;
            if (_canvas)
            {
                _canvas = std::nullopt;
                _redraw = true;
            }
        });
        
        Value<bool> selectable = false;
        Value<int> tabIndex = -1;

        struct Border
        {
            Value<bool> visible;
            Value<float> width;
            Value<float> cornerRadius;
            Value<Color> color;
            Value<Color> selectedColor;
        };
        Border border = {
            .visible = Value<bool>(false, [=](bool& cur, const bool& new_) { _SetValueWithRedraw(cur, new_); }),
            .width = Value<float>(1.0f, [=](float& cur, const float& new_) { _SetValueWithRedraw(cur, new_); }),
            .cornerRadius = Value<float>(0.0f, [=](float& cur, const float& new_) { _SetValueWithRedraw(cur, new_); }),
            .color = Value<Color>(Color(0xFFFFFF), [=](Color& cur, const Color& new_) { _SetValueWithRedraw(cur, new_); }),
            .selectedColor = Value<Color>(Color(0x0080CA), [=](Color& cur, const Color& new_) { _SetValueWithRedraw(cur, new_); })
        };

        Value<Color> backgroundColor = Value<Color>(Color(), [=](Color& cur, const Color& new_) { _SetValueWithRedraw(cur, new_); });
        Value<std::optional<Bitmap>> backgroundImage = Value<std::optional<Bitmap>>(std::optional<Bitmap>(std::nullopt), [=](std::optional<Bitmap>& cur, std::optional<Bitmap> const& new_) { _SetValueWithRedraw(cur, new_); });

        Value<zwnd::CursorIcon> cursorIcon = zwnd::CursorIcon::ARROW;

        Value<std::wstring> hoverText = std::wstring();
        Value<Duration> hoverTextDelay = Duration(400, MILLISECONDS);
    private:
        TimePoint _hoverStart = ztime::Main();
        bool _hoverWaiting = false;
    public:

        Value<Size> selfSize_ = Value<Size>({ 0, 0 }, [=](Size& cur, const Size& new_) { _SetValueWithLayoutChange(cur, new_); });

        Value<Size> size_ = Size{ 0, 0 };
        Value<Point> position_ = Point{ 0, 0 };
        Value<Point> windowPosition_ = Point{ 0, 0 };

        void SetSize(Size size)
        {
            if (size.width < 0) size.width = 0;
            if (size.height < 0) size.height = 0;
            if (size_ == size)
                return;

            size_ = size;
            if (_canvas)
                _canvas = std::nullopt;
            _redraw = true;
        }
        void SetPosition(Point position)
        {
            position_ = position;
        }
        void SetWindowPosition(Point position)
        {
            if (windowPosition_ == position)
                return;

            windowPosition_ = position;
            _OnWindowPosChange(windowPosition_);
        }

        Value<bool> selected_ = false;

        Value<bool> hovered_ = false;
        Value<bool> hoveredArea_ = false;
        Value<bool> leftClicked_ = false;
        Value<bool> rightClicked_ = false;
        Value<Point> mousePosition_ = Point{ 0, 0 };

    protected:
        // Child components set this to true, if they
        // want to do custom rendering when inactive
        bool _customInactiveDraw = false;
    private:

        // Other properties
        std::unordered_map<std::string, std::unique_ptr<Property>> _properties;
        std::unordered_set<std::string> _tags;
        std::unordered_map<std::string, std::unique_ptr<Value<int>>> _styleComputers;

        // Synchronous execution
        struct PendingAction
        {
            std::function<void()> action;
            TimePoint executionTime;
        };
        std::vector<PendingAction> _pendingActions;
        std::mutex _m_pendingActions;

    protected:
        // Pre default handling
        EventEmitter<void, Component*, Point, Point> _onMouseMove;
        EventEmitter<void, Component*> _onMouseEnter;
        EventEmitter<void, Component*> _onMouseEnterArea;
        EventEmitter<void, Component*> _onMouseLeave;
        EventEmitter<void, Component*> _onMouseLeaveArea;
        EventEmitter<void, Component*, Point> _onLeftPressed;
        EventEmitter<void, Component*, Point> _onRightPressed;
        EventEmitter<void, Component*, std::optional<Point>> _onLeftReleased;
        EventEmitter<void, Component*, std::optional<Point>> _onRightReleased;
        EventEmitter<void, Component*, Point> _onWheelUp;
        EventEmitter<void, Component*, Point> _onWheelDown;
        EventEmitter<void, Component*, bool> _onSelected;
        EventEmitter<void, Component*> _onDeselected;
        EventEmitter<void, Component*, Graphics*> _onDraw;
        EventEmitter<void, Component*, Graphics*> _preContentDraw;

        // Post default handling
        EventEmitter<void, Component*, std::vector<EventContext::Params>, Point, Point> _postMouseMove;
        EventEmitter<void, Component*, std::vector<EventContext::Params>, Point> _postLeftPressed;
        EventEmitter<void, Component*, std::vector<EventContext::Params>, Point> _postRightPressed;
        EventEmitter<void, Component*, std::vector<EventContext::Params>, std::optional<Point>> _postLeftReleased;
        EventEmitter<void, Component*, std::vector<EventContext::Params>, std::optional<Point>> _postRightReleased;
        EventEmitter<void, Component*, std::vector<EventContext::Params>, Point> _postWheelUp;
        EventEmitter<void, Component*, std::vector<EventContext::Params>, Point> _postWheelDown;
        EventEmitter<void> _postUpdate;
        EventEmitter<void, Component*, Graphics*> _postDraw;
        EventEmitter<void, Component*, Graphics*> _postContentDraw;

        // Layout events
        EventEmitter<void> _onLayoutChanged;

        // Destroy event
        EventEmitter<void, Component*> _onDestroyed;

    public:
        Component(Scene* scene) : _scene(scene) {}
        virtual ~Component()
        {
            if (_canvas)
                _canvas = std::nullopt;
            _onDestroyed->InvokeAll(this);
        }
        Component(Component&&) = delete;
        Component& operator=(Component&&) = delete;
        Component(const Component&) = delete;
        Component& operator=(const Component&) = delete;

    private:
        void _ApplyCursor();
        void _ShowHoverText();
        void _HideHoverText();
    public:

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
            _onLayoutChanged->InvokeAll();
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

        // Apply application defined tag. Tags are a simpler version of properties, mainly for development convenience
        void AddTag(std::string tag)
        {
            _tags.insert(tag);
        }

        bool HasTag(std::string tag)
        {
            return _tags.count(tag) > 0;
        }

        void RemoveTag(std::string tag)
        {
            _tags.erase(tag);
        }

        template <typename T>
        struct identity
        {
            typedef T type;
        };

        template<class... _Val>
        void SetComputedStyle(std::string styleName, typename identity<std::function<void(Component*, _Val...)>>::type consumer, Value<_Val>&... vals)
        {
            if (!_styleComputers.contains(styleName))
            {
                auto value = std::make_unique<Value<int>>(0);
                value->ComputedFrom([component = this, consumer](_Val... args) {
                    consumer(component, args...);
                    return 0;
                }, vals...);
                _styleComputers.insert({ styleName, std::move(value) });
            }
            else
            {
                _styleComputers[styleName]->ComputedFrom([component = this, consumer](_Val... args) {
                    consumer(component, args...);
                    return 0;
                }, vals...);
            }
        }

        void RemoveComputedStyle(std::string styleName)
        {
            _styleComputers.erase(styleName);
        }

        // Mouse events
        EventContext OnMouseMove(Point point)
        {
            if (disabled)
                return EventContext();

            Point deltaPos = point - mousePosition_;
            mousePosition_ = point;
            _onMouseMove->InvokeAll(this, point, deltaPos);
            auto context = _OnMouseMove(point, deltaPos);
            _postMouseMove->InvokeAll(this, context.GetTargets(), point, deltaPos);

            if (!context.Empty())
                OnMouseEnter();

            if (!context.cursorIconSet && cursorIcon != zwnd::CursorIcon::ARROW)
            {
                _ApplyCursor();
                context.cursorIconSet = true;
            }
            if (!context.hoverTextHandled && !hoverText->empty())
            {
                _hoverStart = ztime::Main();
                _hoverWaiting = true;
                context.hoverTextHandled = true;
            }

            return context;
        }
        void OnMouseEnter()
        {
            if (disabled || hovered_)
                return;

            hovered_ = true;
            _onMouseEnter->InvokeAll(this);
            _OnMouseEnter();
        }
        void OnMouseLeave()
        {
            if (disabled || !hovered_)
                return;

            hovered_ = false;
            _onMouseLeave->InvokeAll(this);
            _OnMouseLeave();
            _hoverWaiting = false;
            _HideHoverText();
        }
        void OnMouseEnterArea()
        {
            if (disabled || hoveredArea_)
                return;

            hoveredArea_ = true;
            _onMouseEnterArea->InvokeAll(this);
            _OnMouseEnterArea();
        }
        void OnMouseLeaveArea()
        {
            if (disabled || !hoveredArea_)
                return;

            hoveredArea_ = false;
            _onMouseLeaveArea->InvokeAll(this);
            _OnMouseLeaveArea();
        }
        EventContext OnLeftPressed(Point point)
        {
            if (disabled || leftClicked_)
                return EventContext();

            // Correct mouse position if it doesn't match click position
            if (mousePosition_ != point)
                OnMouseMove(point);

            leftClicked_ = true;
            _onLeftPressed->InvokeAll(this, point);
            auto context = _OnLeftPressed(point);
            _postLeftPressed->InvokeAll(this, context.GetTargets(), point);
            return context;
        }
        EventContext OnLeftReleased(std::optional<Point> point = std::nullopt)
        {
            if (disabled || !leftClicked_)
                return EventContext();

            // Correct mouse position if it doesn't match release position
            if (point.has_value() && mousePosition_ != point.value())
                OnMouseMove(point.value());

            leftClicked_ = false;
            _onLeftReleased->InvokeAll(this, point);
            auto context = _OnLeftReleased(point);
            _postLeftReleased->InvokeAll(this, context.GetTargets(), point);
            return context;
        }
        EventContext OnRightPressed(Point point)
        {
            if (disabled || rightClicked_)
                return EventContext();

            // Correct mouse position if it doesn't match click position
            if (mousePosition_ != point)
                OnMouseMove(point);

            rightClicked_ = true;
            _onRightPressed->InvokeAll(this, point);
            auto context = _OnRightPressed(point);
            _postRightPressed->InvokeAll(this, context.GetTargets(), point);
            return context;
        }
        EventContext OnRightReleased(std::optional<Point> point = std::nullopt)
        {
            if (disabled || !rightClicked_)
                return EventContext();

            // Correct mouse position if it doesn't match release position
            if (point.has_value() && mousePosition_ != point.value())
                OnMouseMove(point.value());

            rightClicked_ = false;
            _onRightReleased->InvokeAll(this, point);
            auto context = _OnRightReleased(point);
            _postRightReleased->InvokeAll(this, context.GetTargets(), point);
            return context;
        }
        EventContext OnWheelUp(Point point)
        {
            if (disabled)
                return EventContext();

            _onWheelUp->InvokeAll(this, point);
            auto context = _OnWheelUp(point);
            _postWheelUp->InvokeAll(this, context.GetTargets(), point);
            return context;
        }
        EventContext OnWheelDown(Point point)
        {
            if (disabled)
                return EventContext();

            _onWheelDown->InvokeAll(this, point);
            auto context = _OnWheelDown(point);
            _postWheelDown->InvokeAll(this, context.GetTargets(), point);
            return context;
        }
        void OnSelected(bool reverse = false)
        {
            if (disabled || selected_)
                return;

            selected_ = true;
            _redraw = true;
            _onSelected->InvokeAll(this, reverse);
            _OnSelected(reverse);
        }
        void OnDeselected()
        {
            if (disabled || !selected_)
                return;

            selected_ = false;
            _redraw = true;
            _onDeselected->InvokeAll(this);
            _OnDeselected();
        }
    protected:
        virtual EventContext _OnMouseMove(Point point, Point deltaPos) { return EventContext().Add(this, point); }
        virtual void _OnMouseEnter() {}
        virtual void _OnMouseLeave() {}
        virtual void _OnMouseEnterArea() {}
        virtual void _OnMouseLeaveArea() {}
        virtual EventContext _OnLeftPressed(Point point) { return EventContext().Add(this, point); }
        virtual EventContext _OnRightPressed(Point point) { return EventContext().Add(this, point); }
        virtual EventContext _OnLeftReleased(std::optional<Point> point) { return EventContext().Add(this, point); }
        virtual EventContext _OnRightReleased(std::optional<Point> point) { return EventContext().Add(this, point); }
        virtual EventContext _OnWheelUp(Point point) { return eatScrollEvents ? EventContext().Add(this, point) : EventContext(); }
        virtual EventContext _OnWheelDown(Point point) { return eatScrollEvents ? EventContext().Add(this, point) : EventContext(); }
        virtual void _OnSelected(bool reverse) {}
        virtual void _OnDeselected() {}
    public:
        [[nodiscard]] EventSubscription<void, Component*, Point, Point> SubscribeOnMouseMove(std::function<void(Component*, Point, Point)> handler)
        {
            return _onMouseMove->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*> SubscribeOnMouseEnter(std::function<void(Component*)> handler)
        {
            return _onMouseEnter->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*> SubscribeOnMouseLeave(std::function<void(Component*)> handler)
        {
            return _onMouseLeave->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*> SubscribeOnMouseEnterArea(std::function<void(Component*)> handler)
        {
            return _onMouseEnterArea->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*> SubscribeOnMouseLeaveArea(std::function<void(Component*)> handler)
        {
            return _onMouseLeaveArea->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, Point> SubscribeOnLeftPressed(std::function<void(Component*, Point)> handler)
        {
            return _onLeftPressed->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, Point> SubscribeOnRightPressed(std::function<void(Component*, Point)> handler)
        {
            return _onRightPressed->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, std::optional<Point>> SubscribeOnLeftReleased(std::function<void(Component*, std::optional<Point>)> handler)
        {
            return _onLeftReleased->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, std::optional<Point>> SubscribeOnRightReleased(std::function<void(Component*, std::optional<Point>)> handler)
        {
            return _onRightReleased->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, Point> SubscribeOnWheelUp(std::function<void(Component*, Point)> handler)
        {
            return _onWheelUp->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, Point> SubscribeOnWheelDown(std::function<void(Component*, Point)> handler)
        {
            return _onWheelDown->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, bool> SubscribeOnSelected(std::function<void(Component*, bool)> handler)
        {
            return _onSelected->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*> SubscribeOnDeselected(std::function<void(Component*)> handler)
        {
            return _onDeselected->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, Graphics*> SubscribeOnDraw(std::function<void(Component*, Graphics*)> handler)
        {
            return _onDraw->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, Graphics*> SubscribePreContentDraw(std::function<void(Component*, Graphics*)> handler)
        {
            return _preContentDraw->Subscribe(handler);
        }

        [[nodiscard]] EventSubscription<void, Component*, std::vector<EventContext::Params>, Point, Point> SubscribePostMouseMove(std::function<void(Component*, std::vector<EventContext::Params>, Point, Point)> handler)
        {
            return _postMouseMove->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, std::vector<EventContext::Params>, Point> SubscribePostLeftPressed(std::function<void(Component*, std::vector<EventContext::Params>, Point)> handler)
        {
            return _postLeftPressed->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, std::vector<EventContext::Params>, Point> SubscribePostRightPressed(std::function<void(Component*, std::vector<EventContext::Params>, Point)> handler)
        {
            return _postRightPressed->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, std::vector<EventContext::Params>, std::optional<Point>> SubscribePostLeftReleased(std::function<void(Component*, std::vector<EventContext::Params>, std::optional<Point>)> handler)
        {
            return _postLeftReleased->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, std::vector<EventContext::Params>, std::optional<Point>> SubscribePostRightReleased(std::function<void(Component*, std::vector<EventContext::Params>, std::optional<Point>)> handler)
        {
            return _postRightReleased->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, std::vector<EventContext::Params>, Point> SubscribePostWheelUp(std::function<void(Component*, std::vector<EventContext::Params>, Point)> handler)
        {
            return _postWheelUp->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, std::vector<EventContext::Params>, Point> SubscribePostWheelDown(std::function<void(Component*, std::vector<EventContext::Params>, Point)> handler)
        {
            return _postWheelDown->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void> SubscribePostUpdate(std::function<void()> handler)
        {
            return _postUpdate->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, Graphics*> SubscribePostDraw(std::function<void(Component*, Graphics*)> handler)
        {
            return _postDraw->Subscribe(handler);
        }
        [[nodiscard]] EventSubscription<void, Component*, Graphics*> SubscribePostContentDraw(std::function<void(Component*, Graphics*)> handler)
        {
            return _postContentDraw->Subscribe(handler);
        }

        // Layout events

        [[nodiscard]] EventSubscription<void> SubscribeOnLayoutChanged(std::function<void()> handler)
        {
            return _onLayoutChanged->Subscribe(handler);
        }
        // Emits the layout change event. Useful when 
        void NotifyLayoutChanged()
        {
            _onLayoutChanged->InvokeAll();
        }

        // Destroy events

        // NOTE: Handling destruction events should be done with extra caution, since they
        // often fire in the middle of complex component destruction, when other members can
        // already be destroyed, possibly leading to bad access if component logic is invoked
        [[nodiscard]] EventSubscription<void, Component*> SubscribeOnDestroyed(std::function<void(Component*)> handler)
        {
            return _onDestroyed->Subscribe(handler);
        }

        // //////////////
        // Main functions
        // //////////////

        virtual void Update();
        // If this function returns true, the 'Draw()' function should be called to redraw any visual changes
        virtual bool Redraw();
        virtual void InvokeRedraw() { _redraw = true; }
        virtual std::optional<Bitmap> Draw(Graphics* g);
        virtual std::optional<Bitmap> ContentImage();
        virtual void Resize(Size size);

        // ////////////////////
        // Additional functions
        // ////////////////////

        virtual std::vector<Component*> GetChildren()
        {
            return std::vector<Component*>();
        }

        virtual std::vector<Component*> GetAllChildren()
        {
            return std::vector<Component*>();
        }

        virtual Component* IterateTab(bool reverse = false)
        {
            if (!selected_)
                return this;
            else
                return nullptr;
        }

    protected:
        virtual void _OnUpdate() {}
        virtual bool _Redraw() { return false; }
        virtual void _OnDraw(Graphics* g) {}
        virtual void _OnResize(Size size) {}
        virtual void _OnWindowPosChange(Point position) {}

    public:
        virtual const char* GetName() const { return "base"; }

    public:
        virtual std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;
            values.push_back(Size::WidthValueProxy("width", std::make_any<Value<Size>*>(&size)));
            values.push_back(Size::HeightValueProxy("height", std::make_any<Value<Size>*>(&size)));
            values.push_back(SizeF::WidthValueProxy("parent width", std::make_any<Value<SizeF>*>(&parentSize), 3));
            values.push_back(SizeF::HeightValueProxy("parent height", std::make_any<Value<SizeF>*>(&parentSize), 3));
            values.push_back(Point::XValueProxy("x", std::make_any<Value<Point>*>(&position)));
            values.push_back(Point::YValueProxy("y", std::make_any<Value<Point>*>(&position)));
            values.push_back(PointF::XValueProxy("parent x", std::make_any<Value<PointF>*>(&parentPosition), 3));
            values.push_back(PointF::YValueProxy("parent y", std::make_any<Value<PointF>*>(&parentPosition), 3));
            values.push_back(Size::WidthValueProxy("calculated width", std::make_any<Value<Size>*>(&size_)).Computed());
            values.push_back(Size::HeightValueProxy("calculated height", std::make_any<Value<Size>*>(&size_)).Computed());
            values.push_back(Size::WidthValueProxy("calculated self width", std::make_any<Value<Size>*>(&selfSize_)).Computed());
            values.push_back(Size::HeightValueProxy("calculated self height", std::make_any<Value<Size>*>(&selfSize_)).Computed());
            values.push_back(Point::XValueProxy("calculated x", std::make_any<Value<Point>*>(&position_)).Computed());
            values.push_back(Point::YValueProxy("calculated y", std::make_any<Value<Point>*>(&position_)).Computed());
            values.push_back(Point::XValueProxy("calculated window x", std::make_any<Value<Point>*>(&windowPosition_)).Computed());
            values.push_back(Point::YValueProxy("calculated window y", std::make_any<Value<Point>*>(&windowPosition_)).Computed());
            values.push_back(ValueProxy::BasicEnumValueProxy<Alignment>("x align", std::make_any<Value<Alignment>*>(&xAlign), AlignmentValueProxySelectionValues()));
            values.push_back(ValueProxy::BasicEnumValueProxy<Alignment>("y align", std::make_any<Value<Alignment>*>(&yAlign), AlignmentValueProxySelectionValues()));
            values.push_back(ValueProxy::BasicFloatValueProxy<float>("opacity", std::make_any<Value<float>*>(&opacity), 3, ValueProxy::Number(0), ValueProxy::Number(1), ValueProxy::Number("0.1")));
            values.push_back(ValueProxy::BasicBoolValueProxy("disabled", std::make_any<Value<bool>*>(&disabled)));
            values.push_back(ValueProxy::BasicBoolValueProxy("visible", std::make_any<Value<bool>*>(&visible)));
            values.push_back(ValueProxy::BasicBoolValueProxy("interactable", std::make_any<Value<bool>*>(&interactable)));
            values.push_back(ValueProxy::BasicIntValueProxy<int>("z-index", std::make_any<Value<int>*>(&zIndex)));
            values.push_back(ValueProxy::BasicBoolValueProxy("eat scroll events", std::make_any<Value<bool>*>(&eatScrollEvents)));
            values.push_back(ValueProxy::BasicBoolValueProxy("ignore alpha", std::make_any<Value<bool>*>(&ignoreAlpha)));
            values.push_back(ValueProxy::BasicBoolValueProxy("selectable", std::make_any<Value<bool>*>(&selectable)));
            values.push_back(ValueProxy::BasicIntValueProxy<int>("tab index", std::make_any<Value<int>*>(&tabIndex)));
            values.push_back(ValueProxy::BasicBoolValueProxy("border visible", std::make_any<Value<bool>*>(&border.visible)));
            values.push_back(ValueProxy::BasicFloatValueProxy<float>("border width", std::make_any<Value<float>*>(&border.width), 3));
            values.push_back(ValueProxy::BasicFloatValueProxy<float>("corner radius", std::make_any<Value<float>*>(&border.cornerRadius), 3));
            values.push_back(ValueProxy::BasicColorValueProxy("border color", std::make_any<Value<Color>*>(&border.color)));
            values.push_back(ValueProxy::BasicColorValueProxy("selected border color", std::make_any<Value<Color>*>(&border.selectedColor)));
            values.push_back(ValueProxy::BasicColorValueProxy("background color", std::make_any<Value<Color>*>(&backgroundColor)));
            values.push_back(ValueProxy::BasicEnumValueProxy<zwnd::CursorIcon>("cursor icon", std::make_any<Value<zwnd::CursorIcon>*>(&cursorIcon), zwnd::CursorIconValueProxySelectionValues(), true));
            values.push_back(ValueProxy::BasicTextValueProxy("hover text", std::make_any<Value<std::wstring>*>(&hoverText)));
            values.push_back(DurationValueProxy("hover text delay (ms)", std::make_any<Value<Duration>*>(&hoverTextDelay), MILLISECONDS));
            values.push_back(ValueProxy::BasicBoolValueProxy("selected", std::make_any<Value<bool>*>(&selected_)).Computed());
            values.push_back(ValueProxy::BasicBoolValueProxy("hovered", std::make_any<Value<bool>*>(&hovered_)).Computed());
            values.push_back(ValueProxy::BasicBoolValueProxy("area hovered", std::make_any<Value<bool>*>(&hoveredArea_)).Computed());
            values.push_back(ValueProxy::BasicBoolValueProxy("left clicked", std::make_any<Value<bool>*>(&leftClicked_)).Computed());
            values.push_back(ValueProxy::BasicBoolValueProxy("right clicked", std::make_any<Value<bool>*>(&rightClicked_)).Computed());
            values.push_back(Point::XValueProxy("mouse x position", std::make_any<Value<Point>*>(&mousePosition_)).Computed());
            values.push_back(Point::YValueProxy("mouse y position", std::make_any<Value<Point>*>(&mousePosition_)).Computed());

            std::vector<std::pair<std::string, std::vector<ValueProxy>>> data;
            data.push_back({ "Base", std::move(values) });
            return data;
        }
    };

}