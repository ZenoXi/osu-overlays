#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "SmokeSimOverlayComponent.h"
#include "enhanced-smoke/EnhancedSmokeConfig.h"
#include "smoke-trail/SmokeTrailConfig.h"

#include "Shared/Components/OverlayLayoutSetup.h"
#include "Shared/Util/Functions.h"

void zcom::SmokeSimOverlayComponent::Init(std::shared_ptr<const Overlay> overlay, SmokeSimType type)
{
    Panel::Init();

    _overlay = overlay;

    _creationTime = ztime::Main();

    _configValueChangedEvent = _scene->GetApp()->config.SubscribeOnConfigValueChanged();
    _configValueChangedEvent->ResetSynchronousHandler([=](std::optional<std::pair<std::wstring, std::wstring>> changes) {
        ExecuteSynchronously([=]() {
            _UpdateParameters();
            ApplyLayoutStringToComponent(_scene->GetApp()->config.GetConfigValue(type == SmokeSimType::CURSOR_TRAIL ? SmokeTrailConfig::LAYOUT_STRING : EnhancedSmokeConfig::LAYOUT_STRING), this);
        });
    });
    _UpdateParameters();
    ApplyLayoutStringToComponent(_scene->GetApp()->config.GetConfigValue(type == SmokeSimType::CURSOR_TRAIL ? SmokeTrailConfig::LAYOUT_STRING : EnhancedSmokeConfig::LAYOUT_STRING), this);

    _simType = type;
    _cellSize = _scene->GetApp()->config.GetIntConfigValue(type == SmokeSimType::CURSOR_TRAIL ? SmokeTrailConfig::CELL_SIZE : EnhancedSmokeConfig::CELL_SIZE, Config::ADD_IF_MISSING);
    THREAD_COUNT = _scene->GetApp()->config.GetIntConfigValue(type == SmokeSimType::CURSOR_TRAIL ? SmokeTrailConfig::THREAD_COUNT : EnhancedSmokeConfig::THREAD_COUNT, Config::ADD_IF_MISSING);

    _InitSmokeSim();
}

zcom::SmokeSimOverlayComponent::~SmokeSimOverlayComponent()
{
    _UninitSmokeSim();
}

void zcom::SmokeSimOverlayComponent::_InitSmokeSim()
{
    _width = size_->width / _cellSize;
    _height = size_->height / _cellSize;
    _totalWidth = _width + 2;
    _totalHeight = _height + 2;

    cuda_ctx = CudaSmokeSim_Init(_width, _height);

    if (!cuda_ctx)
        for (int i = 0; i < THREAD_COUNT; i++)
            _threadPool.AddThread();

    int size = (_width + 2) * (_height + 2);
    u.resize(size, 0.0f);
    v.resize(size, 0.0f);
    u_prev.resize(size, 0.0f);
    v_prev.resize(size, 0.0f);
    dens.resize(size, 0.0f);
    dens_prev.resize(size, 0.0f);
    temp.resize(size, 0.0f);
    temp_prev.resize(size, 0.0f);
    dens_to_render.resize(size, 0.0f);
    temp_to_render.resize(size, 0.0f);

    _stopSimulation.store(false);
    _simulationThread = std::thread(&SmokeSimOverlayComponent::_Simulate, this);
}

void zcom::SmokeSimOverlayComponent::_UninitSmokeSim()
{
    _stopSimulation.store(true);
    if (_simulationThread.joinable())
        _simulationThread.join();

    if (cuda_ctx)
        CudaSmokeSim_Uninit(cuda_ctx);

    _threadPool.ClearThreads();
}

void zcom::SmokeSimOverlayComponent::_AddSource(int W, int H, float* x, float* s, float dt)
{
    int i, size = (W + 2) * (H + 2);
    for (i = 0; i < size; i++)
        x[i] += dt * s[i];
}

void zcom::SmokeSimOverlayComponent::_SetBoundary(int W, int H, int b, float* x)
{
    for (int i = 1; i <= H; i++)
    {
        x[_IndexAt(0, i)]       = b == 1 ? -x[_IndexAt(1, i)] : x[_IndexAt(1, i)];
        x[_IndexAt(W + 1, i)]   = b == 1 ? -x[_IndexAt(W, i)] : x[_IndexAt(W, i)];
    }
    for (int i = 1; i <= W; i++)
    {
        x[_IndexAt(i, 0)]       = b == 2 ? -x[_IndexAt(i, 1)] : x[_IndexAt(i, 1)];
        x[_IndexAt(i, H + 1)]   = b == 2 ? -x[_IndexAt(i, H)] : x[_IndexAt(i, H)];
    }
    x[_IndexAt(0, 0)]           = 0.5f * (x[_IndexAt(1, 0)] + x[_IndexAt(0, 1)]);
    x[_IndexAt(0, H + 1)]       = 0.5f * (x[_IndexAt(1, H + 1)] + x[_IndexAt(0, H)]);
    x[_IndexAt(W + 1, 0)]       = 0.5f * (x[_IndexAt(W, 0)] + x[_IndexAt(W + 1, 1)]);
    x[_IndexAt(W + 1, H + 1)]   = 0.5f * (x[_IndexAt(W, H + 1)] + x[_IndexAt(W + 1, H)]);
}

void zcom::SmokeSimOverlayComponent::_Diffuse(int W, int H, int b, float* x, float* x0, float diff, float dt)
{
    if (diff <= 0.0f)
    {
        for (int i = 1; i <= W; i++)
        {
            for (int j = 1; j <= H; j++)
            {
                int index = _IndexAt(i, j);
                x[index] = x0[index];
            }
        }
        return;
    }

    float a = dt * diff;

    for (int k = 0; k < 4; k++)
    {
        int INTERVAL = W / THREAD_COUNT;
        std::vector<ThreadPool::ThreadData*> threads;
        for (int idx = 0; idx < THREAD_COUNT; idx++)
        {
            int startIndex = 1;
            int endIndex = W + 1;

            if (idx > 0)
                startIndex = INTERVAL * idx;
            if (idx < THREAD_COUNT - 1)
                endIndex = INTERVAL * (idx + 1);

            ThreadPool::ThreadData* thread = _threadPool.GetThread(idx);
            thread->DoWork(std::move([=](auto unused) {
                for (int i = startIndex; i < endIndex; i++)
                {
                    for (int j = 1; j <= H; j++)
                    {
                        int index = _IndexAt(i, j);
                        x[index] = (
                            x0[index] + a * (
                                x[_IndexToLeft(index)] +
                                x[_IndexToRight(index)] +
                                x[_IndexAbove(index)] +
                                x[_IndexBelow(index)]
                            )
                        ) / (1 + 4 * a);
                    }
                }
            }));
            threads.push_back(thread);
        }

        while (1)
        {
            bool stillRunning = false;
            for (auto thread : threads)
            {
                if (thread->taskRunning.load())
                {
                    stillRunning = true;
                    break;
                }
            }
            if (!stillRunning)
                break;
        }
    }

    _SetBoundary(W, H, b, x);
}

void zcom::SmokeSimOverlayComponent::_Advect(int W, int H, int b, float* d, float* d0, float* u, float* v, float dt, bool conserve)
{
    int i, j, i0, j0, i1, j1;
    float x, y, s0, t0, s1, t1, dt0;

    float oldValSum = 0.0f;
    float newValSum = 0.0f;

    dt0 = dt * H;
    //dt0 = dt * _velocityMultiplier;
    for (i = 1; i <= W; i++) {
        for (j = 1; j <= H; j++) {
            x = i - dt0 * u[_IndexAt(i, j)];
            y = j - dt0 * v[_IndexAt(i, j)];
            if (x < 0.5f)
                x = 0.5f;
            if (x > W + 0.5f)
                x = W + 0.5f;
            i0 = (int)x;
            i1 = i0 + 1;
            if (y < 0.5f)
                y = 0.5f;
            if (y > H + 0.5f)
                y = H + 0.5f;
            j0 = (int)y;
            j1 = j0 + 1;
            
            s1 = x - i0;
            s0 = 1 - s1;
            
            t1 = y - j0;
            t0 = 1 - t1;
            
            d[_IndexAt(i, j)] =
                s0 * (t0 * d0[_IndexAt(i0, j0)] + t1 * d0[_IndexAt(i0, j1)]) +
                s1 * (t0 * d0[_IndexAt(i1, j0)] + t1 * d0[_IndexAt(i1, j1)]);

            oldValSum += d0[_IndexAt(i, j)];
            newValSum += d[_IndexAt(i, j)];
        }
    }

    if (conserve && newValSum != 0.0f)
    {
        float ratio = oldValSum / newValSum;
        for (int idx = 0; idx < W * H; idx++)
            d[idx] *= ratio;
    }

    _SetBoundary(W, H, b, d);
}

void zcom::SmokeSimOverlayComponent::_Project(int W, int H, float* u, float* v, float* p, float* div)
{
    int i, j, k;
    float h;

    h = 1.0f / H;
    for (i = 1; i <= W; i++)
    {
        for (j = 1; j <= H; j++)
        {
            div[_IndexAt(i, j)] = -0.5f * h * (
                u[_IndexAt(i + 1, j)] - u[_IndexAt(i - 1, j)] +
                v[_IndexAt(i, j + 1)] - v[_IndexAt(i, j - 1)]
            );
            p[_IndexAt(i, j)] = 0;
        }
    }
    _SetBoundary(W, H, 0, div);
    _SetBoundary(W, H, 0, p);

    for (k = 0; k < 4; k++)
    {
        for (i = 1; i <= W; i++)
        {
            for (j = 1; j <= H; j++)
            {
                p[_IndexAt(i, j)] = (
                    div[_IndexAt(i, j)] +
                    p[_IndexAt(i - 1, j)] +
                    p[_IndexAt(i + 1, j)] +
                    p[_IndexAt(i, j - 1)] +
                    p[_IndexAt(i, j + 1)]
                ) / 4;
            }
        }
        _SetBoundary(W, H, 0, p);
    }

    for (i = 1; i <= W; i++)
    {
        for (j = 1; j <= H; j++)
        {
            u[_IndexAt(i, j)] -= 0.5f * (p[_IndexAt(i + 1, j)] - p[_IndexAt(i - 1, j)]) / h;
            v[_IndexAt(i, j)] -= 0.5f * (p[_IndexAt(i, j + 1)] - p[_IndexAt(i, j - 1)]) / h;
        }
    }
    _SetBoundary(W, H, 1, u);
    _SetBoundary(W, H, 2, v);
}

void zcom::SmokeSimOverlayComponent::_VelocityStep(int W, int H, float* u, float* v, float* u0, float* v0, float visc, float dt)
{
    //u[_IndexAt(50, 10)] += 2.0f;

    _AddSource(W, H, u, u0, 1.0f);
    _AddSource(W, H, v, v0, 1.0f);

    _SwapPtr(&u0, &u);
    _Diffuse(W, H, 1, u, u0, visc, dt);
    _SwapPtr(&v0, &v);
    _Diffuse(W, H, 2, v, v0, visc, dt);

    _Project(W, H, u, v, u0, v0);

    _SwapPtr(&u0, &u);
    _SwapPtr(&v0, &v);

    _Advect(W, H, 1, u, u0, u0, v0, dt, false);
    _Advect(W, H, 2, v, v0, u0, v0, dt, false);

    _Project(W, H, u, v, u0, v0);
}

void zcom::SmokeSimOverlayComponent::_DensityStep(int W, int H, float* x, float* x0, float* u, float* v, float diff, float dt)
{
    _AddSource(W, H, x, x0, 1.0f);

    _SwapPtr(&x0, &x);
    _Diffuse(W, H, 0, x, x0, diff, dt);

    _SwapPtr(&x0, &x);
    _Advect(W, H, 0, x, x0, u, v, dt, true);

    //float prevDensitySum = 0.0f;
    //float newDensitySum = 0.0f;
    //for (int i = 0; i < dens.size(); i++)
    //{
    //    prevDensitySum += x0[i];
    //    newDensitySum += x[i];
    //}
    //std::cout << newDensitySum << ": " << newDensitySum - prevDensitySum << '\n';
}

void zcom::SmokeSimOverlayComponent::_UpdateParticles(float dt)
{
    //for (auto& particle : _particles)
    //{
    //    int cellX = int(particle.position.x / _cellSize);
    //    int cellY = int(particle.position.y / _cellSize);
    //    if (cellX < 0 || cellX >= _width || cellY < 0 || cellY >= _height)
    //        continue;

    //    float adjustedMultiplier = _velocityMultiplier * _cellSize;

    //    Pos2D<float> flowVec = { u[_IndexAt(cellX, cellY)], v[_IndexAt(cellX, cellY)] };
    //    particle.position += flowVec * adjustedMultiplier * dt;

    //    //Pos2D<float> forceVec = flowVec - (particle.velocity / adjustedMultiplier);
    //    //Pos2D<float> deltaVel = forceVec.of_length(std::powf(forceVec.vector_length(), 2.0f) * _particleDragKoeff) * dt;
    //    //if (deltaVel.vector_length_sqr() > forceVec.vector_length_sqr())
    //    //    deltaVel = forceVec;
    //    particle.velocity += -(particle.velocity.of_length(std::powf(particle.velocity.vector_length(), 2.0f) * _particleDragKoeff) * dt);
    //    particle.position += particle.velocity * dt;
    //}

    //auto it = _particles.begin();
    //while (it != _particles.end() && it->creationTime + _particleLifetime <= ztime::Main())
    //    it++;
    //_particles.erase(_particles.begin(), it);
}

void zcom::SmokeSimOverlayComponent::_UpdateParameters()
{
    _simParams.trailColor = _scene->GetApp()->config.GetIntConfigValue(SmokeTrailConfig::TRAIL_COLOR, Config::ADD_IF_MISSING);
    _simParams.trailWidth = _scene->GetApp()->config.GetIntConfigValue(SmokeTrailConfig::TRAIL_WIDTH, Config::ADD_IF_MISSING);
    _simParams.trailEdgeFadeRange = _scene->GetApp()->config.GetIntConfigValue(SmokeTrailConfig::TRAIL_EDGE_FADE_RANGE, Config::ADD_IF_MISSING);
    _simParams.trailDensity = _scene->GetApp()->config.GetDoubleConfigValue(SmokeTrailConfig::TRAIL_DENSITY, Config::ADD_IF_MISSING);
    _simParams.trailWindWidth = _scene->GetApp()->config.GetIntConfigValue(SmokeTrailConfig::TRAIL_WIND_WIDTH, Config::ADD_IF_MISSING);
    _simParams.trailWindSpeed = _scene->GetApp()->config.GetDoubleConfigValue(SmokeTrailConfig::TRAIL_WIND_SPEED, Config::ADD_IF_MISSING);
    _simParams.cursorTemp = _scene->GetApp()->config.GetDoubleConfigValue(SmokeTrailConfig::CURSOR_TEMP, Config::ADD_IF_MISSING);
    _simParams.trailVelocityDiffusion = _scene->GetApp()->config.GetDoubleConfigValue(SmokeTrailConfig::VELOCITY_DIFFUSION, Config::ADD_IF_MISSING);
    _simParams.trailDensityDiffusion = _scene->GetApp()->config.GetDoubleConfigValue(SmokeTrailConfig::DENSITY_DIFFUSION, Config::ADD_IF_MISSING);
    _simParams.trailTemperatureDiffusion = _scene->GetApp()->config.GetDoubleConfigValue(SmokeTrailConfig::TEMPERATURE_DIFFUSION, Config::ADD_IF_MISSING);
    _simParams.trailDensityReductionRate = _scene->GetApp()->config.GetDoubleConfigValue(SmokeTrailConfig::DENSITY_REDUCTION_RATE, Config::ADD_IF_MISSING);
    _simParams.trailTemperatureReductionRate = _scene->GetApp()->config.GetDoubleConfigValue(SmokeTrailConfig::TEMPERATURE_REDUCTION_RATE, Config::ADD_IF_MISSING);
    _simParams.smokeColor = _scene->GetApp()->config.GetIntConfigValue(EnhancedSmokeConfig::SMOKE_COLOR, Config::ADD_IF_MISSING);
    _simParams.brushWidth = _scene->GetApp()->config.GetIntConfigValue(EnhancedSmokeConfig::BRUSH_WIDTH, Config::ADD_IF_MISSING);
    _simParams.brushEdgeFadeRange = _scene->GetApp()->config.GetIntConfigValue(EnhancedSmokeConfig::BRUSH_EDGE_FADE_RANGE, Config::ADD_IF_MISSING);
    _simParams.smokeDensity = _scene->GetApp()->config.GetDoubleConfigValue(EnhancedSmokeConfig::SMOKE_DENSITY, Config::ADD_IF_MISSING);
    _simParams.cursorWindWidth = _scene->GetApp()->config.GetIntConfigValue(EnhancedSmokeConfig::CURSOR_WIND_WIDTH, Config::ADD_IF_MISSING);
    _simParams.cursorWindSpeed = _scene->GetApp()->config.GetDoubleConfigValue(EnhancedSmokeConfig::CURSOR_WIND_SPEED, Config::ADD_IF_MISSING);
    _simParams.slowdownPersistenceDurationMs = _scene->GetApp()->config.GetIntConfigValue(EnhancedSmokeConfig::SLOWDOWN_PERSISTENCE_DURATION, Config::ADD_IF_MISSING);
    _simParams.smokeVelocityDiffusion = _scene->GetApp()->config.GetDoubleConfigValue(EnhancedSmokeConfig::VELOCITY_DIFFUSION, Config::ADD_IF_MISSING);
    _simParams.smokeDensityDiffusion = _scene->GetApp()->config.GetDoubleConfigValue(EnhancedSmokeConfig::DENSITY_DIFFUSION, Config::ADD_IF_MISSING);
    _simParams.smokeDensityReductionRate = _scene->GetApp()->config.GetDoubleConfigValue(EnhancedSmokeConfig::DENSITY_REDUCTION_RATE, Config::ADD_IF_MISSING);
    _simParams.smokeKeyCode = _scene->GetApp()->config.GetIntConfigValue(EnhancedSmokeConfig::SMOKE_KEY_CODE, Config::ADD_IF_MISSING);
    _slowdownPersistenceDuration = Duration(_simParams.slowdownPersistenceDurationMs, MILLISECONDS);
}

void zcom::SmokeSimOverlayComponent::_OnUpdate()
{
    InvokeRedraw();
}

void zcom::SmokeSimOverlayComponent::_Simulate()
{
    std::unique_lock<std::mutex> lock(_mtx);
    SmokeSimParams paramsCopy = _simParams;
    lock.unlock();

    while (!_stopSimulation)
    {
        _currentStep++;
        float dt = (ztime::Main() - _lastFrameTime).GetDuration(MICROSECONDS) / 1'000'000.0f;
        if (dt > 1.0f / 30.0f)
            dt = 1.0f / 30.0f;

        SimpleTimer timer;

        std::fill(u_prev.begin(), u_prev.end(), 0.0f);
        std::fill(v_prev.begin(), v_prev.end(), 0.0f);
        std::fill(dens_prev.begin(), dens_prev.end(), 0.0f);
        std::fill(temp_prev.begin(), temp_prev.end(), 0.0f);

        //dens_prev[_IndexAt(100, 100)] = 100.0f;
        //dens_prev[_IndexAt(15, 50)] = ((rand() % 101) - 50) * 0.1f;

        POINT p;
        GetCursorPos(&p);
        int deltaX = p.x - prevMouseX;
        int deltaY = p.y - prevMouseY;
        float movedPixels = Pos2D<float>(float(deltaX), float(deltaY)).vector_length();
        float movedCells = movedPixels / _cellSize;

        //bool addWind = GetAsyncKeyState('X') & 0x8000;
        //bool addSmoke = GetAsyncKeyState('C') & 0x8000;
        bool addSmoke = true;
        //bool addWind = !addSmoke;
        bool addWind = true;
        if (_simType == SmokeSimType::ENHANCED_SMOKE)
        {
            addSmoke = GetAsyncKeyState(paramsCopy.smokeKeyCode) & 0x8000;
            bool slowdownPeriodEnded = (_smokeEndTime + _slowdownPersistenceDuration) <= ztime::Main();
            addWind = !_addingSmoke && slowdownPeriodEnded;
        }
        bool addParticles = true;
        if (addSmoke)
            _paused = false;
        // Using while here to be able to exit the block early
        while ((addWind || addSmoke || addParticles) && !_paused)
        {
            RECT windowRect = _scene->GetWindow()->Backend().GetWindowRectangle();
            Pos2D<float> startPos = {
                float(prevMouseX - windowRect.left),
                float(prevMouseY - windowRect.top)
            };
            Pos2D<float> endPos = {
                float(p.x - windowRect.left),
                float(p.y - windowRect.top)
            };

            if (prevMouseX < windowRect.left || prevMouseX >= windowRect.right || prevMouseY < windowRect.top || prevMouseY >= windowRect.bottom)
                break;
            if (p.x < windowRect.left || p.x >= windowRect.right || p.y < windowRect.top || p.y >= windowRect.bottom)
                break;

            //// Add particles
            //if (addParticles)
            //{
            //    int particleCount = movedPixels;
            //    //int particleCount = _particlesPerFrame;
            //    for (int i = 0; i < particleCount; i++)
            //    {
            //        float maxVelocity = 100.0f;
            //        float dt = 1.0f / 144.0f;
            //        Particle particle;
            //        particle.position = {
            //            float(prevMouseX - windowRect.left) + deltaX * (i / float(particleCount)),
            //            float(prevMouseY - windowRect.top) + deltaY * (i / float(particleCount))
            //        };
            //        particle.position += point_rotated_by({ 0.0f, 0.0f }, { 5.0f, 0.0f }, Math::TAU * ((rand() % 1001) / 1000.0f));
            //        //particle.position = { 500.0f, 500.0f };
            //        particle.velocity = point_rotated_by({ 0.0f, 0.0f }, { 1.0f, 0.0f }, Math::TAU * ((rand() % 1001) / 1000.0f)) * ((rand() % 1001) / 1000.0f * maxVelocity);
            //        //particle.velocity += Pos2D<float>(float(deltaX), float(deltaY)) / dt / 2;
            //        particle.creationTime = ztime::Main();
            //        _particles.push_back(particle);
            //    }
            //}

            float lineThickness;
            float fadeRange;
            float lineDensity;
            float windThickness;
            float windMultiplier;
            float cursorTemp;
            if (_simType == SmokeSimType::CURSOR_TRAIL)
            {
                lineThickness = (float)paramsCopy.trailWidth;
                fadeRange = (float)paramsCopy.trailEdgeFadeRange;
                lineDensity = (float)paramsCopy.trailDensity;
                windThickness = (float)paramsCopy.trailWindWidth;
                windMultiplier = (float)paramsCopy.trailWindSpeed;
                cursorTemp = (float)paramsCopy.cursorTemp;
            }
            else
            {
                lineThickness = (float)paramsCopy.brushWidth;
                fadeRange = (float)paramsCopy.brushEdgeFadeRange;
                lineDensity = (float)paramsCopy.smokeDensity;
                windThickness = (float)paramsCopy.cursorWindWidth;
                windMultiplier = (float)paramsCopy.cursorWindSpeed;
                cursorTemp = 0.0f;
            }

            float cursorVelX = (deltaX / float(_cellSize * _height)) / dt * windMultiplier;
            float cursorVelY = (deltaY / float(_cellSize * _height)) / dt * windMultiplier;

            // Find bounding rectangle
            RECT boundingRect{};
            {
                float left = (startPos.x < endPos.x ? startPos.x : endPos.x) - lineThickness;
                float right = (startPos.x > endPos.x ? startPos.x : endPos.x) + lineThickness;
                float top = (startPos.y < endPos.y ? startPos.y : endPos.y) - lineThickness;
                float bottom = (startPos.y > endPos.y ? startPos.y : endPos.y) + lineThickness;
                boundingRect.left = (LONG)std::floorf(left / _cellSize);
                boundingRect.top = (LONG)std::floorf(top / _cellSize);
                boundingRect.right = (LONG)std::ceilf(right / _cellSize);
                boundingRect.bottom = (LONG)std::ceilf(bottom / _cellSize);
                if (boundingRect.left < 0)
                    boundingRect.left = 0;
                if (boundingRect.top < 0)
                    boundingRect.top = 0;
                if (boundingRect.right >= _width)
                    boundingRect.right = _width - 1;
                if (boundingRect.bottom >= _height)
                    boundingRect.bottom = _height - 1;
            }

            // Iterate through cells in bounding rectangle to check which fall inside the line
            for (int x = boundingRect.left; x < boundingRect.right; x++)
            {
                for (int y = boundingRect.top; y < boundingRect.bottom; y++)
                {
                    Pos2D<float> cellCenterPos = {
                        float(x * _cellSize + _cellSize / 2.0f),
                        float(y * _cellSize + _cellSize / 2.0f)
                    };
                    int cellIndex = _IndexAt(x + 1, y + 1);

                    bool cellNearLine = false;
                    bool nearStartPoint = false;
                    float distanceToLine = 0.0f;

                    // Between start and end point                
                    Pos2D<float> normalToEnd = (endPos - startPos).of_length(1.0f);
                    Pos2D<float> normalToStart = -normalToEnd;
                    if (dot_product(cellCenterPos - startPos, normalToEnd) >= 0.0f && dot_product(cellCenterPos - endPos, normalToStart) >= 0.0f)
                    {
                        float distToLineCenter = std::fabsf(dot_product(cellCenterPos - startPos, normalToEnd.perpendicularL()));
                        if (distToLineCenter <= lineThickness)
                        {
                            distanceToLine = distToLineCenter;
                            cellNearLine = true;
                        }
                    }
                    // Near start point
                    if (!cellNearLine && (cellCenterPos - startPos).vector_length_sqr() <= lineThickness * lineThickness)
                    {
                        distanceToLine = (cellCenterPos - startPos).vector_length();
                        cellNearLine = true;
                    }
                    // Near end point
                    if (!cellNearLine && (cellCenterPos - endPos).vector_length_sqr() <= lineThickness * lineThickness)
                    {
                        distanceToLine = (cellCenterPos - endPos).vector_length();
                        cellNearLine = true;
                    }

                    if (!cellNearLine)
                        continue;

                    // Add 
                    if (addWind)
                    {
                        if (distanceToLine <= windThickness)
                        {
                            u_prev[cellIndex] = cursorVelX - u[cellIndex];
                            v_prev[cellIndex] = cursorVelY - v[cellIndex];

                            //if (u[cellIndex] < deltaX * windMultiplier)
                            //    u_prev[cellIndex] = deltaX * windMultiplier - u[cellIndex];
                            //if (v[cellIndex] < deltaY * windMultiplier)
                            //    v_prev[cellIndex] = deltaY * windMultiplier - v[cellIndex];

                            //u_prev[cellIndex] = deltaX * windMultiplier;
                            //v_prev[cellIndex] = deltaY * windMultiplier;
                        }
                    }
                    if (addSmoke)
                    {
                        float targetDensity = lineDensity;
                        if (distanceToLine > lineThickness - fadeRange)
                            targetDensity *= ((lineThickness - distanceToLine) / fadeRange);

                        if (dens[cellIndex] < targetDensity)
                            dens_prev[cellIndex] = targetDensity - dens[cellIndex];
                        if (temp[cellIndex] < cursorTemp)
                            temp_prev[cellIndex] = (cursorTemp - temp[cellIndex]) / (1.0f + movedCells);
                    }

                    //// Near start point
                    //if ((cellCenterPos - startPos).vector_length_sqr() <= lineThickness * lineThickness)
                    //{
                    //    if (addWind)
                    //    {
                    //        if ((cellCenterPos - startPos).vector_length_sqr() <= windThickness * windThickness)
                    //        {
                    //            u_prev[cellIndex] = deltaX * windMultiplier;
                    //            v_prev[cellIndex] = deltaY * windMultiplier;
                    //        }
                    //    }
                    //    if (addSmoke)
                    //    {
                    //        float distance = (cellCenterPos - startPos).vector_length();
                    //        float targetDensity = lineDensity;
                    //        if (distance > lineThickness - fadeRange)
                    //            targetDensity *= ((lineThickness - distance) / fadeRange);

                    //        if (dens[cellIndex] < targetDensity)
                    //            dens_prev[cellIndex] = targetDensity - dens[cellIndex];
                    //        if (temp[cellIndex] < cursorTemp)
                    //            temp_prev[cellIndex] = (cursorTemp - temp[cellIndex]) / (1.0f + movedCells);

                    //    }
                    //}
                    //// Near end point
                    //if ((cellCenterPos - endPos).vector_length_sqr() <= lineThickness * lineThickness)
                    //{
                    //    if (addWind)
                    //    {
                    //        if ((cellCenterPos - endPos).vector_length_sqr() <= windThickness * windThickness)
                    //        {
                    //            u_prev[cellIndex] = deltaX * windMultiplier;
                    //            v_prev[cellIndex] = deltaY * windMultiplier;
                    //        }
                    //    }
                    //    if (addSmoke)
                    //    {
                    //        float distance = (cellCenterPos - endPos).vector_length();
                    //        float targetDensity = lineDensity;
                    //        if (distance > lineThickness - fadeRange)
                    //            targetDensity *= ((lineThickness - distance) / fadeRange);

                    //        if (dens[cellIndex] < targetDensity)
                    //            dens_prev[cellIndex] = targetDensity - dens[cellIndex];
                    //        if (temp[cellIndex] < cursorTemp)
                    //            temp_prev[cellIndex] = (cursorTemp - temp[cellIndex]) / (1.0f + movedCells);
                    //    }
                    //}

                    //// Between start and end point                
                    //Pos2D<float> normalToEnd = (endPos - startPos).of_length(1.0f);
                    //Pos2D<float> normalToStart = -normalToEnd;
                    //if (dot_product(cellCenterPos - startPos, normalToEnd) >= 0.0f && dot_product(cellCenterPos - endPos, normalToStart) >= 0.0f)
                    //{
                    //    float distToLineCenter = std::fabsf(dot_product(cellCenterPos - startPos, normalToEnd.perpendicularL()));
                    //    if (distToLineCenter <= lineThickness)
                    //    {
                    //        if (addWind)
                    //        {
                    //            if (distToLineCenter <= windThickness)
                    //            {
                    //                u_prev[cellIndex] = deltaX * windMultiplier;
                    //                v_prev[cellIndex] = deltaY * windMultiplier;
                    //            }
                    //        }
                    //        if (addSmoke)
                    //        {
                    //            float targetDensity = lineDensity;
                    //            if (distToLineCenter > lineThickness - fadeRange)
                    //                targetDensity *= ((lineThickness - distToLineCenter) / fadeRange);

                    //            if (dens[cellIndex] < targetDensity)
                    //                dens_prev[cellIndex] = targetDensity - dens[cellIndex];
                    //            if (temp[cellIndex] < cursorTemp)
                    //                temp_prev[cellIndex] = (cursorTemp - temp[cellIndex]) / (1.0f + movedCells);
                    //        }
                    //    }
                    //}
                }
            }

            break;
        }

        prevMouseX = p.x;
        prevMouseY = p.y;

        if (!_paused)
        {
            if (addSmoke)
            {
                if (!_addingSmoke)
                {
                    _smokeStartTime = ztime::Main();
                    _addingSmoke = true;
                }
            }
            else
            {
                if (_addingSmoke)
                {
                    _smokeEndTime = ztime::Main();
                    _addingSmoke = false;
                }
            }
            bool slowdownPeriodEnded = (_smokeEndTime + _slowdownPersistenceDuration) <= ztime::Main();

            float dtFinal = dt;
            if (_simType == SmokeSimType::ENHANCED_SMOKE && (_addingSmoke || !slowdownPeriodEnded))
                dtFinal /= 16.0f;

            for (int i = 0; i < u.size(); i++)
            {
                // Kill velocities and densities
                u[i] *= 0.9995f;
                v[i] *= 0.9995f;
                temp[i] *= 0.9995f;
                if (_simType == SmokeSimType::CURSOR_TRAIL)
                {
                    dens[i] -= paramsCopy.trailDensityReductionRate * dtFinal;
                    temp[i] -= paramsCopy.trailTemperatureReductionRate * dtFinal;
                }
                else
                {
                    dens[i] -= paramsCopy.smokeDensityReductionRate * dtFinal;
                }
                if (dens[i] < 0.0f)
                    dens[i] = 0.0f;
                if (temp[i] < 0.0f)
                    temp[i] = 0.0f;

                // Apply heat to velocity
                if (temp[i] > 0.0f)
                    v_prev[i] -= temp[i] * dt;
            }

            float velocityDiffusion;
            float densityDiffusion;
            float temperatureDiffusion;
            if (_simType == SmokeSimType::CURSOR_TRAIL)
            {
                velocityDiffusion = paramsCopy.trailVelocityDiffusion;
                densityDiffusion = paramsCopy.trailDensityDiffusion;
                temperatureDiffusion = paramsCopy.trailTemperatureDiffusion;
            }
            else
            {
                velocityDiffusion = paramsCopy.smokeVelocityDiffusion;
                densityDiffusion = paramsCopy.smokeDensityDiffusion;
                temperatureDiffusion = 0.0f;
            }

            //SimpleTimer timer;
            if (cuda_ctx)
            {
                _AddSource(_width, _height, u.data(), u_prev.data(), 1.0f);
                _AddSource(_width, _height, v.data(), v_prev.data(), 1.0f);
                _AddSource(_width, _height, dens.data(), dens_prev.data(), 1.0f);
                _AddSource(_width, _height, temp.data(), temp_prev.data(), 1.0f);

                CudaSmokeSim_StepData data{};
                data.u = u.data();
                data.v = v.data();
                data.dens = dens.data();
                data.temp = temp.data();
                data.dt = dtFinal;
                data.velDiffusion = velocityDiffusion;
                data.densDiffusion = densityDiffusion;
                data.tempDiffusion = temperatureDiffusion;
                CudaSmokeSim_Step(cuda_ctx, &data);
            }
            else
            {
                _VelocityStep(_width, _height, u.data(), v.data(), u_prev.data(), v_prev.data(), velocityDiffusion, dtFinal);
                _DensityStep(_width, _height, dens.data(), dens_prev.data(), u.data(), v.data(), densityDiffusion, dtFinal);
                if (_simType == SmokeSimType::CURSOR_TRAIL)
                    _DensityStep(_width, _height, temp.data(), temp_prev.data(), u.data(), v.data(), temperatureDiffusion, dtFinal);
                _UpdateParticles(dtFinal);
            }
            //std::cout << timer.MicrosElapsed() << '\n';
        }
        else
        {
            // Reset velocities
            std::fill(v.begin(), v.end(), 0.0f);
            std::fill(u.begin(), u.end(), 0.0f);
        }

        lock.lock();
        std::copy(dens.begin(), dens.end(), dens_to_render.begin());
        std::copy(temp.begin(), temp.end(), temp_to_render.begin());
        paramsCopy = _simParams;
        lock.unlock();

        // Put simulation to sleep if all densities are small enough
        if (_currentStep % 10 == 0)
        {
            bool continueRunning = false;
            float densitySum = 0.0f;
            for (float density : dens)
            {
                if (density > 0.001f)
                    continueRunning = true;
                densitySum += density;
            }
            if (_simType == SmokeSimType::ENHANCED_SMOKE && !continueRunning)
                _paused = true;

            //std::cout << densitySum << '\n';
        }

        if (_paused)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void zcom::SmokeSimOverlayComponent::_OnDraw(Graphics* g)
{
    g->Clear();
    //g.target->Clear(D2D1::ColorF(0, 0.0f));

    auto backgroundBitmap = g->CreateBitmap(_width, _height, SEGMENT_POOL_AUX1);

    //ID2D1Bitmap1* backgroundBitmap = nullptr;
    //g.target->CreateBitmap(
    //    D2D1::SizeU(_width, _height),
    //    nullptr,
    //    0,
    //    D2D1::BitmapProperties1(
    //        D2D1_BITMAP_OPTIONS_TARGET,
    //        { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }
    //    ),
    //    &backgroundBitmap
    //);
    if (backgroundBitmap)
    {
        g->PushTarget(backgroundBitmap.value());

        SimpleTimer timer;

        // Generate source data
        auto sourceData = std::make_unique<unsigned char[]>(_width * _height * 4);
        for (int y = 0; y < _height; y++)
        {
            for (int x = 0; x < _width; x++)
            {
                // Smoke density
                float density = dens_to_render[(y + 1) * (_width + 2) + x + 1];
                float temperature = temp_to_render[(y + 1) * (_width + 2) + x + 1];
                float intensity = std::powf(_Clamp(density, 0.0f, 1.0f), 2.0f);

                Color color = Color::ARGB(_simType == SmokeSimType::CURSOR_TRAIL ? _simParams.trailColor : _simParams.smokeColor);

                sourceData[y * _width * 4 + (x * 4) + 0] = unsigned char(color.b * (color.a / 255.0f) * intensity);
                sourceData[y * _width * 4 + (x * 4) + 1] = unsigned char(color.g * (color.a / 255.0f) * intensity);
                sourceData[y * _width * 4 + (x * 4) + 2] = unsigned char(color.r * (color.a / 255.0f) * intensity);
                sourceData[y * _width * 4 + (x * 4) + 3] = unsigned char(0xFF * (color.a / 255.0f) * intensity);

                // Temperature
                //intensity = _Clamp(temperature / 10.0f, 0.0f, 1.0f);
                //unsigned char r = sourceData[y * _width * 4 + (x * 4) + 2];
                //sourceData[y * _width * 4 + (x * 4) + 2] = r + (0xFF - r) * intensity;
            }
        }

        g->FillFromData(sourceData.get(), _width * 4);
        g->PopTarget();
        g->DrawBitmap(backgroundBitmap.value());

        //D2D1_RECT_U destRect = D2D1::RectU(0, 0, _width, _height);
        //backgroundBitmap->CopyFromMemory(&destRect, sourceData.get(), _width * 4);
        //g.target->DrawBitmap(backgroundBitmap, g.GetTargetRect());
    }
    else
    {
        // TODO: Logging
    }

    //if ((ztime::Main() - _creationTime).GetDuration(SECONDS) < 2)
    //{
    //    float width = 4.0f;
    //    RectF rect = g->GetTargetRect().ToRectF().ShrunkBy(width / 2);
    //    g->DrawRectangle(rect, Color(0xFF0000), width);
    //}

    // Velocity field
    //ID2D1SolidColorBrush* lineBrush;
    //ID2D1SolidColorBrush* densityBrush;
    //g.target->CreateSolidColorBrush(D2D1::ColorF(0x440000), &lineBrush);
    //g.target->CreateSolidColorBrush(D2D1::ColorF(0xAAAAAA), &densityBrush);
    //for (int y = 0; y < _width; y++)
    //{
    //    for (int x = 0; x < _height; x++)
    //    {
    //        int x_ = x * _cellSize;
    //        int y_ = y * _cellSize;
    //        int index = (y + 1) * (_width + 2) + x + 1;
    //        float velocityX = u[index];
    //        float velocityY = v[index];
    //        float density = dens[index];

    //        D2D1_RECT_F cellRect = D2D1::RectF(x_, y_, x_ + _cellSize, y_ + _cellSize);
    //        densityBrush->SetOpacity(_Clamp(density, 0.0f, 1.0f));
    //        //g.target->FillRectangle(cellRect, densityBrush);

    //        D2D1_POINT_2F startPos = D2D1::Point2F(x_ + _cellSize / 2.0f, y_ + _cellSize / 2.0f);
    //        D2D1_POINT_2F endPos = startPos;
    //        endPos.x += velocityX * 100.0f;
    //        endPos.y += velocityY * 100.0f;
    //        g.target->DrawLine(startPos, endPos, lineBrush);
    //    }
    //}
    //densityBrush->Release();
    //lineBrush->Release();

    //std::cout << timer.MicrosElapsed() << '\n';

    //for (auto& particle : _particles)
    //{
    //    int pixelX = int(particle.position.x);
    //    int pixelY = int(particle.position.y);
    //    if (pixelX < 0 || pixelX >= panel->size_->width || pixelY < 0 || pixelY >= panel->size_->height)
    //        continue;

    //    float lifetime = ((ztime::Main() - particle.creationTime).GetTicks() / (float)_particleLifetime.GetTicks());
    //    if (lifetime >= 1.0f)
    //        continue;
    //    float opacity = 1.0f - lifetime;
    //    float r_ = 1.0f;
    //    float g_ = 0.5f - lifetime / 2;
    //    float b_ = 0.0f;
    //    //opacity = std::powf(opacity, 2.0f);

    //    //D2D1_RECT_F pixelRect = D2D1::RectF(pixelX - 1.0f, pixelY - 1.0f, pixelX + 2.0f, pixelY + 2.0f);
    //    D2D1_RECT_F pixelRect = D2D1::RectF(pixelX, pixelY, pixelX + 1.0f, pixelY + 1.0f);
    //    ID2D1SolidColorBrush* brush = nullptr;
    //    g.target->CreateSolidColorBrush(D2D1::ColorF(r_, g_, b_, opacity), &brush);
    //    g.target->FillRectangle(pixelRect, brush);
    //    brush->Release();
    //}
}

void zcom::SmokeSimOverlayComponent::_OnResize(Size size)
{
    _UninitSmokeSim();
    _InitSmokeSim();
}
