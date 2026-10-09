#include "main_menu_component.hpp"
#include "input/input.hpp"
#include "scene/scene_manager.hpp"
#include "components/ui/ui_image_component.hpp"
#include "scene/entity.hpp"
#include "assets/asset_manager.hpp"

void MainMenuComponent::Update(const AppContext &context, float deltaTime) {
    if (Input::IsKeyPressed(Key::SPACE)) {
        UIImageComponent *imageComp = this->GetOwner()->GetComponent<UIImageComponent>();
        std::shared_ptr<Texture2D> texture = context.assetManager->Get<Texture2D>("assets/images/loading_screen.png");
        imageComp->SetTexture(texture);

        context.sceneManager->RequestSceneChange("gameplay");
    }
}