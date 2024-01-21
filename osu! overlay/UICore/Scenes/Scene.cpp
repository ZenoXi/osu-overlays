// App.h and Window.h must be included first
#include "App.h"
#include "Window/Window.h"
#include "Scene.h"

zcom::Scene::Scene(App* app, zwnd::Window* window)
    : _app(app), _window(window)
{

}

zcom::Scene::~Scene()
{
}

void zcom::Scene::Init(SceneOptionsBase* options)
{
    _canvas = new zcom::Canvas(Create<Panel>(), 1, 1);
    _Init(options);
    _initialized = true;
}

void zcom::Scene::Uninit()
{
    Unfocus();
    _Uninit();
    delete _canvas;
    _canvas = nullptr;
    _initialized = false;
}

void zcom::Scene::Focus()
{
    _focused = true;
    _window->keyboardManager.AddHandler(_canvas);
    _Focus();
}

void zcom::Scene::Unfocus()
{
    if (!_focused)
        return;

    _focused = false;
    _canvas->ClearSelection();
    _canvas->OnLeftReleased(std::numeric_limits<int>::min(), std::numeric_limits<int>::min());
    _canvas->OnRightReleased(std::numeric_limits<int>::min(), std::numeric_limits<int>::min());
    _canvas->OnMouseLeave();
    _window->keyboardManager.RemoveHandler(_canvas);
    _Unfocus();
}

bool zcom::Scene::Focused() const
{
    return _focused;
}

void zcom::Scene::Update()
{
    _Update();
}

bool zcom::Scene::Redraw()
{
    return _Redraw();
}

ID2D1Bitmap* zcom::Scene::Draw(Graphics g)
{
    return _Draw(g);
}

ID2D1Bitmap* zcom::Scene::ContentImage()
{
    return _Image();
}

void zcom::Scene::Resize(int width, int height, ResizeInfo info)
{
    _canvas->Resize(width, height);
    _Resize(width, height, info);
}

zcom::Canvas* zcom::Scene::GetCanvas() const
{
    return _canvas;
}