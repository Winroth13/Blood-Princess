#include "combo_ui_component.hpp"
#include "components/ui/ui_text_component.hpp"
#include "core/math.hpp"
#include "core/application.hpp"

void ComboUIComponent::OnStart(const AppContext &context)
{
    this->xLabel->SetText("");
    this->xFontSize = this->xLabel->GetFontSize();

    this->comboLabel->SetText("");
    this->comboSize = this->comboLabel->GetFontSize();

    this->playerController->GetOnComboChanged().Connect([this](uint32_t combo)
    {
        this->combo = combo;
        this->comboBounceTimer = 0.0f;
    });
}

void ComboUIComponent::Update(const AppContext &context, float deltaTime)
{
    this->timer += deltaTime / context.application->GetTimeScale();

    float comboRatio = this->combo / MAX_COMBO;

    if (this->combo <= 0)
    {
        this->xLabel->SetText("");
        this->comboLabel->SetText("");
    }
    else
    {
        this->xLabel->SetText("x");
        this->comboLabel->SetText(std::to_string(combo));

        /* Scale Font with Combo */
        constexpr float MIN_FONT_FACTOR = 1.0;
        constexpr float MAX_FONT_FACTOR = 1.5f;

        float fontSize = Math::Lerp(MIN_FONT_FACTOR, MAX_FONT_FACTOR, comboRatio);

        if (fontSize > MAX_FONT_FACTOR)
            fontSize = MAX_FONT_FACTOR;

        /* Change Combo Color  */
        XMFLOAT4 startColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        XMFLOAT4 middleColor = { 1.0f, 1.0f, 0.0f, 1.0f };
        XMFLOAT4 finalColor = { 1.0f, 0.0f, 0.0f, 1.0f };

        float middleRatio = std::clamp(comboRatio / 0.5f, 0.0f, 1.0f);
        XMFLOAT4 color = Math::Lerp(startColor, middleColor, middleRatio);

        float finalRatio = std::clamp((comboRatio - 0.5f) / 0.5f, 0.0f, 1.0f);
        color = Math::Lerp(color, finalColor, finalRatio);

        this->comboLabel->SetColour(color);

        /* Scale Font with Timer */
        if (this->comboBounceTimer >= 0.0f)
        {
            this->comboBounceTimer += deltaTime / context.application->GetTimeScale();
            float bounceRatio = this->comboBounceTimer / COMBO_BOUNCE_TIME;
            constexpr float MAX_FONT_BOUNCE = 2.0f;
            float bounceFactor = Math::Lerp(1.0f, 1.5f, Math::easeOutQuint(bounceRatio));
            fontSize *= bounceFactor;

            if (this->comboBounceTimer >= COMBO_BOUNCE_TIME)
            {
                this->comboBounceTimer = -1.0f;
            }
        }

        constexpr float SHAKE_FREQ_MIN = 4.0f;
        constexpr float SHAKE_FREQ_MAX = 15.0f;

        constexpr float SHAKE_AMPLITUDE_MIN = 0.95f;
        constexpr float SHAKE_AMPLITUDE_MAX = 1.05f;

        float shakeRatio = (1 + std::sin(this->timer * Math::Lerp(SHAKE_FREQ_MIN, SHAKE_FREQ_MAX, comboRatio))) * 0.5f;
        fontSize *= Math::Lerp(SHAKE_AMPLITUDE_MIN, SHAKE_AMPLITUDE_MAX, shakeRatio);

        this->comboLabel->SetFontSize(fontSize * this->comboSize);
    }
}