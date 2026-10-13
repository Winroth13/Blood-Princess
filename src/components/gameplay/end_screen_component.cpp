#include "end_screen_component.hpp"
#include "input/input.hpp"

void EndScreenComponent::OnStart(const AppContext &context)
{
    Input::SetMouseMode(MouseMode::VISIBLE);
}