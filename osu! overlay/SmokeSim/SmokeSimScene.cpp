#include "App.h" // App.h must be included first
#include "Window/Window.h"
#include "SmokeSimScene.h"

#include "Shared/Util/Navigation.h"
#include "Shared/Util/Functions.h"

zcom::SmokeSimScene::SmokeSimScene(App* app, zwnd::Window* window)
    : Scene(app, window)
{}

void zcom::SmokeSimScene::_Init(SceneOptionsBase* options)
{
    SmokeSimSceneOptions opt;
    if (options)
        opt = *reinterpret_cast<const SmokeSimSceneOptions*>(options);

    _creationTime = ztime::Main();

    _simType = opt.simType;
    _cellSize = opt.cellSize;
    THREAD_COUNT = opt.maxThreads;

    _width = _window->Backend().GetWidth() / _cellSize;
    _height = _window->Backend().GetHeight() / _cellSize;
    _totalWidth = _width + 2;
    _totalHeight = _height + 2;

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

    _canvas->SetBackgroundColor(D2D1::ColorF(0, 1.0f / 255.0f));
    _canvas->BasePanel()->SubscribePostDraw([&](Component* panel, Graphics g) {
        g.target->Clear(D2D1::ColorF(0, 0.0f));

        ID2D1Bitmap1* backgroundBitmap = nullptr;
        g.target->CreateBitmap(
            D2D1::SizeU(_width, _height),
            nullptr,
            0,
            D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET,
                { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }
            ),
            &backgroundBitmap
        );

        SimpleTimer timer;

        // Generate source data
        auto sourceData = std::make_unique<unsigned char[]>(_width * _height * 4);
        for (int y = 0; y < _height; y++)
        {
            for (int x = 0; x < _width; x++)
            {
                // Smoke density
                float density = dens[(y + 1) * (_width + 2) + x + 1];
                float temperature = temp[(y + 1) * (_width + 2) + x + 1];
                float intensity = std::powf(_Clamp(density, 0.0f, 1.0f), 2.0f);

                sourceData[y * _width * 4 + (x * 4) + 0] = 0x88 * intensity;
                sourceData[y * _width * 4 + (x * 4) + 1] = 0x88 * intensity;
                sourceData[y * _width * 4 + (x * 4) + 2] = 0x88 * intensity;
                sourceData[y * _width * 4 + (x * 4) + 3] = 0xFF * intensity;

                // Temperature
                //intensity = _Clamp(temperature / 10.0f, 0.0f, 1.0f);
                //unsigned char r = sourceData[y * _width * 4 + (x * 4) + 2];
                //sourceData[y * _width * 4 + (x * 4) + 2] = r + (0xFF - r) * intensity;
            }
        }

        D2D1_RECT_U destRect = D2D1::RectU(0, 0, _width, _height);
        backgroundBitmap->CopyFromMemory(&destRect, sourceData.get(), _width * 4);
        g.target->DrawBitmap(backgroundBitmap, D2D1::RectF(0.0f, 0.0f, panel->GetWidth(), panel->GetHeight()));

        backgroundBitmap->Release();

        if ((ztime::Main() - _creationTime).GetDuration(SECONDS) < 2)
        {
            float offset = 2.0f;
            D2D1_RECT_F rect = D2D1::RectF(
                offset,
                offset,
                g.target->GetSize().width - offset,
                g.target->GetSize().height - offset
            );
            ID2D1SolidColorBrush* brush;
            g.target->CreateSolidColorBrush(D2D1::ColorF(0xFF0000), &brush);
            g.target->DrawRectangle(rect, brush, offset * 2);
            brush->Release();
        }

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
        //    if (pixelX < 0 || pixelX >= panel->GetWidth() || pixelY < 0 || pixelY >= panel->GetHeight())
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
    }).Detach();
}

void zcom::SmokeSimScene::_Uninit()
{
    _canvas->ClearComponents();
}

void zcom::SmokeSimScene::_Focus()
{

}

void zcom::SmokeSimScene::_Unfocus()
{

}

void zcom::SmokeSimScene::_AddSource(int W, int H, float* x, float* s, float dt)
{
    int i, size = (W + 2) * (H + 2);
    for (i = 0; i < size; i++)
        x[i] += dt * s[i];
}

void zcom::SmokeSimScene::_SetBoundary(int W, int H, int b, float* x)
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
    x[_IndexAt(0, 0)]           = 0.5 * (x[_IndexAt(1, 0)] + x[_IndexAt(0, 1)]);
    x[_IndexAt(0, H + 1)]       = 0.5 * (x[_IndexAt(1, H + 1)] + x[_IndexAt(0, H)]);
    x[_IndexAt(W + 1, 0)]       = 0.5 * (x[_IndexAt(W, 0)] + x[_IndexAt(W + 1, 1)]);
    x[_IndexAt(W + 1, H + 1)]   = 0.5 * (x[_IndexAt(W, H + 1)] + x[_IndexAt(W + 1, H)]);
}

void zcom::SmokeSimScene::_Diffuse(int W, int H, int b, float* x, float* x0, float diff, float dt)
{
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

        _SetBoundary(W, H, b, x);
    }
}

void zcom::SmokeSimScene::_Advect(int W, int H, int b, float* d, float* d0, float* u, float* v, float dt, bool conserve)
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
            if (x < 0.5)
                x = 0.5;
            if (x > W + 0.5)
                x = W + 0.5;
            i0 = (int)x;
            i1 = i0 + 1;
            if (y < 0.5)
                y = 0.5;
            if (y > H + 0.5)
                y = H + 0.5;
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

void zcom::SmokeSimScene::_Project(int W, int H, float* u, float* v, float* p, float* div)
{
    int i, j, k;
    float h, g;

    h = 1.0 / H;
    for (i = 1; i <= W; i++)
    {
        for (j = 1; j <= H; j++)
        {
            div[_IndexAt(i, j)] = -0.5 * h * (
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
            u[_IndexAt(i, j)] -= 0.5 * (p[_IndexAt(i + 1, j)] - p[_IndexAt(i - 1, j)]) / h;
            v[_IndexAt(i, j)] -= 0.5 * (p[_IndexAt(i, j + 1)] - p[_IndexAt(i, j - 1)]) / h;
        }
    }
    _SetBoundary(W, H, 1, u);
    _SetBoundary(W, H, 2, v);
}

void zcom::SmokeSimScene::_VelocityStep(int W, int H, float* u, float* v, float* u0, float* v0, float visc, float dt)
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

void zcom::SmokeSimScene::_DensityStep(int W, int H, float* x, float* x0, float* u, float* v, float diff, float dt)
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

void zcom::SmokeSimScene::_UpdateParticles(float dt)
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

void zcom::SmokeSimScene::_UpdateParameters(bool force)
{
    if (force || ztime::Main() > (_lastParamUpdate + _paramUpdateInterval))
    {
        _lastParamUpdate = ztime::Main();
        _simParams.trailWidth = _app->options.GetIntValue(L"smokesim.cursortrail.trailWidth").value_or(_simParams.trailWidth.Default());
        _simParams.trailEdgeFadeRange = _app->options.GetIntValue(L"smokesim.cursortrail.trailEdgeFadeRange").value_or(_simParams.trailEdgeFadeRange.Default());
        _simParams.trailDensity = _app->options.GetDoubleValue(L"smokesim.cursortrail.trailDensity").value_or(_simParams.trailDensity.Default());
        _simParams.trailWindWidth = _app->options.GetIntValue(L"smokesim.cursortrail.trailWindWidth").value_or(_simParams.trailWindWidth.Default());
        _simParams.trailWindSpeed = _app->options.GetDoubleValue(L"smokesim.cursortrail.trailWindSpeed").value_or(_simParams.trailWindSpeed.Default());
        _simParams.cursorTemp = _app->options.GetDoubleValue(L"smokesim.cursortrail.cursorTemp").value_or(_simParams.cursorTemp.Default());
        _simParams.trailVelocityDiffusion = _app->options.GetDoubleValue(L"smokesim.cursortrail.velocityDiffusion").value_or(_simParams.trailVelocityDiffusion.Default());
        _simParams.trailDensityDiffusion = _app->options.GetDoubleValue(L"smokesim.cursortrail.densityDiffusion").value_or(_simParams.trailDensityDiffusion.Default());
        _simParams.trailTemperatureDiffusion = _app->options.GetDoubleValue(L"smokesim.cursortrail.temperatureDiffusion").value_or(_simParams.trailTemperatureDiffusion.Default());
        _simParams.trailDensityReductionRate = _app->options.GetDoubleValue(L"smokesim.cursortrail.densityReductionRate").value_or(_simParams.trailDensityReductionRate.Default());
        _simParams.trailTemperatureReductionRate = _app->options.GetDoubleValue(L"smokesim.cursortrail.temperatureReductionRate").value_or(_simParams.trailTemperatureReductionRate.Default());
        _simParams.brushWidth = _app->options.GetIntValue(L"smokesim.enhancedsmoke.brushWidth").value_or(_simParams.brushWidth.Default());
        _simParams.brushEdgeFadeRange = _app->options.GetIntValue(L"smokesim.enhancedsmoke.brushEdgeFadeRange").value_or(_simParams.brushEdgeFadeRange.Default());
        _simParams.smokeDensity = _app->options.GetDoubleValue(L"smokesim.enhancedsmoke.smokeDensity").value_or(_simParams.smokeDensity.Default());
        _simParams.cursorWindWidth = _app->options.GetIntValue(L"smokesim.enhancedsmoke.cursorWindWidth").value_or(_simParams.cursorWindWidth.Default());
        _simParams.cursorWindSpeed = _app->options.GetDoubleValue(L"smokesim.enhancedsmoke.cursorWindSpeed").value_or(_simParams.cursorWindSpeed.Default());
        _simParams.slowdownPersistenceDurationMs = _app->options.GetIntValue(L"smokesim.enhancedsmoke.slowdownPersistenceDuration").value_or(_simParams.slowdownPersistenceDurationMs.Default());
        _simParams.smokeVelocityDiffusion = _app->options.GetDoubleValue(L"smokesim.enhancedsmoke.velocityDiffusion").value_or(_simParams.smokeVelocityDiffusion.Default());
        _simParams.smokeDensityDiffusion = _app->options.GetDoubleValue(L"smokesim.enhancedsmoke.densityDiffusion").value_or(_simParams.smokeDensityDiffusion.Default());
        _simParams.smokeDensityReductionRate = _app->options.GetDoubleValue(L"smokesim.enhancedsmoke.densityReductionRate").value_or(_simParams.smokeDensityReductionRate.Default());
        _slowdownPersistenceDuration = Duration(_simParams.slowdownPersistenceDurationMs.Get(), MILLISECONDS);
    }
}

void zcom::SmokeSimScene::_Update()
{
    //std::cout << _particles.size() << '\n';

    _currentStep++;
    float dt = (ztime::Main() - _lastFrameTime).GetDuration(MICROSECONDS) / 1'000'000.0f;
    if (dt > 1.0f / 30.0f)
        dt = 1.0f / 30.0f;

    _canvas->Update();
    _canvas->BasePanel()->InvokeRedraw();
    _UpdateParameters();

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
        addSmoke = GetAsyncKeyState('C') & 0x8000;
        bool slowdownPeriodEnded = (_smokeEndTime + _slowdownPersistenceDuration) <= ztime::Main();
        addWind = !_addingSmoke && slowdownPeriodEnded;
    }
    bool addParticles = true;
    if (addSmoke)
        _paused = false;
    // Using while here to be able to exit the block early
    while ((addWind || addSmoke || addParticles) && !_paused)
    {
        RECT windowRect = _window->Backend().GetWindowRectangle();
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
            lineThickness = _simParams.trailWidth.Get();
            fadeRange = _simParams.trailEdgeFadeRange.Get();
            lineDensity = _simParams.trailDensity.Get();
            windThickness = _simParams.trailWindWidth.Get();
            windMultiplier = _simParams.trailWindSpeed.Get();
            cursorTemp = _simParams.cursorTemp.Get();
        }
        else
        {
            lineThickness = _simParams.brushWidth.Get();
            fadeRange = _simParams.brushEdgeFadeRange.Get();
            lineDensity = _simParams.smokeDensity.Get();
            windThickness = _simParams.cursorWindWidth.Get();
            windMultiplier = _simParams.cursorWindSpeed.Get();
            cursorTemp = 0.0f;
        }

        float cursorVelX = (deltaX / float(_cellSize * _height)) / dt * windMultiplier;
        float cursorVelY = (deltaY / float(_cellSize * _height)) / dt * windMultiplier;

        // Find bounding rectangle
        RECT boundingRect;
        {
            float left = (startPos.x < endPos.x ? startPos.x : endPos.x) - lineThickness;
            float right = (startPos.x > endPos.x ? startPos.x : endPos.x) + lineThickness;
            float top = (startPos.y < endPos.y ? startPos.y : endPos.y) - lineThickness;
            float bottom = (startPos.y > endPos.y ? startPos.y : endPos.y) + lineThickness;
            boundingRect.left = std::floorf(left / _cellSize);
            boundingRect.top = std::floorf(top / _cellSize);
            boundingRect.right = std::ceilf(right / _cellSize);
            boundingRect.bottom = std::ceilf(bottom / _cellSize);
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
        //if (addWind && !addSmoke && !slowdownPeriodEnded)
        //{
        //    slowdownPeriodEnded = false;
        //    _smokeEndTime = ztime::Main() - _slowdownPersistenceDuration;
        //}

        float dtFinal = dt;
        if (_simType == SmokeSimType::ENHANCED_SMOKE && (_addingSmoke || !slowdownPeriodEnded))
            dtFinal /= 8.0f;

        for (int i = 0; i < u.size(); i++)
        {
            // Kill velocities and densities
            u[i] *= 0.9995f;
            v[i] *= 0.9995f;
            temp[i] *= 0.9995f;
            if (_simType == SmokeSimType::CURSOR_TRAIL)
            {
                dens[i] -= _simParams.trailDensityReductionRate.Get() * dtFinal;
                temp[i] -= _simParams.trailTemperatureReductionRate.Get() * dtFinal;
            }
            else
            {
                dens[i] -= _simParams.smokeDensityReductionRate.Get() * dtFinal;
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
            velocityDiffusion = _simParams.trailVelocityDiffusion.Get();
            densityDiffusion = _simParams.trailDensityDiffusion.Get();
            temperatureDiffusion = _simParams.trailTemperatureDiffusion.Get();
        }
        else
        {
            velocityDiffusion = _simParams.smokeVelocityDiffusion.Get();
            densityDiffusion = _simParams.smokeDensityDiffusion.Get();
            temperatureDiffusion = 0.0f;
        }

        _VelocityStep(_width, _height, u.data(), v.data(), u_prev.data(), v_prev.data(), velocityDiffusion, dtFinal);
        _DensityStep(_width, _height, dens.data(), dens_prev.data(), u.data(), v.data(), densityDiffusion, dtFinal);
        if (_simType == SmokeSimType::CURSOR_TRAIL)
            _DensityStep(_width, _height, temp.data(), temp_prev.data(), u.data(), v.data(), temperatureDiffusion, dtFinal);
        _UpdateParticles(dtFinal);
    }
    else
    {
        // Reset velocities
        std::fill(v.begin(), v.end(), 0.0f);
        std::fill(u.begin(), u.end(), 0.0f);
    }

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
}

void zcom::SmokeSimScene::_Resize(int width, int height, ResizeInfo info)
{

}