#pragma once

#include "Components/Base/Canvas.h"
#include "Helper/Time.h"

#include <functional>

enum class NotificationPosition
{
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT
};

#define NOTIF_PANEL_Z_INDEX 255

class App;
namespace zwnd
{
    class Window;
}

namespace zcom
{
    struct ResizeInfo
    {
        bool windowMaximized = false;
        bool windowMinimized = false;
        bool windowRestored = false;
        bool windowFullscreened = false;
    };

    // Options scene initialization
    struct SceneOptionsBase
    {

    };

    class Scene
    {
        friend class App;
        friend class zwnd::Window;
    protected:
        App* _app;
        zwnd::Window* _window;
        Canvas* _canvas;
        bool _initialized = false;
        bool _focused = false;

        Scene(App* app, zwnd::Window* window);
    public:
        virtual ~Scene();
    protected:
        void Init(SceneOptionsBase* options);
        void Uninit();
        void Focus();
        void Unfocus();
    public:
        bool Focused() const;

    protected:
        void Update();
        bool Redraw();
        ID2D1Bitmap* Draw(Graphics g);
        ID2D1Bitmap* ContentImage();
        void Resize(int width, int height, ResizeInfo info = {});

        // Component creation
        template<class T, typename... Args>
        std::unique_ptr<T> Create(Args&&... args)
        {
            auto uptr = std::unique_ptr<T>(new T(this));
            uptr->Init(std::forward<Args>(args)...);
            return uptr;
        }

    public:
        std::unique_ptr<Panel> CreatePanel()
        {
            auto uptr = std::unique_ptr<Panel>(new Panel(this));
            uptr->Init();
            return uptr;
        }

    public:
        App* GetApp() const { return _app; }
        zwnd::Window* GetWindow() const { return _window; }

        Canvas* GetCanvas() const;

    private:
        virtual void _Init(SceneOptionsBase* options) = 0;
        virtual void _Uninit() = 0;
        virtual void _Focus() = 0;
        virtual void _Unfocus() = 0;

        virtual void _Update() = 0;
        virtual bool _Redraw() { return _canvas->Redraw(); }
        virtual ID2D1Bitmap* _Draw(Graphics g) { return _canvas->Draw(g); }
        virtual ID2D1Bitmap* _Image() { return _canvas->ContentImage(); }
        virtual void _Resize(int width, int height, ResizeInfo info) = 0;

    public:
        virtual const char* GetName() const = 0;
    };
}