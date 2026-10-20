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

void SpatialGrid::InsertTrap(const DirectX::XMFLOAT3& pos)
{
    this->trapPositions.push_back(pos);
}

void SpatialGrid::ClearTraps()
{
    this->trapPositions.clear();
}

bool SpatialGrid::IsAreaClearOfTraps(const DirectX::XMFLOAT3 & pos, float checkRadius) const
{
    // 1. Arena boundary check
    float distFromCenterSq = pos.x * pos.x + pos.z * pos.z;
    if (distFromCenterSq > (this->arenaRadius * this->arenaRadius))
    {
        return false;
    }
    // 2. Only check if another trap is within checkRadius
    float checkRadiusSq = checkRadius * checkRadius;
    for (const auto& trapPos : this->trapPositions)
    {
        float dx = pos.x - trapPos.x;
        float dz = pos.z - trapPos.z;
        if ((dx * dx + dz * dz) < checkRadiusSq)
        {
            return false;
        }
    }
    return true;
}

DirectX::XMVECTOR SpatialGrid::ComputeAvoidance(size_t selfIndex, float avoidanceRadius) const
{
    XMVECTOR separation = XMVectorZero();

    if (selfIndex >= this->positions.size() || avoidanceRadius <= 0.0f)
        return separation;

    const XMFLOAT3& myPos = this->positions[selfIndex];
    XMVECTOR myPosVec = XMLoadFloat3(&myPos);
    const float radiusSq = avoidanceRadius * avoidanceRadius;

    const float personalSpace = 0.5f;
    const float personalSpaceSq = personalSpace * personalSpace;

    const int centerGx = static_cast<int>((myPos.x - this->origin.x) / this->cellSize);
    const int centerGz = static_cast<int>((myPos.z - this->origin.y) / this->cellSize);


    const int cellRange = static_cast<int>(std::ceil(avoidanceRadius / this->cellSize));

    for (int dz = -cellRange; dz <= cellRange; ++dz)
    {
        const int gz = centerGz + dz;
        if (gz < 0 || gz >= this->gridHeight) continue;

        for (int dx = -cellRange; dx <= cellRange; ++dx)
        {
            const int gx = centerGx + dx;
            if (gx < 0 || gx >= this->gridWidth) continue;

            const int cellIndex = gz * this->gridWidth + gx;
            int otherIndex = this->cellHead[cellIndex];

            while (otherIndex != -1)
            {
                if (static_cast<size_t>(otherIndex) != selfIndex)
                {
                    XMVECTOR otherPosVec = XMLoadFloat3(&this->positions[otherIndex]);
                    XMVECTOR diff = XMVectorSubtract(myPosVec, otherPosVec);
                    diff = XMVectorSetY(diff, 0.0f);

                    const float distSq = XMVectorGetX(XMVector3LengthSq(diff));
                    if (distSq > 1e-6f && distSq < radiusSq)
                    {
                        const float dist = sqrtf(distSq);
                        const XMVECTOR dir = XMVectorScale(diff, 1.0f / dist);

                        float strength;
                        if (distSq < personalSpaceSq)
                        {
                            strength = 1.0f;
                        }
                        else
                        {
                            float t = (dist - personalSpace) /
                                (avoidanceRadius - personalSpace);
                            t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
                            const float smooth = t * t * (3.0f - 2.0f * t);
                            strength = 1.0f - smooth;
                        }

                        separation = XMVectorAdd(separation,
                            XMVectorScale(dir, strength));
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

    float distFromCenterSq = pos.x * pos.x + pos.z * pos.z;
    if (distFromCenterSq > (this->arenaRadius * this->arenaRadius))
    {
        return false;
    }

    float radiusSq = checkRadius * checkRadius;


    int centerGx = static_cast<int>((pos.x - this->origin.x) / this->cellSize);
    int centerGz = static_cast<int>((pos.z - this->origin.y) / this->cellSize);
    int cellRange = static_cast<int>(ceilf(checkRadius / this->cellSize));

    XMVECTOR testPosVec = DirectX::XMLoadFloat3(&pos);

    for (int dz = -cellRange; dz <= cellRange; ++dz)
    {
        int gz = centerGz + dz;
        if (gz < 0 || gz >= this->gridHeight)
            continue;

        for (int dx = -cellRange; dx <= cellRange; ++dx)
        {
            int gx = centerGx + dx;
            if (gx < 0 || gx >= this->gridWidth)
                continue;

            int cellIndex = gz * this->gridWidth + gx;
            int enemyIndex = this->cellHead[cellIndex];

            while (enemyIndex != -1)
            {
                DirectX::XMVECTOR otherPosVec = DirectX::XMLoadFloat3(&this->positions[enemyIndex]);
                DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(testPosVec, otherPosVec);
                diff = DirectX::XMVectorSetY(diff, 0.0f);

                if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(diff)) < radiusSq)
                {
                    return false;
                }

                enemyIndex = this->nextEnemy[enemyIndex];
            }
        }
    }

    return true;
}