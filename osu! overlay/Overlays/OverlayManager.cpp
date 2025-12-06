#include "UICore/App.h"
#include "OverlayManager.h"
#include "OverlayConfig.h"
#include "OverlayScene.h"

#include "UICore/Scenes/DefaultNonClientAreaScene.h"

void OverlayManager::Init(App* app)
{
    _app = app;
}

void OverlayManager::EnableOverlay(std::shared_ptr<const Overlay> overlay)
{
    std::lock_guard<std::mutex> lock(_mtx);
    
    if (_OverlayEnabled(overlay->Id()))
        return;

    if (!_overlayWindowId)
        _CreateOverlayWindow();

    // Window creation is synchronous so by the time the window is created the overlay scene has subscribed to overlay enable events
    _overlayEventEmitter->InvokeAll({ OverlayEvent::OVERLAY_ENABLED, overlay });
    _activeOverlays.push_back(overlay);
}

void OverlayManager::DisableOverlay(std::uint64_t overlayId)
{
    std::lock_guard<std::mutex> lock(_mtx);

    auto it = std::find_if(_activeOverlays.cbegin(), _activeOverlays.cend(), [&](auto item) { return item->Id() == overlayId; });
    if (it == _activeOverlays.cend())
        return;

    _overlayEventEmitter->InvokeAll({ OverlayEvent::OVERLAY_DISABLED, *it });
    _activeOverlays.erase(it);

    if (_activeOverlays.empty() && _overlayWindowId)
    {
        Handle<zwnd::Window> windowHandle = _app->GetWindow(_overlayWindowId.value());
        if (windowHandle.Valid())
            windowHandle->Close();
        _overlayWindowId = std::nullopt;
    }
}

bool OverlayManager::OverlayEnabled(uint64_t overlayId)
{
    std::lock_guard<std::mutex> lock(_mtx);
    return _OverlayEnabled(overlayId);
}

std::unique_ptr<AsyncEventSubscription<void, OverlayEvent>> OverlayManager::SubscribeToOverlayEvents(std::function<void(OverlayEvent)> handler)
{
    return _overlayEventEmitter->SubscribeAsync(handler);
}

bool OverlayManager::_OverlayEnabled(uint64_t overlayId)
{
    return std::find_if(_activeOverlays.cbegin(), _activeOverlays.cend(), [&](auto item) { return item->Id() == overlayId; }) != _activeOverlays.cend();
}

void OverlayManager::_CreateOverlayWindow()
{
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    zwnd::WindowProperties props = zwnd::WindowProperties()
        .WindowClassName(OverlayConfig::OVERLAY_WINDOW_NAME)
        .InitialSize(width, height)
        .MinSize(1, 1)
        .IgnoreTaskbarForPlacement()
        .TopMost()
        .DisableWindowAnimations()
        .DisableWindowActivation()
        .DisableMouseInteraction()
        .DisableFastTooltips()
        .DisableVSync();

    _overlayWindowId = _app->CreateTopWindow(
        props,
        [=](zwnd::Window* wnd) {
            wnd->resourceManager.SetImageResourceFilePath("Resources/Images/resources.resc");
            wnd->resourceManager.InitAllImages();

            // Remove window decorations
            zcom::DefaultNonClientAreaSceneOptions ncOpt;
            ncOpt.drawWindowShadow = false;
            ncOpt.drawWindowBorder = false;
            ncOpt.resizingBorderWidths = { 0, 0, 0, 0 };
            ncOpt.clientAreaMargins = { 0, 0, 0, 0 };
            wnd->LoadNonClientAreaScene<zcom::DefaultNonClientAreaScene>(&ncOpt);

            zcom::OverlaySceneOptions opt;
            opt.overlayEventEmitter = _overlayEventEmitter;
            wnd->LoadStartingScene<zcom::OverlayScene>(&opt);
        }
    );
}
