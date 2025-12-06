#include "Label.h"
#include "App.h"
#include "Scenes/Scene.h"
#include "Window/Window.h"

zcom::TextMetrics zcom::Label::GetMetrics()
{
    return _scene->GetWindow()->GetTextRenderContext()->GetLayoutTextMetrics(_textDesc);
}

zcom::TextLineMetricsResult zcom::Label::GetLineMetrics()
{
    return _scene->GetWindow()->GetTextRenderContext()->GetLayoutLineMetrics(_textDesc);
}

zcom::TextHitTestResult zcom::Label::HitTestPoint(PointF point)
{
    TextHitTestResult result = _scene->GetWindow()->GetTextRenderContext()->HitTestPoint(_textDesc, MapComponentToTextLayoutCoordinates(point));
    result.hitMetrics.left += padding->left;
    result.hitMetrics.top += _TextTopPos();
    return result;
}

zcom::TextPositionHitTestResult zcom::Label::HitTestTextPosition(size_t textPosition, bool isTrailingHit)
{
    TextPositionHitTestResult result = _scene->GetWindow()->GetTextRenderContext()->HitTestTextPosition(_textDesc, textPosition, isTrailingHit);
    result.position = MapTextLayoutToComponentCoordinates(result.position);
    result.hitMetrics.left += padding->left;
    result.hitMetrics.top += _TextTopPos();
    return result;
}

zcom::TextRangeHitTestResult zcom::Label::HitTestTextRange(size_t textPosition, size_t textLength)
{
    TextRangeHitTestResult result = _scene->GetWindow()->GetTextRenderContext()->HitTestTextRange(_textDesc, textPosition, textLength);
    for (auto& metric : result.hitMetrics)
    {
        metric.left += padding->left;
        metric.top += _TextTopPos();
    }
    return result;
}

void zcom::Label::_OnSelected(bool reverse)
{
    _scene->GetWindow()->keyboardManager.SetExclusiveHandler(this);
    BOOL result = GetKeyboardState(_keyStates);
}

void zcom::Label::_OnDeselected()
{
    _scene->GetWindow()->keyboardManager.ResetExclusiveHandler();

    if (textSelectable)
    {
        _selecting = false;
        selectionStart = 0;
        selectionEnd = 0;
        InvokeRedraw();
    }
}