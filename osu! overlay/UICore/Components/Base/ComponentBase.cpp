#include "ComponentBase.h"

#include "App.h"
#include "Window/Window.h"

void zcom::Component::SafeFullRelease(IUnknown** res)
{
    _scene->GetWindow()->Backend().Graphics()->ReleaseResource(res);
}

void zcom::SafeRelease(IUnknown** res)
{
    if (*res)
    {
        (*res)->Release();
        *res = nullptr;
    }
}

void zcom::Component::_ApplyCursor()
{
    _scene->GetWindow()->Backend().SetCursorIcon(cursorIcon);
}

void zcom::Component::_ShowHoverText()
{
    if (hoverText->empty())
        return;

    TooltipParams params;
    params.displayId = _id;
    params.text = hoverText;
    params.xPos = windowPosition_->x + mousePosition_->x;
    params.yPos = windowPosition_->y + mousePosition_->y;
    _scene->GetWindow()->ShowTooltip(params);
}

void zcom::Component::_HideHoverText()
{
    _scene->GetWindow()->HideTooltip(_id);
}

void zcom::Component::Update()
{
    // Execute pending actions
    // A (possibly expensive, but not relevant for now) copy of the pending actions is
    // created to allow the pending action itself call 'ExecutePending'.
    // TODO: This safeguard is not always necessary, so it would make sense to provide
    // the ability to disable this behavior for a component (or maybe opt-in)
    std::unique_lock<std::mutex> lock(_m_pendingActions);
    std::vector<PendingAction> pendingActionsToExecute = zutil::Extract(_pendingActions, [](const PendingAction& action) { return action.executionTime <= ztime::Main(); });
    lock.unlock();
    for (auto& action : pendingActionsToExecute)
        action.action();

    // Show hover text
    if (_hoverWaiting && (ztime::Main() - _hoverStart) >= hoverTextDelay)
    {
        _hoverWaiting = false;
        _ShowHoverText();
    }

    _OnUpdate();
    _postUpdate->InvokeAll();
}

bool zcom::Component::Redraw()
{
    // _redraw must be above visibility and size in importance, for cases where the component becomes
    // invisible or of size 0 and the parent panel needs to be redrawn to remove this component visually
    if (_redraw)
        return true;
    // invisibility and 0 size override redraw requests of inner components because those components will
    // not have their 'Draw' method called and _redraw will stay 'true'
    if (!visible || size_->width == 0 || size_->height == 0)
        return false;
    return !_canvas || !_canvas->GetSource() || _Redraw();
}

std::optional<zcom::Bitmap> zcom::Component::Draw(Graphics* g)
{
    _redraw = false;

    //static int counter = 0;
    //std::cout << counter++ << '\n';

    if (size_->width == 0 || size_->height == 0)
        return std::nullopt;

    if (!_canvas || !_canvas->GetSource())
    {
        _canvas = g->CreateBitmap(size_->width, size_->height, SEGMENT_POOL_MAIN);
        if (!_canvas)
            return std::nullopt;
    }

    g->PushAndClearTarget(_canvas.value());

    // Invoke pre draw handlers
    _onDraw->InvokeAll(this, g);

    std::optional<Bitmap> contentBitmap = std::nullopt;
    float cornerRadius = border.cornerRadius;
    if (cornerRadius > 0.0f)
    {
        // Create separate target for content
        contentBitmap = g->CreateBitmap(size_->width, size_->height, SEGMENT_POOL_AUX1);
        if (contentBitmap)
        {
            g->PushTarget(contentBitmap.value());
        }
        else
        {
            // TODO: Logging
        }
    }

    // Draw background
    g->Clear(backgroundColor);
    if (backgroundImage->has_value())
        g->DrawBitmap(backgroundImage->value());


    // Draw component
    _preContentDraw->InvokeAll(this, g);
    _OnDraw(g);
    _postContentDraw->InvokeAll(this, g);

    if (cornerRadius > 0.0f)
    {
        if (contentBitmap)
        {
            g->PopTarget(); // contentBitmap
            std::optional<Bitmap> opacityMask = g->CreateBitmap(size_->width, size_->height, SEGMENT_POOL_AUX2);
            if (opacityMask)
            {
                g->PushAndClearTarget(opacityMask.value());
                g->FillRoundedRectangle(RoundedRect(cornerRadius, size_->ToRect().ToRectF()), Color(0));
                g->PopTarget();

                g->FillOpacityMask(contentBitmap.value(), opacityMask.value());
            }
            else
            {
                // TODO: Logging
            }
        }
    }

    // Draw border
    if (border.visible)
    {
        RectF rect = _canvas->GetSize().ToRect().ToRectF().ShrunkBy(border.width * 0.5f);
        if (cornerRadius > 0.0f)
            g->DrawRoundedRectangle(RoundedRect(cornerRadius - border.width * 0.5f, rect), border.color, border.width);
        else
            g->DrawRectangle(rect, border.color, border.width);
    }
    if (selected_)
    {
        RectF rect = _canvas->GetSize().ToRect().ToRectF().ShrunkBy(border.width * 0.5f);
        if (cornerRadius > 0.0f)
            g->DrawRoundedRectangle(RoundedRect(cornerRadius - border.width * 0.5f, rect), border.selectedColor, border.width);
        else
            g->DrawRectangle(rect, border.selectedColor, border.width);
    }

    // If inactive, gray out the canvas
    if (disabled && !_customInactiveDraw)
    {
        BitmapSourceEffect sourceEffect = BitmapSourceEffect(&_canvas.value());
        GrayscaleEffect grayscaleEffect = GrayscaleEffect(&sourceEffect);
        //GrayscaleEffect grayscaleEffect = GrayscaleEffect(&_canvas.value());
        BrightnessEffect brightnessEffect = BrightnessEffect(&grayscaleEffect, 0.6f);
        g->DrawEffectIndirect(&brightnessEffect, SEGMENT_POOL_AUX1, true);
    }

    // Invoke post draw handlers
    _postDraw->InvokeAll(this, g);

    g->PopTarget();

    return _canvas;
}

std::optional<zcom::Bitmap> zcom::Component::ContentImage()
{
    if (!_canvas || !_canvas->GetSource())
        return std::nullopt;
    return _canvas;
}

void zcom::Component::Resize(Size size)
{
    if (size_ != size)
    {
        SetSize(size);
        _OnResize(size_);
    }
}