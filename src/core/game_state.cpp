#include "game_state.hpp"

uint32_t GameState::GetTotalScore()
{
	uint32_t totalScore = 0;

	for (uint32_t score : this->floorScores)
	{
		totalScore += score;
	}

	return totalScore;
}