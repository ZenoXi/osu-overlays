#pragma once

#include "Components/Base/Panel.h"
#include "Components/Base/Label.h"
#include "Helper/AnimationHelper.h"

#include <iomanip>
#include <optional>

namespace zcom
{
    class LeaderboardItem : public Panel
    {
        DEFINE_COMPONENT(LeaderboardItem, Panel)
        DEFAULT_DESTRUCTOR(LeaderboardItem)
    protected:
        void Init(Value<float>* scale)
        {
            Panel::Init();
            parentSize = { 1.0f, 0.0f };
            size = { -10, 60 };
            Panel::backgroundColor = this->backgroundColor;
            border.cornerRadius.ComputedFrom([](float scale) { return 5.0f * scale; }, *scale);
            padding.ComputedFrom([](float scale) { return Rect{ int(10 * scale), int(5 * scale), int(10 * scale), int(5 * scale) }; }, *scale);
            opacity = 1.0f;

            _usernameLabel = Create<Label>(L"-");
            _usernameLabel->parentSize = { 1.0f, 0.0f };
            _usernameLabel->size.ComputedFrom([](float scale) { return Size{ 0, int(30 * scale) }; }, *scale);
            _usernameLabel->font = L"Nunito";
            _usernameLabel->fontSize.ComputedFrom([](float scale) { return 24.0f * scale; }, *scale);
            _usernameLabel->yTextAlign = Alignment::CENTER;
            _usernameLabel->SetProperty(Shadow());

            _rankLabel = Create<Label>(L"-");
            _rankLabel->parentSize = { 1.0f, 1.0f };
            _rankLabel->xAlign = Alignment::END;
            _rankLabel->padding.ComputedFrom([](float scale) { return RectF{ 0.0f, 0.0f, 2.0f * scale, 0.0f }; }, *scale);
            _rankLabel->font = L"Nunito";
            _rankLabel->fontSize.ComputedFrom([](float scale) { return 30.0f * scale; }, *scale);
            _rankLabel->fontStyle = FontStyle::ITALIC;
            _rankLabel->fontWeight = FontWeight::LIGHT;
            _rankLabel->fontColor = Color(0x808080);
            _rankLabel->xTextAlign = TextAlignment::TRAILING;
            _rankLabel->yTextAlign = Alignment::CENTER;
            _rankLabel->zIndex = -2;

            _ppLabel = Create<Label>(L"-");
            _ppLabel->parentSize = { 1.0f, 0.0f };
            _ppLabel->size.ComputedFrom([](float scale) { return Size{ 0, int(20 * scale) }; }, *scale);
            _ppLabel->yAlign = Alignment::END;
            _ppLabel->font = L"Nunito";
            _ppLabel->fontSize.ComputedFrom([](float scale) { return 16.0f * scale; }, *scale);
            _ppLabel->fontStyle = FontStyle::ITALIC;
            _ppLabel->fontWeight = FontWeight::LIGHT;
            _ppLabel->yTextAlign = Alignment::CENTER;
            _ppLabel->SetProperty(Shadow());

            AddItem(_usernameLabel.get());
            AddItem(_rankLabel.get());
            AddItem(_ppLabel.get());
        }

    public:
        Value<Color> backgroundColor = Value<Color>(Color(0x303030, 0.75f), [=](Color& currentValue, const Color& newColor) {
            currentValue = newColor;
            if (!_poppingOut)
                Panel::backgroundColor = newColor;
        });

        Label* GetUsernameLabel() { return _usernameLabel.get(); }
        Label* GetRankLabel() { return _rankLabel.get(); }
        Label* GetPPLabel() { return _ppLabel.get(); }

        int64_t GetRank() const { return _rank; }
        int64_t GetRankOffset() const { return _rankOffset; }

        void SetUsername(const std::wstring& username)
        {
            _usernameLabel->text = username;
        }

        void SetRank(int64_t rank)
        {
            if (_rank == rank)
                return;
            _rank = rank;
            _UpdateRankLabel();
        }

        void SetPP(float pp, std::optional<float> ppIncrease = std::nullopt)
        {
            std::wostringstream ss(L"");
            ss << std::setprecision(2) << std::fixed << pp << " pp";
            if (ppIncrease)
            {
                float value = ppIncrease.value();
                ss << " (+" << (value > 0.0f ? value : 0.0f) << " pp)";
            }
            _ppLabel->text = ss.str();
        }

        void SetRankOffset(int64_t offset)
        {
            if (_rankOffset == offset)
                return;
            _rankOffset = offset;
            _UpdateRankLabel();
        }

        void Move(int targetPosition, Duration moveDuration = Duration(500, MILLISECONDS))
        {
            if (targetPosition == _targetYPosition)
                return;

            if (moveDuration == Duration(0))
            {
                _targetYPosition = targetPosition;
                _yOffset = targetPosition;
                return;
            }

            _moving = true;
            _startYPosition = _yOffset;
            _targetYPosition = targetPosition;
            _moveYStart = ztime::Main();
            _moveYDuration = moveDuration;
        }

        void FadeOut()
        {
            if (_fadingOut)
                return;

            _fadingOut = true;
            _fadingIn = false;
            _startOpacity = opacity;
            _targetOpacity = 1.0f;
            _fadeStart = ztime::Main();
        }

        void FadeIn()
        {
            if (_fadingIn)
                return;

            _fadingIn = true;
            _fadingOut = false;
            _destroy = false;
            _startOpacity = opacity;
            _targetOpacity = 1.0f;
            _fadeStart = ztime::Main();
        }

        void PopOut()
        {
            _poppingOut = true;
            _popOutStart = ztime::Main();
        }

        bool Moving() const { return _moving; }
        bool MovingUp() const { return _targetYPosition < _startYPosition; }
        int Offset() const { return _yOffset; }
        bool Fading() const { return _fadingIn || _fadingOut; }
        bool Destroy() const { return _destroy; }

    private:
        int64_t _rank = 0;
        int64_t _rankOffset = 0;
        std::unique_ptr<Label> _usernameLabel;
        std::unique_ptr<Label> _rankLabel;
        std::unique_ptr<Label> _ppLabel;

        bool _destroy = false;

        int _yOffset = 0;
        bool _moving = false;
        int _startYPosition = 0;
        int _targetYPosition = 0;
        TimePoint _moveYStart = TimePoint(0);
        Duration _moveYDuration = Duration(150, MILLISECONDS);

        bool _fadingIn = false;
        bool _fadingOut = false;
        float _startOpacity = 0.0f;
        float _targetOpacity = 1.0f;
        TimePoint _fadeStart = TimePoint(0);
        Duration _fadeDuration = Duration(500, MILLISECONDS);

        bool _poppingOut = false;
        int _popOutAmount = 0;
        TimePoint _popOutStart = TimePoint(0);
        Duration _popOutDuration = Duration(500, MILLISECONDS);

        void _UpdateRankLabel()
        {
            _rankLabel->text = L"#" + std::to_wstring(_rank + _rankOffset);
        }

    protected:
        void _OnUpdate() override
        {
            Panel::_OnUpdate();

            if (_moving)
            {
                float x = (float)(ztime::Main() - _moveYStart).GetDuration() / _moveYDuration.GetDuration();
                if (x >= 1.0f)
                {
                    _yOffset = _targetYPosition;
                    _moving = false;
                }
                else
                {
                    _yOffset = zanim::Interpolate(_startYPosition, _targetYPosition, zanim::EaseOutPow(x, 2.0f));
                }
            }
            if (_fadingIn || _fadingOut)
            {
                float x = (float)(ztime::Main() - _fadeStart).GetDuration() / _fadeDuration.GetDuration();
                if (x >= 1.0f)
                {
                    opacity = _targetOpacity;
                    if (_fadingOut)
                    {
                        _destroy = true;
                        _fadingOut = false;
                    }
                    else
                    {
                        _fadingIn = false;
                    }
                }
                else
                {
                    opacity = zanim::Interpolate(_startOpacity, _targetOpacity, zanim::EaseOutPow(x, 2.0f));
                }
            }
            if (_poppingOut)
            {
                float x = (float)(ztime::Main() - _popOutStart).GetDuration() / _popOutDuration.GetDuration();
                if (x >= 1.0f)
                {
                    Panel::backgroundColor = this->backgroundColor;
                    _poppingOut = false;
                }
                else
                {
                    Panel::backgroundColor = this->backgroundColor->Multiply(zanim::Interpolate(1.3f, 1.0f, zanim::EaseOutPow(x, 2.0f)));
                }
            }
            else
            {
                Panel::backgroundColor = backgroundColor;
            }
        }
    };
}