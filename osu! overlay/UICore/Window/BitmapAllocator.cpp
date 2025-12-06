#include "BitmapAllocator.h"

void zwnd::BitmapSegment::Release()
{
    _allocator->Release(*this);
}
