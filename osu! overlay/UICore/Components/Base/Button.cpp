#include "Button.h"
#include "App.h"
#include "Scenes/Scene.h"
#include "Window/Window.h"

void zcom::Button::_OnSelected(bool reverse)
{
    _scene->GetWindow()->keyboardManager.SetExclusiveHandler(this);
}

void zcom::Button::_OnDeselected()
{
    _scene->GetWindow()->keyboardManager.ResetExclusiveHandler();
}