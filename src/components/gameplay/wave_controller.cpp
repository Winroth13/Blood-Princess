#include "wave_controller.hpp"
#include "assets/types/floor.hpp"

void WaveController::NextWave()
{
	this->currentWave++;

	if (this->currentWave < this->floor->GetWaves().size())
	{

	}
}

void WaveController::OnStart(const AppContext &context)
{

}

void WaveController::Update(const AppContext &context, float deltaTime)
{

}
