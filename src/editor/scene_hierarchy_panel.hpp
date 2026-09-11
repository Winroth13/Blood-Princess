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
	Entity *selectedEntity = nullptr;

public:
	SceneHierarchyPanel() = default;
	~SceneHierarchyPanel() = default;

	bool Initialize(SceneManager &sceneManager);

	void ImGuiRender();

	void DrawEntityNode(Entity *entity);

	Entity *GetSelectedEntity() const { return this->selectedEntity; }
};