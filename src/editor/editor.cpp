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

	this->inspectorPanel.SetEntity(sceneHierarchyPanel.GetSelectedEntity());
	this->inspectorPanel.ImGuiRender();

	this->assetViewerPanel.ImGuiRender();

	/* ImGuizmo */
	Entity *entity = sceneHierarchyPanel.GetSelectedEntity();
	if (entity != nullptr)
	{
		ImGuizmo::SetOrthographic(true);
		ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());

		uint32_t width = appContext.renderer->GetWidth();
		uint32_t height = appContext.renderer->GetHeight();
		ImGuizmo::SetRect(0, 0, static_cast<float>(width), static_cast<float>(height));

		auto transformComponent = entity->GetComponent<TransformComponent>();
		if (transformComponent)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_T))
				this->currentOperation = ImGuizmo::TRANSLATE;
			if (ImGui::IsKeyPressed(ImGuiKey_R))
				this->currentOperation = ImGuizmo::ROTATE;
			if (ImGui::IsKeyPressed(ImGuiKey_E))
				this->currentOperation = ImGuizmo::SCALE;
			if (ImGui::IsKeyPressed(ImGuiKey_B))
				this->currentOperation = ImGuizmo::BOUNDS;


			std::vector<RenderView> &renderViews = appContext.renderer->GetViews();
			RenderView *primaryView = RenderUtils::GetView(renderViews, ViewType::PRIMARY);

			XMFLOAT4X4 worldMatrix;
			XMStoreFloat4x4(&worldMatrix, transformComponent->GetWorldMatrix());

			XMMATRIX viewT = XMLoadFloat4x4(&primaryView->viewMatrix);
			XMMATRIX projT = XMLoadFloat4x4(&primaryView->projectionMatrix);

			XMFLOAT4X4 viewF{};
			XMStoreFloat4x4(&viewF, viewT);

			XMFLOAT4X4 projF{};
			XMStoreFloat4x4(&projF, projT);


			if (entity->HasComponent<BoxCollisionComponent>() && this->currentOperation == ImGuizmo::BOUNDS)
			{
				auto boxCollisionComponent = entity->GetComponent<BoxCollisionComponent>();

				XMFLOAT3 localExtents = boxCollisionComponent->GetExtents();
				XMFLOAT3 localCenter = boxCollisionComponent->GetCenterOffset();

				float bounds[6] = {
					localCenter.x - localExtents.x,
					localCenter.y - localExtents.y,
					localCenter.z - localExtents.z,

					localCenter.x + localExtents.x,
					localCenter.y + localExtents.y,
					localCenter.z + localExtents.z
				};

				constexpr int MIN_X = 0;
				constexpr int MIN_Y = 1;
				constexpr int MIN_Z = 2;

				constexpr int MAX_X = 3;
				constexpr int MAX_Y = 4;
				constexpr int MAX_Z = 5;

				ImGuizmo::Manipulate(
					*viewF.m,
					*projF.m,
					this->currentOperation,
					ImGuizmo::MODE::LOCAL,
					*worldMatrix.m,
					nullptr,
					nullptr,
					bounds,
					nullptr
				);

				if (ImGuizmo::IsUsing())
				{
					
					XMFLOAT3 newExtents;
					newExtents.x = std::abs(bounds[MAX_X] - bounds[MIN_X]) / 2.0f;
					newExtents.y = std::abs(bounds[MAX_Y] - bounds[MIN_Y]) / 2.0f;
					newExtents.z = std::abs(bounds[MAX_Z] - bounds[MIN_Z]) / 2.0f;

					XMFLOAT3 newCenter;
					newCenter.x = (bounds[MAX_X] + bounds[MIN_X]) / 2.0f;
					newCenter.y = (bounds[MAX_Y] + bounds[MIN_Y]) / 2.0f;
					newCenter.z = (bounds[MAX_Z] + bounds[MIN_Z]) / 2.0f;

					boxCollisionComponent->SetExtents(newExtents);
					boxCollisionComponent->SetCenterOffset(newCenter);

					
					float translate[3], rotation[3], scale[3];
					ImGuizmo::DecomposeMatrixToComponents(*worldMatrix.m, translate, rotation, scale);

					transformComponent->SetLocalPosition(translate[0], translate[1], translate[2]);

					boxCollisionComponent->UpdateWorldOBB();
				}

			}
			else
			{
				XMFLOAT3 originalRotation = transformComponent->GetLocalRotation();

				ImGuizmo::Manipulate(
					*viewF.m,
					*projF.m,
					this->currentOperation,
					ImGuizmo::MODE::WORLD,
					*worldMatrix.m
				);

				float translate[3] = { -1, -1, -1 };
				float rotation[3] = { -1, -1, -1 };
				float scale[3] = { -1, -1, -1 };

				XMMATRIX worldMat = XMLoadFloat4x4(&worldMatrix);
				XMFLOAT4 quat = transformComponent->ExtractRotation(worldMat);

				ImGuizmo::DecomposeMatrixToComponents(*worldMatrix.m, translate, rotation, scale);

				if (ImGuizmo::IsUsing())
				{
					if (this->currentOperation == ImGuizmo::OPERATION::TRANSLATE)
					{
						transformComponent->SetLocalPosition(
							translate[0],
							translate[1],
							translate[2]
						);
					}
					else if (this->currentOperation == ImGuizmo::OPERATION::ROTATE)
					{
						transformComponent->SetLocalRotationQuaternion(quat);
					}
					else if (this->currentOperation == ImGuizmo::OPERATION::SCALE)
					{
						transformComponent->SetLocalScale(
							scale[0],
							scale[1],
							scale[2]
						);
					}
				}
			}
		}
	}
}