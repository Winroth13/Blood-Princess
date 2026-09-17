#include "Rect_2d.hpp"

bool Rect2D::Intersects(const Rect2D& other) const
{
    return !(
        this->maxX < other.minX ||
        this->minX > other.maxX ||
        this->maxZ < other.minZ ||
        this->minZ > other.maxZ
        );
}

bool Rect2D::Contains(const Rect2D& other) const
{
    return (
        this->minX <= other.minX &&
        this->maxX >= other.maxX &&
        this->minZ <= other.minZ &&
        this->maxZ >= other.maxZ);
}

bool Rect2D::Contains(float x, float z) const
{
    return (
        x >= this->minX && x <=this->maxX &&
        z >= this->minZ && z <= this->maxZ
        );
}

float Rect2D::GetWidth() const
{
    return this->maxX - this->minX;
}

float Rect2D::GetHeight() const
{
    return this->maxZ - this->minZ;
}

float Rect2D::GetCenterX() const
{
    return (maxX + minX) * 0.5f;
}

float Rect2D::GetCenterZ() const
{
    return (maxZ + minZ) * 0.5f;
}
