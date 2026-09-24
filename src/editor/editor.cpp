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

#include "core/app_context.hpp"
#include "rendering/renderer.hpp"
#include "rendering/render_utils.hpp"
#include "components/transform_component.hpp"
#include "components/box_collision_component.hpp"
#include "components/model_component.hpp"

#include "rendering/lines/line_utils.hpp"

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

void Editor::ImGuiRender(const AppContext &appContext)
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

	if (sceneHierarchyPanel.GetSelectCount() == 1)
	{
		this->inspectorPanel.SetEntity(
			sceneHierarchyPanel.GetSelectedEntities()[0]
		);
	}
	else
	{
		this->inspectorPanel.SetEntity(nullptr);
	}

	this->inspectorPanel.ImGuiRender();
	this->assetViewerPanel.ImGuiRender();

	this->GizmoRender(appContext);
	this->RenderSelectionBoundingBoxes(appContext);
}

void Editor::GizmoRender(const AppContext &appContext)
{
	if (this->sceneHierarchyPanel.GetSelectCount() == 0)
		return;

	if (ImGui::IsKeyPressed(ImGuiKey_T))
		this->currentOperation = ImGuizmo::TRANSLATE;

	if (ImGui::IsKeyPressed(ImGuiKey_R))
		this->currentOperation = ImGuizmo::ROTATE;

	if (ImGui::IsKeyPressed(ImGuiKey_E))
		this->currentOperation = ImGuizmo::SCALE;

	if (ImGui::IsKeyPressed(ImGuiKey_B))
		this->currentOperation = ImGuizmo::BOUNDS;

	// Setup
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());

	uint32_t width = appContext.renderer->GetWidth();
	uint32_t height = appContext.renderer->GetHeight();
	ImGuizmo::SetRect(0, 0, static_cast<float>(width), static_cast<float>(height));

	// Make sure we have a primary view
	std::vector<RenderView> &renderViews = appContext.renderer->GetViews();
	RenderView *primaryView = RenderUtils::GetView(renderViews, ViewType::PRIMARY);
	if (!primaryView)
		return;

	// Get View and Projection Matrices
	XMMATRIX viewT = XMLoadFloat4x4(&primaryView->viewMatrix);
	XMMATRIX projT = XMLoadFloat4x4(&primaryView->projectionMatrix);

	XMFLOAT4X4 viewF{};
	XMStoreFloat4x4(&viewF, viewT);

	XMFLOAT4X4 projF{};
	XMStoreFloat4x4(&projF, projT);

	if (this->currentOperation != ImGuizmo::BOUNDS)
	{
		RenderTransformGizmo(viewF, projF);
		return;
	}
}

void Editor::RenderTransformGizmo(const XMFLOAT4X4 &view, const XMFLOAT4X4 &proj)
{
	std::vector<TransformComponent *> transforms;
	for (const auto entity : this->sceneHierarchyPanel.GetSelectedEntities())
	{
		TransformComponent *transform = entity->GetComponent<TransformComponent>();
		if (transform)
			transforms.push_back(transform);
	}

	if (transforms.empty())
		return;

	// Find Origin
	XMVECTOR pivotV = XMVectorSet(0, 0, 0, 0);
	for (TransformComponent *transform : transforms)
	{
		XMFLOAT3 position = transform->GetWorldPosition();
		pivotV = XMVectorAdd(pivotV, XMLoadFloat3(&position));
	}
	pivotV = XMVectorScale(pivotV, 1.0f / transforms.size());

	XMFLOAT3 pivot;
	XMStoreFloat3(&pivot, pivotV);

	XMMATRIX worldMatrix = XMMatrixTranslation(pivot.x, pivot.y, pivot.z);

	XMFLOAT4X4 gizmoMatrix;
	XMStoreFloat4x4(&gizmoMatrix, worldMatrix);

	XMFLOAT4X4 oldGizmoMatrix = gizmoMatrix;

	bool manipulating = ImGuizmo::Manipulate(
		*view.m,
		*proj.m,
		this->currentOperation,
		ImGuizmo::MODE::WORLD,
		*gizmoMatrix.m
	);

	if (manipulating && ImGuizmo::IsUsing())
	{
		XMMATRIX oldMatrix = XMLoadFloat4x4(&oldGizmoMatrix);
		XMMATRIX newMatrix = XMLoadFloat4x4(&gizmoMatrix);

		XMVECTOR det;
		XMMATRIX deltaMatrix = XMMatrixMultiply(XMMatrixInverse(&det, oldMatrix), newMatrix);
		for (TransformComponent *transform : transforms)
		{
			XMMATRIX transformWorldMatrix = transform->GetWorldMatrix();
			XMMATRIX newTransformWorldMatrix = XMMatrixMultiply(transformWorldMatrix, deltaMatrix);
			transform->SetWorldMatrix(newTransformWorldMatrix);
		}
	}
}

void Editor::RenderSelectionBoundingBoxes(const AppContext &appContext)
{
	/*
	* Maybe we shouldn't render these lines like this, but oh well...
	*/
	std::vector<RenderView> &renderViews = appContext.renderer->GetViews();
	RenderView *primaryView = RenderUtils::GetView(renderViews, ViewType::PRIMARY);

	if (!primaryView)
		return;

	/* Right now we only draw bounding boxes if it has a model. */
	for (Entity *entity : this->sceneHierarchyPanel.GetSelectedEntities())
	{
		TransformComponent *transformComp = entity->GetComponent<TransformComponent>();

		if (!transformComp)
			continue;

		ModelComponent *modelComp = entity->GetComponent<ModelComponent>();

		if (!modelComp)
			return;

		std::vector<LineVertex> vertices{};

		LineUtils::GetBoxVertices(
			vertices,
			modelComp->GetModel()->GetBoundingBox(),
			transformComp->GetRenderMatrix(),
			{ 1.0f, 0.75f, 0.0f }
		);

		LineCommand command = {};
		command.points = vertices;
		primaryView->queue.Submit(command);
	}
}
