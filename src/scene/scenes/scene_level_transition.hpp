#pragma once

#include "scene_headers.hpp"
#include "assets/types/texture_cube.hpp"
namespace Scenes 
{
	void LevelTransition(Scene &outScene, const AppContext &appContext) 
	{
		Entity *enviromentEntity = outScene.AddEntity("enviroment_entity");
		
		auto environment = enviromentEntity->AddComponent<EnvironmentComponent>();
		std::shared_ptr<TextureCube> skyboxTexture = appContext.assetManager->Get<TextureCube>("assets/images/red_skybox/sky.jpg");
		environment->SetSkybox(skyboxTexture);

		enviromentEntity->AddComponent<TransformComponent>();
		enviromentEntity->AddComponent<CameraComponent>();
	}
}
