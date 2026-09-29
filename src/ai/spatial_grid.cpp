#include "ai/spatial_grid.hpp"

#include <algorithm>
#include <cmath>

using namespace DirectX;

SpatialGrid::SpatialGrid(float cellSize, int width, int height, DirectX::XMFLOAT2 origin)
	: cellSize(cellSize), gridWidth(width), gridHeight(height), origin(origin)
{
	this->cellHead.resize(width * height, -1);
}

void SpatialGrid::BeginFrame(size_t enemyCount)
{
	std::fill(this->cellHead.begin(), this->cellHead.end(), -1);

	this->nextEnemy.resize(enemyCount, -1);
	this->positions.resize(enemyCount);
}

void SpatialGrid::Insert(size_t index, const DirectX::XMFLOAT3& pos)
{
	if (index >= this->positions.size())
	{
		return;
	}

	this->positions[index] = pos;

	int gx = static_cast<int>((pos.x - this->origin.x) / this->cellSize);
	int gz = static_cast<int>((pos.z - this->origin.y) / this->cellSize);

	
	if (gx >= 0 && gx < this->gridWidth && gz >= 0 && gz < this->gridHeight)
	{
		int cellIndex = gz * this->gridWidth + gx;

		this->nextEnemy[index] = this->cellHead[cellIndex];
		this->cellHead[cellIndex] = static_cast<int>(index);
	}
}

DirectX::XMVECTOR SpatialGrid::ComputeAvoidance(size_t selfIndex, float avoidanceRadius) const
{
	XMVECTOR separation = XMVectorZero();

	if (selfIndex >= this->positions.size())
	{
		return separation;
	}

	const XMFLOAT3& myPos = this->positions[selfIndex];
	XMVECTOR myPosVec = XMLoadFloat3(&myPos);
	float radiusSq = avoidanceRadius * avoidanceRadius;

	int centerGx = static_cast<int>((myPos.x - this->origin.x) / this->cellSize);
	int centerGz = static_cast<int>((myPos.z - this->origin.y) / this->cellSize);

	for (int dz = -1; dz <= 1; ++dz)
	{
		int gz = centerGz + dz;
		if (gz < 0 || gz >= this->gridHeight)
		{
			continue;
		}

		for (int dx = -1; dx <= 1; ++dx)
		{
			int gx = centerGx + dx;
			if (gx < 0 || gx >= this->gridWidth)
			{
				continue;
			}

			int cellIndex = gz * this->gridWidth + gx;
			int otherIndex = this->cellHead[cellIndex];

			while (otherIndex != -1)
			{
				if (static_cast<size_t>(otherIndex) != selfIndex)
				{
					XMVECTOR otherPosVec = XMLoadFloat3(&this->positions[otherIndex]);
					XMVECTOR diff = XMVectorSubtract(myPosVec, otherPosVec);

					diff = XMVectorSetY(diff, 0.0f);

					float distSq = XMVectorGetX(XMVector3LengthSq(diff));
					if (distSq > 0.0001f && distSq < radiusSq)
					{
						float dist = sqrtf(distSq);

						float strength = (avoidanceRadius - dist) / avoidanceRadius;
						XMVECTOR push = XMVectorScale(XMVector3Normalize(diff), strength);
						separation = XMVectorAdd(separation, push);
					}
				}

				otherIndex = this->nextEnemy[otherIndex];
			}
		}
	}

	return separation;
}

bool SpatialGrid::IsAreaClear(const DirectX::XMFLOAT3& pos, float checkRadius) const
{
	XMVECTOR testPosVec = XMLoadFloat3(&pos);
	float radiusSq = checkRadius * checkRadius;

	int centerGx = static_cast<int>((pos.x - this->origin.x) / this->cellSize);
	int centerGz = static_cast<int>((pos.z - this->origin.y) / this->cellSize);

	for (int dz = -1; dz <= 1; ++dz)
	{
		int gz = centerGz + dz;
		if (gz < 0 || gz >= this->gridHeight)
		{
			continue;
		}

		for (int dx = -1; dx <= 1; ++dx)
		{
			int gx = centerGx + dx;
			if (gx < 0 || gx >= this->gridWidth)
			{
				continue;
			}

			int cellIndex = gz * this->gridWidth + gx;
			int enemyIndex = this->cellHead[cellIndex];

			while (enemyIndex != -1)
			{
				XMVECTOR otherPosVec = XMLoadFloat3(&this->positions[enemyIndex]);
				XMVECTOR diff = XMVectorSubtract(testPosVec, otherPosVec);
				diff = XMVectorSetY(diff, 0.0f);

				if (XMVectorGetX(XMVector3LengthSq(diff)) < radiusSq)
				{
					return false; 
				}

				enemyIndex = this->nextEnemy[enemyIndex];
			}
		}
	}

	return true;
}