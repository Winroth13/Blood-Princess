#include "main_menu_component.hpp"
#include "input/input.hpp"
#include "scene/scene_manager.hpp"
#include "components/ui/ui_image_component.hpp"
#include "scene/entity.hpp"
#include "assets/asset_manager.hpp"

void MainMenuComponent::OnStart(const AppContext &context) {
    Input::SetMouseMode(MouseMode::VISIBLE);
}