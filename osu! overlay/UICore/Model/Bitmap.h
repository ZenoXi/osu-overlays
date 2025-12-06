#pragma once

#include "UICore/Components/ComHelper.h"

#include <memory>

struct ID2D1Bitmap;

namespace zcom
{
    class BitmapStorage
    {
    public:
        virtual ~BitmapStorage() {};

        virtual Size GetSize() const = 0;
        virtual bool CanBeTarget() const = 0;
        virtual ID2D1Bitmap* GetSource() const = 0;
        virtual Rect GetSourceRect() const = 0;
    };

    // Refcounted bitmap storage wrapper
    class Bitmap
    {
        std::shared_ptr<BitmapStorage> _storage;

    public:
        Bitmap(std::shared_ptr<BitmapStorage> storage) : _storage(storage) {}

        Size GetSize() const { return _storage->GetSize(); }
        bool CanBeTarget() const { return _storage->CanBeTarget(); }
        ID2D1Bitmap* GetSource() const { return _storage->GetSource(); }
        Rect GetSourceRect() const { return _storage->GetSourceRect(); }

        bool operator==(const Bitmap& other) const
        {
            return _storage.get() == other._storage.get();
        }
    };
}