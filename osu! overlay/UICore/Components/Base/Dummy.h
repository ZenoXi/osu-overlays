#pragma once

#include "ComponentBase.h"

namespace zcom
{
    // Completely barebones component, only contains base component functionality
    class Dummy : public Component
    {
#pragma region base_class
    protected:
        void _OnUpdate() override {}
        void _OnDraw(Graphics g) override {}
        void _OnResize(int width, int height) override {}

    public:
        const char* GetName() const override { return Name(); }
        static const char* Name() { return "dummy"; }
#pragma endregion

    protected:
        friend class Scene;
        friend class Component;
        Dummy(Scene* scene) : Component(scene) {}
        void Init() {}
    public:
        ~Dummy() {}
        Dummy(Dummy&&) = delete;
        Dummy& operator=(Dummy&&) = delete;
        Dummy(const Dummy&) = delete;
        Dummy& operator=(const Dummy&) = delete;
    };
}