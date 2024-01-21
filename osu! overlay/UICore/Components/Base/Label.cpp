#include "Label.h"
#include "App.h"
#include "Scenes/Scene.h"
#include "Window/Window.h"

void zcom::Label::_OnSelected(bool reverse)
{
    _scene->GetWindow()->keyboardManager.SetExclusiveHandler(this);
    GetKeyboardState(_keyStates);
}

void zcom::Label::_OnDeselected()
{
    _scene->GetWindow()->keyboardManager.ResetExclusiveHandler();

    if (_textSelectable)
    {
        _selecting = false;
        _selectionStart = 0;
        _selectionLength = 0;
        InvokeRedraw();
    }
}