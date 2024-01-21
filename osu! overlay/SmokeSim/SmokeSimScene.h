#pragma once

#include "Scenes/Scene.h"

#include "Components/Base/Label.h"

#include "Shared/Util/ThreadPool.h"
#include "Shared/Util/Navigation.h"

namespace zcom
{
    struct SmokeSimSceneOptions : public SceneOptionsBase
    {

    };

    class SmokeSimScene : public Scene
    {
        struct Cell
        {
            int index;
            int x;
            int y;
            int leftNeighbourIndex = -1;
            int rightNeighbourIndex = -1;
            int topNeighbourIndex = -1;
            int bottomNeighbourIndex = -1;

            float pressure;
            float flow;
            float velocityX;
            float velocityY;
            
            float newPressure;
            float newVelocityX;
            float newVelocityY;
        };

        struct Particle
        {
            Pos2D<float> position;
            Pos2D<float> velocity;
            D2D1_COLOR_F color;
            TimePoint creationTime;
        };

        int _currentStep = 0;
        bool _paused = false;

        int _width = 300;
        int _height = 200;
        int _totalWidth = _width + 2;
        int _totalHeight = _height + 2;
        int _cellSize = 4;
        std::vector<Cell> _cells;

        Duration _particleLifetime = Duration(1000, MILLISECONDS);
        std::vector<Particle> _particles;
        float _particleDragKoeff = 1.0f;
        int _particlesPerFrame = 4;

        float _velocityMultiplier = 200.0f;

        const int THREAD_COUNT = 4;
        ThreadPool _threadPool;

        float _velocityTransferRate = 0.01f;
        float _velocityDiffusion = 0.01f;
        float _velocityPullRate = 0.2f;
        float _velocityFrictionKoeff = 0.001f;

        int prevMouseX = 0;
        int prevMouseY = 0;

        bool zClicked = false;

        Duration _speedShiftDuration = Duration(0, MILLISECONDS);
        Duration _slowdownPersistenceDuration = Duration(250, MILLISECONDS);
        TimePoint _smokeStartTime = TimePoint(0);
        TimePoint _smokeEndTime = TimePoint(0) - _slowdownPersistenceDuration - _speedShiftDuration;
        bool _addingSmoke = false;

        std::vector<float> u;
        std::vector<float> v;
        std::vector<float> u_prev;
        std::vector<float> v_prev;
        std::vector<float> dens;
        std::vector<float> dens_prev;
        std::vector<float> temp;
        std::vector<float> temp_prev;
        int _IndexAt(int x, int y) { return y * _totalWidth + x; }
        inline int _IndexAbove(int index) { return index - _totalWidth; }
        inline int _IndexBelow(int index) { return index + _totalWidth; }
        inline int _IndexToLeft(int index) { return index - 1; }
        inline int _IndexToRight(int index) { return index + 1; }
        void _SwapPtr(float** l, float** r) { float* temp = *r; *r = *l; *l = temp; }
        void _AddSource(int W, int H, float* x, float* s, float dt);
        void _SetBoundary(int W, int H, int b, float* x);
        void _Diffuse(int W, int H, int b, float* x, float* x0, float diff, float dt);
        void _Advect(int W, int H, int b, float* d, float* d0, float* u, float* v, float dt, bool conserve);
        void _Project(int W, int H, float* u, float* v, float* p, float* div);
        void _VelocityStep(int W, int H, float* u, float* v, float* u0, float* v0, float visc, float dt);
        void _DensityStep(int W, int H, float* x, float* x0, float* u, float* v, float diff, float dt);
        void _TemperatureStep(int W, int H, float* x, float* x0, float* u, float* v, float diff, float dt);
        void _UpdateParticles(float dt);

        float _Clamp(const float value, const float lowerBound, const float upperBound)
        {
            if (value < lowerBound)
                return lowerBound;
            if (value > upperBound)
                return upperBound;
            return value;
        }

    public:
        SmokeSimScene(App* app, zwnd::Window* window);

        const char* GetName() const { return "SmokeSimScene"; }
        static const char* StaticName() { return "SmokeSimScene"; }

    private:
        void _Init(SceneOptionsBase* options);
        void _Uninit();
        void _Focus();
        void _Unfocus();
        void _Update();
        void _Resize(int width, int height, ResizeInfo info);
    };
}