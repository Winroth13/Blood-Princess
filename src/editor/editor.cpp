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
#include "assets/types/sub_types/skeleton.hpp"
#include "components/animator_component.hpp"

#include "rendering/lines/line_utils.hpp"

#include <algorithm>

#undef min
#undef max

static XMFLOAT3 GetJointWorldPosition(const XMFLOAT4X4 &jointMatrix, const XMMATRIX &renderMatrix)
{
    XMFLOAT3 position;
    XMStoreFloat3(&position, XMVector3Transform(XMVectorSet(jointMatrix._41, jointMatrix._42, jointMatrix._43, 1.0f), renderMatrix));
    return position;
}

void Editor::DrawSkeleton(const AppContext &appContext)
{
    constexpr size_t MAX_POINTS_PER_COMMAND = 32;
    const XMFLOAT3 colour = { 1.0f, 0.0f, 1.0f };

    Scene *scene = appContext.sceneManager->GetCurrentScene();

    RenderView *primaryView = RenderUtils::GetView(appContext.renderer->GetViews(), ViewType::PRIMARY);
    if (!primaryView)
        return;

    std::vector<LineVertex> vertices;

    for (Entity *entity : scene->GetEntities()) 
    {
        if (!entity->IsActive())
            continue;

        AnimatorComponent *animator = entity->GetComponent<AnimatorComponent>();
        if (!animator || !animator->IsActive() || !animator->IsSkeletonVisible())
            continue;

        TransformComponent *transform = entity->GetComponent<TransformComponent>();
        if (!transform)
            continue;

        std::shared_ptr<Skeleton> skeleton = animator->GetSkeleton();
        if (!skeleton)
            continue;

        const std::vector<XMFLOAT4X4> &jointMatrices = animator->GetJointModelMatrices();
        const std::vector<Joint> &joints = skeleton->GetJoints();
        const XMMATRIX renderMatrix = transform->GetRenderMatrix();

        vertices.clear();
        for (size_t i = 0; i < joints.size(); ++i)
        {
            const int32_t jointIndex = static_cast<int32_t>(i);
            const int32_t parentIndex = joints[i].parentIndex;

            if (parentIndex == -1 || !skeleton->IsSkinJoint(jointIndex) || !skeleton->IsSkinJoint(parentIndex))
                continue;

            vertices.push_back({ GetJointWorldPosition(jointMatrices[parentIndex], renderMatrix), colour });
            vertices.push_back({ GetJointWorldPosition(jointMatrices[i], renderMatrix), colour });
        }

        for (size_t start = 0; start < vertices.size(); start += MAX_POINTS_PER_COMMAND)
        {
            const size_t end = std::min(start + MAX_POINTS_PER_COMMAND, vertices.size());

            LineCommand command{};
            command.points.assign(vertices.begin() + start, vertices.begin() + end);
			command.enableDepth = false;
            primaryView->queue.Submit(command);
        }
    }
}

void Editor::Update(const AppContext &appContext, float deltaTime)
{
	if (Input::IsKeyPressed(Key::F1))
	{
		ToggleEditor(appContext);
	}

	if (this->GetShowEditor())
	{
		RenderView *primaryView = RenderUtils::GetView(appContext.renderer->GetViews(), ViewType::PRIMARY);

		if (!primaryView)
			return;

		this->freeCam.Update(deltaTime);

		primaryView->cameraPosition = this->freeCam.GetPosition();
		XMStoreFloat4x4(&primaryView->viewMatrix, this->freeCam.GetViewMatrix());
	}
}

void Editor::Render(const AppContext &appContext)
{
	if (this->GetShowEditor())
		this->RenderEditor(appContext);
}

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

void Editor::RenderEditor(const AppContext &appContext)
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

	this->DrawSkeleton(appContext);

	this->GizmoRender(appContext);
	this->RenderSelectionBoundingBoxes(appContext);
}

void Editor::GizmoRender(const AppContext &appContext)
{
	uint32_t width = appContext.renderer->GetWidth();
	uint32_t height = appContext.renderer->GetHeight();
	ImGuizmo::SetRect(0, 0, static_cast<float>(width), static_cast<float>(height));

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
	{
		this->initialTransforms.clear();
		this->wasUsingGizmo = false;
		return;
	}

	if (!ImGuizmo::IsUsing())
	{
		this->wasUsingGizmo = false;

		// Find Origin
		XMVECTOR pivotV = XMVectorSet(0, 0, 0, 0);
		for (TransformComponent *transform : transforms)
		{
			XMFLOAT3 position = transform->GetWorldPosition();
			pivotV = XMVectorAdd(pivotV, XMLoadFloat3(&position));
		}
		pivotV = XMVectorScale(pivotV, 1.0f / static_cast<float>(transforms.size()));

		XMFLOAT3 pivot;
		XMStoreFloat3(&pivot, pivotV);

		XMMATRIX worldMatrix = XMMatrixTranslation(pivot.x, pivot.y, pivot.z);
		XMStoreFloat4x4(&this->initialGizmoMatrix, worldMatrix);

		this->initialTransforms.clear();
		for (const auto entity : this->sceneHierarchyPanel.GetSelectedEntities())
		{
			TransformComponent *transform = entity->GetComponent<TransformComponent>();
			if (transform)
			{
				InitialEntityTransform init{};
				init.entity = entity;
				XMStoreFloat4x4(&init.initialWorldMatrix, transform->GetWorldMatrix());
				init.initialLocalScale = transform->GetLocalScale();
				this->initialTransforms.push_back(init);
			}
		}
	}

	XMFLOAT4X4 gizmoMatrix = this->initialGizmoMatrix;

	bool manipulating = ImGuizmo::Manipulate(
		*view.m,
		*proj.m,
		this->currentOperation,
		ImGuizmo::MODE::WORLD,
		*gizmoMatrix.m
	);

	if (ImGuizmo::IsUsing())
	{
		this->wasUsingGizmo = true;

		XMMATRIX oldMatrix = XMLoadFloat4x4(&this->initialGizmoMatrix);
		XMMATRIX newMatrix = XMLoadFloat4x4(&gizmoMatrix);

		XMVECTOR det;
		XMMATRIX deltaMatrix = XMMatrixMultiply(XMMatrixInverse(&det, oldMatrix), newMatrix);

		bool isScale = (this->currentOperation & ImGuizmo::SCALE) != 0 || (this->currentOperation & ImGuizmo::SCALEU) != 0;

		XMVECTOR deltaScaleV, deltaRotV, deltaTransV;
		XMMatrixDecompose(&deltaScaleV, &deltaRotV, &deltaTransV, deltaMatrix);
		XMFLOAT3 deltaScale;
		XMStoreFloat3(&deltaScale, deltaScaleV);

		for (const auto &init : this->initialTransforms)
		{
			if (!init.entity)
				continue;

			TransformComponent *transform = init.entity->GetComponent<TransformComponent>();
			if (!transform)
				continue;

			if (isScale)
			{
				XMFLOAT3 newScale = {
					init.initialLocalScale.x * deltaScale.x,
					init.initialLocalScale.y * deltaScale.y,
					init.initialLocalScale.z * deltaScale.z
				};
				transform->SetLocalScale(newScale);
			}
			else
			{
				XMMATRIX initialWorldMatrix = XMLoadFloat4x4(&init.initialWorldMatrix);
				XMMATRIX newTransformWorldMatrix = XMMatrixMultiply(initialWorldMatrix, deltaMatrix);
				transform->SetWorldMatrix(newTransformWorldMatrix);
			}
		}
	}
	else if (this->wasUsingGizmo)
	{
		this->wasUsingGizmo = false;
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

bool Editor::IsUsingEditor()
{
	return GetShowEditor();
}

void Editor::ToggleEditor(const AppContext &appContext)
{
	this->SetShowEditor(
		!this->GetShowEditor()
	);

	if (this->GetShowEditor())
	{
		RenderView *primaryView = RenderUtils::GetView(
			appContext.renderer->GetViews(), ViewType::PRIMARY
		);

		if (primaryView)
		{
			this->freeCam.SetPosition(primaryView->cameraPosition);

			XMMATRIX viewMatrix = XMLoadFloat4x4(&primaryView->viewMatrix);
			XMMATRIX viewInvMatrix = XMMatrixInverse(nullptr, viewMatrix);

			XMFLOAT4 quat = TransformComponent::ExtractRotation(viewInvMatrix);
			
			XMFLOAT3 eulerAngles = TransformComponent::QuaternionToEulerAngles(quat);

			XMFLOAT3 eulerRadians = {
				XMConvertToRadians(eulerAngles.x),
				XMConvertToRadians(eulerAngles.y),
				XMConvertToRadians(eulerAngles.z)
			};

			this->freeCam.SetRotation(eulerRadians);
		}
	}
}

void Editor::SetShowEditor(bool value)
{
	if (value)
	{
		this->flags |= EDITOR_FLAGS_SHOW_EDITOR;
		this->lastMouseMode = Input::GetMouseMode();
		Input::SetMouseMode(MouseMode::VISIBLE);
	}
	else
	{
		this->flags &= ~EDITOR_FLAGS_SHOW_EDITOR;
		Input::SetMouseMode(this->lastMouseMode);
	}
}
