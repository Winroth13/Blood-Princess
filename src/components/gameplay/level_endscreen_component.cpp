#include "level_endscreen_component.hpp"
#include "core/application.hpp"
#include "core/math.hpp"
#include "assets/types/audio_clip.hpp"
#include "audio/audio_manager.hpp"
#include "input/input.hpp"

#include <format>

void LevelEndscreenComponent::OnStart(const AppContext &context)
{
	this->scoreLabel->SetText("");
	this->killsLabel->SetText("");

	this->tickAudio = context.assetManager->Get<AudioClip>("assets/audio/sfx/endscreen_tick.ogg");
	this->tickDoneAudio = context.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/sword_parry_1.ogg");
}

void LevelEndscreenComponent::Update(const AppContext &context, float deltaTime)
{
	this->timer += deltaTime / context.application->GetTimeScale();

	this->UpdateAscendLabel(context, deltaTime);
	this->UpdateTimeLabel();

	if (this->UpdateRubricLabel(context))
	{
		/* Advance Score */
		if (this->score < this->targetScore)
		{
			if (Input::IsKeyPressed(Key::SPACE)) {
				this->score = static_cast<float>(this->targetScore);
			}

			this->AdvanceStat(
				this->score,
				static_cast<float>(this->targetScore),
				1.0f,
				this->scoreLabel,
				deltaTime,
				context
			);
		}
		/* Advance Kills */
		else if (this->kills < this->targetKills)
		{
			if (Input::IsKeyPressed(Key::SPACE)) {
				this->kills = static_cast<float>(this->targetKills);
			}

			this->AdvanceStat(
				this->kills,
				static_cast<float>(this->targetKills),
				0.2f,
				this->killsLabel,
				deltaTime,
				context
			);
		}
	}
}

bool LevelEndscreenComponent::UpdateRubricLabel(const AppContext &context)
{
	constexpr float RUBRIC_ANIMATION_DURATION = 1.5;

	constexpr float RUBRIC_FONT_SIZE_MIN = 48.0f;
	constexpr float RUBRIC_FONT_SIZE_MAX = 750.0f;

	float ratio = this->timer / RUBRIC_ANIMATION_DURATION;

	if (this->timer < RUBRIC_ANIMATION_DURATION)
	{
		
		this->rubricLabel->SetFontSize(Math::Lerp(RUBRIC_FONT_SIZE_MAX, RUBRIC_FONT_SIZE_MIN, Math::easeOutElastic(ratio)));

		if (ratio >= 0.2f && !this->rubricLanded)
		{
			context.audioManager->PlaySoundEffect(
				this->tickDoneAudio,
				false,
				0.5f,
				0.85f
			);
			this->rubricLanded = true;
		}
	}

	return ratio >= 0.2f;
}

void LevelEndscreenComponent::UpdateTimeLabel()
{
	uint32_t minutes = static_cast<uint32_t>(this->time / 60.0f);
	uint32_t seconds = static_cast<uint32_t>(this->time - minutes * 60.0f);

	this->timeLabel->SetText(std::format("{}:{:02}", minutes, seconds));
}

void LevelEndscreenComponent::UpdateAscendLabel(const AppContext &context, float deltaTime)
{
	constexpr float ALPHA_MIN = 0.2f;
	constexpr float PULSE_FREQUENCY = 2.0f;

	float alpha = (std::sin(this->timer * PULSE_FREQUENCY) + 1.0f) / 2.0f;

	XMFLOAT4 color = this->ascendLabel->GetColour();
	color.w = Math::Lerp(ALPHA_MIN, 1.0f, alpha);

	XMFLOAT4 outLineColor = this->ascendLabel->GetOutlineColour();
	outLineColor.w = Math::Lerp(ALPHA_MIN, 1.0f, alpha);

	this->ascendLabel->SetColour(color);
	this->ascendLabel->SetOutlineColour(outLineColor);
}

void LevelEndscreenComponent::AdvanceStat(float &current, float target, float advanceSpeedFactor, UITextComponent *statLabel, float deltaTime, const AppContext &context)
{
	float realDeltaTime = deltaTime / context.application->GetTimeScale();
	this->advanceTimer += realDeltaTime;
	this->tickTimer += realDeltaTime;

	constexpr float ADVANCE_SPEED = 0.4f;

	constexpr float MIN_TICK_TIME = 0.007f;
	constexpr float MAX_TICK_TIME = 0.1f;

	constexpr float MIN_TICK_PITCH = 0.9f;
	constexpr float MAX_TICK_PITCH = 1.3f;

	current += std::powf((advanceTimer + 1.0f) * ADVANCE_SPEED * advanceSpeedFactor, 2);

	float ratio = current / static_cast<float>(target);

	float timeScale = context.application->GetTimeScale();
	if (timeScale <= 0.25f)
	{
		timeScale = 0.25f;
	}

	constexpr float STAT_FONT_SIZE_MIN = 32.0f;
	constexpr float STAT_FONT_SIZE_MAX = 40.0f;
	float fontSize = Math::Lerp(STAT_FONT_SIZE_MIN, STAT_FONT_SIZE_MAX, Math::easeOutQuint(ratio));
	statLabel->SetFontSize(fontSize);

	if (this->tickTimer > Math::Lerp(MAX_TICK_TIME, MIN_TICK_TIME, ratio))
	{
		context.audioManager->PlaySoundEffect(
			this->tickAudio,
			false,
			1.0f,
			Math::Lerp(MIN_TICK_PITCH, MAX_TICK_PITCH, ratio) / timeScale
		);

		this->tickTimer = 0.0f;
	}

	if (current >= target)
	{
		this->advanceTimer = 0.0f;
		current = std::floor(target);

		context.audioManager->PlaySoundEffect(
			this->tickDoneAudio,
			false,
			0.5f,
			1.1f / timeScale
		);
		statLabel->SetFontSize(STAT_FONT_SIZE_MIN);
	}

	statLabel->SetText(std::format("{}", static_cast<uint32_t>(current)));
}
