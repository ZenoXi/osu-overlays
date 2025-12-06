#pragma once

#include "DirectX.h"
#include "UICore/Components/ComHelper.h"
#include "SegmentedBitmapAllocator.h"

namespace zwnd
{
    class BitmapAllocator;

    struct BitmapSegment
    {
        BitmapSegment(int index, bool standalone, uint64_t path, zcom::Rect rect, std::shared_ptr<ID2D1Bitmap1*> bitmap, BitmapAllocator* allocator)
            : index(index), standalone(standalone), path(path), rect(rect), _bitmap(bitmap), _allocator(allocator)
        {}

        int index;
        bool standalone;
        uint64_t path;
        zcom::Rect rect;
        ID2D1Bitmap1* Bitmap() const { return *_bitmap; }
        D2D1_RECT_F RectF() const { return D2D1::RectF((float)rect.left, (float)rect.top, (float)rect.right, (float)rect.bottom); }

        void Release();
    private:
        std::shared_ptr<ID2D1Bitmap1*> _bitmap = nullptr;
        BitmapAllocator* _allocator = nullptr;
    };

    class BitmapAllocator
    {
        struct _BitmapSource
        {
            _BitmapSource(int size, std::shared_ptr<ID2D1Bitmap1*> bitmapRef)
                : size(size), bitmapRef(bitmapRef), rectAllocator(size)
            {}

            int size;
            std::shared_ptr<ID2D1Bitmap1*> bitmapRef;
            zcom::SegmentedBitmapAllocator rectAllocator;
        };

        struct _BitmapSourceGroup
        {
            int baseSize;
            std::vector<_BitmapSource> sources;
            D2D1_BITMAP_PROPERTIES1 bitmapProperties;
        };

        int nextSourceIndex = -1;
        std::unordered_map<int, _BitmapSourceGroup> _bitmapSourceGroups;
        ID2D1DeviceContext* _target = nullptr;

    public:
        void SetTarget(ID2D1DeviceContext* target)
        {
            _target = target;
        }

        std::optional<int> AddSource(int baseSize, D2D1_BITMAP_PROPERTIES1 bitmapProperties, std::optional<int> index = std::nullopt)
        {
            bool powerOfTwo = !(baseSize == 0) && !(baseSize & (baseSize - 1));
            if (!powerOfTwo)
                return std::nullopt;
            if (index && (index.value() < 0 || _bitmapSourceGroups.contains(index.value())))
                return std::nullopt;

            int finalIndex;
            if (index)
                finalIndex = index.value();
            else
                finalIndex = nextSourceIndex--;

            auto bitmapStorage = std::make_shared<ID2D1Bitmap1*>(nullptr);
            HRESULT hr = _target->CreateBitmap(
                D2D1::SizeU(baseSize, baseSize),
                nullptr,
                0,
                bitmapProperties,
                bitmapStorage.get()
            );
            if (hr == S_OK)
            {
                _BitmapSourceGroup group;
                group.baseSize = baseSize;
                group.bitmapProperties = bitmapProperties;
                group.sources.push_back(_BitmapSource(baseSize, bitmapStorage));

                _bitmapSourceGroups.insert({ finalIndex, std::move(group) });
                return finalIndex;
            }

            return std::nullopt;
        }

        std::optional<BitmapSegment> Allocate(int w, int h, int index)
        {
            if (w <= 0 || h <= 0)
                return std::nullopt;
            auto group = _bitmapSourceGroups.find(index);
            if (group == _bitmapSourceGroups.end())
                return std::nullopt;

            std::optional<BitmapSegment> segment = std::nullopt;
            for (auto& source : group->second.sources)
            {
                auto rect = source.rectAllocator.Allocate(w, h);
                if (rect)
                {
                    segment = BitmapSegment(index, false, rect->path, rect->rect, source.bitmapRef, this);
                    break;
                }
            }

            if (!segment)
            {
                int size = group->second.baseSize;
                if (w > size || h > size)
                {
                    auto bitmapStorage = std::make_shared<ID2D1Bitmap1*>(nullptr);
                    HRESULT hr = _target->CreateBitmap(
                        D2D1::SizeU(w, h),
                        nullptr,
                        0,
                        group->second.bitmapProperties,
                        bitmapStorage.get()
                    );
                    if (hr == S_OK)
                    {
                        group->second.sources.push_back(_BitmapSource(0, bitmapStorage));
                        segment = BitmapSegment(index, true, 0, { 0, 0, w, h }, group->second.sources.back().bitmapRef, this);
                    }
                }

                while (w > size || h > size)
                    size *= 2;

                auto bitmapStorage = std::make_shared<ID2D1Bitmap1*>(nullptr);
                HRESULT hr = _target->CreateBitmap(
                    D2D1::SizeU(size, size),
                    nullptr,
                    0,
                    group->second.bitmapProperties,
                    bitmapStorage.get()
                );
                if (hr == S_OK)
                {
                    group->second.sources.push_back(_BitmapSource(size, bitmapStorage));
                    auto rect = group->second.sources.back().rectAllocator.Allocate(w, h);
                    if (rect)
                        segment = BitmapSegment(index, false, rect->path, rect->rect, group->second.sources.back().bitmapRef, this);
                }
            }

            return segment;
        }

        void Release(const BitmapSegment& segment)
        {
            auto groupIt = _bitmapSourceGroups.find(segment.index);
            if (groupIt == _bitmapSourceGroups.end())
                return;
            auto& group = groupIt->second;

            for (auto it = group.sources.begin(); it != group.sources.end(); it++)
            {
                if (segment.Bitmap() == *it->bitmapRef)
                {
                    if (segment.standalone)
                    {
                        (*it->bitmapRef)->Release();
                        (*it->bitmapRef) = nullptr;
                        group.sources.erase(it);
                    }
                    else
                    {
                        it->rectAllocator.Release(segment.path);
                    }
                    break;
                }
            }
        }

        void ClearSources()
        {
            for (auto& group : _bitmapSourceGroups)
            {
                for (auto& source : group.second.sources)
                {
                    (*source.bitmapRef)->Release();
                    (*source.bitmapRef) = nullptr;
                }
                group.second.sources.clear();
            }
        }

        void RecreateInitialGroupSource()
        {
            // TODO: test if this actually has any benefit
            for (auto& group : _bitmapSourceGroups)
            {
                auto bitmapStorage = std::make_shared<ID2D1Bitmap1*>(nullptr);
                HRESULT hr = _target->CreateBitmap(
                    D2D1::SizeU(group.second.baseSize, group.second.baseSize),
                    nullptr,
                    0,
                    group.second.bitmapProperties,
                    bitmapStorage.get()
                );
                if (hr == S_OK)
                {
                    group.second.sources.push_back(_BitmapSource(group.second.baseSize, bitmapStorage));
                }
            }
        }
    };
}