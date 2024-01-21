#include "ComponentBase.h"

#include "App.h"
#include "Window/Window.h"

void zcom::Component::SafeFullRelease(IUnknown** res)
{
    _scene->GetWindow()->Backend().Graphics()->ReleaseResource(res);
    //App::Instance()->window.gfx.ReleaseResource(res);
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
    _scene->GetWindow()->Backend().SetCursorIcon(_cursor);
}

void zcom::Component::_ShowHoverText()
{
    if (_hoverText.empty())
        return;

    TooltipParams params;
    params.displayId = _id;
    params.text = _hoverText;
    params.xPos = GetWindowX() + GetMousePosX();
    params.yPos = GetWindowY() + GetMousePosY();
    params.mouseMovementBounds = Rect{
        GetWindowX(),
        GetWindowY(),
        GetWindowX() + GetWidth(),
        GetWindowY() + GetHeight()
    };
    _scene->GetWindow()->ShowTooltip(params);
}