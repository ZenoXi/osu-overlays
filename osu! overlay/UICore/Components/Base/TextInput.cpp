#include "TextInput.h"
#include "App.h"
#include "Scenes/Scene.h"
#include "Window/Window.h"

void zcom::TextInput::_OnSelected(bool reverse)
{
    _scene->GetWindow()->keyboardManager.SetExclusiveHandler(this);
    GetKeyboardState(_keyStates);

    _initialText = _textLabel->GetText();
}

void zcom::TextInput::_OnDeselected()
{
    _scene->GetWindow()->keyboardManager.ResetExclusiveHandler();

    if (!_TextMatches(_textLabel->GetText(), _pattern))
    //if (!_pattern.empty() && !_textLabel->GetText().empty() && !std::regex_match(_textLabel->GetText(), std::wregex(_pattern)))
        _textLabel->SetText(_initialText);

    _textLabel->SetSelectionStart(0);
    _textLabel->SetSelectionLength(0);
}