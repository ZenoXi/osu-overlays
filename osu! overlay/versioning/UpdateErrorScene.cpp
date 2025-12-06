#include "UICore/App.h"
#include "UICore/Window/Window.h"
#include "UpdateErrorScene.h"

#include "Shared/Styles/Styles.h"

void zcom::UpdateErrorScene::Init(SceneOptionsBase* options)
{
    UpdateErrorSceneOptions opt{};
    if (options)
        opt = *reinterpret_cast<const UpdateErrorSceneOptions*>(options);

    auto contentPanel = Create<FlexPanel>(FlexDirection::DOWN);
    contentPanel->size = { 400, 0 };
    contentPanel->autoHeight = true;
    contentPanel->padding = { 20, 20, 20, 20 };
    contentPanel->spacing = 15;
    contentPanel->xAlign = Alignment::CENTER;
    contentPanel->yAlign = Alignment::CENTER;
    contentPanel->itemAlignment = Alignment::CENTER;
    if (opt.clearBackground)
    {
        contentPanel->backgroundColor = Color(0x202020);
        contentPanel->SetProperty(Shadow());
        _basePanel->backgroundColor = Color(0, 0.4f);
    }
    else
    {
        _basePanel->backgroundColor = Color(0x202020);
    }

    auto errorLabel = Create<Label>(opt.errorText ? opt.errorText.value() : std::wstring(L"Unspecified error occured while updating"));
    errorLabel->parentSize = { 1.0f, 0.0f };
    errorLabel->autoHeight = true;
    errorLabel->xTextAlign = TextAlignment::CENTER;
    errorLabel->wordWrapping = WordWrapping::EMERGENCY_BREAK;
    errorLabel->textSelectable = true;
    contentPanel->AddItem(std::move(errorLabel));

    if (opt.showExit || opt.showClose)
    {
        auto buttonRow = Create<FlexPanel>(FlexDirection::RIGHT);
        buttonRow->autoWidth = true;
        buttonRow->autoHeight = true;
        buttonRow->padding = { 5, 5, 5, 5 };
        buttonRow->spacing = 20;

        if (opt.showClose)
        {
            auto closeButton = Create<Button>(L"Got it");
            NeutralButtonStyle::Apply(closeButton.get());
            closeButton->size = { 70, 30 };
            closeButton->SubscribeOnActivated([=]() {
                _window->UninitScene<UpdateErrorScene>();
            }).Detach();
            buttonRow->AddItem(std::move(closeButton));
        }
        
        if (opt.showExit)
        {
            auto exitButton = Create<Button>(L"Exit");
            NeutralButtonStyle::Apply(exitButton.get());
            exitButton->size = { 70, 30 };
            exitButton->SubscribeOnActivated([=]() {
                _app->Exit();
            }).Detach();
            buttonRow->AddItem(std::move(exitButton));
        }

        contentPanel->AddItem(std::move(buttonRow));
    }

    _basePanel->AddItem(std::move(contentPanel));
}
