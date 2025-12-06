#include "TextInput.h"
#include "App.h"
#include "Scenes/Scene.h"
#include "Window/Window.h"

void zcom::TextInput::_OnSelected(bool reverse)
{
    _scene->GetWindow()->keyboardManager.SetExclusiveHandler(this);
    BOOL result = GetKeyboardState(_keyStates);
    // TODO: Logging

    _initialText = _textLabel->text;
}

void zcom::TextInput::_OnDeselected()
{
    _scene->GetWindow()->keyboardManager.ResetExclusiveHandler();

    if (!_TextMatches(_textLabel->text, pattern))
        _textLabel->text = _initialText;

    _textLabel->selectionStart = 0;
    _textLabel->selectionEnd = 0;
}