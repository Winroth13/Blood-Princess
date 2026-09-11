#pragma once
#include "reflection/MemberVars.h"

#include <string>

class Entity;
class Component;

class InspectorPanel
{
	Entity *entity;

public:
	InspectorPanel() = default;
	~InspectorPanel() = default;

	void ImGuiRender();

	void DrawComponentNode(Component* component);
	void DrawComponentMemberNode(Component *component, const MemberId::MemberInfo &member);

	void SetEntity(Entity *entity) { this->entity = entity; }

	static std::string FormatName(const std::string &name);
};