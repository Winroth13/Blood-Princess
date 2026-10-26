#pragma once

#include <vector>

class GameState
{
	std::vector<uint32_t> floorScores;
	uint32_t totalKills = 0;
	uint32_t totalParries = 0;
	uint32_t topCombo = 0;

public:
	std::vector<uint32_t> &GetScores() { return this->floorScores; }
	void ClearStats();

	void AddScore(uint32_t score) { this->floorScores.push_back(score); }
	void AddKills(uint32_t numKills) { this->totalKills += numKills; }
	void AddParries(uint32_t numParries) { this->totalParries += numParries; }
	void SubmitTopCombo(uint32_t topCombo);

	uint32_t GetNumFloorsComplete() const { return static_cast<uint32_t>(this->floorScores.size()); }
	uint32_t GetTotalScore() const;
	uint32_t GetTotalKills() const { return this->totalKills; }
	uint32_t GetTotalParries() const { return this->totalParries; }
	uint32_t GetTopCombo() const { return this->topCombo; }
};