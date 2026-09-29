#pragma once
#include "editor/scene_hierarchy_panel.hpp"
#include "editor/inspector_panel.hpp"
#include "editor/asset_viewer_panel.hpp"

#include "imgui/imgui.h"
#include "imguizmo/ImGuizmo.h"

#include <DirectXMath.h>

class SceneManager;
struct AppContext;
class Entity;

struct InitialEntityTransform
{
	Entity *entity{ nullptr };
	DirectX::XMFLOAT4X4 initialWorldMatrix{};
	DirectX::XMFLOAT3 initialLocalScale{ 1.0f, 1.0f, 1.0f };
};

class Editor
{
	SceneHierarchyPanel sceneHierarchyPanel;
	InspectorPanel inspectorPanel;
	AssetViewerPanel assetViewerPanel;

	ImGuizmo::OPERATION currentOperation{ ImGuizmo::OPERATION::TRANSLATE };

	std::vector<InitialEntityTransform> initialTransforms;
	DirectX::XMFLOAT4X4 initialGizmoMatrix{};
	bool wasUsingGizmo{ false };

    void DrawSkeleton(const AppContext &appContext);


public:
	Editor() = default;
	~Editor() = default;

	Editor(const Editor &other) = delete;
	Editor &operator=(const Editor &other) = delete;

	bool Initialize(SceneManager &sceneManager);
	void Shutdown();

	void ImGuiRender(const AppContext &appContext);
	void GizmoRender(const AppContext &appContext);

	void RenderTransformGizmo(const XMFLOAT4X4& view, const XMFLOAT4X4 &proj);

	void RenderSelectionBoundingBoxes(const AppContext &appContext);
};