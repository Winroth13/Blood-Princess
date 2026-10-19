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
#include <format>

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
	this->multiSelect = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);

	if (ImGui::IsKeyPressed(ImGuiKey_Escape))
		this->selectedEntities.clear();

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
				this->DrawEntityNode(entity);
			
			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void SceneHierarchyPanel::DrawEntityNode(Entity *entity)
{
	ImGuiTreeNodeFlags flags = 
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_SpanAvailWidth |
		ImGuiTreeNodeFlags_DrawLinesFull;

	auto it = std::find(selectedEntities.begin(), selectedEntities.end(), entity);

	if (entity->GetChildren().size() == 0)
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	if (it != selectedEntities.end())
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	if (!entity->IsActive())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, { 0.5f, 0.5f, 0.5f, 0.5f });
	}

	std::string name = std::format("{}##{}", entity->GetName(), (uint64_t)entity);

	bool open = ImGui::TreeNodeEx(name.c_str(), flags);

	if (ImGui::IsItemClicked())
	{
		if (this->multiSelect)
		{
			if (it != selectedEntities.end())
			{
				selectedEntities.erase(it);
			}
			else
			{
				selectedEntities.push_back(entity);
			}
		}
		else
		{
			this->selectedEntities.clear();
			selectedEntities.push_back(entity);
		}
	}

	if (open)
	{
		for (auto child : entity->GetChildren())
		{
			DrawEntityNode(child);
		}
		ImGui::TreePop();
	}

	if (!entity->IsActive())
	{
		ImGui::PopStyleColor();
	}
}