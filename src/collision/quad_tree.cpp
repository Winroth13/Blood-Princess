#include "collision/quad_tree.hpp"
#include "collision/collision_component.hpp"

QuadTree::QuadTree(const Rect2D& boundary, size_t depth)
	: boundary(boundary), depth(depth), isDivided(false)
{
}

void QuadTree::Init(const Rect2D& boundary, size_t depth)
{
	this->Clear();
	this->boundary = boundary;
	this->depth = depth;
	this->isDivided = false;
}

void QuadTree::Subdivide()
{
	float minX = this->boundary.minX;
	float maxX = this->boundary.maxX;
	float minZ = this->boundary.minZ;
	float maxZ = this->boundary.maxZ;

	float midX = this->boundary.GetCenterX();
	float midZ = this->boundary.GetCenterZ();

	this->children[0] = std::make_unique<QuadTree>(Rect2D(minX, midX, midZ, maxZ), this->depth + 1);
	this->children[1] = std::make_unique<QuadTree>(Rect2D(midX, maxX, midZ, maxZ), this->depth + 1);
	this->children[2] = std::make_unique<QuadTree>(Rect2D(minX, midX, minZ, midZ), this->depth + 1);
	this->children[3] = std::make_unique<QuadTree>(Rect2D(midX, maxX, minZ, midZ), this->depth + 1);

	this->isDivided = true;
}

bool QuadTree::Insert(CollisionComponent* collider, const Rect2D& bounds)
{
	if (!collider || !this->boundary.Intersects(bounds))
	{
		return false;
	}

	if (!this->isDivided)
	{
		if (this->items.size() < MAX_OBJECTS || this->depth >= MAX_DEPTH)
		{
			this->items.push_back({ collider, bounds });
			return true;
		}

		this->Subdivide();

		std::vector<QuadTreeItem> existingItems = std::move(this->items);
		this->items.clear();

		for (const QuadTreeItem& item : existingItems)
		{
			for (int i = 0; i < 4; ++i)
			{
				if (this->children[i]->boundary.Intersects(item.bounds))
				{
					this->children[i]->Insert(item.collider, item.bounds);
				}
			}
		}
	}

	bool inserted = false;
	for (int i = 0; i < 4; ++i)
	{
		if (this->children[i]->boundary.Intersects(bounds))
		{
			if (this->children[i]->Insert(collider, bounds))
			{
				inserted = true;
			}
		}
	}

	return inserted;
}

void QuadTree::Query(const Rect2D& range, std::vector<CollisionComponent*>& outResults) const
{
	if (!this->boundary.Intersects(range))
	{
		return;
	}

	for (const QuadTreeItem& item : this->items)
	{
		if (item.collider && item.bounds.Intersects(range))
		{
			outResults.push_back(item.collider);
		}
	}

	if (this->isDivided)
	{
		for (int i = 0; i < 4; ++i)
		{
			this->children[i]->Query(range, outResults);
		}
	}
}

void QuadTree::Clear()
{
	this->items.clear();
	for (int i = 0; i < 4; ++i)
	{
		this->children[i].reset();
	}
	this->isDivided = false;
}
