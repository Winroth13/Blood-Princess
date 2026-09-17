#pragma once
class Rect2D 
{
	float minX = 0.0f;
	float maxX = 0.0f;
	float minZ = 0.0f;
	float maxZ = 0.0f;

	Rect2D() = default;
	Rect2D(float minX, float maxX, float minZ, float maxZ) : minX(minX), maxX(maxX), minZ(minZ), maxZ(maxZ){}
	bool Intersects(const Rect2D &other) const;
	bool Contains(const Rect2D& other) const;
	bool Contains(float x, float z) const;
	float GetWidth() const;
	float GetHeight() const;
	float GetCenterX() const;
	float GetCenterZ() const;
};