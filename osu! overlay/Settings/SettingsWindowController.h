#pragma once

#include "SettingsTab.h"
#include "SettingsConfig.h"
#include "UICore/Helper/EventEmitter.h"
#include "UICore/WindowEvent.h"

#include <optional>
#include <any>
#include <mutex>
#include <thread>
#include <atomic>

class App;

class SettingsWindowController
{
    App* _app;
    std::unique_ptr<AsyncEventSubscription<void, WindowEvent>> _windowEventSubscription;

    bool _open = false;
    std::optional<zwnd::WindowId> _id;
    std::mutex _mtx;
    std::atomic<std::thread::id> _currentThreadId;

    EventEmitter<void, SettingsTab, std::optional<std::any>> _openSettingsEventEmitter = EventEmitter<void, SettingsTab, std::optional<std::any>>(EventEmitterThreadMode::MULTITHREADED);

public:
    void Init(App* app);
    void OpenSettings(SettingsTab tab, std::optional<std::any> extraOptions = std::nullopt);
    [[nodiscard]] std::unique_ptr<AsyncEventSubscription<void, SettingsTab, std::optional<std::any>>> SubscribeToOpenSettingsRequests();
};