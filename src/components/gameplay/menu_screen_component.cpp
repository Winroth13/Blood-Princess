#include "menu_screen_component.hpp"
#include "input/input.hpp"

void MenuScreenComponent::OnStart(const AppContext &context)
{
    Input::SetMouseMode(MouseMode::VISIBLE);
}