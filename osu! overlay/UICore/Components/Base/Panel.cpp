#include "Panel.h"

#include <algorithm>

void zcom::Panel::_OnDraw(Graphics* g)
{
    _OnDraw(g, {});
}

void zcom::Panel::_OnDraw(Graphics* g, DrawParams params)
{
    struct ComponentBitmap
    {
        std::optional<Bitmap> bitmap;
        Component* component;
    };

    // Get bitmaps of all items
    std::vector<ComponentBitmap> bitmaps;
    bitmaps.reserve(_items.size());
    for (auto& item : _items)
    {
        // Order by z-index
        auto it = bitmaps.rbegin();
        for (; it != bitmaps.rend(); it++)
        {
            if (item.item->zIndex >= it->component->zIndex)
                break;
        }
        if (item.item->Redraw())
            item.item->Draw(g);
        bitmaps.insert(it.base(), { item.item->ContentImage(), item.item });
    }

    std::vector<std::pair<int, std::vector<ComponentBitmap>>> shadowGroups;
    for (auto& bitmap : bitmaps)
    {
        if (!bitmap.bitmap || bitmap.component->opacity <= 0.0f || !bitmap.component->visible)
            continue;

        auto prop = bitmap.component->GetProperty<Shadow>();
        if (!prop.valid || !prop.group)
            continue;

        bool groupFound = false;
        for (auto& group : shadowGroups)
        {
            if (group.first == prop.group.value())
            {
                group.second.push_back(bitmap);
                groupFound = true;
                break;
            }
        }

        if (!groupFound)
            shadowGroups.push_back({ prop.group.value(), { bitmap } });
    }

    // Draw content to separate bitmap since drawing a bitmap to itself isn't supported in D2D

    auto panelContentOpt = g->CreateBitmap(size_->width, size_->height, SegmentPoolIndex::SEGMENT_POOL_AUX2);
    if (!panelContentOpt)
    {
        // TODO: logging
        std::cout << "Couldn't allocate bitmap for panel contents\n";
        return;
    }
    auto& panelContent = panelContentOpt.value();
    g->PushAndClearTarget(panelContent);

    // Draw the bitmaps
    for (auto& bitmap : bitmaps)
    {
        if (!bitmap.bitmap || bitmap.component->opacity <= 0.0f || !bitmap.component->visible)
            continue;

        // Draw shadow
        auto prop = bitmap.component->GetProperty<Shadow>();
        if (prop.valid)
        {
            std::vector<ComponentBitmap> shadowInputs;
            if (prop.group)
            {
                if (!shadowGroups.empty() && shadowGroups.front().first == prop.group.value())
                {
                    shadowInputs = std::move(shadowGroups.front().second);
                    shadowGroups.erase(shadowGroups.begin());
                }
            }
            else
            {
                shadowInputs.push_back(bitmap);
            }

            if (!shadowInputs.empty())
            {
                std::vector<std::unique_ptr<Effect>> inputEffects;
                std::vector<std::unique_ptr<Effect>> otherEffects;

                for (auto& input : shadowInputs)
                {
                    if (input.component->opacity < 1.0f)
                    {
                        std::unique_ptr<Effect> bitmapSourceEffect = std::make_unique<BitmapSourceEffect>(&input.bitmap.value(), PointF((float)input.component->position_->x, (float)input.component->position_->y));
                        inputEffects.push_back(std::make_unique<OpacityEffect>(bitmapSourceEffect.get(), input.component->opacity));
                        otherEffects.push_back(std::move(bitmapSourceEffect));
                    }
                    else
                    {
                        inputEffects.push_back(std::make_unique<BitmapSourceEffect>(&input.bitmap.value(), PointF((float)input.component->position_->x, (float)input.component->position_->y)));
                    }
                }

                std::vector<Effect*> compositeEffectInputs;
                for (auto& effect : inputEffects)
                    compositeEffectInputs.push_back(effect.get());
                CompositeEffect compositeEffect = CompositeEffect(compositeEffectInputs);
                ShadowEffect shadowEffect = ShadowEffect(&compositeEffect, PointF(prop.offsetX, prop.offsetY), prop.blurStandardDeviation, prop.color);
                g->DrawEffect(&shadowEffect);
            }
        }

        g->DrawBitmap(bitmap.bitmap.value(), RectF(
            params.contentOffset.x + (FLOAT)bitmap.component->position_->x,
            params.contentOffset.y + (FLOAT)bitmap.component->position_->y,
            params.contentOffset.x + (FLOAT)(bitmap.component->position_->x + bitmap.component->size_->width),
            params.contentOffset.y + (FLOAT)(bitmap.component->position_->y + bitmap.component->size_->height)
        ), std::nullopt, bitmap.component->opacity.Get());

        // The flush here is necessary to avoid weird visual bugs.
        // Specifically, window shadow does not render when the window is small enough
        // and a different shadow is being drawn deep in the component tree.
        // It's possible that it's a fault of the default shadow effect implementation,
        // but there is no point in figuring that out, since just calling flush here
        // works fine and has seemingly no performance impact.
        g->Flush();
    }

    g->PopTarget();
    g->DrawBitmap(panelContent);
}