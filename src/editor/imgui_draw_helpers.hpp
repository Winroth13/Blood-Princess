#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "assets/asset.hpp"

#include <DirectXMath.h>
using namespace DirectX;

#include <string>

/*
*	Every Draw* helper that edits a value returns true on the frames where the value
*	was actually modified, so the caller can fire the property changed callbacks.
*	Returning the flag instead of diffing the bytes keeps drag operations precise:
*	a drag that ends up on the same value does not report a change.
*/

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

static void DrawFramedText(const std::string &text)
{
    static char textBuffer[128] = "";
    snprintf(textBuffer, sizeof(textBuffer), "%s", text.c_str());
    ImGui::InputText("##framedText", textBuffer, IM_ARRAYSIZE(textBuffer), ImGuiInputTextFlags_ReadOnly);
}

static bool DrawFloat(const std::string &name, float &value, const std::string& unit = "", float min = 0.0f, float max = 0.0f, float speed = 0.1f, bool angle = false)
{
    BeginMemberDraw(name);

    bool changed = false;

    if (!angle)
    {
        changed = ImGui::DragFloat("##value", &value, speed, min, max, "%.2f");
    }
    else
    {
        float minDeg = DirectX::XMConvertToDegrees(min);
        float maxDeg = DirectX::XMConvertToDegrees(max);
        float degrees = DirectX::XMConvertToDegrees(value);
        if (ImGui::DragFloat("##value", &degrees, speed, minDeg, maxDeg, "%.2f"))
        {
            value = DirectX::XMConvertToRadians(degrees);
            changed = true;
        }
    }

    ImGui::SameLine();
    ImGui::Text(unit.c_str());

    EndMemberDraw();

    return changed;
}

static bool DrawInt(const std::string &name, int &value, const std::string &unit = "", int min = 0, int max = 0, float speed = 1.0f)
{
    BeginMemberDraw(name);

    bool changed = false;

    changed = ImGui::DragInt("##value", &value, speed, min, max);

    ImGui::SameLine();
    ImGui::Text(unit.c_str());

    EndMemberDraw();

    return changed;
}

static bool DrawFloat3(const std::string &name, XMFLOAT3 &values, float resetValue = 0.0f, bool isAngle = false)
{
    ImGuiIO &io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

    BeginMemberDraw(name);

    bool changed = false;

    XMFLOAT3 newValues = values;

    if (isAngle)
    {
        newValues.x = XMConvertToDegrees(values.x);
        newValues.y = XMConvertToDegrees(values.y);
        newValues.z = XMConvertToDegrees(values.z);
    }

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
        newValues.x = resetValue;
        changed = true;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    changed |= ImGui::DragFloat("##X", &newValues.x, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();


    /* Y */
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.5f, 0.1f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.15f, 0.6f, 0.15f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.5f, 0.1f, 1.0f });
    ImGui::PushFont(boldFont);
    if (ImGui::Button("Y", buttonSize))
    {
        newValues.y = resetValue;
        changed = true;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    changed |= ImGui::DragFloat("##Y", &newValues.y, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();


    /* Z */
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
    ImGui::PushFont(boldFont);
    if (ImGui::Button("Z", buttonSize))
    {
        newValues.z = resetValue;
        changed = true;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    changed |= ImGui::DragFloat("##Z", &newValues.z, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();

    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar();

    EndMemberDraw();

    if (isAngle)
    {
        newValues.x = XMConvertToRadians(newValues.x);
        newValues.y = XMConvertToRadians(newValues.y);
        newValues.z = XMConvertToRadians(newValues.z);
    }

    values = newValues;

    return changed;
}

static bool DrawFloat4(const std::string &name, XMFLOAT4 &values, float resetValue = 0.0f)
{
    ImGuiIO &io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[0];

    BeginMemberDraw(name);

    bool changed = false;

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
        changed = true;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    changed |= ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
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
        changed = true;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    changed |= ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
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
        changed = true;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    changed |= ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
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
        changed = true;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    changed |= ImGui::DragFloat("##W", &values.w, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();

    ImGui::PopStyleVar();

    EndMemberDraw();

    return changed;
}

static bool DrawColor3(const std::string &name, XMFLOAT3 &values)
{
    BeginMemberDraw(name);
    const bool changed = ImGui::ColorEdit3("##color", &values.x);
    EndMemberDraw();
    return changed;
}

static bool DrawColor4(const std::string &name, XMFLOAT4 &values)
{
    BeginMemberDraw(name);
    const bool changed = ImGui::ColorEdit4("##color", &values.x);
    EndMemberDraw();
    return changed;
}

static bool DrawAsset(const std::string &name, std::shared_ptr<Asset> asset)
{
    BeginMemberDraw(name);

    bool pressed = false;

    std::string path = "<NONE>";
    if (asset)
        path = asset->GetPath();

    DrawFramedText(path);

    ImGui::SameLine();
    float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
    ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

    if (ImGui::Button("...", buttonSize))
    {
        pressed = true;
    }

    EndMemberDraw();

    return pressed;
}

static bool DrawCheckbox(const std::string &name, bool &value)
{
    BeginMemberDraw(name);
    const bool changed = ImGui::Checkbox("##value", &value);
    EndMemberDraw();
    return changed;
}