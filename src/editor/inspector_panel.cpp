#include "inspector_panel.hpp"
#include "scene/entity.hpp"
#include "core/logging.hpp"
#include "imgui/imgui.h"
#include "editor/imgui_draw_helpers.hpp"

#include "reflection/reflection_utils.hpp"
#include "reflection/reflection_hashes.hpp"

#include <sstream>
#include <inttypes.h>

void InspectorPanel::ImGuiRender()
{
	ImGui::Begin("Inspector");

	if (this->entity)
	{
		for (auto component : entity->GetComponents())
			DrawComponentNode(component);
	}
	else
	{
		ImGui::Text("Select an entity in the scene hierarchy to inspect it's properties.");
	}

	ImGui::End();
}

void InspectorPanel::DrawComponentNode(Component *component)
{
	TypeId id = component->GetTypeId();

	std::string name = std::string(id.GetTypeName());
	name.erase(0, 6); // remove the class prefix
	name = FormatName(name);

	const ImGuiTreeNodeFlags flags = 
		ImGuiTreeNodeFlags_DefaultOpen | 
		ImGuiTreeNodeFlags_Framed | 
		ImGuiTreeNodeFlags_SpanAvailWidth;

	if (ImGui::TreeNodeEx(name.c_str(), flags))
	{
		for (auto &member : MemberId::GetMemberInfos(id))
		{
			DrawComponentMemberNode(component, member);
		}
		ImGui::TreePop();
	}
}

void InspectorPanel::DrawComponentMemberNode(
	Component *component, 
	const MemberId::MemberInfo &member
)
{
	std::string name = FormatName(member.Name);
	
	switch (member.VariableId.GetHash())
	{
	case HASH_XMFLOAT3:
	{
		XMFLOAT3 &values = GetMemberByOffset<XMFLOAT3>(component, member.Offset);
		DrawFloat3(name, values);
	} break;

	case HASH_XMFLOAT4:
	{
		XMFLOAT4 &values = GetMemberByOffset<XMFLOAT4>(component, member.Offset);
		DrawFloat4(name, values);
	} break;

	case HASH_float: {
		float &value = GetMemberByOffset<float>(component, member.Offset);
		DrawFloat(name, value);
	} break;

	default:
		LogInfo("%" PRIu64, member.VariableId.GetHash());
		ImGui::TextColored({ 1, 1, 0, 1 }, "Unsupported data type: %s", member.VariableId.GetTypeId().GetTypeName().data());
		break;
	}
}

std::string InspectorPanel::FormatName(const std::string &name)
{
	std::stringstream formattedName{};

	// Capitalize first letter
	if (name.size() > 0)
		formattedName << static_cast<char>(std::toupper(name[0]));
	
	for (size_t i = 1; i < name.size(); ++i)
	{
		// Add Space between capital letters
		if (std::isupper(static_cast<char>(name[i])))
			formattedName << ' ';
		formattedName << name[i];
	}

	return formattedName.str();
}
