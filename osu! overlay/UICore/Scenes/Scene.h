#pragma once

#include "Components/Base/Canvas.h"
#include "Helper/Time.h"

#include <functional>

#define DEFINE_SCENE(scene_name, parent) \
public: \
scene_name() : parent() {} \
const char* GetName() const override { return StaticName(); } \
static const char* StaticName() { return #scene_name; } \
private:

class App;
namespace zwnd
{
    class Window;
}

namespace zcom
{
    // Options scene initialization
    struct SceneOptionsBase
    {

    };

    class Scene
    {
        friend class zwnd::Window;
    protected:
        App* _app = nullptr;
        zwnd::Window* _window = nullptr;
        Panel* _basePanel = nullptr;

        Scene() {};
    public:
        virtual ~Scene() {};
    protected:
        //void SetReferences(App* app, zwnd::Window* window, Panel* basePanel);
        void SetApp(App* app) { _app = app; };
        void SetWindow(zwnd::Window* window) { _window = window; };
        void SetBasePanel(Panel* basePanel) { _basePanel = basePanel; };
        virtual void Init(SceneOptionsBase* options) {}
        virtual void Uninit() {}

        // Component creation
        template<class T, typename... Args>
        std::unique_ptr<T> Create(Args&&... args)
        {
            auto uptr = std::unique_ptr<T>(new T(this));
            uptr->Init(std::forward<Args>(args)...);
            return uptr;
        }

        std::unique_ptr<Panel> CreatePanelForScene(Scene* scene)
        {
            return std::unique_ptr<Panel>(new Panel(scene));
        }

    public:
        App* GetApp() const { return _app; }
        zwnd::Window* GetWindow() const { return _window; }
        Panel* GetBasePanel() const { return _basePanel; }

    public:
        virtual const char* GetName() const = 0;
    };
}