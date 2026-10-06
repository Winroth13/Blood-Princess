#include "light_flicker_component.hpp"
#include "components/lights/point_light_component.hpp"
#include "core/rng.hpp"

#include <cmath>

void LightFlickerComponent::OnStart(const AppContext &context) {
    PointLightComponent *pointLight = this->GetOwner()->GetComponent<PointLightComponent>();
    if (!pointLight)
        return;

    this->startIntensity = pointLight->GetIntensity();

    this->timer = RNG::Float(0.0f, 20.0f);
}
void LightFlickerComponent::Update(const AppContext &context, float deltaTime) {
    PointLightComponent *pointLight = this->GetOwner()->GetComponent<PointLightComponent>();
    if (!pointLight)
        return;

    this->timer += deltaTime;

    pointLight->SetIntensity(
        this->startIntensity + 
        std::abs(this->flickerIntensity * std::sin(this->timer * this->flickerSpeed) + 
        2 * this->flickerIntensity * std::sin(this->timer * this->flickerSpeed / 2.0f))
    );
}