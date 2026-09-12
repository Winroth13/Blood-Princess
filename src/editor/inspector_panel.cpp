#include "inspector_panel.hpp"
#include "scene/entity.hpp"
#include "core/logging.hpp"
#include "imgui/imgui.h"
#include "editor/imgui_draw_helpers.hpp"

#include "reflection/reflection_utils.hpp"
#include "reflection/reflection_hashes.hpp"

#include "assets/types/audio_clip.hpp"

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
		std::unordered_map<std::string, std::vector<MemberId::MemberInfo>> memberGroups;

		const ImGuiTreeNodeFlags flags =
			ImGuiTreeNodeFlags_DefaultOpen |
			ImGuiTreeNodeFlags_SpanAvailWidth;

		// find groups
		for (auto &member : MemberId::GetMemberInfos(id))
		{
			if (memberGroups.find(member.ExportInfo.Group) == memberGroups.end())
				memberGroups[member.ExportInfo.Group] = { member };
			else
				memberGroups[member.ExportInfo.Group].push_back(member);
		}

		// draw groups
		for (auto &[key, value] : memberGroups)
		{
			if (!key.empty())
			{
				if (ImGui::TreeNodeEx(key.c_str(), flags))
				{
					for (auto &member : value)
						DrawComponentMemberNode(component, member);
					ImGui::TreePop();
				}
			}
		}
		
		// draw non grouped nodes
		if (memberGroups.find("") != memberGroups.end())
		{
			for (auto &member : memberGroups[""])
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
	case HASH_Model: {
		std::shared_ptr<Model> model = GetMemberByOffset<std::shared_ptr<Model>>(component, member.Offset);
		DrawAsset(name, model);
	} break;

	case HASH_AudioClip: {
		std::shared_ptr<AudioClip> audioClip = GetMemberByOffset<std::shared_ptr<AudioClip>>(component, member.Offset);
		DrawAsset(name, audioClip);
	} break;

	case HASH_XMFLOAT3:
	{
		XMFLOAT3 &values = GetMemberByOffset<XMFLOAT3>(component, member.Offset);
		if ((member.ExportInfo.flags & ExportFlags_ColorWheel) == ExportFlags_ColorWheel)
		{
			DrawColor3(name, values);
		}
		else
		{
			DrawFloat3(name, values);
		}
	} break;

	case HASH_XMFLOAT4:
	{
		XMFLOAT4 &values = GetMemberByOffset<XMFLOAT4>(component, member.Offset);
		if ((member.ExportInfo.flags & ExportFlags_ColorWheel) == ExportFlags_ColorWheel)
		{
			DrawColor4(name, values);
		}
		else
		{
			DrawFloat4(name, values);
		}
	} break;

	case HASH_float: {
		float &value = GetMemberByOffset<float>(component, member.Offset);
		DrawFloat(
			name, 
			value, 
			member.ExportInfo.Unit, 
			member.ExportInfo.Min,
			member.ExportInfo.Max,
			member.ExportInfo.Speed,
			(member.ExportInfo.flags & ExportFlags_Angle) == ExportFlags_Angle
		);
	} break;
	
	case HASH_bool: {
		bool &value = GetMemberByOffset<bool>(component, member.Offset);
		DrawCheckbox(name, value);
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
