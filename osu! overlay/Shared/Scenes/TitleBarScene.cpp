#include "UICore/App.h"
#include "UICore/Window/Window.h"
#include "TitleBarScene.h"

#include "Shared/Styles/Styles.h"
#include "SharedContext.h"

zcom::TitleBarScene::~TitleBarScene()
{
    _updateCheckSubscription.reset();
}

void zcom::TitleBarScene::Init(SceneOptionsBase* options)
{
    DefaultTitleBarScene::Init(options);

    _updateLabel = Create<Label>(L"Major update available");
    _updateLabel->autoWidth = true;
    _updateLabel->autoHeight = true;
    _updateLabel->padding = { 0.0f, 0.0f, 7.0f, 0.0f };
    _updateLabel->yAlign = Alignment::CENTER;
    _updateLabel->font = L"Segoe UI";
    _updateLabel->fontSize = 12.0f;
    _updateLabel->fontColor.ComputedFrom([=](bool active) { return active ? _activeItemTint : _inactiveItemTint; }, _windowIsActive);
    _updateLabel->visible = false;

    _updateButton = Create<Button>(L"");
    NeutralButtonStyle::Apply(_updateButton.get());
    _updateButton->size = { 22, 22 };
    _updateButton->yAlign = Alignment::CENTER;
    _updateButton->SetProperty(FlexMarginAfter(5));
    _updateButton->backgroundColor.ComputedFrom([=](bool active) { return active ? Color(0xAF9055) : Color(0x404040); }, _windowIsActive);
    _updateButton->visible = false;
    _updateButton->SubscribeOnActivated([=]() {
        _app->Shared<SharedContext*>()->settingsWindow.OpenSettings(SettingsTab::UPDATES);
    }).Detach();

    _updateButton->Image()->image = _window->resourceManager.GetImage("update");
    _updateButton->Image()->imagePlacement = ImagePlacement::CENTER;
    _updateButton->Image()->snapToPixels = true;
    _updateButton->Image()->tintColor.ComputedFrom([=](bool active) { return active ? _activeItemTint : _inactiveItemTint; }, _windowIsActive);

    _updateCheckSubscription = _app->Shared<SharedContext*>()->versionManager.CheckForUpdates();
    _updateCheckSubscription->ResetSynchronousHandler([=](std::vector<UpdateData> availableUpdates) {
        if (availableUpdates.empty())
            return;

        _basePanel->ExecuteSynchronously([=]() {
            auto& update = availableUpdates.front();
            if (update.versionTag.GetMajorVersion() > OVERLAY_ENGINE_VERSION.GetMajorVersion())
                _updateLabel->text = L"Major update available";
            else if (update.versionTag.GetMinorVersion() > OVERLAY_ENGINE_VERSION.GetMinorVersion())
                _updateLabel->text = L"Update available";
            else if (update.versionTag.GetPatchVersion() > OVERLAY_ENGINE_VERSION.GetPatchVersion())
                _updateLabel->text = L"Patch available";
            else
                return;

            _updateLabel->visible = true;
            _updateButton->visible = true;
        });
    });

    _contentPanel->InsertItemAfter(_updateLabel.get(), _titleLabel.get());
    _contentPanel->InsertItemAfter(_updateButton.get(), _updateLabel.get());
}

std::vector<RECT> zcom::TitleBarScene::ExcludedCaptionRects()
{
    auto excludedRects = DefaultTitleBarScene::ExcludedCaptionRects();

    excludedRects.push_back({
        _updateButton->position_->x,
        _updateButton->position_->y,
        _updateButton->position_->x + _updateButton->size_->width,
        _updateButton->position_->y + _updateButton->size_->height
    });

    return excludedRects;
}
