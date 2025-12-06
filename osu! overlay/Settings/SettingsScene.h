#pragma once

#include "Scenes/Scene.h"
#include "Components/Base/FlexPanel.h"
#include "Components/Base/Button.h"

#include "SettingsTab.h"

namespace zcom
{
    struct SettingsSceneOptions : public SceneOptionsBase
    {
        SettingsTab tab = SettingsTab::NONE;
        std::optional<std::any> extraOptions;
    };

    class SettingsScene : public Scene
    {
        DEFINE_SCENE(SettingsScene, Scene)
    protected:
        void Init(SceneOptionsBase* options) override;
        void Uninit() override;

    private:
        std::unique_ptr<FlexPanel> _mainPanel;
        std::unique_ptr<FlexPanel> _tabsPanel;
        std::unique_ptr<Component> _settingsPanel;

        Value<SettingsTab> _currentTab = SettingsTab::NONE;
        std::unique_ptr<AsyncEventSubscription<void, SettingsTab, std::optional<std::any>>> _openSettingsRequestSubscription;

        void _HandleOpenSettingsRequest(SettingsTab tab, std::optional<std::any> extraOptions);
        void _CreateTabs();
        std::unique_ptr<Button> _CreateTabButton(std::wstring text, std::optional<Bitmap> icon, SettingsTab tab);
        void _ShowSettingsTabPanel(std::unique_ptr<Component> panel);
    };
}