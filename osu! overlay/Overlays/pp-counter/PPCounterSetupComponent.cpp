#include "App.h"
#include "SharedContext.h"
#include "PPCounterSetupComponent.h"
#include "PPCounterConfig.h"

#include "UICore/Components/Base/FlexPanel.h"
#include "UICore/Components/Base/Button.h"
#include "UICore/Components/Base/ColorSelector.h"

#include "Shared/Components/OverlayLayoutSetup.h"
#include "Shared/Components/SectionHeader.h"
#include "Shared/Styles/Styles.h"

void zcom::PPCounterSetupComponent::Init(std::shared_ptr<const Overlay> overlay)
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
    auto generalLabel = Create<Label>(L"PP counter");
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

    auto overlayLayoutSetupPanel = Create<OverlayLayoutSetup>(_scene->GetApp()->config.GetConfigValue(PPCounterConfig::LAYOUT_STRING));
    overlayLayoutSetupPanel->parentSize = { 1.0f, 0.0f };
    overlayLayoutSetupPanel->padding = { 0, 0, 0, 10 };
    overlayLayoutSetupPanel->autoHeight = true;
    overlayLayoutSetupPanel->SubscribeOnOverlayLayoutStringChanged([=](std::wstring layoutString) {
        _scene->GetApp()->config.SetValue(PPCounterConfig::LAYOUT_STRING.name, layoutString);
    }).Detach();

    auto appearancePanel = Create<FlexPanel>(FlexDirection::DOWN);
    appearancePanel->parentSize = { 1.0f, 0.0f };
    appearancePanel->autoHeight = true;

    auto backgroundColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    backgroundColorRow->parentSize = { 1.0f, 0.0f };
    backgroundColorRow->autoHeight = true;
    backgroundColorRow->spacing = 10;
    backgroundColorRow->padding = { 15, 0, 15, 10 };
    auto backgroundColorInput = Create<ColorSelector>();
    backgroundColorInput->UseConfigValue(PPCounterConfig::BACKGROUND_COLOR);
    backgroundColorInput->size = { 60, 26 };
    backgroundColorInput->border.cornerRadius = 2.0f;
    auto backgroundColorLabel = Create<Label>(L"Background color");
    backgroundColorLabel->size = { 0, 26 };
    backgroundColorLabel->yTextAlign = Alignment::CENTER;
    backgroundColorLabel->SetProperty(FlexGrow());
    backgroundColorRow->AddItem(std::move(backgroundColorInput));
    backgroundColorRow->AddItem(std::move(backgroundColorLabel));

    auto backgroundAccentColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    backgroundAccentColorRow->parentSize = { 1.0f, 0.0f };
    backgroundAccentColorRow->autoHeight = true;
    backgroundAccentColorRow->spacing = 10;
    backgroundAccentColorRow->padding = { 15, 0, 15, 10 };
    auto backgroundAccentColorInput = Create<ColorSelector>();
    backgroundAccentColorInput->UseConfigValue(PPCounterConfig::BACKGROUND_ACCENT_COLOR);
    backgroundAccentColorInput->size = { 60, 26 };
    backgroundAccentColorInput->border.cornerRadius = 2.0f;
    auto backgroundAccentColorLabel = Create<Label>(L"Background accent color");
    backgroundAccentColorLabel->size = { 0, 26 };
    backgroundAccentColorLabel->yTextAlign = Alignment::CENTER;
    backgroundAccentColorLabel->SetProperty(FlexGrow());
    backgroundAccentColorRow->AddItem(std::move(backgroundAccentColorInput));
    backgroundAccentColorRow->AddItem(std::move(backgroundAccentColorLabel));

    auto showDifficultyGraphRow = Create<FlexPanel>(FlexDirection::RIGHT);
    showDifficultyGraphRow->parentSize = { 1.0f, 0.0f };
    showDifficultyGraphRow->autoHeight = true;
    showDifficultyGraphRow->spacing = 10;
    showDifficultyGraphRow->padding = { 15, 0, 15, 10 };
    auto showDifficultyGraphCheckbox = Create<Checkbox>();
    showDifficultyGraphCheckbox->size = { 20, 20 };
    showDifficultyGraphCheckbox->backgroundColor = Color(0x101010);
    showDifficultyGraphCheckbox->border.cornerRadius = 2.0f;
    showDifficultyGraphCheckbox->yAlign = Alignment::CENTER;
    showDifficultyGraphCheckbox->checked = _scene->GetApp()->config.GetIntConfigValue(PPCounterConfig::SHOW_DIFFICULTY_GRAPH, Config::ADD_IF_MISSING);
    showDifficultyGraphCheckbox->SubscribeOnStateChanged([=](bool state) {
        _scene->GetApp()->config.SetIntValue(PPCounterConfig::SHOW_DIFFICULTY_GRAPH.name, state);
    }).Detach();
    auto showDifficultyGraphLabel = Create<Label>(L"Show difficulty graph");
    showDifficultyGraphLabel->size = { 0, 26 };
    showDifficultyGraphLabel->yTextAlign = Alignment::CENTER;
    showDifficultyGraphLabel->SetProperty(FlexGrow());
    showDifficultyGraphLabel->hoverText = L"Difficulty graph appears in the background of the pp counter and displays the strains of the map as a graph";
    showDifficultyGraphRow->AddItem(std::move(showDifficultyGraphCheckbox));
    showDifficultyGraphRow->AddItem(std::move(showDifficultyGraphLabel));

    auto difficultyGraphForegroundColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    difficultyGraphForegroundColorRow->parentSize = { 1.0f, 0.0f };
    difficultyGraphForegroundColorRow->autoHeight = true;
    difficultyGraphForegroundColorRow->spacing = 10;
    difficultyGraphForegroundColorRow->padding = { 15, 0, 15, 10 };
    auto difficultyGraphForegroundColorInput = Create<ColorSelector>();
    difficultyGraphForegroundColorInput->UseConfigValue(PPCounterConfig::DIFFICULTY_GRAPH_FOREGROUND_COLOR);
    difficultyGraphForegroundColorInput->size = { 60, 26 };
    difficultyGraphForegroundColorInput->border.cornerRadius = 2.0f;
    auto difficultyGraphForegroundColorLabel = Create<Label>(L"Difficulty graph foreground color");
    difficultyGraphForegroundColorLabel->size = { 0, 26 };
    difficultyGraphForegroundColorLabel->yTextAlign = Alignment::CENTER;
    difficultyGraphForegroundColorLabel->SetProperty(FlexGrow());
    difficultyGraphForegroundColorRow->AddItem(std::move(difficultyGraphForegroundColorInput));
    difficultyGraphForegroundColorRow->AddItem(std::move(difficultyGraphForegroundColorLabel));

    auto difficultyGraphBackgroundColorRow = Create<FlexPanel>(FlexDirection::RIGHT);
    difficultyGraphBackgroundColorRow->parentSize = { 1.0f, 0.0f };
    difficultyGraphBackgroundColorRow->autoHeight = true;
    difficultyGraphBackgroundColorRow->spacing = 10;
    difficultyGraphBackgroundColorRow->padding = { 15, 0, 15, 10 };
    auto difficultyGraphBackgroundColorInput = Create<ColorSelector>();
    difficultyGraphBackgroundColorInput->UseConfigValue(PPCounterConfig::DIFFICULTY_GRAPH_BACKGROUND_COLOR);
    difficultyGraphBackgroundColorInput->size = { 60, 26 };
    difficultyGraphBackgroundColorInput->border.cornerRadius = 2.0f;
    auto difficultyGraphBackgroundColorLabel = Create<Label>(L"Difficulty graph background color");
    difficultyGraphBackgroundColorLabel->size = { 0, 26 };
    difficultyGraphBackgroundColorLabel->yTextAlign = Alignment::CENTER;
    difficultyGraphBackgroundColorLabel->SetProperty(FlexGrow());
    difficultyGraphBackgroundColorRow->AddItem(std::move(difficultyGraphBackgroundColorInput));
    difficultyGraphBackgroundColorRow->AddItem(std::move(difficultyGraphBackgroundColorLabel));

    appearancePanel->AddItem(Create<SectionHeader>(L"Appearance"));
    appearancePanel->AddItem(std::move(backgroundColorRow));
    appearancePanel->AddItem(std::move(backgroundAccentColorRow));
    appearancePanel->AddItem(std::move(showDifficultyGraphRow));
    appearancePanel->AddItem(std::move(difficultyGraphForegroundColorRow));
    appearancePanel->AddItem(std::move(difficultyGraphBackgroundColorRow));

    flexPanel->AddItem(std::move(generalPanel));
    flexPanel->AddItem(std::move(overlayLayoutSetupPanel));
    flexPanel->AddItem(std::move(appearancePanel));

    AddItem(std::move(flexPanel));
}
