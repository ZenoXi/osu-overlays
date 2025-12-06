#pragma once

#include "UICore/Components/ComHelper.h"

#include <memory>

namespace zcom
{
    struct AllocatedRect
    {
        uint64_t path = 0;
        Rect rect{};
    };

    class SegmentedBitmapAllocator
    {
        enum _SegmentLayout
        {
            FOUR_RECTS,
            SINGLE,
            TOP_BOTTOM,
            LEFT_RIGHT,
            TOP_BOTTOM_RIGHT,
            LEFT_RIGHT_BOTTOM
        };

        enum _SegmentIndex
        {
            TOP_LEFT = 0,
            TOP_RIGHT = 1,
            BOTTOM_LEFT = 2,
            BOTTOM_RIGHT = 3
        };

        struct _Segment
        {
            bool filled = false;
            bool clean = true;
            bool regular = true;
            _SegmentLayout layout = FOUR_RECTS;

            int x = 0;
            int y = 0;
            int w = 0;
            int h = 0;

            std::unique_ptr<_Segment> segments[4];

            uint64_t path = 0;
            int depth = 0;
            AllocatedRect GetAllocatedRect() { return { path, Rect{ x, y, x + w, y + h } }; }
        };

        void CreateSegment(_Segment* parent, _SegmentIndex index)
        {
            auto segment = std::make_unique<_Segment>();

            if (parent != nullptr)
            {
                if (index == TOP_LEFT)
                {
                    segment->x = parent->x;
                    segment->y = parent->y;
                }
                else if (index == TOP_RIGHT)
                {
                    segment->x = parent->x + parent->segments[TOP_LEFT]->w;
                    segment->y = parent->y;
                }
                else if (index == BOTTOM_LEFT)
                {
                    segment->x = parent->x;
                    segment->y = parent->y + parent->segments[TOP_LEFT]->h;
                }
                else if (index == BOTTOM_RIGHT)
                {
                    segment->x = parent->x + parent->segments[TOP_LEFT]->w;
                    segment->y = parent->y + parent->segments[TOP_LEFT]->h;
                }
                segment->depth = parent->depth + 1;
                segment->path = parent->path | ((uint64_t)index) << (parent->depth * 2);
                parent->segments[index] = std::move(segment);
            }
        }

        std::unique_ptr<_Segment> _rootSegment;

        bool _CanAttemptAllocation(_Segment* segment, int w, int h)
        {
            return segment && !segment->filled && w <= segment->w && h <= segment->h;
        }

    public:
        SegmentedBitmapAllocator(int size)
        {
            bool powerOfTwo = !(size == 0) && !(size & (size - 1));
            if (!powerOfTwo)
                return;

            _rootSegment = std::make_unique<_Segment>();
            _rootSegment->w = size;
            _rootSegment->h = size;
        }

        std::optional<AllocatedRect> Allocate(int w, int h)
        {
            if (!_rootSegment || w < 1 || h < 1 || w > _rootSegment->w || h > _rootSegment->h)
                return std::nullopt;
            return Allocate(_rootSegment.get(), w, h);
        }

        std::optional<AllocatedRect> Allocate(_Segment* segment, int w, int h)
        {
            if (segment->clean)
            {
                std::optional<AllocatedRect> rect;
                if (segment->regular && w <= segment->w / 2 && h <= segment->h / 2)
                {
                    CreateSegment(segment, TOP_LEFT);
                    segment->segments[TOP_LEFT]->w = segment->w / 2;
                    segment->segments[TOP_LEFT]->h = segment->h / 2;
                    rect = Allocate(segment->segments[TOP_LEFT].get(), w, h);
                }
                else
                {
                    if (w == segment->w && h == segment->h)
                    {
                        segment->filled = true;
                        segment->layout = SINGLE;
                        rect = segment->GetAllocatedRect();
                    }
                    else
                    {
                        CreateSegment(segment, TOP_LEFT);
                        segment->segments[TOP_LEFT]->w = w;
                        segment->segments[TOP_LEFT]->h = h;
                        segment->segments[TOP_LEFT]->filled = true;
                        segment->segments[TOP_LEFT]->regular = false;

                        if (w == segment->w)
                            segment->layout = TOP_BOTTOM;
                        if (h == segment->h)
                            segment->layout = LEFT_RIGHT;

                        rect = segment->segments[TOP_LEFT]->GetAllocatedRect();
                    }
                }
                segment->clean = false;
                return rect;
            }
            else
            {
                if (_CanAttemptAllocation(segment->segments[TOP_LEFT].get(), w, h)) {

                    auto rect = Allocate(segment->segments[TOP_LEFT].get(), w, h);
                    if (rect)
                        return rect;
                }

                if (segment->layout == FOUR_RECTS)
                {
                    std::optional<AllocatedRect> rect;
                    for (int i = 1; i < 4 && !rect; i++)
                    {
                        if (_CanAttemptAllocation(segment->segments[i].get(), w, h))
                            rect = Allocate(segment->segments[i].get(), w, h);
                    }

                    if (!rect)
                    {
                        int rightW = segment->w - segment->segments[TOP_LEFT]->w;
                        int bottomH = segment->h - segment->segments[TOP_LEFT]->h;

                        if (w <= rightW && !segment->segments[TOP_RIGHT])
                        {
                            if (h <= segment->segments[TOP_LEFT]->h)
                            {
                                CreateSegment(segment, TOP_RIGHT);
                                segment->segments[TOP_RIGHT]->w = rightW;
                                segment->segments[TOP_RIGHT]->h = segment->segments[TOP_LEFT]->h;
                                segment->segments[TOP_RIGHT]->regular = segment->segments[TOP_LEFT]->regular;
                                rect = Allocate(segment->segments[TOP_RIGHT].get(), w, h);
                            }
                            else if (!segment->segments[BOTTOM_RIGHT])
                            {
                                CreateSegment(segment, TOP_RIGHT);
                                segment->segments[TOP_RIGHT]->w = rightW;
                                segment->segments[TOP_RIGHT]->h = segment->h;
                                segment->segments[TOP_RIGHT]->regular = false;
                                segment->layout = TOP_BOTTOM_RIGHT;
                                rect = Allocate(segment->segments[TOP_RIGHT].get(), w, h);
                            }
                        }
                        else if (h <= bottomH && !segment->segments[BOTTOM_LEFT])
                        {
                            if (w <= segment->segments[TOP_LEFT]->w)
                            {
                                CreateSegment(segment, BOTTOM_LEFT);
                                segment->segments[BOTTOM_LEFT]->w = segment->segments[TOP_LEFT]->w;
                                segment->segments[BOTTOM_LEFT]->h = bottomH;
                                segment->segments[BOTTOM_LEFT]->regular = segment->segments[TOP_LEFT]->regular;
                                rect = Allocate(segment->segments[BOTTOM_LEFT].get(), w, h);
                            }
                            else if (!segment->segments[BOTTOM_RIGHT])
                            {
                                CreateSegment(segment, BOTTOM_LEFT);
                                segment->segments[BOTTOM_LEFT]->w = segment->w;
                                segment->segments[BOTTOM_LEFT]->h = bottomH;
                                segment->segments[BOTTOM_LEFT]->regular = false;
                                segment->layout = LEFT_RIGHT_BOTTOM;
                                rect = Allocate(segment->segments[BOTTOM_LEFT].get(), w, h);
                            }
                        }
                        else if (w <= rightW && h <= bottomH && !segment->segments[BOTTOM_RIGHT])
                        {
                            CreateSegment(segment, BOTTOM_RIGHT);
                            segment->segments[BOTTOM_RIGHT]->w = rightW;
                            segment->segments[BOTTOM_RIGHT]->h = bottomH;
                            segment->segments[BOTTOM_RIGHT]->regular = segment->segments[TOP_LEFT]->regular;
                            rect = Allocate(segment->segments[BOTTOM_RIGHT].get(), w, h);
                        }
                    }

                    return rect;
                }
                else if (segment->layout == TOP_BOTTOM)
                {
                    std::optional<AllocatedRect> rect;
                    if (_CanAttemptAllocation(segment->segments[BOTTOM_LEFT].get(), w, h))
                        rect = Allocate(segment->segments[BOTTOM_LEFT].get(), w, h);

                    if (!rect)
                    {
                        int bottomH = segment->h - segment->segments[TOP_LEFT]->h;
                        if (h <= bottomH && !segment->segments[BOTTOM_LEFT])
                        {
                            CreateSegment(segment, BOTTOM_LEFT);
                            segment->segments[BOTTOM_LEFT]->w = segment->w;
                            segment->segments[BOTTOM_LEFT]->h = bottomH;
                            segment->segments[BOTTOM_LEFT]->regular = false;
                            rect = Allocate(segment->segments[BOTTOM_LEFT].get(), w, h);
                        }
                    }

                    return rect;
                }
                else if (segment->layout == LEFT_RIGHT)
                {
                    std::optional<AllocatedRect> rect;
                    if (_CanAttemptAllocation(segment->segments[TOP_RIGHT].get(), w, h))
                        rect = Allocate(segment->segments[TOP_RIGHT].get(), w, h);

                    if (!rect)
                    {
                        int rightW = segment->w - segment->segments[TOP_LEFT]->w;
                        if (w <= rightW && !segment->segments[TOP_RIGHT])
                        {
                            CreateSegment(segment, TOP_RIGHT);
                            segment->segments[TOP_RIGHT]->w = rightW;
                            segment->segments[TOP_RIGHT]->h = segment->h;
                            segment->segments[TOP_RIGHT]->regular = false;
                            rect = Allocate(segment->segments[TOP_RIGHT].get(), w, h);
                        }
                    }

                    return rect;
                }
                else if (segment->layout == TOP_BOTTOM_RIGHT)
                {
                    std::optional<AllocatedRect> rect;
                    if (!rect && _CanAttemptAllocation(segment->segments[TOP_RIGHT].get(), w, h))
                        rect= Allocate(segment->segments[TOP_RIGHT].get(), w, h);
                    if (!rect && _CanAttemptAllocation(segment->segments[BOTTOM_LEFT].get(), w, h))
                        rect = Allocate(segment->segments[BOTTOM_LEFT].get(), w, h);

                    if (!rect)
                    {
                        int rightW = segment->w - segment->segments[TOP_LEFT]->w;
                        int bottomH = segment->h - segment->segments[TOP_LEFT]->h;

                        if (h <= bottomH && w <= segment->segments[TOP_LEFT]->w && !segment->segments[BOTTOM_LEFT])
                        {
                            CreateSegment(segment, BOTTOM_LEFT);
                            segment->segments[BOTTOM_LEFT]->w = segment->segments[TOP_LEFT]->w;
                            segment->segments[BOTTOM_LEFT]->h = bottomH;
                            segment->segments[BOTTOM_LEFT]->regular = segment->segments[TOP_LEFT]->regular;
                            rect = Allocate(segment->segments[BOTTOM_LEFT].get(), w, h);
                        }
                        else if (w <= rightW && !segment->segments[TOP_RIGHT])
                        {
                            CreateSegment(segment, TOP_RIGHT);
                            segment->segments[TOP_RIGHT]->w = rightW;
                            segment->segments[TOP_RIGHT]->h = segment->h;
                            segment->segments[TOP_RIGHT]->regular = false;
                            rect = Allocate(segment->segments[TOP_RIGHT].get(), w, h);
                        }
                    }

                    return rect;
                }
                else if (segment->layout == LEFT_RIGHT_BOTTOM)
                {
                    std::optional<AllocatedRect> rect;

                    if (!rect && _CanAttemptAllocation(segment->segments[TOP_RIGHT].get(), w, h))
                        rect = Allocate(segment->segments[TOP_RIGHT].get(), w, h);
                    if (!rect && _CanAttemptAllocation(segment->segments[BOTTOM_LEFT].get(), w, h))
                        rect = Allocate(segment->segments[BOTTOM_LEFT].get(), w, h);

                    if (!rect)
                    {
                        int rightW = segment->w - segment->segments[TOP_LEFT]->w;
                        int bottomH = segment->h - segment->segments[TOP_LEFT]->h;

                        if (w <= rightW && h <= segment->segments[TOP_LEFT]->h && !segment->segments[TOP_RIGHT])
                        {
                            CreateSegment(segment, TOP_RIGHT);
                            segment->segments[TOP_RIGHT]->w = rightW;
                            segment->segments[TOP_RIGHT]->h = segment->segments[TOP_LEFT]->h;
                            segment->segments[TOP_RIGHT]->regular = segment->segments[TOP_LEFT]->regular;
                            rect = Allocate(segment->segments[TOP_RIGHT].get(), w, h);
                        }
                        else if (h <= bottomH && !segment->segments[BOTTOM_LEFT])
                        {
                            CreateSegment(segment, BOTTOM_LEFT);
                            segment->segments[BOTTOM_LEFT]->w = segment->w;
                            segment->segments[BOTTOM_LEFT]->h = bottomH;
                            segment->segments[BOTTOM_LEFT]->regular = false;
                            rect = Allocate(segment->segments[BOTTOM_LEFT].get(), w, h);
                        }
                    }

                    return rect;
                }
            }
            return std::nullopt;
        }

        void Release(uint64_t path)
        {
            if (_rootSegment)
                Release(path, _rootSegment.get());
        }

        bool Release(uint64_t path, _Segment* segment)
        {
            if (path == 0 && segment->filled)
                return true;

            _SegmentIndex nextIndex = _SegmentIndex(path & 0b11ull);
            if (segment->segments[nextIndex])
            {
                bool released = Release(path >> 2, segment->segments[nextIndex].get());
                if (released)
                {
                    int tlw = segment->segments[TOP_LEFT]->w;
                    int tlh = segment->segments[TOP_LEFT]->h;
                    bool tlRegular = segment->segments[TOP_LEFT]->regular;

                    segment->segments[nextIndex].reset();

                    if (!segment->segments[TOP_LEFT] &&
                        !segment->segments[TOP_RIGHT] &&
                        !segment->segments[BOTTOM_LEFT] &&
                        !segment->segments[BOTTOM_RIGHT])
                    {
                        segment->clean = true;
                        return true;
                    }

                    if (segment->layout == TOP_BOTTOM_RIGHT)
                    {
                        if (!segment->segments[TOP_RIGHT])
                            segment->layout = FOUR_RECTS;
                    }
                    else if (segment->layout == LEFT_RIGHT_BOTTOM)
                    {
                        if (!segment->segments[BOTTOM_LEFT])
                            segment->layout = FOUR_RECTS;
                    }

                    if (!segment->segments[TOP_LEFT])
                    {
                        // Top left segment is assumed to always be allocated, so it cannot be left deleted
                        CreateSegment(segment, TOP_LEFT);
                        segment->segments[TOP_LEFT]->w = tlw;
                        segment->segments[TOP_LEFT]->h = tlh;
                        segment->segments[TOP_LEFT]->regular = tlRegular;
                    }
                }
            }

            return false;
        }
    };
}