#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "Dwrite.h"
#include "d2d1.h"

#include "Helper/ValueProxy.h"
#include "Helper/Time.h"

#include <sstream>
#include <iomanip>

namespace zcom
{
    struct Point;
    struct PointF;
    struct Size;
    struct SizeF;
    struct Rect;
    struct RectF;

    struct X { int x; };
    struct Y { int y; };
    struct Point
    {
        int x;
        int y;

        PointF ToPointF() const;

        Point operator+(const Point& other) const { return { x + other.x, y + other.y }; }
        Point operator-(const Point& other) const { return { x - other.x, y - other.y }; }
        bool operator==(const Point& other) const { return x == other.x && y == other.y; }
        bool operator!=(const Point& other) const { return !(*this == other); }

        Point& operator=(X value)
        {
            x = value.x;
            return *this;
        }
        Point& operator=(Y value)
        {
            y = value.y;
            return *this;
        }

        static ValueProxy XValueProxy(std::string name, std::any valuePtr, int minValue = std::numeric_limits<int>::min(), int maxValue = std::numeric_limits<int>::max(), int stepSize = 1)
        {
            return _ValueProxy(0, name, valuePtr, minValue, maxValue, stepSize);
        }
        static ValueProxy YValueProxy(std::string name, std::any valuePtr, int minValue = std::numeric_limits<int>::min(), int maxValue = std::numeric_limits<int>::max(), int stepSize = 1)
        {
            return _ValueProxy(1, name, valuePtr, minValue, maxValue, stepSize);
        }
    private:
        static ValueProxy _ValueProxy(int field, std::string name, std::any valuePtr, int minValue, int maxValue, int stepSize)
        {
            ValueProxy proxy;
            proxy.name = name;
            proxy.valuePtr = valuePtr;
            proxy.type = ValueProxy::NUMBER;
            proxy.optional = false;
            proxy.numberMinValue = ValueProxy::Number(minValue);
            proxy.numberMaxValue = ValueProxy::Number(maxValue);
            proxy.numberStepSize = ValueProxy::Number(stepSize);
            proxy.numberMapper = ValueProxy::Mapper<ValueProxy::Number>{
                .fromValue = [field](const std::any& valuePtr) {
                    auto ptr = std::any_cast<Value<Point>*>(valuePtr);
                    return std::optional<ValueProxy::Number>(ValueProxy::Number(field == 0 ? ptr->Get().x : ptr->Get().y));
                },
                .toValue = [field](const std::optional<ValueProxy::Number>& number, std::any& valuePtr) {
                    auto ptr = std::any_cast<Value<Point>*>(valuePtr);
                    if (field == 0)
                        ptr->Assign(X((int)number->getAsInteger()));
                    else
                        ptr->Assign(Y((int)number->getAsInteger()));
                    return true;
                }
            };
            return proxy;
        }
    };

    struct XF { float x; };
    struct YF { float y; };
    struct PointF
    {
        float x;
        float y;

        PointF operator+(const PointF& other) const { return { x + other.x, y + other.y }; }
        PointF operator-(const PointF& other) const { return { x - other.x, y - other.y }; }
        bool operator==(const PointF& other) const { return x == other.x && y == other.y; }
        bool operator!=(const PointF& other) const { return !(*this == other); }

        PointF& operator=(XF value)
        {
            x = value.x;
            return *this;
        }
        PointF& operator=(YF value)
        {
            y = value.y;
            return *this;
        }

        static ValueProxy XValueProxy(std::string name, std::any valuePtr, int precision, float minValue = (float)std::numeric_limits<int>::min(), float maxValue = (float)std::numeric_limits<int>::max(), float stepSize = 1)
        {
            return _ValueProxy(0, name, valuePtr, precision, minValue, maxValue, stepSize);
        }
        static ValueProxy YValueProxy(std::string name, std::any valuePtr, int precision, float minValue = (float)std::numeric_limits<int>::min(), float maxValue = (float)std::numeric_limits<int>::max(), float stepSize = 1)
        {
            return _ValueProxy(1, name, valuePtr, precision, minValue, maxValue, stepSize);
        }
    private:
        static ValueProxy _ValueProxy(int field, std::string name, std::any valuePtr, int precision, float minValue, float maxValue, float stepSize)
        {
            ValueProxy proxy;
            proxy.name = name;
            proxy.valuePtr = valuePtr;
            proxy.type = ValueProxy::NUMBER;
            proxy.optional = false;
            proxy.numberPrecision = precision;
            proxy.numberMinValue = ValueProxy::Number(minValue);
            proxy.numberMaxValue = ValueProxy::Number(maxValue);
            proxy.numberStepSize = ValueProxy::Number(stepSize);
            proxy.numberMapper = ValueProxy::Mapper<ValueProxy::Number>{
                .fromValue = [field](const std::any& valuePtr) {
                    auto ptr = std::any_cast<Value<PointF>*>(valuePtr);
                    return std::optional<ValueProxy::Number>(ValueProxy::Number(field == 0 ? ptr->Get().x : ptr->Get().y));
                },
                .toValue = [field](const std::optional<ValueProxy::Number>& number, std::any& valuePtr) {
                    auto ptr = std::any_cast<Value<PointF>*>(valuePtr);
                    if (field == 0)
                        ptr->Assign(XF((float)number->getAsDouble()));
                    else
                        ptr->Assign(YF((float)number->getAsDouble()));
                    return true;
                }
            };
            return proxy;
        }
    };

    struct Width { int width; };
    struct Height { int height; };
    struct Size
    {
        int width;
        int height;

        SizeF ToSizeF() const;
        Rect ToRect(Point offset = {0, 0}) const;

        Size operator+(const Size& other) const { return { width + other.width, height + other.height }; }
        Size operator-(const Size& other) const { return { width - other.width, height - other.height }; }
        bool operator==(const Size& other) const { return width == other.width && height == other.height; }
        bool operator!=(const Size& other) const { return !(*this == other); }
        
        Size& operator=(Width value)
        {
            width = value.width;
            return *this;
        }
        Size& operator=(Height value)
        {
            height = value.height;
            return *this;
        }

        static ValueProxy WidthValueProxy(std::string name, std::any valuePtr, int minValue = std::numeric_limits<int>::min(), int maxValue = std::numeric_limits<int>::max(), int stepSize = 1)
        {
            return _ValueProxy(0, name, valuePtr, minValue, maxValue, stepSize);
        }
        static ValueProxy HeightValueProxy(std::string name, std::any valuePtr, int minValue = std::numeric_limits<int>::min(), int maxValue = std::numeric_limits<int>::max(), int stepSize = 1)
        {
            return _ValueProxy(1, name, valuePtr, minValue, maxValue, stepSize);
        }
    private:
        static ValueProxy _ValueProxy(int field, std::string name, std::any valuePtr, int minValue, int maxValue, int stepSize)
        {
            ValueProxy proxy;
            proxy.name = name;
            proxy.valuePtr = valuePtr;
            proxy.type = ValueProxy::NUMBER;
            proxy.optional = false;
            proxy.numberMinValue = ValueProxy::Number(minValue);
            proxy.numberMaxValue = ValueProxy::Number(maxValue);
            proxy.numberStepSize = ValueProxy::Number(stepSize);
            proxy.numberMapper = ValueProxy::Mapper<ValueProxy::Number>{
                .fromValue = [field](const std::any& valuePtr) {
                    auto ptr = std::any_cast<Value<Size>*>(valuePtr);
                    return std::optional<ValueProxy::Number>(ValueProxy::Number(field == 0 ? ptr->Get().width : ptr->Get().height));
                },
                .toValue = [field](const std::optional<ValueProxy::Number>& number, std::any& valuePtr) {
                    auto ptr = std::any_cast<Value<Size>*>(valuePtr);
                    if (field == 0)
                        ptr->Assign(Width((int)number->getAsInteger()));
                    else
                        ptr->Assign(Height((int)number->getAsInteger()));
                    return true;
                }
            };
            return proxy;
        }
    };

    struct WidthF { float width; };
    struct HeightF { float height; };
    struct SizeF
    {
        float width;
        float height;

        RectF ToRectF(PointF offset = { 0.0f, 0.0f }) const;

        SizeF operator+(const SizeF& other) const { return { width + other.width, height + other.height }; }
        SizeF operator-(const SizeF& other) const { return { width - other.width, height - other.height }; }
        bool operator==(const SizeF& other) const { return width == other.width && height == other.height; }
        bool operator!=(const SizeF& other) const { return !(*this == other); }

        SizeF& operator=(WidthF value)
        {
            width = value.width;
            return *this;
        }
        SizeF& operator=(HeightF value)
        {
            height = value.height;
            return *this;
        }

        static ValueProxy WidthValueProxy(std::string name, std::any valuePtr, int precision, float minValue = (float)std::numeric_limits<int>::min(), float maxValue = (float)std::numeric_limits<int>::max(), float stepSize = 1)
        {
            return _ValueProxy(0, name, valuePtr, precision, minValue, maxValue, stepSize);
        }
        static ValueProxy HeightValueProxy(std::string name, std::any valuePtr, int precision, float minValue = (float)std::numeric_limits<int>::min(), float maxValue = (float)std::numeric_limits<int>::max(), float stepSize = 1)
        {
            return _ValueProxy(1, name, valuePtr, precision, minValue, maxValue, stepSize);
        }
    private:
        static ValueProxy _ValueProxy(int field, std::string name, std::any valuePtr, int precision, float minValue, float maxValue, float stepSize)
        {
            ValueProxy proxy;
            proxy.name = name;
            proxy.valuePtr = valuePtr;
            proxy.type = ValueProxy::NUMBER;
            proxy.optional = false;
            proxy.numberPrecision = precision;
            proxy.numberMinValue = ValueProxy::Number(minValue);
            proxy.numberMaxValue = ValueProxy::Number(maxValue);
            proxy.numberStepSize = ValueProxy::Number(stepSize);
            proxy.numberMapper = ValueProxy::Mapper<ValueProxy::Number>{
                .fromValue = [field](const std::any& valuePtr) {
                    auto ptr = std::any_cast<Value<SizeF>*>(valuePtr);
                    return std::optional<ValueProxy::Number>(ValueProxy::Number(field == 0 ? ptr->Get().width : ptr->Get().height));
                },
                .toValue = [field](const std::optional<ValueProxy::Number>& number, std::any& valuePtr) {
                    auto ptr = std::any_cast<Value<SizeF>*>(valuePtr);
                    if (field == 0)
                        ptr->Assign(WidthF((float)number->getAsDouble()));
                    else
                        ptr->Assign(HeightF((float)number->getAsDouble()));
                    return true;
                }
            };
            return proxy;
        }
    };

    struct Left { int left; };
    struct Top { int top; };
    struct Right { int right; };
    struct Bottom { int bottom; };
    struct Rect
    {
        int left;
        int top;
        int right;
        int bottom;

        int GetWidth() const { return right - left; }
        int GetHeight() const { return bottom - top; }
        Point GetTopLeftPoint() const { return Point{ left, top }; }
        Point GetTopRightPoint() const { return Point{ right, top }; }
        Point GetBottomLeftPoint() const { return Point{ left, bottom }; }
        Point GetBottomRightPoint() const { return Point{ right, bottom }; }

        Rect ExpandedBy(int left, int top, int right, int bottom) { return Rect{ this->left - left, this->top - top, this->right + right, this->bottom + bottom }; }
        Rect ExpandedBy(int x, int y) { return ShrunkBy(x, y, x, y); }
        Rect ExpandedBy(int amount) { return ShrunkBy(amount, amount, amount, amount); }
        Rect ShrunkBy(int left, int top, int right, int bottom) { return Rect{ this->left + left, this->top + top, this->right - right, this->bottom - bottom }; }
        Rect ShrunkBy(int x, int y) { return ShrunkBy(x, y, x, y); }
        Rect ShrunkBy(int amount) { return ShrunkBy(amount, amount, amount, amount); }

        // Returns a subrect described by the 'inner' parameter. 0,0 in the 'inner' rect maps to the top left corner of this rect.
        // Example:
        // - this: [100,100,200,200]
        // - inner: [40,50,80,80]
        // - result: [140,150,180,180]
        Rect Subrect(Rect inner) const
        {
            Rect sub;
            sub.left = left + inner.left;
            sub.top = top + inner.top;
            sub.right = left + inner.right;
            sub.bottom = top + inner.bottom;
            return sub;
        }

        // Same as Rect::Subrect, but the resulting rect is clipped to the parent rect
        Rect SubrectBounded(Rect inner) const
        {
            return Subrect(inner).BoundedBy(*this);
        }

        // Returns this rect clipped to the provided rect
        // Example:
        // - this: [100,100,200,200]
        // - provided: [50,50,150,150]
        // - result: [100,100,150,150]
        Rect BoundedBy(Rect rect) const
        {
            Rect sub = *this;
            sub.left = sub.left < rect.left ? rect.left : sub.left;
            sub.left = sub.left > rect.right ? rect.right : sub.left;
            sub.top = sub.top < rect.top ? rect.top : sub.top;
            sub.top = sub.top > rect.bottom ? rect.bottom : sub.top;
            sub.right = sub.right < rect.left ? rect.left : sub.right;
            sub.right = sub.right > rect.right ? rect.right : sub.right;
            sub.bottom = sub.bottom < rect.top ? rect.top : sub.bottom;
            sub.bottom = sub.bottom > rect.bottom ? rect.bottom : sub.bottom;
            return sub;
        }

        RectF ToRectF() const;

        bool operator!=(const Rect& other) const
        {
            return
                left != other.left ||
                top != other.top ||
                right != other.right ||
                bottom != other.bottom;
        }

        bool operator==(const Rect& other) const
        {
            return
                left == other.left &&
                top == other.top &&
                right == other.right &&
                bottom == other.bottom;
        }

        Rect& operator=(Left value)
        {
            left = value.left;
            return *this;
        }
        Rect& operator=(Top value)
        {
            top = value.top;
            return *this;
        }
        Rect& operator=(Right value)
        {
            right = value.right;
            return *this;
        }
        Rect& operator=(Bottom value)
        {
            bottom = value.bottom;
            return *this;
        }

        static ValueProxy LeftValueProxy(std::string name, std::any valuePtr, int minValue = std::numeric_limits<int>::min(), int maxValue = std::numeric_limits<int>::max(), int stepSize = 1)
        {
            return _ValueProxy(0, name, valuePtr, minValue, maxValue, stepSize);
        }
        static ValueProxy TopValueProxy(std::string name, std::any valuePtr, int minValue = std::numeric_limits<int>::min(), int maxValue = std::numeric_limits<int>::max(), int stepSize = 1)
        {
            return _ValueProxy(1, name, valuePtr, minValue, maxValue, stepSize);
        }
        static ValueProxy RightValueProxy(std::string name, std::any valuePtr, int minValue = std::numeric_limits<int>::min(), int maxValue = std::numeric_limits<int>::max(), int stepSize = 1)
        {
            return _ValueProxy(2, name, valuePtr, minValue, maxValue, stepSize);
        }
        static ValueProxy BottomValueProxy(std::string name, std::any valuePtr, int minValue = std::numeric_limits<int>::min(), int maxValue = std::numeric_limits<int>::max(), int stepSize = 1)
        {
            return _ValueProxy(3, name, valuePtr, minValue, maxValue, stepSize);
        }
    private:
        static ValueProxy _ValueProxy(int field, std::string name, std::any valuePtr, int minValue, int maxValue, int stepSize)
        {
            ValueProxy proxy;
            proxy.name = name;
            proxy.valuePtr = valuePtr;
            proxy.type = ValueProxy::NUMBER;
            proxy.optional = false;
            proxy.numberMinValue = ValueProxy::Number(minValue);
            proxy.numberMaxValue = ValueProxy::Number(maxValue);
            proxy.numberStepSize = ValueProxy::Number(stepSize);
            proxy.numberMapper = ValueProxy::Mapper<ValueProxy::Number>{
                .fromValue = [field](const std::any& valuePtr) {
                    auto ptr = std::any_cast<Value<Rect>*>(valuePtr);
                    if (field == 0)
                        return std::optional<ValueProxy::Number>(ValueProxy::Number(ptr->Get().left));
                    else if (field == 1)
                        return std::optional<ValueProxy::Number>(ValueProxy::Number(ptr->Get().top));
                    else if (field == 2)
                        return std::optional<ValueProxy::Number>(ValueProxy::Number(ptr->Get().right));
                    else if (field == 3)
                        return std::optional<ValueProxy::Number>(ValueProxy::Number(ptr->Get().bottom));
                    else
                        return std::optional<ValueProxy::Number>();
                },
                .toValue = [field](const std::optional<ValueProxy::Number>& number, std::any& valuePtr) {
                    auto ptr = std::any_cast<Value<Rect>*>(valuePtr);
                    if (field == 0)
                        ptr->Assign(Left((int)number->getAsInteger()));
                    else if (field == 1)
                        ptr->Assign(Top((int)number->getAsInteger()));
                    else if (field == 2)
                        ptr->Assign(Right((int)number->getAsInteger()));
                    else if (field == 3)
                        ptr->Assign(Bottom((int)number->getAsInteger()));
                    return true;
                }
            };
            return proxy;
        }
    };

    struct LeftF { float left; };
    struct TopF { float top; };
    struct RightF { float right; };
    struct BottomF { float bottom; };
    struct RectF
    {
        float left;
        float top;
        float right;
        float bottom;

        float GetWidth() const { return right - left; }
        float GetHeight() const { return bottom - top; }
        PointF GetTopLeftPoint() const { return PointF{ left, top }; }
        PointF GetTopRightPoint() const { return PointF{ right, top }; }
        PointF GetBottomLeftPoint() const { return PointF{ left, bottom }; }
        PointF GetBottomRightPoint() const { return PointF{ right, bottom }; }

        RectF ExpandedBy(float left, float top, float right, float bottom) { return RectF{ this->left - left, this->top - top, this->right + right, this->bottom + bottom }; }
        RectF ExpandedBy(float x, float y) { return ShrunkBy(x, y, x, y); }
        RectF ExpandedBy(float amount) { return ShrunkBy(amount, amount, amount, amount); }
        RectF ShrunkBy(float left, float top, float right, float bottom) { return RectF{ this->left + left, this->top + top, this->right - right, this->bottom - bottom }; }
        RectF ShrunkBy(float x, float y) { return ShrunkBy(x, y, x, y); }
        RectF ShrunkBy(float amount) { return ShrunkBy(amount, amount, amount, amount); }

        // Returns a subrect described by the 'inner' parameter. 0,0 in the 'inner' rect maps to the top left corner of this rect.
        // Example:
        // - this: [100,100,200,200]
        // - inner: [40,50,80,80]
        // - result: [140,150,180,180]
        RectF Subrect(RectF inner) const
        {
            RectF sub;
            sub.left = left + inner.left;
            sub.top = top + inner.top;
            sub.right = left + inner.right;
            sub.bottom = top + inner.bottom;
            return sub;
        }

        // Same as Rect::Subrect, but the resulting rect is clipped to the parent rect
        RectF SubrectBounded(RectF inner) const
        {
            return Subrect(inner).BoundedBy(*this);
        }

        // Returns this rect clipped to the provided rect
        // Example:
        // - this: [100,100,200,200]
        // - provided: [50,50,150,150]
        // - result: [100,100,150,150]
        RectF BoundedBy(RectF rect) const
        {
            RectF sub = *this;
            sub.left = sub.left < rect.left ? rect.left : sub.left;
            sub.left = sub.left > rect.right ? rect.right : sub.left;
            sub.top = sub.top < rect.top ? rect.top : sub.top;
            sub.top = sub.top > rect.bottom ? rect.bottom : sub.top;
            sub.right = sub.right < rect.left ? rect.left : sub.right;
            sub.right = sub.right > rect.right ? rect.right : sub.right;
            sub.bottom = sub.bottom < rect.top ? rect.top : sub.bottom;
            sub.bottom = sub.bottom > rect.bottom ? rect.bottom : sub.bottom;
            return sub;
        }

        bool operator!=(const RectF& other) const
        {
            return
                left != other.left ||
                top != other.top ||
                right != other.right ||
                bottom != other.bottom;
        }

        bool operator==(const RectF& other) const
        {
            return
                left == other.left &&
                top == other.top &&
                right == other.right &&
                bottom == other.bottom;
        }

        RectF& operator=(LeftF value)
        {
            left = value.left;
            return *this;
        }
        RectF& operator=(TopF value)
        {
            top = value.top;
            return *this;
        }
        RectF& operator=(RightF value)
        {
            right = value.right;
            return *this;
        }
        RectF& operator=(BottomF value)
        {
            bottom = value.bottom;
            return *this;
        }

        static ValueProxy LeftValueProxy(std::string name, std::any valuePtr, int precision, float minValue = (float)std::numeric_limits<int>::min(), float maxValue = (float)std::numeric_limits<int>::max(), float stepSize = 1)
        {
            return _ValueProxy(0, name, valuePtr, precision, minValue, maxValue, stepSize);
        }
        static ValueProxy TopValueProxy(std::string name, std::any valuePtr, int precision, float minValue = (float)std::numeric_limits<int>::min(), float maxValue = (float)std::numeric_limits<int>::max(), float stepSize = 1)
        {
            return _ValueProxy(1, name, valuePtr, precision, minValue, maxValue, stepSize);
        }
        static ValueProxy RightValueProxy(std::string name, std::any valuePtr, int precision, float minValue = (float)std::numeric_limits<int>::min(), float maxValue = (float)std::numeric_limits<int>::max(), float stepSize = 1)
        {
            return _ValueProxy(2, name, valuePtr, precision, minValue, maxValue, stepSize);
        }
        static ValueProxy BottomValueProxy(std::string name, std::any valuePtr, int precision, float minValue = (float)std::numeric_limits<int>::min(), float maxValue = (float)std::numeric_limits<int>::max(), float stepSize = 1)
        {
            return _ValueProxy(3, name, valuePtr, precision, minValue, maxValue, stepSize);
        }
    private:
        static ValueProxy _ValueProxy(int field, std::string name, std::any valuePtr, int precision, float minValue, float maxValue, float stepSize)
        {
            ValueProxy proxy;
            proxy.name = name;
            proxy.valuePtr = valuePtr;
            proxy.type = ValueProxy::NUMBER;
            proxy.optional = false;
            proxy.numberPrecision = precision;
            proxy.numberMinValue = ValueProxy::Number(minValue);
            proxy.numberMaxValue = ValueProxy::Number(maxValue);
            proxy.numberStepSize = ValueProxy::Number(stepSize);
            proxy.numberMapper = ValueProxy::Mapper<ValueProxy::Number>{
                .fromValue = [field](const std::any& valuePtr) {
                    auto ptr = std::any_cast<Value<RectF>*>(valuePtr);
                    if (field == 0)
                        return std::optional<ValueProxy::Number>(ValueProxy::Number(ptr->Get().left));
                    else if (field == 1)
                        return std::optional<ValueProxy::Number>(ValueProxy::Number(ptr->Get().top));
                    else if (field == 2)
                        return std::optional<ValueProxy::Number>(ValueProxy::Number(ptr->Get().right));
                    else if (field == 3)
                        return std::optional<ValueProxy::Number>(ValueProxy::Number(ptr->Get().bottom));
                    else
                        return std::optional<ValueProxy::Number>();
                },
                .toValue = [field](const std::optional<ValueProxy::Number>& number, std::any& valuePtr) {
                    auto ptr = std::any_cast<Value<RectF>*>(valuePtr);
                    if (field == 0)
                        ptr->Assign(LeftF((float)number->getAsDouble()));
                    else if (field == 1)
                        ptr->Assign(TopF((float)number->getAsDouble()));
                    else if (field == 2)
                        ptr->Assign(RightF((float)number->getAsDouble()));
                    else if (field == 3)
                        ptr->Assign(BottomF((float)number->getAsDouble()));
                    return true;
                }
            };
            return proxy;
        }

    public:
        static ValueProxy TextValueProxy(std::string name, std::any valuePtr, bool optional = false)
        {
            ValueProxy proxy;
            proxy.name = name;
            proxy.valuePtr = valuePtr;
            proxy.type = ValueProxy::TEXT;
            proxy.optional = optional;
            proxy.textMapper = ValueProxy::Mapper<std::wstring>{
                .fromValue = [optional](const std::any& valuePtr) -> std::optional<std::wstring> {
                    RectF rect;
                    if (optional)
                    {
                        auto ptr = std::any_cast<Value<std::optional<RectF>>*>(valuePtr);
                        if (!ptr->Get())
                            return std::nullopt;
                        rect = ptr->Get().value();
                    }
                    else
                    {
                        auto ptr = std::any_cast<Value<RectF>*>(valuePtr);
                        rect = ptr->Get();
                    }

                    std::wostringstream ss;
                    ss << std::setprecision(3) << rect.left << ' ' << rect.top << ' ' << rect.right << ' ' << rect.bottom;
                    return std::optional<std::wstring>(ss.str());
                },
                .toValue = [optional](const std::optional<std::wstring>& text, std::any& valuePtr) {
                    if (optional)
                    {
                        auto ptr = std::any_cast<Value<std::optional<RectF>>*>(valuePtr);
                        if (text.has_value())
                        {
                            std::wistringstream ss(text.value());
                            RectF rect{};
                            ss >> rect.left >> rect.top >> rect.right >> rect.bottom;
                            ptr->Assign(std::optional<RectF>(rect));
                        }
                        else
                        {
                            ptr->Assign(std::nullopt);
                        }
                    }
                    else
                    {
                        auto ptr = std::any_cast<Value<RectF>*>(valuePtr);
                        std::wistringstream ss(text.value());
                        RectF rect{};
                        ss >> rect.left >> rect.top >> rect.right >> rect.bottom;
                        ptr->Assign(rect);
                    }
                    return true;
                }
            };
            return proxy;
        }
    };

    struct RoundedRect
    {
        float radiusX;
        float radiusY;
        RectF rect;

        RoundedRect(float radiusX, float radiusY, RectF rect) : radiusX(radiusX), radiusY(radiusY), rect(rect) {}
        RoundedRect(float radius, RectF rect) : RoundedRect(radius, radius, rect) {}
        RoundedRect() {}

        bool operator!=(const RoundedRect& other) const
        {
            return
                radiusX != other.radiusX ||
                radiusY != other.radiusY ||
                rect != other.rect;
        }

        bool operator==(const RoundedRect& other) const
        {
            return
                radiusX == other.radiusX &&
                radiusY == other.radiusY &&
                rect == other.rect;
        }
    };

    static ValueProxy DurationValueProxy(std::string name, std::any valuePtr, TimeUnits units, int minValue = 0, int maxValue = std::numeric_limits<int>::max(), int stepSize = 1)
    {
        ValueProxy proxy;
        proxy.name = name;
        proxy.valuePtr = valuePtr;
        proxy.type = ValueProxy::NUMBER;
        proxy.optional = false;
        proxy.numberMinValue = ValueProxy::Number(minValue);
        proxy.numberMaxValue = ValueProxy::Number(maxValue);
        proxy.numberStepSize = ValueProxy::Number(stepSize);
        proxy.numberMapper = ValueProxy::Mapper<ValueProxy::Number>{
            .fromValue = [units](const std::any& valuePtr) {
                auto ptr = std::any_cast<Value<Duration>*>(valuePtr);
                return std::optional<ValueProxy::Number>(ValueProxy::Number(ptr->Get().GetDuration(units)));
            },
            .toValue = [units](const std::optional<ValueProxy::Number>& number, std::any& valuePtr) {
                auto ptr = std::any_cast<Value<Duration>*>(valuePtr);
                ptr->Assign(Duration(number->getAsInteger(), units));
                return true;
            }
        };
        return proxy;
    }

    static ValueProxy TimePointValueProxy(std::string name, std::any valuePtr, TimeUnits units, int minValue = 0, int maxValue = std::numeric_limits<int>::max(), int stepSize = 1)
    {
        ValueProxy proxy;
        proxy.name = name;
        proxy.valuePtr = valuePtr;
        proxy.type = ValueProxy::NUMBER;
        proxy.optional = false;
        proxy.numberMinValue = ValueProxy::Number(minValue);
        proxy.numberMaxValue = ValueProxy::Number(maxValue);
        proxy.numberStepSize = ValueProxy::Number(stepSize);
        proxy.numberMapper = ValueProxy::Mapper<ValueProxy::Number>{
            .fromValue = [units](const std::any& valuePtr) {
                auto ptr = std::any_cast<Value<TimePoint>*>(valuePtr);
                return std::optional<ValueProxy::Number>(ValueProxy::Number(ptr->Get().GetTime(units)));
            },
            .toValue = [units](const std::optional<ValueProxy::Number>& number, std::any& valuePtr) {
                auto ptr = std::any_cast<Value<TimePoint>*>(valuePtr);
                ptr->Assign(TimePoint(number->getAsInteger(), units));
                return true;
            }
        };
        return proxy;
    }

    static RECT RectToRECT(const Rect& rect) { return { rect.left, rect.top, rect.right, rect.bottom }; }
    static D2D1_RECT_F RectToD2D1_RECT_F(const Rect& rect) { return { (FLOAT) rect.left, (FLOAT) rect.top, (FLOAT) rect.right, (FLOAT) rect.bottom }; }
    static D2D1_RECT_F RectFToD2D1_RECT_F(const RectF& rect) { return { rect.left, rect.top, rect.right, rect.bottom }; }
    static D2D1_COLOR_F ColorToD2D1_COLOR_F(const Color& color) { return D2D1::ColorF(color.ToIntNoAlpha(), color.a / 255.0f); }

    inline bool operator==(const DWRITE_TEXT_RANGE& r1, const DWRITE_TEXT_RANGE& r2)
    {
        return r1.startPosition == r2.startPosition && r1.length == r2.length;
    }
}