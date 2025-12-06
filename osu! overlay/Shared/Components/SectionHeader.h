#pragma once

#include "Components/Base/FlexPanel.h"
#include "Components/Base/Label.h"
#include "LoadingAnimation.h"
#include "Shared/Styles/Styles.h"

namespace zcom
{
    class SectionHeader : public FlexPanel
    {
        DEFINE_COMPONENT(SectionHeader, FlexPanel)
        DEFAULT_DESTRUCTOR(SectionHeader)
    protected:
        void Init(std::wstring sectionName, Rect padding = { 15, 5, 15, 2 })
        {
            FlexPanel::Init(FlexDirection::RIGHT);
            parentSize = { 1.0f, 0.0f };
            autoHeight = true;
            spacing = 5;
            this->padding = padding;

            _label = Create<Label>(sectionName);
            _label->autoWidth = true;
            _label->size = { 0, 30 };
            _label->yTextAlign = Alignment::CENTER;
            _label->fontSize = 14.0f;
            _label->fontColor = Color(0xCCCCCC);
            _label->font = L"Arial";

            _separator = Create<LoadingAnimation>();
            _separator->mainColor = Color(0x5421FF, 0.5f);
            _separator->accentColor = Color(0xF966AB, 1.0f);
            HorizontalSeparatorStyle::Apply(_separator.get());
            _separator->position = { 0, 1 };
            _separator->yAlign = Alignment::CENTER;
            _separator->SetProperty(FlexShrink());

            AddItem(_label.get());
            AddItem(_separator.get());
        }

    public:
        Label* GetLabel() { return _label.get(); }
        LoadingAnimation* GetSeparator() { return _separator.get(); }

    private:
        std::unique_ptr<Label> _label = nullptr;
        std::unique_ptr<LoadingAnimation> _separator = nullptr;
    };
}