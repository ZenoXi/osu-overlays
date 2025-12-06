#pragma once

#include "Overlay.h"
#include "OverlayEvent.h"
#include "UICore/Window/WindowId.h"

#include <mutex>

class App;

class OverlayManager
{
public:
    void Init(App* app);
    void EnableOverlay(std::shared_ptr<const Overlay> overlay);
    void DisableOverlay(uint64_t overlayId);
    bool OverlayEnabled(uint64_t overlayId);

    [[nodiscard]] std::unique_ptr<AsyncEventSubscription<void, OverlayEvent>> SubscribeToOverlayEvents(std::function<void(OverlayEvent)> handler = nullptr);

private:

    App* _app;

    std::optional<zwnd::WindowId> _overlayWindowId;
    std::mutex _mtx;

    std::vector<std::shared_ptr<const Overlay>> _activeOverlays;

    EventEmitter<void, OverlayEvent> _overlayEventEmitter = EventEmitter<void, OverlayEvent>(EventEmitterThreadMode::MULTITHREADED);

    bool _OverlayEnabled(uint64_t overlayId);
    void _CreateOverlayWindow();
};