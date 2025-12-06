#pragma once

#include "Value.h"
#include "Handle.h"
#include "Window/Window.h"
#include "Components/Base/ComponentBase.h"
#include "App.h"

// The purpose of this class is to keep track whether a window with a specific class name is open or not.
// Value<T> objects for the window state are provided, which get updated on owner component post-update event
class WindowView
{
public:
    WindowView(const std::wstring& windowName, zcom::Component* owner) : _windowName(windowName)
    {
        _updateEventSubscription = owner->SubscribePostUpdate([=]() {

            // Create copy of window events since that should be very cheap compared to potentially updating lots of UI and hogging the mutex when values change
            std::vector<WindowEvent> windowUpdates;
            _windowEventSubscription->HandlePendingEvents([&](const WindowEvent& e) {
                windowUpdates.push_back(e);
            });

            for (auto& update : windowUpdates)
            {
                if (update.eventType == WindowEvent::CREATED && update.windowProperties->windowClassName == _windowName)
                {
                    open_ = true;
                    id_ = update.windowId;
                }
                else if (update.eventType == WindowEvent::CLOSED && id_->has_value() && update.windowId == id_->value())
                {
                    open_ = false;
                    id_ = std::nullopt;
                }
            }
        });

        Handle<zwnd::Window> windowHandle = owner->GetScene()->GetApp()->FindWindowByClassName(_windowName);
        if (windowHandle.Valid())
        {
            open_ = true;
            id_ = windowHandle->GetWindowId();
        }
        _windowEventSubscription = owner->GetScene()->GetApp()->SubscribeOnWindowEvent();
    }

    Value<bool> open_ = false;
    Value<std::optional<zwnd::WindowId>> id_;

private:
    const std::wstring _windowName;
    EventSubscription<void> _updateEventSubscription;
    std::unique_ptr<AsyncEventSubscription<void, WindowEvent>> _windowEventSubscription;
};