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

    _width = _window->Backend().GetWidth() / _cellSize;
    _height = _window->Backend().GetHeight() / _cellSize;
    _totalWidth = _width + 2;
    _totalHeight = _height + 2;

    for (int i = 0; i < THREAD_COUNT; i++)
        _threadPool.AddThread();

    _cells.resize(_width * _height);
    for (int i = 0; i < _cells.size(); i++)
    {
        int x = i % _width;
        int y = i / _width;
        _cells[i].index = i;
        _cells[i].x = x;
        _cells[i].y = y;
        if (x > 0)
            _cells[i].leftNeighbourIndex = y * _width + x - 1;
        if (x + 1 < _width)
            _cells[i].rightNeighbourIndex = y * _width + x + 1;
        if (y > 0)
            _cells[i].topNeighbourIndex = (y - 1) * _width + x;
        if (y + 1 < _height)
            _cells[i].bottomNeighbourIndex = (y + 1) * _width + x;

        _cells[i].flow = 0.005f;
        _cells[i].pressure = 0.0f;
        _cells[i].velocityX = 0.0f;
        _cells[i].velocityY = 0.0f;
        _cells[i].newPressure = 0.0f;
        _cells[i].newVelocityX = 0.0f;
        _cells[i].newVelocityY = 0.0f;
    }
    //_cells[50 * _width + 50].pressure = 1000.0f;
    //_cells[50 * _width + 50].newPressure = 1000.0f;
    //_cells[4].pressure = 1000.0f;
    //_cells[4].newPressure = 1000.0f;

    //_cells[50 * _width + 1].velocityX = 0.2f;
    //_cells[50 * _width + 1].newVelocityX = 0.2f;

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

        // Gnerate source data
        auto sourceData = std::make_unique<unsigned char[]>(_width * _height * 4);
        //std::fill_n(sourceData.get(), _width * _height * 4, 0);
        //for (int i = 0; i < _width * _height; i++)
        //    sourceData[i * 4 + 3] = 255;

        //for (auto& cell : _cells)
        //{
        //    int x = cell.index % _width;
        //    int y = cell.index / _width;

        //    float color = _Clamp(cell.pressure / 2.0f, 0.0f, 1.0f);

        //    sourceData[y * panel->GetWidth() * 4 + (x * 4) + 0] = color * 255;
        //    sourceData[y * panel->GetWidth() * 4 + (x * 4) + 1] = color * 255;
        //    sourceData[y * panel->GetWidth() * 4 + (x * 4) + 2] = color * 255;
        //}

        //for (auto& cell : _cells)
        //{
        //    float color = _Clamp(std::fabs(cell.velocityX) + std::fabs(cell.velocityY), 0.0f, 1.0f);

        //    sourceData[cell.y * panel->GetWidth() * 4 + (cell.x * 4) + 0] = color * 255;
        //    sourceData[cell.y * panel->GetWidth() * 4 + (cell.x * 4) + 1] = color * 255;
        //    sourceData[cell.y * panel->GetWidth() * 4 + (cell.x * 4) + 2] = color * 255;
        //}

        //ID2D1SolidColorBrush* brush;
        //g.target->CreateSolidColorBrush(D2D1::ColorF(0xBB0000), &brush);
        //for (auto& cell : _cells)
        //{
        //    int cellSize = 10;
        //    int x = cell.x * cellSize;
        //    int y = cell.y * cellSize;

        //    D2D1_POINT_2F startPos = D2D1::Point2F(x + cellSize / 2.0f, y + cellSize / 2.0f);
        //    D2D1_POINT_2F endPos = startPos;
        //    endPos.x += cell.velocityX * 50.0f;
        //    endPos.y += cell.velocityY * 50.0f;
        //    g.target->DrawLine(startPos, endPos, brush);
        //}
        //brush->Release();

        SimpleTimer timer;

        for (int y = 0; y < _height; y++)
        {
            for (int x = 0; x < _width; x++)
            {
                float density = dens[(y + 1) * (_width + 2) + x + 1];
                float temperature = temp[(y + 1) * (_width + 2) + x + 1];
                float intensity = std::powf(_Clamp(density, 0.0f, 1.0f), 2.0f);

                sourceData[y * _width * 4 + (x * 4) + 0] = 0x88 * intensity;
                sourceData[y * _width * 4 + (x * 4) + 1] = 0x88 * intensity;
                sourceData[y * _width * 4 + (x * 4) + 2] = 0x88 * intensity;
                sourceData[y * _width * 4 + (x * 4) + 3] = 0xFF * intensity;

                //intensity = _Clamp(temperature / 10.0f, 0.0f, 1.0f);
                //unsigned char r = sourceData[y * _width * 4 + (x * 4) + 2];
                //sourceData[y * _width * 4 + (x * 4) + 2] = r + (0xFF - r) * intensity;

                //if (x - y > -10 && x - y < 10)
                //    ((uint32_t*)sourceData.get())[y * _width + x] = 0;
            }
        }

        D2D1_RECT_U destRect = D2D1::RectU(0, 0, _width, _height);
        backgroundBitmap->CopyFromMemory(&destRect, sourceData.get(), _width * 4);

        ID2D1Effect* scaleEffect;
        //ID2D1Effect* blurEffect;
        g.target->CreateEffect(CLSID_D2D1Scale, &scaleEffect);
        //g.target->CreateEffect(CLSID_D2D1GaussianBlur, &blurEffect);

        scaleEffect->SetInput(0, backgroundBitmap);
        scaleEffect->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(_cellSize, _cellSize));
        scaleEffect->SetValue(D2D1_SCALE_PROP_INTERPOLATION_MODE, D2D1_SCALE_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);
        scaleEffect->SetValue(D2D1_SCALE_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);

        //blurEffect->SetInputEffect(0, scaleEffect);
        //blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 5.0f);
        //blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION, D2D1_DIRECTIONALBLUR_OPTIMIZATION_QUALITY);
        //blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);

        //g.target->DrawImage(scaleEffect, D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR/*, D2D1_COMPOSITE_MODE_SOURCE_COPY*/);
        g.target->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_ADD);
        g.target->DrawBitmap(backgroundBitmap, D2D1::RectF(0.0f, 0.0f, panel->GetWidth(), panel->GetHeight()));
        g.target->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_SOURCE_OVER);
        //blurEffect->Release();
        scaleEffect->Release();

        //ID2D1BitmapBrush* bbrush;
        //g.target->CreateBitmapBrush(backgroundBitmap, &bbrush);
        //g.target->DrawRectangle(D2D1::RectF(0.0f, 0.0f, panel->GetWidth(), panel->GetHeight()), bbrush, 1000.0f);
        //bbrush->Release();

        ID2D1SolidColorBrush* brush;
        g.target->CreateSolidColorBrush(D2D1::ColorF(0x888888, 0.5f), &brush);
        //g.target->DrawLine(D2D1::Point2F(0.0f, 0.0f), D2D1::Point2F(1000.0f, 1000.0f), brush, 10.0f);
        brush->Release();

        backgroundBitmap->Release();
        
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
    float x, y, s0, t0, s1, t1, dt0, dt1;

    float oldValSum = 0.0f;
    float newValSum = 0.0f;

    //dt0 = dt * W;
    //dt1 = dt * H;
    dt0 = dt * _velocityMultiplier;
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

    _AddSource(W, H, u, u0, dt);
    _AddSource(W, H, v, v0, dt);

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
    for (auto& particle : _particles)
    {
        int cellX = int(particle.position.x / _cellSize);
        int cellY = int(particle.position.y / _cellSize);
        if (cellX < 0 || cellX >= _width || cellY < 0 || cellY >= _height)
            continue;

        float adjustedMultiplier = _velocityMultiplier * _cellSize;

        Pos2D<float> flowVec = { u[_IndexAt(cellX, cellY)], v[_IndexAt(cellX, cellY)] };
        particle.position += flowVec * adjustedMultiplier * dt;

        //Pos2D<float> forceVec = flowVec - (particle.velocity / adjustedMultiplier);
        //Pos2D<float> deltaVel = forceVec.of_length(std::powf(forceVec.vector_length(), 2.0f) * _particleDragKoeff) * dt;
        //if (deltaVel.vector_length_sqr() > forceVec.vector_length_sqr())
        //    deltaVel = forceVec;
        particle.velocity += -(particle.velocity.of_length(std::powf(particle.velocity.vector_length(), 2.0f) * _particleDragKoeff) * dt);
        particle.position += particle.velocity * dt;
    }

    auto it = _particles.begin();
    while (it != _particles.end() && it->creationTime + _particleLifetime <= ztime::Main())
        it++;
    _particles.erase(_particles.begin(), it);
}

void zcom::SmokeSimScene::_Update()
{
    //std::cout << _particles.size() << '\n';

    _currentStep++;

    _canvas->Update();
    _canvas->BasePanel()->InvokeRedraw();

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

        // Add particles
        if (addParticles)
        {
            int particleCount = movedPixels;
            //int particleCount = _particlesPerFrame;
            for (int i = 0; i < particleCount; i++)
            {
                float maxVelocity = 100.0f;
                float dt = 1.0f / 144.0f;
                Particle particle;
                particle.position = {
                    float(prevMouseX - windowRect.left) + deltaX * (i / float(particleCount)),
                    float(prevMouseY - windowRect.top) + deltaY * (i / float(particleCount))
                };
                particle.position += point_rotated_by({ 0.0f, 0.0f }, { 5.0f, 0.0f }, Math::TAU * ((rand() % 1001) / 1000.0f));
                //particle.position = { 500.0f, 500.0f };
                particle.velocity = point_rotated_by({ 0.0f, 0.0f }, { 1.0f, 0.0f }, Math::TAU * ((rand() % 1001) / 1000.0f)) * ((rand() % 1001) / 1000.0f * maxVelocity);
                //particle.velocity += Pos2D<float>(float(deltaX), float(deltaY)) / dt / 2;
                particle.creationTime = ztime::Main();
                _particles.push_back(particle);
            }
        }

        float lineThickness = 2.5f * _cellSize;
        float fadeRange = 2.0f * _cellSize;
        float lineDensity = 1.0f;
        float velocityThickness = 3.0f * _cellSize;
        float velocityMultiplier = 2.0f;
        float cursorTemp = 1.0f;

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

                // Near start point
                if ((cellCenterPos - startPos).vector_length_sqr() <= lineThickness * lineThickness)
                {
                    if (addWind)
                    {
                        if ((cellCenterPos - startPos).vector_length_sqr() <= velocityThickness * velocityThickness)
                        {
                            u_prev[cellIndex] = deltaX * velocityMultiplier;
                            v_prev[cellIndex] = deltaY * velocityMultiplier;
                        }
                    }
                    if (addSmoke)
                    {
                        float distance = (cellCenterPos - startPos).vector_length();
                        float targetDensity = lineDensity;
                        if (distance > lineThickness - fadeRange)
                            targetDensity *= ((lineThickness - distance) / fadeRange);

                        if (dens[cellIndex] < targetDensity)
                            dens_prev[cellIndex] = targetDensity - dens[cellIndex];
                        if (temp[cellIndex] < cursorTemp)
                            temp_prev[cellIndex] = (cursorTemp - temp[cellIndex]) / (1.0f + movedCells);

                    }
                }
                // Near end point
                if ((cellCenterPos - endPos).vector_length_sqr() <= lineThickness * lineThickness)
                {
                    if (addWind)
                    {
                        if ((cellCenterPos - endPos).vector_length_sqr() <= velocityThickness * velocityThickness)
                        {
                            u_prev[cellIndex] = deltaX * velocityMultiplier;
                            v_prev[cellIndex] = deltaY * velocityMultiplier;
                        }
                    }
                    if (addSmoke)
                    {
                        float distance = (cellCenterPos - endPos).vector_length();
                        float targetDensity = lineDensity;
                        if (distance > lineThickness - fadeRange)
                            targetDensity *= ((lineThickness - distance) / fadeRange);

                        if (dens[cellIndex] < targetDensity)
                            dens_prev[cellIndex] = targetDensity - dens[cellIndex];
                        if (temp[cellIndex] < cursorTemp)
                            temp_prev[cellIndex] = (cursorTemp - temp[cellIndex]) / (1.0f + movedCells);
                    }
                }

                // Between start and end point                
                Pos2D<float> normalToEnd = (endPos - startPos).of_length(1.0f);
                Pos2D<float> normalToStart = -normalToEnd;
                if (dot_product(cellCenterPos - startPos, normalToEnd) >= 0.0f && dot_product(cellCenterPos - endPos, normalToStart) >= 0.0f)
                {
                    float distToLineCenter = std::fabsf(dot_product(cellCenterPos - startPos, normalToEnd.perpendicularL()));
                    if (distToLineCenter <= lineThickness)
                    {
                        if (addWind)
                        {
                            if (distToLineCenter <= velocityThickness)
                            {
                                u_prev[cellIndex] = deltaX * velocityMultiplier;
                                v_prev[cellIndex] = deltaY * velocityMultiplier;
                            }
                        }
                        if (addSmoke)
                        {
                            float targetDensity = lineDensity;
                            if (distToLineCenter > lineThickness - fadeRange)
                                targetDensity *= ((lineThickness - distToLineCenter) / fadeRange);

                            if (dens[cellIndex] < targetDensity)
                                dens_prev[cellIndex] = targetDensity - dens[cellIndex];
                            if (temp[cellIndex] < cursorTemp)
                                temp_prev[cellIndex] = (cursorTemp - temp[cellIndex]) / (1.0f + movedCells);
                        }
                    }
                }
            }
        }

        break;
    }

    prevMouseX = p.x;
    prevMouseY = p.y;

    //if (GetAsyncKeyState('Z') & 0x8000)
    //{
    //    if (!zClicked)
    //    {
    //        zClicked = true;
    //        float speed = 50000.0f;
    //        for (int i = 90; i <= 110; i++)
    //        {
    //            u_prev[_IndexAt(49, i)] = speed;
    //            u_prev[_IndexAt(50, i)] = speed;
    //            u_prev[_IndexAt(51, i)] = speed;
    //        }
    //    }
    //}
    //else if (zClicked)
    //    zClicked = false;

    if (!_paused)
    {
        float dt = 1.0f / 144.0f;
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

        //if (_addingSmoke || !slowdownPeriodEnded)
        //    dt /= 8.0f;

        for (int i = 0; i < u.size(); i++)
        {
            // Kill velocities and densities
            u[i] *= 0.9995f;
            v[i] *= 0.9995f;
            dens[i] -= 0.005f;
            temp[i] -= 0.001f;
            if (dens[i] < 0.0f)
                dens[i] = 0.0f;

            // Apply heat to velocity
            if (temp[i] > 0.0f)
                v_prev[i] -= temp[i];
        }

        _VelocityStep(_width, _height, u.data(), v.data(), u_prev.data(), v_prev.data(), 6.0f, dt);
        _DensityStep(_width, _height, dens.data(), dens_prev.data(), u.data(), v.data(), 2.6f, dt);
        _DensityStep(_width, _height, temp.data(), temp_prev.data(), u.data(), v.data(), 6.0f, dt);
        _UpdateParticles(dt);
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
        //if (!continueRunning)
            //_paused = true;

        //std::cout << densitySum << '\n';
    }

    for (int i = 0; i < 0; i++)
    {
        //_cells[50 * _width + 5].velocityX += 0.01f;
        //_cells[50 * _width + 5].newVelocityX += 0.01f;

        if (GetAsyncKeyState(VK_NUMPAD0) & 0x8000)
        {
            Cell& cell = _cells[50 * _width + 20];
            if (cell.velocityX <= 0.3f)
            {
                cell.velocityX += 0.4f;
                cell.newVelocityX += 0.4f;
            }
        }

        // Apply friction to all cells
        for (auto& cell : _cells)
        {
            if (cell.rightNeighbourIndex == -1 || cell.bottomNeighbourIndex == -1)
                continue;

            { // Horizontal friction
                Cell& neighbour = _cells[cell.bottomNeighbourIndex];
                float velocityDiff = cell.velocityX - neighbour.velocityX;
                float velocityChange = 0.0f;
                if (velocityDiff > 0.0f)
                    velocityChange = std::min(_velocityFrictionKoeff, velocityDiff / 2);
                else
                    velocityChange = std::max(-_velocityFrictionKoeff, velocityDiff / 2);
                cell.newVelocityX -= velocityChange;
                neighbour.newVelocityX += velocityChange;
            }
            { // Vertical friction
                Cell& neighbour = _cells[cell.rightNeighbourIndex];
                float velocityDiff = cell.velocityY - neighbour.velocityY;
                float velocityChange = 0.0f;
                if (velocityDiff > 0.0f)
                    velocityChange = std::min(_velocityFrictionKoeff, velocityDiff / 2);
                else
                    velocityChange = std::max(-_velocityFrictionKoeff, velocityDiff / 2);
                cell.newVelocityY -= velocityChange;
                neighbour.newVelocityY += velocityChange;
            }
        }

        // Update velocities for further processing
        for (auto& cell : _cells)
        {
            cell.velocityX = cell.newVelocityX;
            cell.velocityY = cell.newVelocityY;
        }

        // Move velocities along
        for (auto& cell : _cells)
        {
            // Horizontal movement
            if (cell.rightNeighbourIndex != -1)
            {
                Cell& neighbour = _cells[cell.rightNeighbourIndex];
                float velocityChange = cell.velocityX * std::fabs(cell.velocityX);
                if (velocityChange > 0.0f)
                {
                    float velocityPulled = velocityChange * _velocityPullRate;
                    float velocityPushed = velocityChange - velocityPulled;

                    { // Push velocity
                        float velocityDiffused = velocityPushed * _velocityDiffusion;
                        int neighboursAvailable = (neighbour.topNeighbourIndex != -1) + (neighbour.bottomNeighbourIndex != -1);
                        if (neighbour.rightNeighbourIndex == -1)
                            velocityDiffused = velocityPushed;
                        else if (neighboursAvailable == 0)
                            velocityDiffused = 0;
                        velocityPushed -= velocityDiffused;

                        if (neighboursAvailable > 0)
                        {
                            if (neighbour.topNeighbourIndex != -1)
                                _cells[neighbour.topNeighbourIndex].newVelocityY -= velocityDiffused / neighboursAvailable;
                            if (neighbour.bottomNeighbourIndex != -1)
                                neighbour.newVelocityY += velocityDiffused / neighboursAvailable;
                        }

                        neighbour.newVelocityX += velocityPushed;
                    }
                    { // Pull velocity
                        float velocityDiffused = velocityPulled * _velocityDiffusion;
                        int neighboursAvailable = (cell.topNeighbourIndex != -1) + (cell.bottomNeighbourIndex != -1);
                        if (cell.leftNeighbourIndex == -1)
                            velocityDiffused = velocityPulled;
                        else if (neighboursAvailable == 0)
                            velocityDiffused = 0;
                        velocityPulled -= velocityDiffused;

                        if (neighboursAvailable > 0)
                        {
                            if (cell.topNeighbourIndex != -1)
                                _cells[cell.topNeighbourIndex].newVelocityY += velocityDiffused / neighboursAvailable;
                            if (cell.bottomNeighbourIndex != -1)
                                cell.newVelocityY -= velocityDiffused / neighboursAvailable;
                        }

                        if (cell.leftNeighbourIndex != -1)
                            _cells[cell.leftNeighbourIndex].newVelocityX += velocityPulled;
                    }

                    cell.newVelocityX -= velocityChange;
                }
                else if (velocityChange < 0.0f)
                {
                    float velocityPulled = velocityChange * _velocityPullRate;
                    float velocityPushed = velocityChange - velocityPulled;

                    { // Push velocity
                        float velocityDiffused = velocityPushed * _velocityDiffusion;
                        int neighboursAvailable = (cell.topNeighbourIndex != -1) + (cell.bottomNeighbourIndex != -1);
                        if (cell.leftNeighbourIndex == -1)
                            velocityDiffused = velocityPushed;
                        else if (neighboursAvailable == 0)
                            velocityDiffused = 0;
                        velocityPushed -= velocityDiffused;

                        if (neighboursAvailable > 0)
                        {
                            if (cell.topNeighbourIndex != -1)
                                _cells[cell.topNeighbourIndex].newVelocityY -= velocityDiffused / neighboursAvailable;
                            if (cell.bottomNeighbourIndex != -1)
                                cell.newVelocityY += velocityDiffused / neighboursAvailable;
                        }

                        if (cell.leftNeighbourIndex != -1)
                            _cells[cell.leftNeighbourIndex].newVelocityX += velocityPushed;
                    }
                    { // Pull velocity
                        float velocityDiffused = velocityPulled * _velocityDiffusion;
                        int neighboursAvailable = (neighbour.topNeighbourIndex != -1) + (neighbour.bottomNeighbourIndex != -1);
                        if (neighbour.rightNeighbourIndex == -1)
                            velocityDiffused = velocityPulled;
                        else if (neighboursAvailable == 0)
                            velocityDiffused = 0;
                        velocityPulled -= velocityDiffused;

                        if (neighboursAvailable > 0)
                        {
                            if (neighbour.topNeighbourIndex != -1)
                                _cells[neighbour.topNeighbourIndex].newVelocityY += velocityDiffused / neighboursAvailable;
                            if (neighbour.bottomNeighbourIndex != -1)
                                neighbour.newVelocityY -= velocityDiffused / neighboursAvailable;
                        }

                        neighbour.newVelocityX += velocityPulled;
                    }

                    cell.newVelocityX -= velocityChange;
                }
            }

            // Vertical movement
            if (cell.bottomNeighbourIndex != -1)
            {
                Cell& neighbour = _cells[cell.bottomNeighbourIndex];
                float velocityChange = cell.velocityY * std::fabs(cell.velocityY);
                if (velocityChange > 0.0f)
                {
                    float velocityPulled = velocityChange * _velocityPullRate;
                    float velocityPushed = velocityChange - velocityPulled;

                    { // Push velocity
                        float velocityDiffused = velocityPushed * _velocityDiffusion;
                        int neighboursAvailable = (neighbour.leftNeighbourIndex != -1) + (neighbour.rightNeighbourIndex != -1);
                        if (neighbour.bottomNeighbourIndex == -1)
                            velocityDiffused = velocityPushed;
                        else if (neighboursAvailable == 0)
                            velocityDiffused = 0;
                        velocityPushed -= velocityDiffused;

                        if (neighboursAvailable > 0)
                        {
                            if (neighbour.leftNeighbourIndex != -1)
                                _cells[neighbour.leftNeighbourIndex].newVelocityX -= velocityDiffused / neighboursAvailable;
                            if (neighbour.rightNeighbourIndex != -1)
                                neighbour.newVelocityX += velocityDiffused / neighboursAvailable;
                        }

                        neighbour.newVelocityY += velocityPushed;
                    }
                    { // Pull velocity
                        float velocityDiffused = velocityPulled * _velocityDiffusion;
                        int neighboursAvailable = (cell.leftNeighbourIndex != -1) + (cell.rightNeighbourIndex != -1);
                        if (cell.topNeighbourIndex == -1)
                            velocityDiffused = velocityPulled;
                        else if (neighboursAvailable == 0)
                            velocityDiffused = 0;
                        velocityPulled -= velocityDiffused;

                        if (neighboursAvailable > 0)
                        {
                            if (cell.leftNeighbourIndex != -1)
                                _cells[cell.leftNeighbourIndex].newVelocityX += velocityDiffused / neighboursAvailable;
                            if (cell.rightNeighbourIndex != -1)
                                cell.newVelocityX -= velocityDiffused / neighboursAvailable;
                        }

                        if (cell.topNeighbourIndex != -1)
                            _cells[cell.topNeighbourIndex].newVelocityY += velocityPulled;
                    }

                    cell.newVelocityY -= velocityChange;
                }
                else if (velocityChange < 0.0f)
                {
                    float velocityPulled = velocityChange * _velocityPullRate;
                    float velocityPushed = velocityChange - velocityPulled;

                    { // Push velocity
                        float velocityDiffused = velocityPushed * _velocityDiffusion;
                        int neighboursAvailable = (cell.leftNeighbourIndex != -1) + (cell.rightNeighbourIndex != -1);
                        if (cell.topNeighbourIndex == -1)
                            velocityDiffused = velocityPushed;
                        else if (neighboursAvailable == 0)
                            velocityDiffused = 0;
                        velocityPushed -= velocityDiffused;

                        if (neighboursAvailable > 0)
                        {
                            if (cell.leftNeighbourIndex != -1)
                                _cells[cell.leftNeighbourIndex].newVelocityX -= velocityDiffused / neighboursAvailable;
                            if (cell.rightNeighbourIndex != -1)
                                cell.newVelocityX += velocityDiffused / neighboursAvailable;
                        }

                        if (cell.topNeighbourIndex != -1)
                            _cells[cell.topNeighbourIndex].newVelocityY += velocityPushed;
                    }
                    { // Pull velocity
                        float velocityDiffused = velocityPulled * _velocityDiffusion;
                        int neighboursAvailable = (neighbour.leftNeighbourIndex != -1) + (neighbour.rightNeighbourIndex != -1);
                        if (neighbour.bottomNeighbourIndex == -1)
                            velocityDiffused = velocityPulled;
                        else if (neighboursAvailable == 0)
                            velocityDiffused = 0;
                        velocityPulled -= velocityDiffused;

                        if (neighboursAvailable > 0)
                        {
                            if (neighbour.leftNeighbourIndex != -1)
                                _cells[neighbour.leftNeighbourIndex].newVelocityX += velocityDiffused / neighboursAvailable;
                            if (neighbour.rightNeighbourIndex != -1)
                                neighbour.newVelocityX -= velocityDiffused / neighboursAvailable;
                        }

                        neighbour.newVelocityY += velocityPulled;
                    }

                    cell.newVelocityY -= velocityChange;
                }
            }
        }

        // Update velocities for display
        for (auto& cell : _cells)
        {
            cell.velocityX = cell.newVelocityX;
            cell.velocityY = cell.newVelocityY;
        }
    }
    for (int i = 0; i < 0; i++)
    {
        _cells[50 * _width].pressure += 0.1f;
        _cells[50 * _width].velocityX += 10.0f;
        _cells[50 * _width].newVelocityX += 10.0f;

        for (auto& cell : _cells)
        {
            int x = cell.index % _width;
            int y = cell.index / _width;

            for (int i = 0; i < 2; i++)
            {
                int nx = x; // Neighbour x
                int ny = y; // Neighbour y

                if (i == 0)
                    if (x + 1 < _width)
                        nx = x + 1;
                    else
                        continue;
                else if (i == 1)
                    if (y + 1 < _height)
                        ny = y + 1;
                    else
                        continue;
                Cell& neighbor = _cells[ny * _width + nx];

                float dPress = cell.pressure - neighbor.pressure;
                float flow = cell.flow * dPress;
                //flow = _Clamp(flow, -neighbor.pressure / 4.0f, cell.pressure / 4.0f);
                if (nx > x)
                    cell.newVelocityX += flow;
                else if (ny > y)
                    cell.newVelocityY += flow;
                //cell.newPressure -= flow;
                //neighbor.newPressure += flow;
            }
        }
        for (auto& cell : _cells)
        {
            int x = cell.index % _width;
            int y = cell.index / _width;

            //if (cell.newVelocityX > 0.01f)
                cell.newVelocityX *= 0.995f;
            //if (cell.newVelocityY > 0.01f)
                cell.newVelocityY *= 0.995f;
            cell.velocityX = cell.newVelocityX;
            cell.velocityY = cell.newVelocityY;

            if (x + 1 < _width)
            {
                Cell& neighbour = _cells[y * _width + x + 1];
                float pressureMoved = _Clamp(cell.velocityX, -neighbour.pressure / 4.0f, cell.pressure / 4.0f);
                cell.pressure -= pressureMoved;
                neighbour.pressure += pressureMoved;
            }
            if (y + 1 < _height)
            {
                Cell& neighbour = _cells[(y + 1) * _width + x];
                float pressureMoved = _Clamp(cell.velocityY, -neighbour.pressure / 4.0f, cell.pressure / 4.0f);
                cell.pressure -= pressureMoved;
                neighbour.pressure += pressureMoved;
            }
        }
    }

    //std::cout << timer.MicrosElapsed() << '\n';
}

void zcom::SmokeSimScene::_Resize(int width, int height, ResizeInfo info)
{

}