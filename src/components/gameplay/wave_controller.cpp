#include "wave_controller.hpp"
#include "scene/entity.hpp"
#include "assets/types/floor.hpp"
#include "core/logging.hpp"
#include "components/ai/enemy_spawner_component.hpp"
#include "components/gameplay/enemy_database_component.hpp"
#include "components/gameplay/environment_component.hpp"
#include "components/traps/trap_spawner_component.hpp"
#include "assets/types/enemy.hpp"

#include "imgui/imgui.h"

#include <format>

void WaveController::StartFloor(std::shared_ptr<Floor> floor)
{
	this->floor = floor;

	auto *environment = this->GetOwner()->GetComponent<EnvironmentComponent>();

	if (environment)
	{
		environment->SetHsvShift(floor->GetHSV());
		environment->SetFogColour(floor->GetFogColour());
		environment->SetFogDensity(floor->GetFogDensity());
		environment->SetFogStartDepth(floor->GetFogStartDepth());
		environment->SetFogEndDepth(floor->GetFogEndDepth());
	}

	auto *trapSpawner = this->GetOwner()->GetComponent<TrapSpawnerComponent>();
	if (trapSpawner && this->floor->GetSpikeTraps().amount > 0)
	{
		trapSpawner->SpawnSpikeTraps(this->floor->GetSpikeTraps());
	}

	this->NextWave();
}

void WaveController::NextWave()
{
	auto &waves = this->floor->GetWaves();

	if (this->nextWave >= this->floor->GetWaves().size())
		return;

	auto *database = this->GetOwner()->GetComponent<EnemyDatabaseComponent>();

	LogInfo("Starting Wave %d", this->nextWave);

	Wave wave = waves[this->nextWave++];

	ActiveWave activeWave = {};
	activeWave.currentScore = 0;
	activeWave.completionScore = 0;

	for (auto waveEnemy : wave.enemies)
	{
		activeWave.toSpawn[+waveEnemy.type] = waveEnemy.amount;
		activeWave.spawnDuration[+waveEnemy.type] = waveEnemy.time / waveEnemy.amount;
		activeWave.spawnTimer[+waveEnemy.type] = activeWave.spawnDuration[+waveEnemy.type];

		int score = database->GetEnemy(waveEnemy.type)->score * waveEnemy.amount;

		activeWave.totalScore += static_cast<uint32_t>(score);
		activeWave.completionScore += static_cast<uint32_t>(score * wave.completion);
	}

	this->activeWaves.push_back(activeWave);
}

void WaveController::CompleteWave(uint32_t waveIndex)
{
	this->activeWaves[waveIndex].completed = true;
	this->onWaveCompleted.Emit();
}

float WaveController::GetWaveProgress(ActiveWave &wave)
{
	return wave.currentScore / static_cast<float>(wave.completionScore);
}

float WaveController::GetFloorProgress()
{
	float totalProgress = 0.0f;

	for (size_t i = 0; i < floor->GetWaves().size(); ++i)
	{
		if (i < this->activeWaves.size())
		{
			ActiveWave &wave = this->activeWaves[i];
			totalProgress += wave.currentScore / static_cast<float>(wave.totalScore);
		}
	}

	return totalProgress / floor->GetWaves().size();
}

bool WaveController::IsFloorCompleted()
{
	// we haven't started all waves yet
	if (this->nextWave != this->floor->GetWaves().size())
		return false;

	// check if we have killed all enemies in all waves
	for (size_t i = 0; i < this->activeWaves.size(); ++i)
	{
		if (activeWaves[i].currentScore != activeWaves[i].totalScore)
		{
			return false;
		}
	}

	return true;
}

void WaveController::OnStart(const AppContext &context)
{
	EnemySpawnerComponent *spawner = this->GetOwner()->GetComponent<EnemySpawnerComponent>();

	if (!spawner)
	{
		LogError("Must have an enemy spawner component");
		return;
	}

	auto *database = this->GetOwner()->GetComponent<EnemyDatabaseComponent>();

	if (!database)
	{
		LogError("Must have an enemy database component");
		return;
	}

	spawner->GetOnEnemyKilled().Connect([this, database](EnemyType type, uint32_t wave) 
	{
		ActiveWave &activeWave = this->activeWaves[wave];
		
		auto enemy = database->GetEnemy(type);
		activeWave.currentScore += enemy->score;

		if (!activeWave.completed)
		{
			if (activeWave.currentScore >= activeWave.completionScore)
			{
				this->CompleteWave(wave);
			}
		}

		this->onFloorProgressChanged.Emit(this->GetFloorProgress());

		if (this->IsFloorCompleted())
		{
			this->onFloorCompleted.Emit();
		}
	});
}

void WaveController::Update(const AppContext &context, float deltaTime)
{
	EnemySpawnerComponent *spawner = this->GetOwner()->GetComponent<EnemySpawnerComponent>();

	for (uint32_t j = 0; j < static_cast<uint32_t>(this->activeWaves.size()); ++j)
	{
		ActiveWave &wave = this->activeWaves[j];

		for (int i = 0; i < +EnemyType::COUNT; ++i)
		{
			// none to spawn
			if (wave.toSpawn[i] == 0)
				continue;

			// timer is out so let's spawn
			if (wave.spawnTimer[i] <= 0.0f)
			{
				if (spawner->SpawnEnemy(i, j))
				{
					wave.toSpawn[i]--;
					wave.spawnTimer[i] = wave.spawnDuration[i];
				}
			}
			else
			{
				wave.spawnTimer[i] -= deltaTime;
			}
		}
	}
}

// MEGA HACK to get some debug ui for the wave controller
void WaveController::Render(const RenderView &view, RenderQueue &queue)
{
	if (view.type != ViewType::PRIMARY)
		return;

	ImGui::Begin("Wave Info");
	
	for (size_t i = 0; i < this->activeWaves.size(); ++i)
	{
		ActiveWave &activeWave = this->activeWaves[i];

		ImGui::SeparatorText(std::format("Wave {} ({}%)", i, this->GetWaveProgress(activeWave) * 100).c_str());
		ImGui::Text("Score: %u / %u", activeWave.currentScore, activeWave.completionScore);
		ImGui::Text("Total Score: %u / %u", activeWave.currentScore, activeWave.totalScore);

		ImGui::SeparatorText("Spawn");

		for (size_t j = 0; j < +EnemyType::COUNT; ++j)
		{
			ImGui::Text(
				"%s: %d (%.2fs)", 
				Enemy::GetStringFromEnemyType(static_cast<EnemyType>(j)).c_str(),
				activeWave.toSpawn[j],
				activeWave.spawnTimer[j]
			);
		}
	}

	ImGui::End();
}
