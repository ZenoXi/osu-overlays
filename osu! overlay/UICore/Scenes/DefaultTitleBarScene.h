#pragma once

#include "Scene.h"

#include "Components/Base/ComponentBase.h"
#include "Components/Base/Button.h"
#include "Components/Base/Label.h"
#include "Components/Base/Image.h"
#include "Helper/EventEmitter.h"

#include <optional>

namespace zcom
{
    struct DefaultTitleBarSceneOptions : public SceneOptionsBase
    {
        std::optional<std::string> windowIconResourceName = std::nullopt;
        std::wstring windowTitle = L"UI Framework";
        bool showCloseButton = true;
        bool showMaximizeButton = true;
        bool showMinimizeButton = true;
        bool showTitle = true;
        bool showIcon = true;
        int titleBarHeight = 30;
        int captionHeight = 30;
    };

    class DefaultTitleBarScene : public Scene
    {
    public:
        void SetBackground(D2D1_COLOR_F color);
        void AddCloseButton();
        void AddMaximizeButton();
        void AddMinimizeButton();
        void AddIcon(ID2D1Bitmap* icon);
        void AddTitle(std::wstring title);
        void AddMenuButton(std::wstring name);

        void SubscribeToWindowStateChanges();
        void HandleWindowStateChanges();

        // The physical height of the title bar scene
        virtual int TitleBarSceneHeight();
        // The height of the area which acts as a window caption for moving the window. Can be larger than the title bar
        virtual int CaptionHeight();
        // Returns the RECT (in title bar scene coordinates) which can be considered as the window menu button
        virtual RECT WindowMenuButtonRect();
        // Returns a list of rects (in title bar scene coordinates) which should not be considered as caption area
        virtual std::vector<RECT> ExcludedCaptionRects();

    public:
        DefaultTitleBarScene(App* app, zwnd::Window* window);

    protected:
        std::unique_ptr<Button> _closeButton = nullptr;
        std::unique_ptr<Button> _maximizeButton = nullptr;
        std::unique_ptr<Button> _minimizeButton = nullptr;
        std::unique_ptr<Image> _iconImage = nullptr;
        std::unique_ptr<Label> _titleLabel = nullptr;
        std::vector<std::unique_ptr<Button>> _menuButtons;

        int _titleBarHeight = 0;
        int _captionHeight = 0;
        bool _tintIcon = true;

        D2D1_COLOR_F _activeItemTint = D2D1::ColorF(0);
        D2D1_COLOR_F _inactiveItemTint = D2D1::ColorF(0.5f, 0.5f, 0.5f);
        std::unique_ptr<AsyncEventSubscription<bool, zwnd::WindowMessage>> _windowActivationSubscription;

    private:
        void _Init(SceneOptionsBase* options);
        void _Uninit();
        void _Focus();
        void _Unfocus();
        void _Update();
        void _Resize(int width, int height, ResizeInfo info);
    public:
        const char* GetName() const { return StaticName(); }
        static const char* StaticName() { return "title_bar"; }
    };
}