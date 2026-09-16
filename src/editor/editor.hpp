#pragma once
#include "editor/scene_hierarchy_panel.hpp"
#include "editor/inspector_panel.hpp"
#include "editor/asset_viewer_panel.hpp"

#include "imgui/imgui.h"
#include "imguizmo/ImGuizmo.h"

#include <DirectXMath.h>

class SceneManager;
struct AppContext;

class Editor
{
	SceneHierarchyPanel sceneHierarchyPanel;
	InspectorPanel inspectorPanel;
	AssetViewerPanel assetViewerPanel;

	ImGuizmo::OPERATION currentOperation{ ImGuizmo::OPERATION::TRANSLATE };

public:
	Editor() = default;
	~Editor() = default;

	Editor(const Editor &other) = delete;
	Editor &operator=(const Editor &other) = delete;

	bool Initialize(SceneManager &sceneManager);
	void Shutdown();

	void ImGuiRender(const AppContext &appContext);
};