#include "inspector_panel.hpp"
#include "scene/entity.hpp"
#include "core/logging.hpp"
#include "imgui/imgui.h"
#include "editor/imgui_draw_helpers.hpp"
#include "editor/asset_viewer_panel.hpp"

#include "reflection/reflection_utils.hpp"

#include "assets/types/audio_clip.hpp"

#include <sstream>
#include <inttypes.h>

bool InspectorPanel::Initialize(AssetViewerPanel &assetViewerPanel)
{
    LogInfo("Creating Asset Viewer Panel...");
    LogIndent();

    this->assetViewerPanel = &assetViewerPanel;

    LogUnindent();

    return true;
}

void InspectorPanel::ImGuiRender()
{
    ImGui::Begin("Inspector");

    if (this->entity)
    {
        std::unordered_map<TypeId, size_t> componentCounter{};

        for (auto component : entity->GetComponents())
        {
            TypeId id = component->GetTypeId();

            std::string name = std::string(id.GetTypeName());
            name.erase(0, 6); // remove the class prefix
            name = FormatName(name);

            if (componentCounter.find(id) == componentCounter.end())
            {
                componentCounter[id] = 0;
            }
            else
            {
                componentCounter[id]++;
                //name += '(' + std::to_string(componentCounter[id]) + ')';
                name += " #" + std::to_string(componentCounter[id]);
            }

            DrawComponentNode(component, name, id);
        }
    }
    else
    {
        ImGui::Text("Select an entity in the scene hierarchy to inspect it's properties.");
    }

    ImGui::End();
}

void InspectorPanel::DrawComponentNode(Component *component, const std::string& name, TypeId id)
{
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

    // Set by every widget that writes to the member, so the component can be notified
    // that one of its properties was edited from the outside.
    bool changed = false;

    switch (member.VariableId.GetHash())
    {
        case GetTypeHash<std::shared_ptr<Model>>():
        {
            std::shared_ptr<Model> model = GetMemberByOffset<std::shared_ptr<Model>>(component, member.Offset);
            if (DrawAsset(name, model))
                this->assetViewerPanel->SetViewingAsset(model);
        } break;

        case GetTypeHash<std::shared_ptr<AudioClip>>(): 
        {
            std::shared_ptr<AudioClip> audioClip = GetMemberByOffset<std::shared_ptr<AudioClip>>(component, member.Offset);

            if (DrawAsset(name, audioClip))
                this->assetViewerPanel->SetViewingAsset(audioClip);

        } break;

        case GetTypeHash<XMFLOAT3>():
        {
            XMFLOAT3 &values = GetMemberByOffset<XMFLOAT3>(component, member.Offset);
            if ((member.ExportInfo.flags & ExportFlags_ColorWheel) == ExportFlags_ColorWheel)
            {
                changed = DrawColor3(name, values);
            }
            else
            {
                changed = DrawFloat3(name,
                                    values,
                                    0.0f,
                    (member.ExportInfo.flags & ExportFlags_Angle) == ExportFlags_Angle
                );
            }
        } break;

        case GetTypeHash<XMFLOAT4>():
        {
            XMFLOAT4 &values = GetMemberByOffset<XMFLOAT4>(component, member.Offset);
            if ((member.ExportInfo.flags & ExportFlags_ColorWheel) == ExportFlags_ColorWheel)
            {
                changed = DrawColor4(name, values);
            }
            else
            {
                changed = DrawFloat4(name, values);
            }
        } break;

        case GetTypeHash<float>(): 
        {
            float &value = GetMemberByOffset<float>(component, member.Offset);
            changed = DrawFloat(
                name, 
                value, 
                member.ExportInfo.Unit, 
                member.ExportInfo.Min,
                member.ExportInfo.Max,
                member.ExportInfo.Speed,
                (member.ExportInfo.flags & ExportFlags_Angle) == ExportFlags_Angle
            );
        } break;

        case GetTypeHash<int>():
        {
            int &value = GetMemberByOffset<int>(component, member.Offset);
            changed = DrawInt(
                name,
                value,
                member.ExportInfo.Unit,
                static_cast<int>(member.ExportInfo.Min),
                static_cast<int>(member.ExportInfo.Max),
                member.ExportInfo.Speed
            );
        } break;

        case GetTypeHash<bool>(): 
        {
            bool &value = GetMemberByOffset<bool>(component, member.Offset);
            changed = DrawCheckbox(name, value);
        } break;

        default:
            LogInfo("%" PRIu64, member.VariableId.GetHash());
            ImGui::TextColored({ 1, 1, 0, 1 }, "Unsupported data type: %s", member.VariableId.GetTypeId().GetTypeName().data());
            break;
    }

    if (changed)
        MemberId::NotifyChanged(component->GetTypeId(), member.Offset, component);
}

std::string InspectorPanel::FormatName(const std::string &name)
{
    std::stringstream formattedName{};

    // Capitalize first letter
    if (name.size() > 0)
        formattedName << static_cast<char>(std::toupper(name[0]));

    bool digit = false;

    for (size_t i = 1; i < name.size(); ++i)
    {
        if (!digit)
        {
            // Add Space between capital letters
            if (std::isupper(static_cast<char>(name[i])))
                formattedName << ' ';
            else if (std::isdigit(static_cast<char>(name[i])))
            {
                digit = true;
                formattedName << ' ';
            }
        }

        formattedName << name[i];
    }

    return formattedName.str();
}