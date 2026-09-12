#include "asset_viewer_panel.hpp"
#include "imgui/imgui.h"
#include "editor/imgui_draw_helpers.hpp"

#include "assets/types/audio_clip.hpp"
#include "assets/types/model.hpp"
#include "assets/types/pixel_shader.hpp"
#include "assets/types/vertex_shader.hpp"
#include "assets/types/texture_2d.hpp"

void AssetViewerPanel::ImGuiRender()
{
	if (!this->viewingAsset.expired())
	{
		ImGui::Begin("Asset Viewer");

		auto asset = viewingAsset.lock();

		ImGui::SetNextItemWidth(-1.0f);
		DrawFramedText(asset->GetPath());

		asset->DrawImGui();

		ImGui::End();
	}
}

