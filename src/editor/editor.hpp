#pragma once
#include "editor/scene_hierarchy_panel.hpp"
#include "editor/inspector_panel.hpp"

#include <DirectXMath.h>

class SceneManager;

class Editor
{
	SceneHierarchyPanel sceneHierarchyPanel;
	InspectorPanel inspectorPanel;

public:
	Editor() = default;
	~Editor() = default;

	Editor(const Editor &other) = delete;
	Editor &operator=(const Editor &other) = delete;

	bool Initialize(SceneManager &sceneManager);
	void Shutdown();

	void ImGuiRender();
};