#include "UICore/App.h"
#include "SettingsWindowController.h"

#include "UICore/Scenes/DefaultNonClientAreaScene.h"
#include "UICore/Scenes/DefaultTitleBarScene.h"
#include "SettingsScene.h"

void SettingsWindowController::Init(App* app)
{
    _app = app;

    Handle<zwnd::Window> handle = _app->FindWindowByClassName(SettingsConfig::WINDOW_NAME);
    if (handle.Valid())
    {
        _open = true;
        _id = handle->GetWindowId();
    }

    _windowEventSubscription = _app->SubscribeOnWindowEvent([=](WindowEvent event) {
        std::unique_lock<std::mutex> lock(_mtx, std::defer_lock);
        if (std::this_thread::get_id() != _currentThreadId.load())
            lock.lock();

        if (event.eventType == WindowEvent::CREATED && event.windowProperties->windowClassName == SettingsConfig::WINDOW_NAME)
        {
            _open = true;
            _id = event.windowId;
        }
        else if (event.eventType == WindowEvent::CLOSED && _id.has_value() && event.windowId == _id.value())
        {
            _open = false;
            _id = std::nullopt;
        }
    });
}

void SettingsWindowController::OpenSettings(SettingsTab tab, std::optional<std::any> extraOptions)
{
    std::lock_guard<std::mutex> lock(_mtx);
    _currentThreadId.store(std::this_thread::get_id());

    if (_open)
    {
        Handle<zwnd::Window> handle = _app->FindWindowByClassName(SettingsConfig::WINDOW_NAME);
        if (handle.Valid())
            handle->Backend().Focus();
        _openSettingsEventEmitter->InvokeAll(tab, extraOptions);
    }
    else
    {
        auto props = zwnd::WindowProperties()
            .WindowClassName(SettingsConfig::WINDOW_NAME)
            .InitialSize(800, 550)
            .MinSize(600, 500);

        _app->CreateTopWindow(
            props,
            [=](zwnd::Window* wnd) {
                wnd->resourceManager.SetImageResourceFilePath("Resources/Images/resources.resc");
                wnd->resourceManager.InitAllImages();

                wnd->LoadNonClientAreaScene<zcom::DefaultNonClientAreaScene>(nullptr);

                zcom::DefaultTitleBarSceneOptions tbOpt;
                tbOpt.showIcon = false;
                tbOpt.windowTitle = L"Settings";
                tbOpt.darkMode = true;
                wnd->LoadTitleBarScene<zcom::DefaultTitleBarScene>(&tbOpt);

                zcom::SettingsSceneOptions opt;
                opt.tab = tab;
                opt.extraOptions = extraOptions;
                wnd->LoadStartingScene<zcom::SettingsScene>(&opt);
            }
        );
    }

    _currentThreadId.store({});
}

std::unique_ptr<AsyncEventSubscription<void, SettingsTab, std::optional<std::any>>> SettingsWindowController::SubscribeToOpenSettingsRequests()
{
    return _openSettingsEventEmitter->SubscribeAsync();
}