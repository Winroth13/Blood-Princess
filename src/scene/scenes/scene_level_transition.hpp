#pragma once

#include "scene_headers.hpp"
#include "assets/types/texture_cube.hpp"
#include "assets/types/model.hpp"

namespace Scenes 
{
	void LevelTransition(Scene &outScene, const AppContext &appContext) 
	{
		Entity *enviromentEntity = outScene.AddEntity("enviroment_entity");
		
		auto *ambient = enviromentEntity->AddComponent<AmbientLightComponent>();
		ambient->SetColour({ 1.0f, 0.0f, 0.0f });
		ambient->SetIntensity(0.05f);

		auto environment = enviromentEntity->AddComponent<EnvironmentComponent>();
		std::shared_ptr<TextureCube> skyboxTexture = appContext.assetManager->Get<TextureCube>("assets/images/red_skybox/sky.jpg");
		environment->SetSkybox(skyboxTexture);
		environment->SetFogColour({0.0165f, 0.0402f, 0.0299f});
		environment->SetFogEndDepth(1500.0f);
		enviromentEntity->AddComponent<TransformComponent>();

		auto camera = enviromentEntity->AddComponent<CameraComponent>();
		camera->SetFarPlane(4000.0f);

		Entity *towerEntity = outScene.AddEntity("tower");
		towerEntity->AddComponent<TransformComponent>();
		towerEntity->AddComponent<ModelComponent>()->SetModel(appContext.assetManager->Get<Model>("assets/models/tower.glb"));
	}
}
