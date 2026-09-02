#pragma once

#include "scene/scene.hpp"

class SceneManager
{
	Scene scene;

public:
	void Update(float deltaTime)
	{
		scene.Update(deltaTime);
	}
};