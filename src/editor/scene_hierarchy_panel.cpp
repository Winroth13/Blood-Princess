#include "editor/scene_hierarchy_panel.hpp"
#include "core/logging.hpp"
#include "editor/editor.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "scene/scene_manager.hpp"
#include "scene/entity.hpp"
#include "scene/component.hpp"

#include <string>
#include <sstream>
#include <inttypes.h>

bool SceneHierarchyPanel::Initialize(SceneManager &sceneManager)
{
	LogInfo("Creating Scene Hierarchy Panel...");
	LogIndent();

	this->sceneManager = &sceneManager;

	LogUnindent();

	return true;
}

void SceneHierarchyPanel::ImGuiRender()
{
	ImGui::Begin("Scene Hierarchy");
	Scene *scene = this->sceneManager->GetCurrentScene();

	if (scene)
	{
		ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen;
		std::string sceneName = scene->GetName();
		if (sceneName.empty())
		{
			sceneName = "Unnamed Scene";
		}

		if (ImGui::TreeNodeEx(sceneName.c_str(), flag))
		{
			for (auto entity : scene->GetRootEntities())
			{
				this->DrawEntityNode(entity);
			}
			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void SceneHierarchyPanel::DrawEntityNode(Entity *entity)
{
	ImGuiTreeNodeFlags flags = 
		ImGuiTreeNodeFlags_DefaultOpen | 
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_SpanAvailWidth |
		ImGuiTreeNodeFlags_DrawLinesFull;

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

	if (ImGui::IsKeyPressed(ImGuiKey_Escape))
		this->selectedEntity = nullptr;
}