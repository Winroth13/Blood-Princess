#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "assets/asset.hpp"
#include "SDL3/SDL.h"

#include <string>

static void BeginMemberDraw(const std::string &name)
{
	constexpr float COLUMN_WIDTH = 125.0f;
	ImGui::PushID(name.c_str());

	ImGui::Columns(2, 0, false);
	ImGui::SetColumnWidth(0, COLUMN_WIDTH);
	ImGui::Text(name.c_str());
	ImGui::NextColumn();
}

static void EndMemberDraw()
{
	ImGui::Columns(1);
	ImGui::PopID();
}

static void DrawFloat(const std::string &name, float &value, const std::string& unit = "", float min = 0.0f, float max = 0.0f, float speed = 0.1f, bool angle = false)
{
	BeginMemberDraw(name);

	if (!angle)
	{
		ImGui::DragFloat("##value", &value, speed, min, max, "%.2f");
	}
	else
	{
		float minDeg = DirectX::XMConvertToDegrees(min);
		float maxDeg = DirectX::XMConvertToDegrees(max);
		float degrees = DirectX::XMConvertToDegrees(value);
		if (ImGui::DragFloat("##value", &degrees, speed, minDeg, maxDeg, "%.2f"))
		{
			value = DirectX::XMConvertToRadians(degrees);
		}
	}

	ImGui::SameLine();
	ImGui::Text(unit.c_str());

	EndMemberDraw();
}

static void DrawFloat3(const std::string &name, XMFLOAT3 &values, float resetValue = 0.0f)
{
	ImGuiIO &io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0];

	BeginMemberDraw(name);

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });
	ImGui::PushStyleColor(ImGuiCol_SeparatorActive, { 0,0,0,0 });

	float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	/* X */
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("X", buttonSize))
	{
		values.x = resetValue;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();


	/* Y */
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.5f, 0.1f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.15f, 0.6f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.5f, 0.1f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Y", buttonSize))
	{
		values.y = resetValue;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();


	/* Z */
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Z", buttonSize))
	{
		values.z = resetValue;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleColor(1);
	ImGui::PopStyleVar();

	EndMemberDraw();
}

static void DrawFloat4(const std::string &name, XMFLOAT4 &values, float resetValue = 0.0f)
{
	ImGuiIO &io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0];

	BeginMemberDraw(name);

	ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });

	float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	/* X */
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("X", buttonSize))
	{
		values.x = resetValue;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();


	/* Y */
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.5f, 0.1f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.15f, 0.6f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.5f, 0.1f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Y", buttonSize))
	{
		values.y = resetValue;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();


	/* Z */
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Z", buttonSize))
	{
		values.z = resetValue;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	/* W */
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.4f, 0.4f, 0.4f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.4f, 0.4f, 0.4f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("W", buttonSize))
	{
		values.w = resetValue;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##W", &values.w, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	EndMemberDraw();
}

static void DrawColor3(const std::string &name, XMFLOAT3 &values)
{
	BeginMemberDraw(name);
	ImGui::ColorEdit3("##color", &values.x);
	EndMemberDraw();
}

static void DrawColor4(const std::string &name, XMFLOAT4 &values)
{
	BeginMemberDraw(name);
	ImGui::ColorEdit4("##color", &values.x);
	EndMemberDraw();
}

static void DrawAsset(const std::string &name, std::shared_ptr<Asset> asset)
{
	BeginMemberDraw(name);

	std::string path = "<NONE>";
	if (asset)
		path = asset->GetPath();

	static char textBuffer[128] = "";
	snprintf(textBuffer, sizeof(textBuffer), "%s", path.c_str());
	ImGui::InputText("##path", textBuffer, IM_ARRAYSIZE(textBuffer), ImGuiInputTextFlags_ReadOnly);

	ImGui::SameLine();
	float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
	
	if (ImGui::Button("...", buttonSize))
	{
		
	}

	EndMemberDraw();
}

static void DrawCheckbox(const std::string &name, bool &value)
{
	BeginMemberDraw(name);
	ImGui::Checkbox("##value", &value);
	EndMemberDraw();
}