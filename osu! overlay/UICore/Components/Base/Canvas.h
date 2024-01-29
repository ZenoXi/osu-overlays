#pragma once

#include "Panel.h"

#include "Window/MouseEventHandler.h"
#include "Window/KeyboardEventHandler.h"
#include "Helper/EventEmitter.h"

#include <vector>

namespace zcom
{
    // Base storage for all components
    class Canvas : public KeyboardEventHandler
    {
        EventEmitter<bool, const EventTargets*> _mouseMoveHandlers;
        EventEmitter<bool, const EventTargets*> _leftPressedHandlers;
        EventEmitter<bool, const EventTargets*> _leftReleasedHandlers;
        EventEmitter<bool, const EventTargets*> _rightPressedHandlers;
        EventEmitter<bool, const EventTargets*> _rightReleasedHandlers;
        EventEmitter<bool, const EventTargets*> _wheelUpHandlers;
        EventEmitter<bool, const EventTargets*> _wheelDownHandlers;

        bool _mouseInside = false;
        bool _mouseLeftClicked = false;
        bool _mouseRightClicked = false;
        int _mousePosX = 0;
        int _mousePosY = 0;

        std::unique_ptr<Panel> _panel;

    public:
        Canvas(std::unique_ptr<Panel> panel, int width, int height) : _panel(std::move(panel))
        {
            _panel->SetSize(width, height);
            _panel->SetBackgroundColor(D2D1::ColorF(0, 0));
            _panel->SetSelectedBorderColor(D2D1::ColorF(0, 0.0f));
        }
        ~Canvas() {}
        Canvas(Canvas&&) = delete;
        Canvas& operator=(Canvas&&) = delete;
        Canvas(const Canvas&) = delete;
        Canvas& operator=(const Canvas&) = delete;

        void AddComponent(Component* comp)
        {
            _panel->AddItem(comp);
        }

        void RemoveComponent(Component* comp)
        {
            _panel->RemoveItem(comp);
        }

        void RemoveComponent(int index)
        {
            _panel->RemoveItem(index);
        }

        int ComponentCount() const
        {
            return _panel->ItemCount();
        }

        Component* GetComponent(int index)
        {
            return _panel->GetItem(index);
        }

        void ClearComponents()
        {
            _panel->ClearItems();
        }

        Panel* BasePanel()
        {
            return _panel.get();
        }

        // true by default
        void SetOcclusive(bool occlusive)
        {
            occlusive ? _panel->DisableMouseEventFallthrough() : _panel->EnableMouseEventFallthrough();
        }

        void Update()
        {
            _panel->Update();
        }

        bool Redraw()
        {
            return _panel->Redraw();
        }

        void InvokeRedraw()
        {
            _panel->InvokeRedraw();
        }

        ID2D1Bitmap* Draw(Graphics g)
        {
            return _panel->Draw(g);
        }

        ID2D1Bitmap* ContentImage()
        {
            return _panel->ContentImage();
        }

        void Resize(int width, int height)
        {
            if (width < 1) width = 1;
            if (height < 1) height = 1;
            _panel->Resize(width, height);
        }

        int GetWidth() const
        {
            return _panel->GetWidth();
        }

        int GetHeight() const
        {
            return _panel->GetHeight();
        }

        void SetBackgroundColor(D2D1_COLOR_F color)
        {
            _panel->SetBackgroundColor(color);
        }

        D2D1_COLOR_F GetBackgroundColor() const
        {
            return _panel->GetBackgroundColor();
        }

        void ClearSelection(Component* exception = nullptr)
        {
            auto allComponents = GetAllItems();
            for (auto& component : allComponents)
            {
                if (component != exception)
                {
                    if (component->Selected())
                    {
                        component->OnDeselected();
                    }
                }
            }
        }

        Component* GetSelectedComponent()
        {
            auto allComponents = GetAllItems();
            for (auto& component : allComponents)
            {
                if (component->Selected())
                {
                    return component;
                }
            }
            return nullptr;
            //return _selected;
        }

        // Recursively gets all components in the canvas
        // *Might* (unlikely) cause performance issues if used often
        std::list<Component*> GetAllItems()
        {
            return _panel->GetAllChildren();
        }

        //
        // MOUSE EVENT FUNCTIONS
        //

        bool MouseInside() { return _mouseInside; }
        bool MouseLeftClicked() { return _mouseLeftClicked; }
        bool MouseRightClicked() { return _mouseRightClicked; }
        int MousePosX() { return _mousePosX; }
        int MousePosY() { return _mousePosY; }

        Component* OnMouseMove(int x, int y)
        {
            _mousePosX = x;
            _mousePosY = y;

            if (!_panel->GetMouseInside())
                _panel->OnMouseEnter();
            if (!_panel->GetMouseInsideArea())
                _panel->OnMouseEnterArea();

            auto targets = _panel->OnMouseMove(x, y);
            targets.Remove(_panel.get());
            _mouseMoveHandlers->InvokeAll(&targets);
            return targets.MainTarget();
        }

        void OnMouseLeave()
        {
            _mouseInside = false;

            _panel->OnMouseLeave();
            _panel->OnMouseLeaveArea();
        }

        void OnMouseEnter()
        {
            _mouseInside = true;

            _panel->OnMouseEnter();
        }

        Component* OnLeftPressed(int x, int y)
        {
            _mouseLeftClicked = true;

            auto targets = _panel->OnLeftPressed(x, y);
            targets.Remove(_panel.get());
            _leftPressedHandlers->InvokeAll(&targets);
            return targets.MainTarget();

            //// Clear selection
            //ClearSelection(mainTarget);

            //if (mainTarget != nullptr)
            //{
            //    // Set selection
            //    if (!mainTarget->Selected() && mainTarget->GetSelectable())
            //    {
            //        mainTarget->OnSelected();
            //    }
            //}

            //bool handled = false;
            //_leftPressedHandlers.Lock();
            //for (auto& handler : _leftPressedHandlers)
            //{
            //    if (handler && handler(&targets))
            //        handled = true;
            //}
            //_leftPressedHandlers.Unlock();
            //return handled;
        }

        Component* OnLeftReleased(int x, int y)
        {
            _mouseLeftClicked = false;

            auto targets = _panel->OnLeftReleased(x, y);
            targets.Remove(_panel.get());
            _leftReleasedHandlers->InvokeAll(&targets);
            return targets.MainTarget();
        }

        Component* OnRightPressed(int x, int y)
        {
            _mouseRightClicked = true;

            auto targets = _panel->OnRightPressed(x, y);
            targets.Remove(_panel.get());
            _rightPressedHandlers->InvokeAll(&targets);
            return targets.MainTarget();
        }

        Component* OnRightReleased(int x, int y)
        {
            _mouseRightClicked = false;

            auto targets = _panel->OnRightReleased(x, y);
            targets.Remove(_panel.get());
            _rightReleasedHandlers->InvokeAll(&targets);
            return targets.MainTarget();
        }

        Component* OnWheelUp(int x, int y)
        {
            auto targets = _panel->OnWheelUp(x, y);
            targets.Remove(_panel.get());
            _wheelUpHandlers->InvokeAll(&targets);
            return targets.MainTarget();
        }

        Component* OnWheelDown(int x, int y)
        {
            auto targets = _panel->OnWheelDown(x, y);
            targets.Remove(_panel.get());
            _wheelDownHandlers->InvokeAll(&targets);
            return targets.MainTarget();
        }

        //
        // KEYBOARD EVENT MANAGER FUNCTIONS
        //

        bool _OnHotkey(int id) override
        {
            return false;
        }

        bool _OnKeyDown(BYTE vkCode) override
        {
            if (vkCode == VK_TAB)
            {
                // Advance selected element
                Component* item = _panel->IterateTab(KeyState(VK_SHIFT));
                ClearSelection();
                if (item != nullptr)
                {
                    item->OnSelected(KeyState(VK_SHIFT));
                }
            }
            return false;
        }

        bool _OnKeyUp(BYTE vkCode) override
        {
            return false;
        }

        bool _OnChar(wchar_t ch) override
        {
            return false;
        }


        EventSubscription<bool, const EventTargets*> SubscribeOnMouseMove(std::function<bool(const EventTargets*)> func)
        {
            return _mouseMoveHandlers->Subscribe(func);
        }

        EventSubscription<bool, const EventTargets*> SubscribeOnLeftPressed(std::function<bool(const EventTargets*)> func)
        {
            return _leftPressedHandlers->Subscribe(func);
        }

        EventSubscription<bool, const EventTargets*> SubscribeOnLeftReleased(std::function<bool(const EventTargets*)> func)
        {
            return _leftReleasedHandlers->Subscribe(func);
        }

        EventSubscription<bool, const EventTargets*> SubscribeOnRightPressed(std::function<bool(const EventTargets*)> func)
        {
            return _rightPressedHandlers->Subscribe(func);
        }

        EventSubscription<bool, const EventTargets*> SubscribeOnRightReleased(std::function<bool(const EventTargets*)> func)
        {
            return _rightReleasedHandlers->Subscribe(func);
        }

        EventSubscription<bool, const EventTargets*> SubscribeOnWheelUp(std::function<bool(const EventTargets*)> func)
        {
            return _wheelUpHandlers->Subscribe(func);
        }

        EventSubscription<bool, const EventTargets*> SubscribeOnWheelDown(std::function<bool(const EventTargets*)> func)
        {
            return _wheelDownHandlers->Subscribe(func);
        }
    };
}