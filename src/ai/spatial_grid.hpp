#pragma once

#include <DirectXMath.h>
#include <vector>

class SpatialGrid
{
private:
	float cellSize = 2.0f;
	int gridWidth = 100;
	int gridHeight = 100;
	DirectX::XMFLOAT2 origin = { -100.0f, -100.0f };
	float arenaRadius = 22.0f;
	std::vector<int> cellHead;
	std::vector<int> nextEnemy;
	std::vector<DirectX::XMFLOAT3> positions;

	std::vector<DirectX::XMFLOAT3> trapPositions;
	std::vector<DirectX::XMFLOAT3> circlePositions;

	void InitCirclePositions();

public:
	SpatialGrid();
	SpatialGrid(float cellSize, int width, int height, DirectX::XMFLOAT2 origin);
	~SpatialGrid() = default;

	void BeginFrame(size_t enemyCount);
	void Insert(size_t index, const DirectX::XMFLOAT3 &pos);

	void InsertTrap(const DirectX::XMFLOAT3& pos);
	void ClearTraps();
	bool IsAreaClearOfTraps(const DirectX::XMFLOAT3& pos, float checkRadius) const;
	const std::vector<DirectX::XMFLOAT3>& GetTrapPositions() const { return this->trapPositions; }

	DirectX::XMVECTOR ComputeAvoidance(size_t selfIndex, float avoidanceRadius) const;
	bool IsAreaClear(const DirectX::XMFLOAT3 &pos, float checkRadius) const;

	float GetCellSize() const { return this->cellSize; }
	int GetGridWidth() const { return this->gridWidth; }
	int GetGridHeight() const { return this->gridHeight; }
	DirectX::XMFLOAT2 GetOrigin() const { return this->origin; }
};
