#pragma once

#include "scene/scene.hpp"

class SceneManager
{
	Scene scene;

public:
	Scene *GetActiveScene() { return &this->scene; }

	void Update(float deltaTime)
	{
		scene.Update(deltaTime);
	}
};