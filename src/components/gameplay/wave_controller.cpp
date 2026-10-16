#include "wave_controller.hpp"
#include "scene/entity.hpp"
#include "assets/types/floor.hpp"
#include "core/logging.hpp"
#include "components/ai/enemy_spawner_component.hpp"
#include "assets/types/enemy.hpp"

#include "imgui/imgui.h"

#include <format>

void WaveController::NextWave()
{
	auto &waves = this->floor->GetWaves();

	if (this->nextWave >= this->floor->GetWaves().size())
		return;

	LogInfo("Starting Wave %d", this->nextWave);

	Wave wave = waves[this->nextWave++];

	ActiveWave activeWave = {};
	activeWave.completionProgress = wave.completion;

	for (auto waveEnemy : wave.enemies)
	{
		activeWave.toSpawn[+waveEnemy.type] = waveEnemy.amount;
		activeWave.toKill[+waveEnemy.type] = waveEnemy.amount;
		activeWave.spawnDuration[+waveEnemy.type] = waveEnemy.time / waveEnemy.amount;
		activeWave.spawnTimer[+waveEnemy.type] = activeWave.spawnDuration[+waveEnemy.type];
	}

	this->activeWaves.push_back(activeWave);
}

bool WaveController::IsWaveCompleted(ActiveWave &wave)
{
	float progress = this->GetWaveProgress(wave);
	return progress >= wave.completionProgress;
}

void WaveController::CompleteWave(uint32_t waveIndex)
{
	this->activeWaves.erase(this->activeWaves.begin() + waveIndex);
	this->NextWave();
}

float WaveController::GetWaveProgress(ActiveWave &wave)
{
	float totalProgress = 0.0f;
	uint32_t count = 0;

	for (size_t i = 0; i < +EnemyType::COUNT; ++i)
	{
		if (wave.toKill[i] == 0)
			continue;

		float progress = wave.killed[i] / static_cast<float>(wave.toKill[i]);

		totalProgress += progress;
		count++;
	}
	return totalProgress / count;
}

void WaveController::OnStart(const AppContext &context)
{
	EnemySpawnerComponent *spawner = this->GetOwner()->GetComponent<EnemySpawnerComponent>();

	if (!spawner)
	{
		LogError("Must have an enemy spawner component");
		return;
	}

	spawner->GetOnEnemyKilled().Connect([this](EnemyType type, uint32_t wave) 
	{
		ActiveWave &activeWave = this->activeWaves[wave];

		activeWave.killed[+type]++;

		if (this->IsWaveCompleted(activeWave))
		{
			this->CompleteWave(wave);
		}
	});

	this->NextWave();
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

		ImGui::SeparatorText("Killed");

		for (size_t j = 0; j < +EnemyType::COUNT; ++j)
		{
			ImGui::Text(
				"%s: %d / %d",
				Enemy::GetStringFromEnemyType(static_cast<EnemyType>(j)).c_str(),
				activeWave.killed[j],
				activeWave.toKill[j]
			);
		}
	}

	ImGui::End();
}
