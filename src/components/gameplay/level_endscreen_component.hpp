#pragma once
#include "scene/component.hpp"
#include "assets/types/enemy.hpp"
#include "components/ui/ui_text_component.hpp"
#include "components/audio/audio_component.hpp"
#include "events/signal.hpp"

#include <array>
#include <memory>

class AudioClip;

class LevelEndscreenComponent : public Component
{
	RegisterComponent(LevelEndscreenComponent);

	Signal<> onAscend;

	float time = 63.0f;
	uint32_t targetKills = 500;
	uint32_t targetScore = 200;

	float kills = 0.0f;
	float score = 0.0f;

	UITextComponent *rubricLabel = nullptr;
	bool rubricLanded = false;
	bool isFinal = false;

	UITextComponent *timeLabel = nullptr;
	UITextComponent *scoreLabel = nullptr;
	UITextComponent *killsLabel = nullptr;
	UITextComponent *ascendLabel = nullptr;

	float timer = 0.0f;
	float advanceTimer = 0.0f;
	float tickTimer = 0.0f;

	std::shared_ptr<AudioClip> tickAudio;
	std::shared_ptr<AudioClip> tickDoneAudio;

private:
	void OnStart(const AppContext &context) override;
	void Update(const AppContext &context, float deltaTime) override;

	bool UpdateRubricLabel(const AppContext &context);
	void UpdateTimeLabel();
	void UpdateAscendLabel(const AppContext &context, float deltaTime);

	void AdvanceStat(float &current, float target, float advanceSpeedFactor, UITextComponent* statLabel, float deltaTime, const AppContext &context);

public:
	LevelEndscreenComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); };
	~LevelEndscreenComponent() = default;

	void SetTime(float time) { this->time = time; }
	void SetKills(uint32_t kills) { this->targetKills = kills; }
	void SetScore(uint32_t score) { this->targetScore = score; }

	void SetRubricLabel(UITextComponent *label) { this->rubricLabel = label; }
	void SetTimeLabel(UITextComponent *label) { this->timeLabel = label; }
	void SetScoreLabel(UITextComponent *label) { this->scoreLabel = label; }
	void SetKillsLabel(UITextComponent *label) { this->killsLabel = label; }
	void SetAscendLabel(UITextComponent *label) { this->ascendLabel = label; }
	void SetIsFinal(bool value);

	Signal<> &GetOnAscend() { return this->onAscend; }
};