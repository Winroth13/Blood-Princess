#pragma once

#include <vector>

class GameState
{
	std::vector<uint32_t> floorScores;

public:
	std::vector<uint32_t> &GetScores() { return floorScores; }
	void clearScores() { floorScores.clear(); }

	void addScore(uint32_t score) { floorScores.push_back(score); }

	uint32_t GetNumFloorsComplete() { return static_cast<uint32_t>(floorScores.size()); }
	uint32_t GetTotalScore();
};