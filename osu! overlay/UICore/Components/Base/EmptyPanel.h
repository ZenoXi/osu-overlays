#pragma once

#include "ComponentBase.h"

namespace zcom
{
    // Completely barebones component, only contains base component functionality
    class EmptyPanel : public Component
    {
#pragma region base_class
    protected:
        void _OnUpdate() {}
        void _OnDraw(Graphics g) {}
        void _OnResize(int width, int height) {}

    public:
        const char* GetName() const { return Name(); }
        static const char* Name() { return "empty_panel"; }
#pragma endregion

    protected:
        friend class Scene;
        friend class Component;
        EmptyPanel(Scene* scene) : Component(scene) {}
        void Init() {}
    public:
        ~EmptyPanel() {}
        EmptyPanel(EmptyPanel&&) = delete;
        EmptyPanel& operator=(EmptyPanel&&) = delete;
        EmptyPanel(const EmptyPanel&) = delete;
        EmptyPanel& operator=(const EmptyPanel&) = delete;
    };
}