#include "editor/editor.hpp"
#include "core/logging.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "scene/scene_manager.hpp"
#include "scene/entity.hpp"
#include "scene/component.hpp"
#include "editor/scene_hierarchy_panel.hpp"
#include "editor/asset_viewer_panel.hpp"
#include <sstream>
#include <inttypes.h>

bool Editor::Initialize(SceneManager &sceneManager)
{
	LogInfo("Creating Editor...");
	LogIndent();

	if (!sceneHierarchyPanel.Initialize(sceneManager))
	{
		LogError("Failed to create Scene Hierarchy Panel");
		return false;
	}

	if (!inspectorPanel.Initialize(this->assetViewerPanel))
	{
		LogError("Failed to create Asset Viewer Panel");
		return false;
	}

	LogUnindent();

	return true;
}

void Editor::Shutdown()
{
}

void Editor::ImGuiRender()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	this->sceneHierarchyPanel.ImGuiRender();

	this->inspectorPanel.SetEntity(sceneHierarchyPanel.GetSelectedEntity());
	this->inspectorPanel.ImGuiRender();

	this->assetViewerPanel.ImGuiRender();
}