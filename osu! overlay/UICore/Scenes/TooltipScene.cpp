#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "TooltipScene.h"

#include <iostream>

void zcom::TooltipScene::Init(SceneOptionsBase* options)
{
    if (options)
    {
        TooltipSceneOptions* optionsPtr = reinterpret_cast<TooltipSceneOptions*>(options);
        _showRequestSubscription = optionsPtr->showRequestEventEmitter->SubscribeAsync([=](TooltipParams params) {
            _basePanel->ExecuteSynchronously([=] { _ShowTooltip(params); });
        });
        _hideRequestSubscription = optionsPtr->hideRequestEventEmitter->SubscribeAsync([=](std::optional<uint64_t> displayId) {
            _basePanel->ExecuteSynchronously([=] { _HideTooltip(displayId); });
        });
    }

    _label = Create<Label>(L"-");
    _label->autoWidth = true;
    _label->autoHeight = true;
    _label->padding = { 3.0f, 3.0f, 3.0f, 3.0f };
    _label->wordWrapping = WordWrapping::WRAP;
    _label->yTextAlign = Alignment::CENTER;
    _label->font = L"Segoe UI";
    _label->fontSize = 13.0f;
    _label->fontColor = Color(0xCCCCCC);
    _label->backgroundColor = Color(0x0D0D0D);
    // Enable ClearType
    _label->ignoreAlpha = true;

    _basePanel->AddItem(_label.get());
    _basePanel->backgroundColor = Color(0);
}

void zcom::TooltipScene::_ShowTooltip(TooltipParams params)
{
    if (_currentlyDisplayed && _displayId && _displayId == params.displayId)
        return;

    _currentlyDisplayed = true;
    _displayId = params.displayId;

    // Resize component
    _label->text = params.text;
    _label->maxAutoWidth = params.maxWidth;

    Rect clientAreaMargins = _window->GetNonClientAreaScene()->GetClientAreaMargins();
    RECT finalRect = {
        params.xPos - clientAreaMargins.left,
        params.yPos - _label->selfSize_->height - clientAreaMargins.top,
        params.xPos + _label->selfSize_->width + clientAreaMargins.right,
        params.yPos + clientAreaMargins.bottom
    };
    _window->Backend().SetWindowRectangle(finalRect);
    _window->Backend().SetDisplayType(zwnd::WindowDisplayType::NORMAL_NOACTIVATE);
}

void zcom::TooltipScene::_HideTooltip(std::optional<uint64_t> displayId)
{
    if (_currentlyDisplayed && (!displayId || _displayId == displayId))
    {
        _currentlyDisplayed = false;
        _window->Backend().SetDisplayType(zwnd::WindowDisplayType::HIDDEN);
    }
}
