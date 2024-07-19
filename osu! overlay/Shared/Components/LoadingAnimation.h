#pragma once

#include "Components/Base/ComponentBase.h"
#include "Helper/Time.h"

namespace zcom
{
    // Completely barebones component, only contains base component functionality
    class LoadingAnimation : public Component
    {
    public:
        void ShowAnimation()
        {
            _showAnimation = true;
        }
        void HideAnimation()
        {
            _showAnimation = false;
            InvokeRedraw();
        }
        void ResetTimer()
        {
            _startTime = ztime::Main();
        }
        void SetMainColor(D2D1_COLOR_F color)
        {
            _mainColor = color;
        }
        void SetAccentColor(D2D1_COLOR_F color)
        {
            _accentColor = color;
        }
        void SetAnimationPeriod(Duration period)
        {
            _period = period;
        }
        D2D1_COLOR_F GetMainColor() const
        {
            return _mainColor;
        }
        D2D1_COLOR_F GetAccentColor() const
        {
            return _accentColor;
        }
        Duration GetAnimationPeriod() const
        {
            return _period;
        }

#pragma region base_class
    protected:
        void _OnUpdate() override
        {
            if (_showAnimation)
                InvokeRedraw();
        }
        void _OnDraw(Graphics g) override
        {
            if (!_showAnimation)
                return;

            float animationProgress = ((ztime::Main() - _startTime).GetDuration() % _period.GetDuration()) / (float)_period.GetDuration();

            ID2D1GradientStopCollection* pGradientStops = nullptr;
            ID2D1LinearGradientBrush* pGradientBrush = nullptr;

            D2D1_GRADIENT_STOP gradientStops[4];
            gradientStops[0].color = _accentColor;
            gradientStops[0].position = -2.0f + animationProgress * 2.0f;
            gradientStops[1].color = _mainColor;
            gradientStops[1].position = -1.0f + animationProgress * 2.0f;
            gradientStops[2].color = _accentColor;
            gradientStops[2].position = 0.0f + animationProgress * 2.0f;
            gradientStops[3].color = _mainColor;
            gradientStops[3].position = 1.0f + animationProgress * 2.0f;
            g.target->CreateGradientStopCollection(
                gradientStops,
                4,
                D2D1_GAMMA_2_2,
                D2D1_EXTEND_MODE_CLAMP,
                &pGradientStops
            );
            if (pGradientStops)
            {
                g.target->CreateLinearGradientBrush(
                    D2D1::LinearGradientBrushProperties(
                        D2D1::Point2F(0, 0),
                        D2D1::Point2F((FLOAT)GetWidth(), 0)),
                    pGradientStops,
                    &pGradientBrush
                );
                if (pGradientBrush)
                {
                    g.target->FillRectangle(D2D1::RectF(0, 0, (FLOAT)GetWidth(), (FLOAT)GetHeight()), pGradientBrush);
                    pGradientBrush->Release();
                }
                else
                {
                    // TODO: Logging
                }
                pGradientStops->Release();
            }
            else
            {
                // TODO: Logging
            }
        }

    public:
        const char* GetName() const override { return Name(); }
        static const char* Name() { return "loading_animation"; }
#pragma endregion

    private:
        bool _showAnimation = false;
        TimePoint _startTime = TimePoint(0);
        Duration _period = Duration(2, SECONDS);
        D2D1_COLOR_F _mainColor = D2D1::ColorF(0x808080, 0.0f);
        D2D1_COLOR_F _accentColor = D2D1::ColorF(0x808080, 0.5f);

    protected:
        friend class Scene;
        friend class Component;
        LoadingAnimation(Scene* scene) : Component(scene) {}
        void Init()
        {
            _startTime = ztime::Main();
        }
    public:
        ~LoadingAnimation() {}
        LoadingAnimation(LoadingAnimation&&) = delete;
        LoadingAnimation& operator=(LoadingAnimation&&) = delete;
        LoadingAnimation(const LoadingAnimation&) = delete;
        LoadingAnimation& operator=(const LoadingAnimation&) = delete;
    };
}