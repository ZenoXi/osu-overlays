#pragma once

#include "ComponentBase.h"

#include "Panel.h"

namespace zcom
{
    enum class PanelDirection
    {
        DOWN,
        UP,
        RIGHT,
        LEFT
    };

    class DirectionalPanel : public Panel
    {
    public:
        void SetSpacing(float spacing)
        {
            if (spacing == _spacing)
                return;

            _spacing = spacing;
            _RecalculateLayout(GetWidth(), GetHeight());
        }
        void SetDirection(PanelDirection direction)
        {
            if (direction == _direction)
                return;

            _direction = direction;
            _RecalculateLayout(GetWidth(), GetHeight());
        }
        float GetSpacing() const { return _spacing; }
        PanelDirection GetDirection() const { return _direction; }

    protected:
        friend class Scene;
        friend class Component;
        DirectionalPanel(Scene* scene) : Panel(scene) {}
        void Init(PanelDirection direction)
        {
            Panel::Init();
            _direction = direction;
        }
    public:
        ~DirectionalPanel() {}
        DirectionalPanel(DirectionalPanel&&) = delete;
        DirectionalPanel& operator=(DirectionalPanel&&) = delete;
        DirectionalPanel(const DirectionalPanel&) = delete;
        DirectionalPanel& operator=(const DirectionalPanel&) = delete;

    protected:
        void _RecalculateLayout(int width, int height);

    private:
        PanelDirection _direction = PanelDirection::DOWN;
        float _spacing = 0.0f;

#pragma region base_class
    protected:

    public:
        const char* GetName() const { return Name(); }
        static const char* Name() { return "directional_panel"; }
#pragma endregion
    };
}
