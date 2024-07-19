#pragma once

#include "Components/Base/FlexPanel.h"
#include "Components/Base/Label.h"
#include "Components/Base/Dummy.h"
#include "Shared/Styles/Styles.h"

namespace zcom
{
    class SectionHeader : public FlexPanel
    {
        DEFINE_COMPONENT(SectionHeader, FlexPanel)
        DEFAULT_DESTRUCTOR(SectionHeader)
    protected:
        void Init(std::wstring sectionName, RECT padding = RECT{ 15, 0, 15, 10 })
        {
            FlexPanel::Init(FlexDirection::RIGHT);
            FillContainerWidth();
            SetBaseHeight(30);
            SetSpacing(5);
            SetPadding(padding);

            _label = Create<Label>(sectionName);
            _label->AutomaticWidth();
            _label->SetBaseHeight(30);
            _label->SetVerticalTextAlignment(Alignment::CENTER);
            _label->SetFontSize(14.0f);
            _label->SetFontColor(D2D1::ColorF(0.8f, 0.8f, 0.8f));
            _label->SetFont(L"Arial");

            _separator = Create<Dummy>();
            HorizontalSeparatorStyle::Apply(_separator.get());
            _separator->SetVerticalOffsetPixels(1);
            _separator->SetVerticalAlignment(Alignment::CENTER);
            _separator->SetProperty(FlexShrink());

            AddItem(_label.get());
            AddItem(_separator.get());
        }

    public:
        Label* GetLabel() { return _label.get(); }
        Dummy* GetSeparator() { return _separator.get(); }

    private:
        std::unique_ptr<Label> _label = nullptr;
        std::unique_ptr<Dummy> _separator = nullptr;
    };
}