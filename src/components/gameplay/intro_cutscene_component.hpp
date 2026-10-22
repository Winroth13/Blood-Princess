#pragma once

#include "scene/component.hpp"

#include <memory>
#include <vector>

class Texture2D;
class AudioClip;
class UIImageComponent;
class UITextComponent;
class AudioManager;

enum class WhichCutsceneAmICurrentlyIn {
    INTRO,
    DEATH,
    WIN
};

class IntroCutsceneComponent : public Component {
    RegisterComponent(IntroCutsceneComponent);

    float time = 0.0f;

    std::shared_ptr<Texture2D> imgCuddle;
    std::shared_ptr<Texture2D> imgProne;
    std::shared_ptr<Texture2D> imgKidnap;

    std::shared_ptr<AudioClip> sfxLaugh;
    std::shared_ptr<AudioClip> sfxPurr;
    std::shared_ptr<AudioClip> sfxBonk;
    std::shared_ptr<AudioClip> sfxScream;
    std::shared_ptr<AudioClip> sfxGoblin;
    std::shared_ptr<AudioClip> sfxMjau;
    std::shared_ptr<AudioClip> sfxCough;
    std::shared_ptr<AudioClip> sfxBreathing;

    std::shared_ptr<AudioClip> musWind;

    std::vector<UIImageComponent *> imgComps;
    std::vector<UITextComponent *> txtComps;

    WhichCutsceneAmICurrentlyIn cutscene = WhichCutsceneAmICurrentlyIn::INTRO;

    void CutsceneIntro(const AppContext& context, float prevTime);
    void CutsceneDeath(const AppContext& context, float prevTime);
    void CutsceneWin(const AppContext& context, float prevTime);

public:
    IntroCutsceneComponent(Entity *owner, bool isActive) : Component(owner, isActive) { UsesUpdate(); }

private:
    void OnStart(const AppContext &context) override;
    void Update(const AppContext& context, float deltaTime) override;

public:
    void AddImgComp(UIImageComponent *imgComp) { this->imgComps.push_back(imgComp); }
    void AddTxtComp(UITextComponent *txtComp) { this->txtComps.push_back(txtComp); }

    void SetWhichCutsceneThatIAmCurrentlyIn(WhichCutsceneAmICurrentlyIn cutscene) { this->cutscene = cutscene; }
};