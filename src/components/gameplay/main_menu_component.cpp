#include "main_menu_component.hpp"
#include "input/input.hpp"
#include "scene/scene_manager.hpp"

void MainMenuComponent::Update(const AppContext &context, float deltaTime) {
    if (Input::IsKeyPressed(Key::SPACE))
        context.sceneManager->RequestSceneChange("demo_gameplay");
}