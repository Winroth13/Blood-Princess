#include "assets/asset.hpp"
#include "core/logging.hpp"
#include "imgui/imgui.h"

void Asset::DrawImGui()
{
	ImGui::TextColored(ImVec4{ 1, 1, 0, 1 }, "This asset doesn't support being viewed in the Asset Viewer");
}

void Asset::SetPath(const std::string &path)
{
	this->path = path;
}

std::string Asset::GetPath() const
{
	return this->path;
}
