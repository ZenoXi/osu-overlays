#include "App.h"
#include "SharedContext.h"
#include "Window/Window.h"
#include "CursorTrailOverlayComponent.h"
#include "CursorTrailEffect.h"

#include "Shared/Components/OverlayLayoutSetup.h"
#include "Shared/Util/Functions.h"

void zcom::CursorTrailOverlayComponent::Init(std::shared_ptr<const Overlay> overlay)
{
    Panel::Init();

    _overlay = overlay;
    _dataProviderView = std::make_unique<DataProviderView>(this);

    _configValueChangedEventSubscription = _scene->GetApp()->config.SubscribeOnConfigValueChanged();
    _configValueChangedEventSubscription->ResetSynchronousHandler([=](std::optional<std::pair<std::wstring, std::wstring>> changes) {
        ExecuteSynchronously([=]() {
            _UpdateParameters();
            ApplyLayoutStringToComponent(_scene->GetApp()->config.GetConfigValue(CursorTrailConfig::LAYOUT_STRING), this);
        });
    });
    _UpdateParameters();
    ApplyLayoutStringToComponent(_scene->GetApp()->config.GetConfigValue(CursorTrailConfig::LAYOUT_STRING), this);

    _headIconBitmap = _scene->GetWindow()->resourceManager.GetImage("trail_head_icon");
}

void zcom::CursorTrailOverlayComponent::_UpdateParameters()
{
    _headColor = Color::ARGB(_scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::HEAD_COLOR, Config::ADD_AND_SAVE_IF_MISSING));
    _trailWidth = _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::TRAIL_WIDTH, Config::ADD_AND_SAVE_IF_MISSING);
    _headSize = _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::HEAD_SIZE, Config::ADD_AND_SAVE_IF_MISSING);
    _trailEdgeWidth = _scene->GetApp()->config.GetDoubleConfigValue(CursorTrailConfig::TRAIL_EDGE_WIDTH, Config::ADD_AND_SAVE_IF_MISSING);
    _headEdgeWidth = _scene->GetApp()->config.GetDoubleConfigValue(CursorTrailConfig::HEAD_EDGE_WIDTH, Config::ADD_AND_SAVE_IF_MISSING);
    _trailDuration = Duration(_scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::TRAIL_LIFETIME, Config::ADD_AND_SAVE_IF_MISSING), MILLISECONDS);
    _colorCycleDuration = Duration(_scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::COLOR_CYCLE_DURATION, Config::ADD_AND_SAVE_IF_MISSING), MILLISECONDS);
    _iconSpinDuration = Duration(_scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::ICON_SPIN_DURATION, Config::ADD_AND_SAVE_IF_MISSING), MILLISECONDS);
    _trailResolution = _scene->GetApp()->config.GetIntConfigValue(CursorTrailConfig::TRAIL_RESOLUTION, Config::ADD_AND_SAVE_IF_MISSING);

    _ParsePaletteString(_scene->GetApp()->config.GetConfigValue(CursorTrailConfig::PALETTE, Config::ADD_AND_SAVE_IF_MISSING));
    if (_palette.empty())
    {
        _ParsePaletteString(CursorTrailConfig::PALETTE.defaultValue);
        _scene->GetApp()->config.SetValue(CursorTrailConfig::PALETTE.name, CursorTrailConfig::PALETTE.defaultValue);
    }
}

void zcom::CursorTrailOverlayComponent::_ParsePaletteString(const std::wstring& str)
{
    _palette.clear();
    std::vector<std::wstring> stopStrings;
    split_wstr(str, stopStrings, L'|', true);
    for (auto& stopString : stopStrings)
    {
        std::array<std::wstring, 2> parts;
        split_wstr(stopString, parts, L',');
        _palette.push_back(std::move(GradientStop{ Color::ARGB(std::stoi(parts[0])), std::stof(parts[1]) }));
    }
    std::sort(_palette.begin(), _palette.end(), [](const GradientStop& stop1, const GradientStop& stop2) { return stop1.position < stop2.position; });
}

void zcom::CursorTrailOverlayComponent::_OnUpdate()
{
    //static int counter = 0;

    //std::unique_lock<std::mutex> lock(_m_input);
    //if (counter % 100 == 0)
    //    GetCursorPos(&_currentMousePos);
    //counter += _pendingInput.size();
    //for (auto& point : _pendingInput)
    //    _points.push_back(point);
    //_pendingInput.clear();
    //_inputClock.Update();
    //TimePoint inputClockTime = _inputClock.Now();
    //lock.unlock();

    //std::cout << counter << '\n';

    POINT p;
    GetCursorPos(&p);

    RECT windowRect = _scene->GetWindow()->Backend().GetWindowRectangle();

    Pos2D<float> newPoint = Pos2D{ float(p.x - windowRect.left), float(p.y - windowRect.top) };
    if (!_points.empty())
    {
        Pos2D<float> previousPoint = _points.back().position;
        Pos2D<float> movedVec = newPoint - previousPoint;
        Pos2D<float> previousBezierAnchorPos = _points.back().bezierAnchorPosition;
        float movedDist = movedVec.vector_length();
        float previousMovedDist = _previousMove.vector_length();
        Pos2D<float> bezierAnchorPos = previousPoint;
        if (movedDist > 0.0f)
        {
            Pos2D<float> movedVecNormalized = movedVec.of_length(1.0f);
            float distRatio = previousMovedDist / movedDist;
            float overshootRatio = distRatio / (1 + distRatio);
            if (!(previousPoint == previousBezierAnchorPos))
            {
                Pos2D<float> overshootVector = (previousPoint - previousBezierAnchorPos).of_length(overshootRatio * std::min(movedDist / 2, previousMovedDist));
                bezierAnchorPos += overshootVector;
                if (dot_product(movedVecNormalized.perpendicularR(), overshootVector) > 0)
                {
                    Pos2D<float> rightBoundNormal = (movedVecNormalized.perpendicularR() + movedVecNormalized).of_length(1.0f);
                    float boundOvershoot = dot_product(rightBoundNormal, bezierAnchorPos - newPoint);
                    if (boundOvershoot > 0.0f)
                        bezierAnchorPos += rightBoundNormal * -boundOvershoot;
                }
                else
                {
                    Pos2D<float> leftBoundNormal = (movedVecNormalized.perpendicularL() + movedVecNormalized).of_length(1.0f);
                    float boundOvershoot = dot_product(leftBoundNormal, bezierAnchorPos - newPoint);
                    if (boundOvershoot > 0.0f)
                        bezierAnchorPos += leftBoundNormal * -boundOvershoot;
                }
            }
        }
        _points.push_back({ newPoint, ztime::Main(), bezierAnchorPos });
        _previousMove = newPoint - previousPoint;
    }
    else
    {
        _points.push_back({ newPoint, ztime::Main(), newPoint });
    }

    while (!_points.empty() && _points.front().time + _trailDuration <= ztime::Main())
        _points.erase(_points.begin());
    InvokeRedraw();
}

void zcom::CursorTrailOverlayComponent::_OnDraw(Graphics* g)
{
    PointF targetTopLeft = g->AdjustPointToTargetSpace(PointF{ 0.0f, 0.0f });

    auto props = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET, { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED });
    ComPtr<ID2D1Bitmap1> bitmap = nullptr;
    g->GetRenderContext()->CreateBitmap(D2D1::SizeU((UINT32)size_->width, (UINT32)size_->height), nullptr, 0, props, bitmap.GetAddressOf());

    ID2D1Effect* trailEffect;
    HRESULT hr = g->GetRenderContext()->CreateEffect(CLSID_CursorTrailEffect, &trailEffect);
    if (trailEffect)
    {
        trailEffect->SetInput(0, bitmap.Get());
        D2D1_VECTOR_4F headColor = { _headColor.r / 255.0f, _headColor.g / 255.0f, _headColor.b / 255.0f, _headColor.a / 255.0f };
        headColor.x *= headColor.w;
        headColor.y *= headColor.w;
        headColor.z *= headColor.w;
        std::array<D2D1_VECTOR_4F, CursorTrailEffect::MAX_POINT_COUNT> points{};
        std::array<D2D1_VECTOR_4F, CursorTrailEffect::MAX_POINT_COUNT> pointData{};
        for (int i = 0; i < _points.size() && i < CursorTrailEffect::MAX_POINT_COUNT; i++)
        {
            points[i].x = _points[i].position.x;
            points[i].y = _points[i].position.y;
            points[i].z = _points[i].bezierAnchorPosition.x;
            points[i].w = _points[i].bezierAnchorPosition.y;
            pointData[i].x = _points[i].time.GetTime(MICROSECONDS) / 1'000'000.0f;
        }
        std::array<D2D1_VECTOR_4F, CursorTrailEffect::MAX_STOP_COUNT> stops{};
        std::array<D2D1_VECTOR_4F, CursorTrailEffect::MAX_STOP_COUNT> stopData{};
        for (int i = 0; i < _palette.size() && i < CursorTrailEffect::MAX_STOP_COUNT; i++)
        {
            stops[i].x = (_palette[i].color.r / 255.0f) * (_palette[i].color.a / 255.0f);
            stops[i].y = (_palette[i].color.g / 255.0f) * (_palette[i].color.a / 255.0f);
            stops[i].z = (_palette[i].color.b / 255.0f) * (_palette[i].color.a / 255.0f);
            stops[i].w = _palette[i].color.a / 255.0f;
            stopData[i].x = _palette[i].position;
        }
        trailEffect->SetValue(CURSOR_TRAIL_PROP_HEAD_COLOR, headColor);
        trailEffect->SetValue(CURSOR_TRAIL_PROP_TRAIL_WIDTH, (FLOAT)_trailWidth);
        trailEffect->SetValue(CURSOR_TRAIL_PROP_HEAD_SIZE, (FLOAT)_headSize);
        trailEffect->SetValue(CURSOR_TRAIL_PROP_TRAIL_EDGE_WIDTH, _trailEdgeWidth);
        trailEffect->SetValue(CURSOR_TRAIL_PROP_HEAD_EDGE_WIDTH, _headEdgeWidth);
        trailEffect->SetValue(CURSOR_TRAIL_PROP_POINT_COUNT, (INT)std::min(_points.size(), CursorTrailEffect::MAX_POINT_COUNT));
        trailEffect->SetValue(CURSOR_TRAIL_PROP_STOP_COUNT, (INT)std::min(_palette.size(), CursorTrailEffect::MAX_STOP_COUNT));
        trailEffect->SetValue(CURSOR_TRAIL_PROP_COLOR_CYCLE_DURATION, _colorCycleDuration.GetDuration(MICROSECONDS) / 1'000'000.0f);
        trailEffect->SetValue(CURSOR_TRAIL_PROP_TRAIL_RESOLUTION, _trailResolution);
        trailEffect->SetValue(CURSOR_TRAIL_PROP_POINTS, points);
        trailEffect->SetValue(CURSOR_TRAIL_PROP_POINT_DATA, pointData);
        trailEffect->SetValue(CURSOR_TRAIL_PROP_STOPS, stops);
        trailEffect->SetValue(CURSOR_TRAIL_PROP_STOP_DATA, stopData);
        g->GetRenderContext()->DrawImage(trailEffect, D2D1::Point2F(targetTopLeft.x, targetTopLeft.y), D2D1_INTERPOLATION_MODE_CUBIC);
        trailEffect->Release();
    }

    if (!_points.empty() && _headIconBitmap)
    {
        ID2D1Effect* rotationEffect = nullptr;
        g->GetRenderContext()->CreateEffect(CLSID_D2D12DAffineTransform, &rotationEffect);
        if (rotationEffect)
        {
            float rotation = std::fmodf(ztime::Main().GetTime(MICROSECONDS) / (float)_iconSpinDuration.GetDuration(MICROSECONDS), 1.0f) * 360.0f;
            D2D1_SIZE_F bitmapSize = D2D1::SizeF((float)_headIconBitmap->GetSize().width, (float)_headIconBitmap->GetSize().height);
            float xScale = _headSize / bitmapSize.width;
            float yScale = _headSize / bitmapSize.height;

            rotationEffect->SetInput(0, _headIconBitmap->GetSource());
            rotationEffect->SetValue(D2D1_2DAFFINETRANSFORM_PROP_TRANSFORM_MATRIX,
                D2D1::Matrix3x2F::Rotation(rotation, D2D1::Point2F(bitmapSize.width / 2.0f, bitmapSize.height / 2.0f)) *
                D2D1::Matrix3x2F::Scale(D2D1::SizeF(xScale, yScale), D2D1::Point2F(bitmapSize.width / 2.0f, bitmapSize.height / 2.0f))
            );
            rotationEffect->SetValue(D2D1_2DAFFINETRANSFORM_PROP_INTERPOLATION_MODE, D2D1_2DAFFINETRANSFORM_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);
            g->GetRenderContext()->DrawImage(
                rotationEffect,
                D2D1::Point2F(
                    targetTopLeft.x + _points.back().position.x - bitmapSize.width / 2.0f,
                    targetTopLeft.y + _points.back().position.y - bitmapSize.height / 2.0f
                ),
                D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC
            );
            rotationEffect->Release();
        }
    }
}

D2D1_COLOR_F zcom::CursorTrailOverlayComponent::_GetColorByTime(TimePoint time)
{
    if (_palette.empty())
    {
        // Blink black/red to signify error
        return D2D1::ColorF(time.GetTime(MILLISECONDS) % 1000 < 500 ? 1.0f : 0.0f, 0.0f, 0.0f);
    }

    float pos = (float)std::fmod(time.GetTime() / (double)_colorCycleDuration.GetDuration(), 1.0);
    //std::cout << pos << ' ';
    if (pos <= _palette.front().position)
    {
        //std::cout << "front\n";
        Color color = _palette.front().color;
        return D2D1::ColorF(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
    }
    else if (pos >= _palette.back().position)
    {
        //std::cout << "back\n";
        Color color = _palette.back().color;
        return D2D1::ColorF(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
    }
    else
    {
        size_t index = 1;
        while (pos > _palette[index].position && index < _palette.size() - 1)
            index++;

        Color color1 = _palette[index - 1].color;
        Color color2 = _palette[index].color;
        D2D1_COLOR_F colorf1 = D2D1::ColorF(color1.r / 255.0f, color1.g / 255.0f, color1.b / 255.0f, color1.a / 255.0f);
        D2D1_COLOR_F colorf2 = D2D1::ColorF(color2.r / 255.0f, color2.g / 255.0f, color2.b / 255.0f, color2.a / 255.0f);
        float x = (pos - _palette[index - 1].position) / (_palette[index].position - _palette[index - 1].position);
        //std::cout << index << ' ' << x << '\n';
        return D2D1::ColorF(
            colorf1.r + (colorf2.r - colorf1.r) * x,
            colorf1.g + (colorf2.g - colorf1.g) * x,
            colorf1.b + (colorf2.b - colorf1.b) * x,
            colorf1.a + (colorf2.a - colorf1.a) * x
        );
    }
}
