#pragma once

#include "Panel.h"

#include <optional>

namespace zcom
{
    class FlexGrow : public Property
    {
    public:
        static std::string _NAME_() { return "flex_grow"; }
        FlexGrow(float ratio = 1.0f) : ratio(ratio) {}
        float ratio;
    };

    class FlexShrink : public Property
    {
    public:
        static std::string _NAME_() { return "flex_shrink"; }
        FlexShrink(float ratio = 1.0f) : ratio(ratio) {}
        float ratio;
    };

    class FlexMaxSize : public Property
    {
    public:
        static std::string _NAME_() { return "flex_max_size"; }
        FlexMaxSize(int value = std::numeric_limits<int>::max()) : value(value) {}
        int value;
    };

    class FlexMinSize : public Property
    {
    public:
        static std::string _NAME_() { return "flex_min_size"; }
        FlexMinSize(int value = 0) : value(value) {}
        int value;
    };

    class FlexAlign : public Property
    {
    public:
        static std::string _NAME_() { return "flex_align"; }
        FlexAlign(Alignment value = Alignment::START) : value(value) {}
        Alignment value;
    };

    enum class FlexDirection
    {
        DOWN,
        UP,
        RIGHT,
        LEFT
    };

    class FlexPanel : public Panel
    {
    public:
        void SetSpacing(int spacing)
        {
            if (spacing == _spacing)
                return;

            _spacing = spacing;
            _RecalculateLayout(GetWidth(), GetHeight());
        }
        void SetDirection(FlexDirection direction)
        {
            if (direction == _direction)
                return;

            _direction = direction;
            _RecalculateLayout(GetWidth(), GetHeight());
        }
        // When alignment is not null, all items use the specified alignment
        void SetItemAlignment(std::optional<Alignment> alignment)
        {
            if (alignment == _itemAlignment)
                return;

            _itemAlignment = alignment;
            _RecalculateLayout(GetWidth(), GetHeight());
        }
        void SetSizeFixed(bool widthFixed, bool heightFixed)
        {
            if (widthFixed == _widthFixed && heightFixed == _heightFixed)
                return;

            _widthFixed = widthFixed;
            _heightFixed = heightFixed;
            _RecalculateLayout(GetWidth(), GetHeight());
        }
        void SetWidthFixed(bool widthFixed)
        {
            SetSizeFixed(widthFixed, IsHeightFixed());
        }
        void SetHeightFixed(bool heightFixed)
        {
            SetSizeFixed(IsWidthFixed(), heightFixed);
        }
        int GetSpacing() const { return _spacing; }
        FlexDirection GetDirection() const { return _direction; }
        std::optional<Alignment> GetItemAlignment() const { return _itemAlignment; }
        bool IsWidthFixed() const { return _widthFixed; }
        bool IsHeightFixed() const { return _heightFixed; }

        void FillContainerWidth()
        {
            SetWidthFixed(true);
            SetParentWidthPercent(1.0f);
        }
        void FillContainerHeight()
        {
            SetHeightFixed(true);
            SetParentHeightPercent(1.0f);
        }
        void FillContainerSize()
        {
            FillContainerWidth();
            FillContainerHeight();
        }

    protected:
        friend class Scene;
        friend class Component;
        FlexPanel(Scene* scene) : Panel(scene) {}
        void Init(FlexDirection direction)
        {
            Panel::Init();
            _direction = direction;
        }
    public:
        ~FlexPanel() {}
        FlexPanel(FlexPanel&&) = delete;
        FlexPanel& operator=(FlexPanel&&) = delete;
        FlexPanel(const FlexPanel&) = delete;
        FlexPanel& operator=(const FlexPanel&) = delete;

    protected:
        void _RecalculateLayout(int width, int height) override;

    private:
        FlexDirection _direction = FlexDirection::DOWN;
        int _spacing = 0;
        std::optional<Alignment> _itemAlignment = std::nullopt;
        bool _widthFixed = false;
        bool _heightFixed = false;

#pragma region base_class
    protected:

    public:
        const char* GetName() const override { return Name(); }
        static const char* Name() { return "flex_panel"; }
#pragma endregion
    };
}