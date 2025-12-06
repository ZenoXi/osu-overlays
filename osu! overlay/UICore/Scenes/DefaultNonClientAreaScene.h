#pragma once

#include "Scene.h"

#include "Helper/EventEmitter.h"
#include "Window/WindowMessage.h"
#include "Window/ResizeFlags.h"

#include <optional>

namespace zcom
{
    class DefaultTitleBarScene;

    struct DefaultNonClientAreaSceneOptions : public SceneOptionsBase
    {
        Rect resizingBorderWidths = { 7, 7, 7, 7 };
        Rect clientAreaMargins = { 7, 7, 7, 7 };
        bool drawWindowShadow = true;
        bool drawWindowBorder = true;
    };

    class DefaultNonClientAreaScene : public Scene
    {
        DEFINE_SCENE(DefaultNonClientAreaScene, Scene)
    protected:
        void Init(SceneOptionsBase* options) override;
    public:
        virtual Panel* ProcessCreatedTitleBarScene(DefaultTitleBarScene* titleBarScene);
        virtual void ProcessDeletedTitleBarScene(DefaultTitleBarScene* titleBarScene);
        virtual Panel* ProcessCreatedScene(Scene* scene);
        virtual Panel* ProcessRecreatedScene(Scene* scene);
        virtual void ProcessDeletedScene(Scene* scene);
        void ProcessWindowResize(int newWidth, int newHeight, zwnd::ResizeFlags flags);

        // Returns the width of the resizing area on each side of the window
        virtual Rect GetResizingBorderWidths();
        // Returns the margins on each side of the client area (title bar and content) to the window edges
        virtual Rect GetClientAreaMargins();

    protected:
        std::unique_ptr<Panel> _nonClientAreaPanel = nullptr;
        std::unique_ptr<Panel> _clientAreaPanel = nullptr;
        std::unique_ptr<Panel> _titleBarPanel = nullptr;
        std::unique_ptr<Panel> _contentPanel = nullptr;

        DefaultTitleBarScene* _titleBarScene = nullptr;

        Rect _resizingBorderWidths{};
        // TODO: These don't work when window is maximized
        Rect _clientAreaMargins{};
        bool _drawWindowShadow = true;
        bool _drawWindowBorder = true;

        //D2D1_COLOR_F _borderColor = D2D1::ColorF(0.3f, 0.3f, 0.3f, 0.5f);
        //D2D1_VECTOR_4F _shadowColor = D2D1::Vector4F(0.0f, 0.0f, 0.0f, 0.4f);
        Color _borderColor = Color(0x4D4D4D, 0.6f);
        Color _shadowColor = Color(0, 0.6f);
        std::unique_ptr<AsyncEventSubscription<bool, zwnd::WindowMessage>> _windowActivationSubscription;

    private:
        void _Update();
        void _Draw(Graphics* g);
        void _UpdateClientAreaShadow();
    };
}