#include "StandardNotificationTemplate.h"

#include "UICore/Components/Base/FlexPanel.h"
#include "UICore/Components/Base/Label.h"

NotificationInfo StandardNotificationTemplate::ToNotificationInfo() const
{
    NotificationInfo info;
    info.showDuration = showDuration;

    zcom::Color color = zcom::Color(0x404040);
    if (customBorderColor)
        color = customBorderColor.value();
    else if (type == NotificationType::SUCCESS)
        color = zcom::Color(0x32C032);
    else if (type == NotificationType::WARNING)
        color = zcom::Color(0xA48000);
    else if (type == NotificationType::FAULT)
        color = zcom::Color(0xCE3232);

    info.componentBuilder = [title = title, text = text, color = color](zcom::Component* parent) {
        auto panel = parent->Create<zcom::FlexPanel>(zcom::FlexDirection::DOWN);
        panel->autoHeight = true;
        panel->padding = { 10, 10, 10, 10 };
        panel->spacing = 2;
        panel->backgroundColor = zcom::Color(0x282824);
        panel->border.cornerRadius = 5.0f;
        panel->border.visible = true;
        panel->border.color = color;
        panel->SetProperty(zcom::Shadow());

        if (title)
        {
            auto titleLabel = parent->Create<zcom::Label>(title.value());
            titleLabel->wordWrapping = zcom::WordWrapping::WRAP;
            titleLabel->parentSize = { 1.0f, 0.0f };
            titleLabel->autoHeight = true;
            titleLabel->fontSize = 17.0f;
            titleLabel->fontWeight = zcom::FontWeight::BOLD;
            panel->AddItem(std::move(titleLabel));
        }

        auto textLabel = parent->Create<zcom::Label>(text);
        textLabel->wordWrapping = zcom::WordWrapping::WRAP;
        textLabel->parentSize = { 1.0f, 0.0f };
        textLabel->autoHeight = true;
        textLabel->fontSize = 16.0f;
        textLabel->fontColor = zcom::Color(0xC0C0C0);
        panel->AddItem(std::move(textLabel));

        return std::move(panel);
    };

    return info;
}
