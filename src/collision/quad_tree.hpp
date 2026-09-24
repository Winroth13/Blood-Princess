#pragma once

#include "collision/Rect_2d.hpp"
#include <vector>
#include <memory>

class CollisionComponent;

struct QuadTreeItem
{
	CollisionComponent* collider = nullptr;
	Rect2D bounds{};
};

class QuadTree
{
private:
	static constexpr size_t MAX_OBJECTS = 8;
	static constexpr size_t MAX_DEPTH = 6;

	size_t depth = 0;
	Rect2D boundary{};
	std::vector<QuadTreeItem> items;
	std::unique_ptr<QuadTree> children[4];
	bool isDivided = false;

	void Subdivide();

public:
	QuadTree() = default;
	QuadTree(const Rect2D& boundary, size_t depth = 0);
	~QuadTree() = default;

	QuadTree(QuadTree&&) noexcept = default;
	QuadTree& operator=(QuadTree&&) noexcept = default;
	QuadTree(const QuadTree&) = delete;
	QuadTree& operator=(const QuadTree&) = delete;

	void Init(const Rect2D& boundary, size_t depth = 0);
	bool Insert(CollisionComponent* collider, const Rect2D& bounds);
	void Query(const Rect2D& range, std::vector<CollisionComponent*>& outResults) const;
	void Clear();

	const Rect2D& GetBoundary() const { return this->boundary; }
	void SetBoundary(const Rect2D& boundary) { this->boundary = boundary; }
};
