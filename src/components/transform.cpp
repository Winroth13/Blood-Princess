#include "transform.hpp"
#include "core/logging.hpp"
#include "scene/entity.hpp"

void Transform::OnStart(const AppContext &context) {
}

void Transform::Update(const AppContext &context, float deltaTime) {
    LogInfo("Hello from %s!", this->GetOwner()->GetName().c_str());
    this->SetActive(false);
}