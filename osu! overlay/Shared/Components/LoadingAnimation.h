#pragma once

#include "Components/Base/ComponentBase.h"
#include "Helper/Time.h"

namespace zcom
{
    class LoadingAnimation : public Component
    {
        DEFINE_COMPONENT(LoadingAnimation, Component)
        DEFAULT_DESTRUCTOR(LoadingAnimation)
    protected:
        void Init()
        {
            _startTime = ztime::Main();
        }

    public:
        Value<bool> showAnimation = Value<bool>(false, [=](bool& currentValue, const bool& show) {
            currentValue = show;
            if (!show)
                // One more redraw is necessary after hiding
                InvokeRedraw();
        });
        Value<Color> mainColor = Color(0x808080, 0.0f);
        Value<Color> accentColor = Color(0x808080, 0.5f);
        Value<Duration> animationPeriod = Duration(2, SECONDS);

        void ResetTimer()
        {
            _startTime = ztime::Main();
        }

    private:
        TimePoint _startTime = TimePoint(0);

    protected:
        void _OnUpdate() override
        {
            if (showAnimation)
                InvokeRedraw();
        }
        void _OnDraw(Graphics* g) override
        {
            if (!showAnimation)
                return;

            float animationProgress = ((ztime::Main() - _startTime).GetDuration() % animationPeriod->GetDuration()) / (float)animationPeriod->GetDuration();

            LinearGradient gradient{};
            gradient.startPosition = { 0.0f, 0.0f };
            gradient.endPosition = { (float)size_->width, 0.0f};
            gradient.gradientStops.push_back({ -2.0f + animationProgress * 2.0f, accentColor });
            gradient.gradientStops.push_back({ -1.0f + animationProgress * 2.0f, mainColor });
            gradient.gradientStops.push_back({ 0.0f + animationProgress * 2.0f, accentColor });
            gradient.gradientStops.push_back({ 1.0f + animationProgress * 2.0f, mainColor });
            g->FillRectangle(size_->ToRect().ToRectF(), gradient);
        }
    };
}