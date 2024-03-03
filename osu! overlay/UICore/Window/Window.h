#pragma once

#include "WindowProperties.h"
#include "WindowBackend.h"
#include "WindowId.h"
#include "WindowType.h"
#include "Scenes/TooltipParams.h"

#include "Window/KeyboardManager.h"
#include "Helper/ResourceManager.h"
#include "Scenes/Scene.h"
#include "Scenes/DefaultTitleBarScene.h"
#include "Scenes/DefaultNonClientAreaScene.h"
#include "Components/Base/MenuTemplate.h"

#include <thread>
#include <atomic>
#include <future>

class App;

namespace zwnd
{
    class Window
    {
    public: // Initialization
        Window(
            App* app,
            WindowType type,
            std::optional<WindowId> parentWindowId,
            WindowProperties props,
            HINSTANCE hinst,
            std::function<void(Window* window)> initFunction,
            std::function<void(Window* window)> onClosed
        );
        // Constructor for message-only window
        Window(HINSTANCE hinst);
        ~Window();
        // Initialize the specified title bar scene. Should be called in the window init function *AT MOST ONCE*
        template<class _Scene>
        void LoadTitleBarScene(zcom::SceneOptionsBase* opt);
        // Initialize the specified non-client area scene. Should be called in the window init function *ONCE*
        template<class _Scene>
        void LoadNonClientAreaScene(zcom::SceneOptionsBase* opt);
        // Initialize the specified starting scene. Should be called in the window init function *AT LEAST ONCE*
        template<class _Scene>
        void LoadStartingScene(zcom::SceneOptionsBase* opt);

    public: // Scene control
        // Initializes the scene and places it behind all scenes
        template<class _Scene>
        void InitScene(zcom::SceneOptionsBase* opt);
        // Uninitializes and immediatelly initializes the scene with new options, keeping focus/z-order the same
        // If scene is not initialized, it just gets initialized as usual
        template<class _Scene>
        void ReinitScene(zcom::SceneOptionsBase* opt);
        // Primes the scene to be uninitialized
        template<class _Scene>
        void UninitScene();
        // Moves the specified scene in front of all other scenes
        // Returns:
        //  true - if the scene was moved or was aready in place
        //  false - if the scene is not active
        template<class _Scene>
        bool MoveSceneToFront();
        // Moves the specified scene behind all other scenes
        // Returns:
        //  true - if the scene was moved or was aready in place
        //  false - if the scene is not active
        template<class _Scene>
        bool MoveSceneToBack();
        // Moves the specified scene up by 1 spot
        // If the scene is in front already, nothing is done
        // Returns:
        //  true - if the scene was moved or was aready in place
        //  false - if the scene is not active
        template<class _Scene>
        bool MoveSceneUp();
        // Moves the specified scene down by 1 spot
        // If the scene is in the back already, nothing is done
        // Returns:
        //  true - if the scene was moved or was aready in place
        //  false - if the scene is not active
        template<class _Scene>
        bool MoveSceneDown();
        // Moves the specified scene behind another scene
        // If the scene is already behind, it isn't moved
        // Returns:
        //  true - if the scene was moved or was aready in place
        //  false - if any of the scenes aren't active
        template<class _Scene, class _Behind>
        bool MoveSceneBehind();
        // Moves the specified scene in front of another scene
        // If the scene is already in front, it isn't moved
        // Returns:
        //  true - if the scene was moved or was aready in place
        //  false - if any of the scenes aren't active
        template<class _Scene, class _InFront>
        bool MoveSceneInFront();
        // Return all active scenes
        std::vector<zcom::Scene*> Scenes();
        // Get the specified scene (returns nullptr if the scene isn't active)
        template<class _Scene>
        _Scene* GetScene();
        // Get the Z index of the specified scene (returns -1 if the scene isn't active)
        template<class _Scene>
        int GetSceneZIndex();
        // Get the title bar scene
        zcom::DefaultTitleBarScene* GetTitleBarScene();
        // Get the non-client area scene
        zcom::DefaultNonClientAreaScene* GetNonClientAreaScene();

    public: // Fullscreen
        // Returns the fullscren state
        bool Fullscreen() const { return _fullscreen; }
        // Sets the fullscreen state
        // Fullscreened windows only draw their content area
        void Fullscreen(bool fullscreen);

    public: // Window state and properties
        WindowId GetWindowId() { return _id; }
        WindowType GetWindowType() { return _type; }
        std::optional<WindowId> GetParent() { return _parentId; }

        // Returns a pointer to the window backend view object which contains additional window control functions
        WindowBackendView Backend() const { return WindowBackendView(_window.get()); }
        // Returns the properties specified on window creation
        WindowProperties Properties() const { return _props; }
        // Returns the full window width
        int Width() const { return _window->GetWidth(); }
        // Returns the full window height
        int Height() const { return _window->GetHeight(); }
        // Close the window
        void Close() { _closed.store(true); _window->KillMessageLoop(); }
        // Returns true when window is closing or closed
        bool Closing() const { return _closed.load(); }
        // Returns whether the window is closed
        bool Closed() const { return _closed.load() && !_window; }

        // When a blocking window is set, all mouse events will be ignored and cause the blocking window to flash
        void SetBlockingWindow(zwnd::WindowId windowId);
        void ResetBlockingWindow();
        void NotifyBlockingWindow();

    public: // Other
        // Opens a context menu constructed from the given template
        // 'sourceItemRect' describes the item RECT from which to calculate menu placement in window coordinates
        void OpenContextMenu(zcom::MenuTemplate::Menu menuTemplate, RECT sourceItemRect);
        // Shows a popup tooltip with the given text until the mouse is moved
        // 'params.xPos' and 'params.yPos' parameters describe the position from which to calculate tooltip placement in window coordinates
        void ShowTooltip(zcom::TooltipParams params);

    public: // Managers
        KeyboardManager keyboardManager;
        ResourceManager resourceManager;

    public: // Window messages
        std::unique_ptr<AsyncEventSubscription<bool, WindowMessage>> SubscribeToWindowMessages(const std::function<bool(WindowMessage)>& handler)
        {
            std::lock_guard<std::mutex> lock(_m_windowMessageEvent);
            return _windowMessageEvent->SubscribeAsync(handler);
        }

    private: // Scene control
        // Uninits a scene and removes it from the active scene list
        void _UninitScene(std::string name);
        // Finds an active scene with the specified name and returns a pointer to it (or null if not found)
        zcom::Scene* _GetScene(std::string name);
        // Finds an active scene with the specified name and returns its index (or null if not found)
        int _GetSceneIndex(std::string name);
    private:
        // All currently active scenes
        // The scenes are drawn on top of each other, starting with the index 0
        // This means that the bottom-most scene is at index 0 and top-most scene is at the end of the vector
        std::vector<std::unique_ptr<zcom::Scene>> _activeScenes;
        // Scenes which are set to be uninitialized at the end of the frame
        std::deque<std::string> _scenesToUninitialize;
        // A special scene containing the title bar. Its customization allows for custom title bars
        std::unique_ptr<zcom::DefaultTitleBarScene> _titleBarScene;
        // A special scene containing the non client area and its parameters
        // Used to render behind the title bar and client area
        std::unique_ptr<zcom::DefaultNonClientAreaScene> _nonClientAreaScene;
        // Set to true when any change to '_activeScenes' list occurs
        bool _sceneChanged = true;

    private: // Fullscreen
        // In fullscreen mode title bar and non client area is not drawn
        // and the window is placed into windowed borderless state
        bool _fullscreen = false;
        bool _fullscreenTargetValue = false;
        bool _fullscreenChanged = false;

    private: // Window message handling
        void _HandleMessage(WindowMessage msg);
        std::mutex _m_windowMessageEvent;
        EventEmitter<bool, WindowMessage> _windowMessageEvent;

    private: // Window
        WindowId _id;
        WindowType _type;
        std::optional<WindowId> _parentId;
        std::optional<WindowId> _blockingChild;

        HINSTANCE _hinst;
        WindowProperties _props;
        bool _isMessageOnly = false;
        std::unique_ptr<WindowBackend> _window = nullptr;

        // Becomes true when window is closed
        std::atomic<bool> _closed = false;
        std::function<void(Window* window)> _onClosed;

        std::atomic<bool> _windowCreated = false;
        std::atomic<bool> _scenesInited = false;

        // Pointer to the app object
        App* _app;

        std::optional<WindowSizeMessage> _windowSizeMessage;

    private: // Other

        // Tooltip
        EventEmitter<void, zcom::TooltipParams> _tooltipEventEmitter = EventEmitter<void, zcom::TooltipParams>(EventEmitterThreadMode::MULTITHREADED);

    private: // Threads
        void _MessageThread();
        void _MessageOnlyThread();
        void _UIThread();
        std::thread _messageThread;
        std::thread _uiThread;

    private:
        void _PassParamsToHitTest();
        // Returns a panel containing main panels from all scenes, in screen space
        std::unique_ptr<zcom::Panel> _BuildMasterPanel();
        bool _TitleBarAvailable();
    };

    // /////////////////// //
    // TEMPLATED FUNCTIONS //
    // /////////////////// //

    template<class _Scene>
    void Window::LoadTitleBarScene(zcom::SceneOptionsBase* opt)
    {
        _titleBarScene = std::make_unique<_Scene>(_app, this);
        ((zcom::Scene*)_titleBarScene.get())->Init(opt);
    }

    template<class _Scene>
    void Window::LoadNonClientAreaScene(zcom::SceneOptionsBase* opt)
    {
        _nonClientAreaScene = std::make_unique<_Scene>(_app, this);
        ((zcom::Scene*)_nonClientAreaScene.get())->Init(opt);
    }

    template<class _Scene>
    void Window::LoadStartingScene(zcom::SceneOptionsBase* opt)
    {
        InitScene<_Scene>(opt);
    }

    template<class _Scene>
    void Window::InitScene(zcom::SceneOptionsBase* opt)
    {
        auto scene = std::make_unique<_Scene>(_app, this);
        ((zcom::Scene*)scene.get())->Init(opt);

        _activeScenes.insert(_activeScenes.begin(), std::move(scene));
        _sceneChanged = true;
    }

    template<class _Scene>
    void Window::ReinitScene(zcom::SceneOptionsBase* opt)
    {
        _Scene* scene = GetScene<_Scene>();
        if (scene)
        {
            // Remove scene from uninit pending
            auto it = std::find(_scenesToUninitialize.begin(), _scenesToUninitialize.end(), _Scene::StaticName());
            if (it != _scenesToUninitialize.end())
                _scenesToUninitialize.erase(it);

            scene->Uninit();
            scene->Init(opt);
            _sceneChanged = true;
        }
        else
        {
            InitScene<_Scene>(opt);
        }
    }

    template<class _Scene>
    void Window::UninitScene()
    {
        _scenesToUninitialize.push_back(_Scene::StaticName());
    }

    template<class _Scene>
    bool Window::MoveSceneToFront()
    {
        int index = GetSceneZIndex<_Scene>();
        if (index == -1)
            return false;
        // Check if scene is already in place
        if (index == _activeScenes.size() - 1)
            return true;

        auto sceneSPtr = std::move(_activeScenes[index]);
        _activeScenes.erase(_activeScenes.begin() + index);
        _activeScenes.push_back(std::move(sceneSPtr));
        _sceneChanged = true;
        return true;
    }

    template<class _Scene>
    bool Window::MoveSceneToBack()
    {
        int index = GetSceneZIndex<_Scene>();
        if (index == -1)
            return false;
        // Check if scene is already in place
        if (index == 0)
            return true;

        auto sceneSPtr = std::move(_activeScenes[index]);
        _activeScenes.erase(_activeScenes.begin() + index);
        _activeScenes.insert(_activeScenes.begin(), std::move(sceneSPtr));
        _sceneChanged = true;
        return true;
    }

    template<class _Scene>
    bool Window::MoveSceneUp()
    {
        int index = GetSceneZIndex<_Scene>();
        if (index == -1)
            return false;
        // Check if scene is already at the top
        // This also acts as a check for wether more than one scene is active
        if (index == _activeScenes.size() - 1)
            return true;

        std::swap(_activeScenes[index], _activeScenes[index + 1]);
        _sceneChanged = true;
        return true;
    }

    template<class _Scene>
    bool Window::MoveSceneDown()
    {
        int index = GetSceneZIndex<_Scene>();
        if (index == -1)
            return false;
        // Check if scene is already at the bottom
        // This also acts as a check for wether more than one scene is active
        if (index == 0)
            return true;

        std::swap(_activeScenes[index], _activeScenes[index - 1]);
        _sceneChanged = true;
        return true;
    }

    template<class _Scene, class _Behind>
    bool Window::MoveSceneBehind()
    {
        int sceneIndex = GetSceneZIndex<_Scene>();
        if (sceneIndex == -1)
            return false;
        int behindSceneIndex = GetSceneZIndex<_Behind>();
        if (behindSceneIndex == -1)
            return false;
        // Check if scene is already behind
        if (sceneIndex < behindSceneIndex)
            return true;

        auto sceneSPtr = std::move(_activeScenes[sceneIndex]);
        _activeScenes.erase(_activeScenes.begin() + sceneIndex);
        _activeScenes.insert(_activeScenes.begin() + behindSceneIndex, std::move(sceneSPtr));
        _sceneChanged = true;
        return true;
    }

    template<class _Scene, class _InFront>
    bool Window::MoveSceneInFront()
    {
        int sceneIndex = GetSceneZIndex<_Scene>();
        if (sceneIndex == -1)
            return false;
        int inFrontSceneIndex = GetSceneZIndex<_InFront>();
        if (inFrontSceneIndex == -1)
            return false;
        // Check if scene is already in front
        if (sceneIndex > inFrontSceneIndex)
            return true;

        auto sceneSPtr = std::move(_activeScenes[sceneIndex]);
        _activeScenes.erase(_activeScenes.begin() + sceneIndex);
        _activeScenes.insert(_activeScenes.begin() + inFrontSceneIndex + 1, std::move(sceneSPtr));
        _sceneChanged = true;
        return true;
    }

    template<class _Scene>
    _Scene* Window::GetScene()
    {
        int index = GetSceneZIndex<_Scene>();
        if (index == -1)
            return nullptr;

        return dynamic_cast<_Scene*>(_activeScenes[index].get());
    }

    template<class _Scene>
    int Window::GetSceneZIndex()
    {
        return _GetSceneIndex(_Scene::StaticName());
    }
}