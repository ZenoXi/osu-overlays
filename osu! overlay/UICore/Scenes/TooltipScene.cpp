#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "TooltipScene.h"

#include <iostream>

zcom::TooltipScene::TooltipScene(App* app, zwnd::Window* window)
    : Scene(app, window)
{}

void zcom::TooltipScene::_Init(SceneOptionsBase* options)
{
    if (options)
    {
        TooltipSceneOptions* optionsPtr = reinterpret_cast<TooltipSceneOptions*>(options);
        _showRequestSubscription = std::move(*optionsPtr->showRequestSubscriptionWrapper);
    }

    _label = Create<Label>(L"-");
    _label->SetMargins({ 3.0f, 3.0f, 3.0f, 3.0f });
    _label->SetVerticalTextAlignment(zcom::Alignment::CENTER);
    _label->SetFont(L"Segoe UI");
    _label->SetFontSize(13.0f);
    _label->SetFontColor(D2D1::ColorF(0.8f, 0.8f, 0.8f));
    _label->SetBackgroundColor(D2D1::ColorF(0.05f, 0.05f, 0.05f));
    // Enable ClearType
    _label->IgnoreAlpha(true);

    _canvas->AddComponent(_label.get());
    _canvas->SetBackgroundColor(D2D1::ColorF(0));
}

void zcom::TooltipScene::_Uninit()
{
    _canvas->ClearComponents();
}

void zcom::TooltipScene::_Focus()
{

}

void zcom::TooltipScene::_Unfocus()
{

}

void zcom::TooltipScene::_Update()
{
    _canvas->Update();

    if (_showRequestSubscription)
    {
        _showRequestSubscription->HandlePendingEvents([&](const TooltipParams& params) {
            if (_currentlyDisplayed && _displayId.has_value() && _displayId.value() == params.displayId.value())
                return;
            _currentlyDisplayed = true;
            _displayId = params.displayId;

            // Resize component
            _label->SetWordWrap(false);
            _label->SetText(params.text);
            float textWidth = _label->GetTextWidth();
            int labelWidth = ceilf(textWidth);
            if (labelWidth > params.maxWidth)
                labelWidth = params.maxWidth;
            _label->Resize(labelWidth, 1);
            _label->SetWordWrap(true);
            textWidth = _label->GetTextWidth();
            labelWidth = ceilf(textWidth);
            float textHeight = _label->GetTextHeight();
            int labelHeight = ceilf(textHeight);
            _label->SetBaseSize(labelWidth, labelHeight);
            _label->NotifyLayoutChanged();

            RECT clientAreaMargins = _window->GetNonClientAreaScene()->GetClientAreaMargins();
            RECT finalRect = {
                params.xPos - clientAreaMargins.left,
                params.yPos - _label->GetBaseHeight() - clientAreaMargins.top,
                params.xPos + _label->GetBaseWidth() + clientAreaMargins.right,
                params.yPos + clientAreaMargins.bottom
            };
            _window->Backend().SetWindowRectangle(finalRect);
            _window->Backend().SetDisplayType(zwnd::WindowDisplayType::NORMAL_NOACTIVATE);

            _windowMessageSubscription = _app->GetMessageWindow()->SubscribeToWindowMessages([=](zwnd::WindowMessage message) {
                if (message.id == zwnd::MouseInputMessage::ID())
                {
                    POINT cursorPos;
                    GetCursorPos(&cursorPos);

                    bool closeTooltip = true;
                    if (params.mouseMovementBounds)
                    {
                        Rect bounds = params.mouseMovementBounds.value();
                        if (cursorPos.x >= bounds.left && cursorPos.x < bounds.right && cursorPos.y >= bounds.top && cursorPos.y < bounds.bottom)
                            closeTooltip = false;
                    }
                    if (closeTooltip)
                    {
                        _currentlyDisplayed = false;
                        _windowMessageSubscription->Unsubscribe();
                        _window->Backend().SetDisplayType(zwnd::WindowDisplayType::HIDDEN);
                    }
                }
                return false;
            });
        });
    }
}

void zcom::TooltipScene::_Resize(int width, int height, ResizeInfo info)
{

}