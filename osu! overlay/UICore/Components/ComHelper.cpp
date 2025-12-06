#include "ComHelper.h"

zcom::PointF zcom::Point::ToPointF() const
{
    return { (float)x, (float)y };
}

zcom::SizeF zcom::Size::ToSizeF() const
{
    return { (float)width, (float)height };
}

zcom::Rect zcom::Size::ToRect(Point offset) const
{
    return { offset.x, offset.y, offset.x + width, offset.y + height };
}

zcom::RectF zcom::SizeF::ToRectF(PointF offset) const
{
    return { offset.x, offset.y, offset.x + width, offset.y + height };
}

zcom::RectF zcom::Rect::ToRectF() const
{
    return { (float)left, (float)top, (float)right, (float)bottom };
}
