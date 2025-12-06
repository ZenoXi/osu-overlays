#pragma once

#include "Scene.h"

#include "../Components/Base/ComponentBase.h"
#include "../Components/Base/FlexPanel.h"
#include "../Components/Base/Button.h"
#include "../Components/Base/Label.h"
#include "../Components/Base/Image.h"
#include "../Window/WindowMessage.h"
#include "../Helper/EventEmitter.h"

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
        bool darkMode = false;
        // When true, forces the title label to be opaque
        bool useCleartype = true;
    };

    class DefaultTitleBarScene : public Scene
    {
        DEFINE_SCENE(DefaultTitleBarScene, Scene)
    protected:
        void Init(SceneOptionsBase* options) override;
        void Uninit() override;
    public:
        void SetBackground(Color color);
        void AddCloseButton();
        void AddMaximizeButton();
        void AddMinimizeButton();
        void AddIcon(std::optional<Bitmap> icon);
        void AddTitle(std::wstring title);
        void AddMenuButton(std::wstring name);

        Button* GetCloseButton() { return _closeButton.get(); }
        Button* GetMaximizeButton() { return _maximizeButton.get(); }
        Button* GetMinimizeButton() { return _minimizeButton.get(); }
        Image* GetIconImage() { return _iconImage.get(); }
        Label* GetTitleLabel() { return _titleLabel.get(); }

        void SubscribeToWindowMessages();
        void HandleWindowMessages();

        // The physical height of the title bar scene
        virtual int TitleBarSceneHeight();
        // The height of the area which acts as a window caption for moving the window. Can be larger than the title bar
        virtual int CaptionHeight();
        // Returns the RECT (in title bar scene coordinates) which can be considered as the window menu button
        virtual RECT WindowMenuButtonRect();
        // Returns a list of rects (in title bar scene coordinates) which should not be considered as caption area
        virtual std::vector<RECT> ExcludedCaptionRects();

    protected:
        std::unique_ptr<FlexPanel> _contentPanel = nullptr;
        std::unique_ptr<Button> _closeButton = nullptr;
        std::unique_ptr<Button> _maximizeButton = nullptr;
        std::unique_ptr<Button> _minimizeButton = nullptr;
        std::unique_ptr<Image> _iconImage = nullptr;
        std::unique_ptr<Label> _titleLabel = nullptr;
        std::vector<std::unique_ptr<Button>> _menuButtons;

        Value<bool> _windowIsActive = true;

        int _titleBarHeight = 0;
        int _captionHeight = 0;
        bool _tintIcon = true;
        bool _darkMode = false;
        bool _useCleartype = true;

        Color _activeItemTint = Color(0);
        Color _inactiveItemTint = Color(0x808080);
        std::unique_ptr<AsyncEventSubscription<bool, zwnd::WindowMessage>> _windowMessageSubscription;
    };
}
