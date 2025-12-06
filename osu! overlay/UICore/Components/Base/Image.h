#pragma once

#include "ComponentBase.h"
#include "../ComHelper.h"

#include "D2DEffects/TintEffect.h"

namespace zcom
{
    enum class ImagePlacement
    {
        // Doesn't do any stretching or aligning
        NONE,
        // Stretch the image to fill the entire drawing area
        FILL,
        // Fit and center the image so that nothing is cropped
        FIT,
        // Position the image in the top left corner
        TOP_LEFT,
        // Position the image at the top edge center
        TOP_CENTER,
        // Position the image in the top right corner
        TOP_RIGHT,
        // Position the image at the left edge center
        CENTER_LEFT,
        // Centers the image
        CENTER,
        // Position the image at the right edge center
        CENTER_RIGHT,
        // Position the image in the bottom left corner
        BOTTOM_LEFT,
        // Position the image at the bottom edge center
        BOTTOM_CENTER,
        // Position the image in the bottom right corner
        BOTTOM_RIGHT,
    };
    constexpr std::vector<std::pair<int64_t, std::wstring>> ImagePlacementValueProxySelectionValues()
    {
        return {
            { (int64_t)ImagePlacement::NONE, L"None" },
            { (int64_t)ImagePlacement::FILL, L"Fill" },
            { (int64_t)ImagePlacement::FIT, L"Fit" },
            { (int64_t)ImagePlacement::TOP_LEFT, L"Top left" },
            { (int64_t)ImagePlacement::TOP_CENTER, L"Top center" },
            { (int64_t)ImagePlacement::TOP_RIGHT, L"Top right" },
            { (int64_t)ImagePlacement::CENTER_LEFT, L"Center left" },
            { (int64_t)ImagePlacement::CENTER, L"Center" },
            { (int64_t)ImagePlacement::CENTER_RIGHT, L"Center right" },
            { (int64_t)ImagePlacement::BOTTOM_LEFT, L"Bottom left" },
            { (int64_t)ImagePlacement::BOTTOM_CENTER, L"Bottom center" },
            { (int64_t)ImagePlacement::BOTTOM_RIGHT, L"Bottom right" }
        };
    }

    class Image : public Component
    {
        DEFINE_COMPONENT(Image, Component)
        DEFAULT_DESTRUCTOR(Image)
    protected:
        void Init(std::optional<Bitmap> image = std::nullopt)
        {
            this->image = image;
        }

    public:
        Value<std::optional<Bitmap>> image = Value<std::optional<Bitmap>>(std::optional<Bitmap>(std::nullopt), [=](std::optional<Bitmap>& currentValue, std::optional<Bitmap> const& image) {
            currentValue = image;
            InvokeRedraw();
        });
        // Source image area to use
        Value<std::optional<RectF>> sourceRect = Value<std::optional<RectF>>(std::nullopt, [=](std::optional<RectF>& currentValue, const std::optional<RectF>& rect) {
            currentValue = rect;
            InvokeRedraw();
        });
        // Target area to draw to and perform calculations with
        // A target rect smaller than the draw area will NOT clip content, only scale it when needed
        Value<std::optional<RectF>> targetRect = Value<std::optional<RectF>>(std::nullopt, [=](std::optional<RectF>& currentValue, const std::optional<RectF>& rect) {
            currentValue = rect;
            InvokeRedraw();
        });
        Value<ImagePlacement> imagePlacement = Value<ImagePlacement>(ImagePlacement::NONE, [=](ImagePlacement& currentValue, const ImagePlacement& placement) {
            currentValue = placement;
            InvokeRedraw();
        });
        Value<float> xOffset = Value<float>(0.0f, [=](float& currentValue, const float& offset) {
            currentValue = offset;
            InvokeRedraw();
        });
        Value<float> yOffset = Value<float>(0.0f, [=](float& currentValue, const float& offset) {
            currentValue = offset;
            InvokeRedraw();
        });
        // Scales the image on the x-axis by the specified amount
        // The scaling only applies to placement modes that do not size the image automatically
        // Scaled image size ignores pixel snapping (top left corner placement is still snapped)
        Value<float> xScale = Value<float>(1.0f, [=](float& currentValue, const float& scale) {
            currentValue = scale;
            InvokeRedraw();
        });
        // Scales the image on the y-axis by the specified amount
        // The scaling only applies to placement modes that do not size the image automatically
        // Scaled image size ignores pixel snapping (top left corner placement is still snapped)
        Value<float> yScale = Value<float>(1.0f, [=](float& currentValue, const float& scale) {
            currentValue = scale;
            InvokeRedraw();
        });
        Value<bool> snapToPixels = Value<bool>(false, [=](bool& currentValue, const bool& snap) {
            currentValue = snap;
            InvokeRedraw();
        });
        Value<float> imageOpacity = Value<float>(1.0f, [=](float& currentValue, const float& opacity) {
            currentValue = opacity;
            InvokeRedraw();
        });
        Value<Color> tintColor = Value<Color>(Color(0xFFFFFF), [=](Color& currentValue, const Color& color) {
            currentValue = color;
            InvokeRedraw();
        });

    protected:
        void _OnDraw(Graphics* g) override
        {
            if (!image->has_value())
                return;

            RectF srcRect = image.Get()->GetSize().ToRect().ToRectF();
            if (sourceRect->has_value())
                srcRect = srcRect.SubrectBounded(sourceRect->value());

            RectF targetRect_ = g->GetTargetRect().ToRectF();
            if (targetRect->has_value())
                targetRect_ = targetRect->value();

            RectF destRect{};
            if (imagePlacement == ImagePlacement::NONE)
            {
                float width = (srcRect.right - srcRect.left) * xScale;
                float height = (srcRect.bottom - srcRect.top) * yScale;
                float left = targetRect_.left + xOffset;
                float top = targetRect_.top + yOffset;
                if (snapToPixels)
                {
                    left = std::roundf(left);
                    top = std::roundf(top);
                }
                destRect = { left, top, left + width, top + height };
            }
            else if (imagePlacement == ImagePlacement::FILL)
            {
                float left = targetRect_.left + xOffset;
                float top = targetRect_.top + yOffset;
                if (snapToPixels)
                {
                    left = std::roundf(left);
                    top = std::roundf(top);
                }
                float width = targetRect_.right - targetRect_.left;
                float height = targetRect_.bottom - targetRect_.top;
                destRect = { left, top, left + width, top + height};
            }
            else if (imagePlacement == ImagePlacement::FIT)
            {
                // Scale frame to preserve aspect ratio
                float imageWidth = srcRect.right - srcRect.left;
                float imageHeight = srcRect.bottom - srcRect.top;
                float targetWidth = targetRect_.right - targetRect_.left;
                float targetHeight = targetRect_.bottom - targetRect_.top;
                if (imageWidth / imageHeight < targetWidth / targetHeight)
                {
                    float scale = imageHeight / targetHeight;
                    float newWidth = imageWidth / scale;
                    destRect = {
                        targetRect_.left + (targetWidth - newWidth) * 0.5f,
                        targetRect_.top,
                        targetRect_.left + (targetWidth - newWidth) * 0.5f + newWidth,
                        targetRect_.top + targetHeight
                    };
                }
                else if (imageWidth / imageHeight > targetWidth / targetHeight)
                {
                    float scale = imageWidth / targetWidth;
                    float newHeight = imageHeight / scale;
                    destRect = {
                        targetRect_.left,
                        targetRect_.top + (targetHeight - newHeight) * 0.5f,
                        targetRect_.left + targetWidth,
                        targetRect_.top + (targetHeight - newHeight) * 0.5f + newHeight
                    };
                }
                else
                {
                    destRect = { targetRect_.left, targetRect_.top, targetRect_.left + targetWidth, targetRect_.top + targetHeight };
                }

                // Apply offset
                destRect.left += xOffset;
                destRect.right += xOffset;
                destRect.top += yOffset;
                destRect.bottom += yOffset;

                if (snapToPixels)
                {
                    float width = destRect.right - destRect.left;
                    float height = destRect.bottom - destRect.top;
                    destRect.left = std::roundf(destRect.left);
                    destRect.top = std::roundf(destRect.top);
                    destRect.right = destRect.left + width;
                    destRect.bottom = destRect.top + height;
                }
            }
            // Remaining placement modes involve just position placements
            else
            {
                // Scale image
                float width = (srcRect.right - srcRect.left) * xScale;
                float height = (srcRect.bottom - srcRect.top) * yScale;

                // Calculate placement
                float left = targetRect_.left;
                float top = targetRect_.top;
                switch (imagePlacement)
                {
                case ImagePlacement::TOP_LEFT:
                {
                    break;
                }
                case ImagePlacement::TOP_CENTER:
                {
                    left = targetRect_.left + (targetRect_.right - targetRect_.left - width) * 0.5f;
                    break;
                }
                case ImagePlacement::TOP_RIGHT:
                {
                    left = targetRect_.right - width;
                    break;
                }
                case ImagePlacement::CENTER_LEFT:
                {
                    top = targetRect_.top + (targetRect_.bottom - targetRect_.top - height) * 0.5f;
                    break;
                }
                case ImagePlacement::CENTER:
                {
                    top = targetRect_.top + (targetRect_.bottom - targetRect_.top - height) * 0.5f;
                    left = targetRect_.left + (targetRect_.right - targetRect_.left - width) * 0.5f;
                    break;
                }
                case ImagePlacement::CENTER_RIGHT:
                {
                    top = targetRect_.top + (targetRect_.bottom - targetRect_.top - height) * 0.5f;
                    left = targetRect_.right - width;
                    break;
                }
                case ImagePlacement::BOTTOM_LEFT:
                {
                    top = targetRect_.bottom - height;
                    break;
                }
                case ImagePlacement::BOTTOM_CENTER:
                {
                    top = targetRect_.bottom - height;
                    left = targetRect_.left + (targetRect_.right - targetRect_.left - width) * 0.5f;
                    break;
                }
                case ImagePlacement::BOTTOM_RIGHT:
                {
                    top = targetRect_.bottom - height;
                    left = targetRect_.right - width;
                    break;
                }
                default:
                    break;
                }

                left += xOffset;
                top += yOffset;
                if (snapToPixels)
                {
                    left = std::roundf(left);
                    top = std::roundf(top);
                }

                destRect = { left, top, left + width, top + height };
            }

            if (!(tintColor == Color(0xFFFFFF)))
            {
                // TODO: Content bitmap needs to be separate from image (image can be whatever)
                auto contentBitmap = g->CreateBitmap(image.Get()->GetSize(), SEGMENT_POOL_AUX1);
                if (contentBitmap)
                {
                    BitmapSourceEffect bitmapSource = BitmapSourceEffect(&image->value());
                    TintEffect tintEffect = TintEffect(&bitmapSource, tintColor);
                    g->PushAndClearTarget(contentBitmap.value());
                    g->DrawEffect(&tintEffect);
                    g->PopTarget();
                    // TODO: add high quality cubic interpolation
                    g->DrawBitmap(contentBitmap.value(), destRect, srcRect, imageOpacity);

                    // Comment from old implementation below, probably needs investigation
                    
                    // Flush here (before DrawBitmap) because otherwise some bullshit interaction causes things rendered to the
                    // content bitmap to sometimes not show up (ONLY  when usiNG D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC!!@.!?!?!?2!)
                }
            }
            else
            {
                g->DrawBitmap(image->value(), destRect, srcRect, imageOpacity);
            }
        }

    public:
        std::vector<std::pair<std::string, std::vector<ValueProxy>>> GetReflectionData()
        {
            std::vector<ValueProxy> values;
            values.push_back(RectF::TextValueProxy("source rect", std::make_any<Value<std::optional<RectF>>*>(&sourceRect), true));
            values.push_back(RectF::TextValueProxy("target rect", std::make_any<Value<std::optional<RectF>>*>(&targetRect), true));
            values.push_back(ValueProxy::BasicEnumValueProxy<ImagePlacement>("image placement", std::make_any<Value<ImagePlacement>*>(&imagePlacement), ImagePlacementValueProxySelectionValues()));
            values.push_back(ValueProxy::BasicFloatValueProxy<float>("x offset", std::make_any<Value<float>*>(&xOffset), 3));
            values.push_back(ValueProxy::BasicFloatValueProxy<float>("y offset", std::make_any<Value<float>*>(&yOffset), 3));
            values.push_back(ValueProxy::BasicFloatValueProxy<float>("x scale", std::make_any<Value<float>*>(&xScale), 3));
            values.push_back(ValueProxy::BasicFloatValueProxy<float>("y scale", std::make_any<Value<float>*>(&yScale), 3));
            values.push_back(ValueProxy::BasicBoolValueProxy("snap to pixels", std::make_any<Value<bool>*>(&snapToPixels)));
            values.push_back(ValueProxy::BasicFloatValueProxy<float>("image opacity", std::make_any<Value<float>*>(&imageOpacity), 3));
            values.push_back(ValueProxy::BasicColorValueProxy("tint color", std::make_any<Value<Color>*>(&tintColor)));

            auto data = Component::GetReflectionData();
            data.insert(data.begin(), { "Image", std::move(values) });
            return data;
        }
    };
}