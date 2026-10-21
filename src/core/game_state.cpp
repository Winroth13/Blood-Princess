#include "game_state.hpp"

void GameState::ClearStats()
{
	this->floorScores.clear();
	this->totalKills = 0;
	this->totalParries = 0;
	this->topCombo = 0;
}

void GameState::SubmitTopCombo(uint32_t topCombo)
{
	if (this->topCombo < topCombo)
		this->topCombo = topCombo;
}

uint32_t GameState::GetTotalScore() const
{
	uint32_t totalScore = 0;

	for (uint32_t score : this->floorScores)
	{
		totalScore += score;
	}

	return totalScore;
}