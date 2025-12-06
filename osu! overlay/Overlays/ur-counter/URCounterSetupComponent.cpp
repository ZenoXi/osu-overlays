#include "App.h"
#include "SharedContext.h"
#include "URCounterSetupComponent.h"
#include "URCounterConfig.h"
#include "UICore/Components/Base/FlexPanel.h"
#include "UICore/Components/Base/Button.h"
#include "UICore/Components/Base/ColorSelector.h"
#include "Shared/Components/OverlayLayoutSetup.h"
#include "Shared/Components/SectionHeader.h"
#include "Shared/Styles/Styles.h"

void zcom::URCounterSetupComponent::Init(std::shared_ptr<const Overlay> overlay)
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
    auto generalLabel = Create<Label>(L"UR counter");
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

    auto overlayLayoutSetupPanel = Create<OverlayLayoutSetup>(_scene->GetApp()->config.GetConfigValue(URCounterConfig::LAYOUT_STRING));
    overlayLayoutSetupPanel->parentSize = { 1.0f, 0.0f };
    overlayLayoutSetupPanel->padding = { 0, 0, 0, 10 };
    overlayLayoutSetupPanel->autoHeight = true;
    overlayLayoutSetupPanel->SubscribeOnOverlayLayoutStringChanged([=](std::wstring layoutString) {
        _scene->GetApp()->config.SetValue(URCounterConfig::LAYOUT_STRING.name, layoutString);
    }).Detach();

    flexPanel->AddItem(std::move(generalPanel));
    flexPanel->AddItem(std::move(overlayLayoutSetupPanel));
    //flexPanel->AddItem(std::move(appearancePanel));

    AddItem(std::move(flexPanel));
}
