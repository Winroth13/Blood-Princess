#include "editor/scene_hierarchy_panel.hpp"
#include "core/logging.hpp"
#include "editor/editor.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "scene/scene_manager.hpp"
#include "scene/entity.hpp"
#include "reflection/reflection_hashes.hpp"
#include "scene/component.hpp"

#include <string>
#include <sstream>
#include <inttypes.h>

bool SceneHierarchyPanel::Initialize(SceneManager &sceneManager)
{
	LogInfo("Creating Scene Hierarchy...");
	LogIndent();

	this->sceneManager = &sceneManager;

	LogUnindent();

	return true;
}

void SceneHierarchyPanel::ImGuiRender()
{
	ImGui::Begin("Scene Hierarchy");

	ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen;

	if (ImGui::TreeNodeEx("Scene", flag))
	{
		Scene *scene = this->sceneManager->GetCurrentScene();
		for (auto entity : scene->GetRootEntities())
		{
			this->DrawEntityNode(entity);
		}
		ImGui::TreePop();
	}

	ImGui::End();
}

void SceneHierarchyPanel::DrawEntityNode(Entity *entity)
{
	ImGuiTreeNodeFlags flags = 
		ImGuiTreeNodeFlags_DefaultOpen | 
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_SpanAvailWidth;

	if (entity->GetChildren().size() == 0)
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	if (entity == this->selectedEntity)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	if (ImGui::TreeNodeEx(entity->GetName().c_str(), flags))
	{
		if (ImGui::IsItemClicked())
		{
			this->selectedEntity = entity;
		}

		for (auto child : entity->GetChildren())
		{
			DrawEntityNode(child);
		}
		ImGui::TreePop();
	}
}