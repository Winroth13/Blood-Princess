#pragma once
#include "editor/scene_hierarchy_panel.hpp"
#include "editor/inspector_panel.hpp"
#include "editor/asset_viewer_panel.hpp"
#include "editor/free_cam.hpp"
#include "editor/statistics_panel.hpp"
#include "editor/settings_panel.hpp"

#include "imgui/imgui.h"
#include "imguizmo/ImGuizmo.h"

#include <DirectXMath.h>

#include "input/input.hpp"

class SceneManager;
struct AppContext;
class Entity;

enum EditorFlags
{
	EDITOR_FLAGS_SHOW_EDITOR = 1
};

class Editor
{
	MouseMode lastMouseMode;
	FreeCam freeCam;

	SceneHierarchyPanel sceneHierarchyPanel;
	InspectorPanel inspectorPanel;
	AssetViewerPanel assetViewerPanel;
    StatisticsPanel statisticsPanel;
    SettingsPanel settingsPanel;

	uint32_t flags;

	ImGuizmo::OPERATION currentOperation{ ImGuizmo::OPERATION::TRANSLATE };

    void DrawSkeleton(const AppContext &appContext);

public:
	Editor() = default;
	~Editor() = default;

	Editor(const Editor &other) = delete;
	Editor &operator=(const Editor &other) = delete;

	void Update(const AppContext &appContext, float deltaTime);
	void Render(const AppContext &appContext);

	bool Initialize(SceneManager &sceneManager);
	void Shutdown();

	void RenderEditor(const AppContext &appContext);
	void GizmoRender(const AppContext &appContext);

	void RenderTransformGizmo(const XMFLOAT4X4& view, const XMFLOAT4X4 &proj);

	void RenderSelectionBoundingBoxes(const AppContext &appContext);

	bool IsUsingEditor();

	void ToggleEditor(const AppContext &context);

	void SetShowEditor(bool value);
	bool GetShowEditor() const { return (this->flags & EDITOR_FLAGS_SHOW_EDITOR) == EDITOR_FLAGS_SHOW_EDITOR; }
};