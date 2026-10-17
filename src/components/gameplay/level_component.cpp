#include "level_component.hpp"

#include "scene/entity.hpp"
#include "core/logging.hpp"

#include "components/gameplay/wave_controller.hpp"

void LevelComponent::OnStart(const AppContext &context)
{
	auto *waveController = this->GetOwner()->GetComponent<WaveController>();

	if (!waveController)
	{
		LogError("Must have a wave controller");
		return;
	}

	waveController->GetOnWaveCompleted().Connect([waveController]() {
		waveController->NextWave();
	});

	waveController->GetOnFloorCompleted().Connect([]() {
		LogInfo("Floor Completed!");
	});
}