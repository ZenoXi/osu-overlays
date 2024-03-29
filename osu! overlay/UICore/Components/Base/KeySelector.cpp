#include "KeySelector.h"
#include "App.h"
#include "Scenes/Scene.h"
#include "Window/Window.h"

const std::unordered_map<BYTE, std::wstring> zcom::KeySelector::KeyCodeNameMap = {
    { 0x00, L"" },
    { 0x01, L"Left mouse button" },
    { 0x02, L"Right mouse button" },
    { 0x03, L"Cancel" },
    { 0x04, L"Middle mouse button" },
    { 0x05, L"X1 mouse button" },
    { 0x06, L"X2 mouse button" },
    { 0x07, L"" },
    { 0x08, L"Backspace" },
    { 0x09, L"Tab" },
    { 0x0A, L"" },
    { 0x0B, L"" },
    { 0x0C, L"Clear" },
    { 0x0D, L"Enter" },
    { 0x0E, L"" },
    { 0x0F, L"" },
    { 0x10, L"Shift" },
    { 0x11, L"Ctrl" },
    { 0x12, L"Alt" },
    { 0x13, L"Pause" },
    { 0x14, L"Caps lock" },
    { 0x15, L"IME Kana" },
    { 0x16, L"IME on" },
    { 0x17, L"IME Junja" },
    { 0x18, L"IME final" },
    { 0x19, L"IME Hanja" },
    { 0x1A, L"IME off" },
    { 0x1B, L"Esc" },
    { 0x1C, L"IME convert" },
    { 0x1D, L"IME nonconvert" },
    { 0x1E, L"IME accept" },
    { 0x1F, L"IME mode change" },
    { 0x20, L"Space" },
    { 0x21, L"Page up" },
    { 0x22, L"Page down" },
    { 0x23, L"End" },
    { 0x24, L"Home" },
    { 0x25, L"Left arrow" },
    { 0x26, L"Up arrow" },
    { 0x27, L"Right arrow" },
    { 0x28, L"Down arrow" },
    { 0x29, L"Select" },
    { 0x2A, L"Print" },
    { 0x2B, L"Execute" },
    { 0x2C, L"Print screen" },
    { 0x2D, L"Insert" },
    { 0x2E, L"Delete" },
    { 0x2F, L"Help" },
    { 0x30, L"0" },
    { 0x31, L"1" },
    { 0x32, L"2" },
    { 0x33, L"3" },
    { 0x34, L"4" },
    { 0x35, L"5" },
    { 0x36, L"6" },
    { 0x37, L"7" },
    { 0x38, L"8" },
    { 0x39, L"9" },
    { 0x3A, L"" },
    { 0x3B, L"" },
    { 0x3C, L"" },
    { 0x3D, L"" },
    { 0x3E, L"" },
    { 0x3F, L"" },
    { 0x40, L"" },
    { 0x41, L"A" },
    { 0x42, L"B" },
    { 0x43, L"C" },
    { 0x44, L"D" },
    { 0x45, L"E" },
    { 0x46, L"F" },
    { 0x47, L"G" },
    { 0x48, L"H" },
    { 0x49, L"I" },
    { 0x4A, L"J" },
    { 0x4B, L"K" },
    { 0x4C, L"L" },
    { 0x4D, L"M" },
    { 0x4E, L"N" },
    { 0x4F, L"O" },
    { 0x50, L"P" },
    { 0x51, L"Q" },
    { 0x52, L"R" },
    { 0x53, L"S" },
    { 0x54, L"T" },
    { 0x55, L"U" },
    { 0x56, L"V" },
    { 0x57, L"W" },
    { 0x58, L"X" },
    { 0x59, L"Y" },
    { 0x5A, L"Z" },
    { 0x5B, L"Left Win" },
    { 0x5C, L"Right Win" },
    { 0x5D, L"Apps" },
    { 0x5E, L"" },
    { 0x5F, L"Sleep" },
    { 0x60, L"Numpad 0" },
    { 0x61, L"Numpad 1" },
    { 0x62, L"Numpad 2" },
    { 0x63, L"Numpad 3" },
    { 0x64, L"Numpad 4" },
    { 0x65, L"Numpad 5" },
    { 0x66, L"Numpad 6" },
    { 0x67, L"Numpad 7" },
    { 0x68, L"Numpad 8" },
    { 0x69, L"Numpad 9" },
    { 0x6A, L"Multiply" },
    { 0x6B, L"Add" },
    { 0x6C, L"Separator" },
    { 0x6D, L"Subtract" },
    { 0x6E, L"Decimal" },
    { 0x6F, L"Divide" },
    { 0x70, L"F1" },
    { 0x71, L"F2" },
    { 0x72, L"F3" },
    { 0x73, L"F4" },
    { 0x74, L"F5" },
    { 0x75, L"F6" },
    { 0x76, L"F7" },
    { 0x77, L"F8" },
    { 0x78, L"F9" },
    { 0x79, L"F10" },
    { 0x7A, L"F11" },
    { 0x7B, L"F12" },
    { 0x7C, L"F13" },
    { 0x7D, L"F14" },
    { 0x7E, L"F15" },
    { 0x7F, L"F16" },
    { 0x80, L"F17" },
    { 0x81, L"F18" },
    { 0x82, L"F19" },
    { 0x83, L"F20" },
    { 0x84, L"F21" },
    { 0x85, L"F22" },
    { 0x86, L"F23" },
    { 0x87, L"F24" },
    { 0x88, L"" },
    { 0x89, L"" },
    { 0x8A, L"" },
    { 0x8B, L"" },
    { 0x8C, L"" },
    { 0x8D, L"" },
    { 0x8E, L"" },
    { 0x8F, L"" },
    { 0x90, L"Num lock" },
    { 0x91, L"Scroll lock" },
    { 0x92, L"IDK YET" },
    { 0x93, L"IDK YET" },
    { 0x94, L"IDK YET" },
    { 0x95, L"IDK YET" },
    { 0x96, L"IDK YET" },
    { 0x97, L"" },
    { 0x98, L"" },
    { 0x99, L"" },
    { 0x9A, L"" },
    { 0x9B, L"" },
    { 0x9C, L"" },
    { 0x9D, L"" },
    { 0x9E, L"" },
    { 0x9F, L"" },
    { 0xA0, L"Left shift" },
    { 0xA1, L"Right shift" },
    { 0xA2, L"Left ctrl" },
    { 0xA3, L"Right ctrl" },
    { 0xA4, L"Left alt" },
    { 0xA5, L"Right alt" },
    { 0xA6, L"Browser back" },
    { 0xA7, L"Browser forward" },
    { 0xA8, L"Browser refresh" },
    { 0xA9, L"Browser stop" },
    { 0xAA, L"Browser search" },
    { 0xAB, L"Browser favorites" },
    { 0xAC, L"Browser start" },
    { 0xAD, L"Mute" },
    { 0xAE, L"Volume down" },
    { 0xAF, L"Volume up" },
    { 0xB0, L"Next track" },
    { 0xB1, L"Prev track" },
    { 0xB2, L"Stop" },
    { 0xB3, L"Play/Pause" },
    { 0xB4, L"Start mail" },
    { 0xB5, L"Select media" },
    { 0xB6, L"Start app 1" },
    { 0xB7, L"Start app 2" },
    { 0xB8, L"" },
    { 0xB9, L"" },
    { 0xBA, L";" },
    { 0xBB, L"+" },
    { 0xBC, L"," },
    { 0xBD, L"-" },
    { 0xBE, L"." },
    { 0xBF, L"/" },
    { 0xC0, L"`" },
    { 0xC1, L"" },
    { 0xC2, L"" },
    { 0xC3, L"" },
    { 0xC4, L"" },
    { 0xC5, L"" },
    { 0xC6, L"" },
    { 0xC7, L"" },
    { 0xC8, L"" },
    { 0xC9, L"" },
    { 0xCA, L"" },
    { 0xCB, L"" },
    { 0xCC, L"" },
    { 0xCD, L"" },
    { 0xCE, L"" },
    { 0xCF, L"" },
    { 0xD0, L"" },
    { 0xD1, L"" },
    { 0xD2, L"" },
    { 0xD3, L"" },
    { 0xD4, L"" },
    { 0xD5, L"" },
    { 0xD6, L"" },
    { 0xD7, L"" },
    { 0xD8, L"" },
    { 0xD9, L"" },
    { 0xDA, L"" },
    { 0xDB, L"[" },
    { 0xDC, L"\\" },
    { 0xDD, L"]" },
    { 0xDE, L"'" },
    { 0xDF, L"IDK YET" },
    { 0xE0, L"" },
    { 0xE1, L"IDK YET" },
    { 0xE2, L"\\" },
    { 0xE3, L"IDK YET" },
    { 0xE4, L"IDK YET" },
    { 0xE5, L"IME process" },
    { 0xE6, L"IDK YET" },
    { 0xE7, L"" },
    { 0xE8, L"" },
    { 0xE9, L"IDK YET" },
    { 0xEA, L"IDK YET" },
    { 0xEB, L"IDK YET" },
    { 0xEC, L"IDK YET" },
    { 0xED, L"IDK YET" },
    { 0xEE, L"IDK YET" },
    { 0xEF, L"IDK YET" },
    { 0xF0, L"IDK YET" },
    { 0xF1, L"IDK YET" },
    { 0xF2, L"IDK YET" },
    { 0xF3, L"IDK YET" },
    { 0xF4, L"IDK YET" },
    { 0xF5, L"IDK YET" },
    { 0xF6, L"Attn" },
    { 0xF7, L"CrSel" },
    { 0xF8, L"ExSel" },
    { 0xF9, L"Erase EOF" },
    { 0xFA, L"Play" },
    { 0xFB, L"Zoom" },
    { 0xFC, L"" },
    { 0xFD, L"PA1" },
    { 0xFE, L"Clear" },
    { 0xFF, L"" }
};

void zcom::KeySelector::_OnSelected(bool reverse)
{
    _scene->GetWindow()->keyboardManager.SetExclusiveHandler(this);
}

void zcom::KeySelector::_OnDeselected()
{
    _scene->GetWindow()->keyboardManager.ResetExclusiveHandler();
}