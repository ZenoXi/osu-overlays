#include "App.h"
#include "SharedContext.h"
#include "ComboCounterSetupComponent.h"
#include "ComboCounterConfig.h"

#include "UICore/Components/Base/FlexPanel.h"
#include "UICore/Components/Base/Button.h"
#include "UICore/Components/Base/ColorSelector.h"

#include "Shared/Components/OverlayLayoutSetup.h"
#include "Shared/Components/SectionHeader.h"
#include "Shared/Styles/Styles.h"

void zcom::ComboCounterSetupComponent::Init(std::shared_ptr<const Overlay> overlay)
{
    ScrollPanel::Init();

    size = { 300, 0 };
    parentSize = { 0.0f, 1.0f };
    backgroundColor = Color(0x202020);
    yScrollbar.scrollable = true;
    yScrollbar.backgroundVisible = true;

    _overlay = overlay;
    _overlayView.emplace(overlay->Id(), this);

    auto flexPanel = Create<FlexPanel>(FlexDirection::DOWN);
    flexPanel->parentSize = { 1.0f, 0.0f };
    flexPanel->autoHeight = true;

    auto generalPanel = Create<FlexPanel>(FlexDirection::DOWN);
    generalPanel->parentSize = { 1.0f, 0.0f };
    generalPanel->autoHeight = true;

    auto titleRow = Create<FlexPanel>(FlexDirection::RIGHT);
    titleRow->parentSize = { 1.0f, 0.0f };
    titleRow->autoHeight = true;
    titleRow->padding = { 15, 15, 15, 15 };
    auto generalLabel = Create<Label>(L"Combo counter");
    generalLabel->size = { 0, 30 };
    generalLabel->yTextAlign = Alignment::CENTER;
    generalLabel->fontSize = 20.0f;
    generalLabel->SetProperty(FlexGrow());
    auto enableButton = Create<Button>(L"Enable");
    enableButton->size = { 90, 30 };
    enableButton->SetComputedStyle("style", [](Component* item, bool overlayOpen) {
        Button* button = (Button*)item;
        if (!overlayOpen)
        {
            button->Label()->text = L"Enable";
            EnableButtonStyle::Apply(button);
        }
        else
        {
            button->Label()->text = L"Disable";
            DisableButtonStyle::Apply(button);
        }
    }, _overlayView->enabled_);
    enableButton->activation = ButtonActivation::RELEASE;
    enableButton->SubscribeOnActivated([=]() {
        if (!_overlayView->enabled_)
            _scene->GetApp()->Shared<SharedContext*>()->overlayManager.EnableOverlay(_overlay);
        else
            _scene->GetApp()->Shared<SharedContext*>()->overlayManager.DisableOverlay(_overlay->Id());
    }).Detach();
    titleRow->AddItem(std::move(generalLabel));
    titleRow->AddItem(std::move(enableButton));

    generalPanel->AddItem(std::move(titleRow));

    auto overlayLayoutSetupPanel = Create<OverlayLayoutSetup>(_scene->GetApp()->config.GetConfigValue(ComboCounterConfig::LAYOUT_STRING));
    overlayLayoutSetupPanel->parentSize = { 1.0f, 0.0f };
    overlayLayoutSetupPanel->padding = { 0, 0, 0, 10 };
    overlayLayoutSetupPanel->autoHeight = true;
    overlayLayoutSetupPanel->SubscribeOnOverlayLayoutStringChanged([=](std::wstring layoutString) {
        _scene->GetApp()->config.SetValue(ComboCounterConfig::LAYOUT_STRING.name, layoutString);
    }).Detach();

    auto appearancePanel = Create<FlexPanel>(FlexDirection::DOWN);
    appearancePanel->parentSize = { 1.0f, 0.0f };
    appearancePanel->autoHeight = true;

    auto barColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    barColorRow->parentSize = { 1.0f, 0.0f };
    barColorRow->autoHeight = true;
    barColorRow->spacing = 10;
    barColorRow->padding = { 15, 0, 15, 10 };
    auto barColorInput = Create<ColorSelector>();
    barColorInput->UseConfigValue(ComboCounterConfig::BAR_COLOR);
    barColorInput->size = { 60, 26 };
    barColorInput->border.cornerRadius = 2.0f;
    auto barColorLabel = Create<Label>(L"Bar color");
    barColorLabel->size = { 0, 26 };
    barColorLabel->yTextAlign = Alignment::CENTER;
    barColorLabel->SetProperty(FlexGrow());
    barColorRow->AddItem(std::move(barColorInput));
    barColorRow->AddItem(std::move(barColorLabel));

    auto textColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    textColorRow->parentSize = { 1.0f, 0.0f };
    textColorRow->autoHeight = true;
    textColorRow->spacing = 10;
    textColorRow->padding = { 15, 0, 15, 10 };
    auto textColorInput = Create<ColorSelector>();
    textColorInput->UseConfigValue(ComboCounterConfig::TEXT_COLOR);
    textColorInput->size = { 60, 26 };
    textColorInput->border.cornerRadius = 2.0f;
    auto textColorLabel = Create<Label>(L"Text color");
    textColorLabel->size = { 0, 26 };
    textColorLabel->yTextAlign = Alignment::CENTER;
    textColorLabel->SetProperty(FlexGrow());
    textColorRow->AddItem(std::move(textColorInput));
    textColorRow->AddItem(std::move(textColorLabel));

    auto gridColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    gridColorRow->parentSize = { 1.0f, 0.0f };
    gridColorRow->autoHeight = true;
    gridColorRow->spacing = 10;
    gridColorRow->padding = { 15, 0, 15, 10 };
    auto gridColorInput = Create<ColorSelector>();
    gridColorInput->UseConfigValue(ComboCounterConfig::GRID_COLOR);
    gridColorInput->size = { 60, 26 };
    gridColorInput->border.cornerRadius = 2.0f;
    auto gridColorLabel = Create<Label>(L"Grid color");
    gridColorLabel->size = { 0, 26 };
    gridColorLabel->yTextAlign = Alignment::CENTER;
    gridColorLabel->SetProperty(FlexGrow());
    gridColorRow->AddItem(std::move(gridColorInput));
    gridColorRow->AddItem(std::move(gridColorLabel));

    appearancePanel->AddItem(Create<SectionHeader>(L"Appearance"));
    appearancePanel->AddItem(std::move(barColorRow));
    appearancePanel->AddItem(std::move(textColorRow));
    appearancePanel->AddItem(std::move(gridColorRow));

    flexPanel->AddItem(std::move(generalPanel));
    flexPanel->AddItem(std::move(overlayLayoutSetupPanel));
    flexPanel->AddItem(std::move(appearancePanel));

    AddItem(std::move(flexPanel));
}
