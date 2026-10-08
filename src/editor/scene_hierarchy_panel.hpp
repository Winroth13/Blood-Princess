#pragma once
#include "reflection/MemberVars.h"

#include <DirectXMath.h>

class SceneManager;
class Entity;
class Component;

using namespace DirectX;

class SceneHierarchyPanel
{
	SceneManager *sceneManager = nullptr;
	std::vector<Entity *> selectedEntities;

	bool multiSelect = false;

public:
	SceneHierarchyPanel() = default;
	~SceneHierarchyPanel() = default;

	bool Initialize(SceneManager &sceneManager);

	void ImGuiRender();

	void DrawEntityNode(Entity *entity);

	const bool HasSelectedEntities() const { return this->selectedEntities.size() > 0; }
	const size_t GetSelectCount() const { return this->selectedEntities.size(); }
	const std::vector<Entity*> &GetSelectedEntities() const { return this->selectedEntities; }
};