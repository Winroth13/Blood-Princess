#pragma once

#include "scene_headers.hpp"
#include "core/rng.hpp"
#include "assets/asset_manager.hpp"

#include <cstdint>
#include <format>

static Entity *CreatePlayer(Scene &outScene, const AppContext &appContext, Entity *spawnerEntity)
{
    Entity* playerEntity = outScene.AddEntity("player");
    outScene.AddEntityToGroup("players", playerEntity);
    PlayerControllerComponent *playerController = playerEntity->AddComponent<PlayerControllerComponent>();
    playerController->SetActive(true);
    playerController->SetEnemySpawner(spawnerEntity->GetComponent<EnemySpawnerComponent>());
    playerController->SetEnemyDatabase(spawnerEntity->GetComponent<EnemyDatabaseComponent>());
    playerController->SetWaveController(spawnerEntity->GetComponent<WaveController>());
    playerEntity->AddComponent<TransformComponent>();
    TransformComponent *transform = playerEntity->GetComponent<TransformComponent>();
    transform->SetWorldPosition({-17.0f, 1.6f, -8.0f});
    playerEntity->AddComponent<ListenerComponent>();
    PointLightComponent *pointLight = playerEntity->AddComponent<PointLightComponent>();
    pointLight->SetColour({ 228.0f / 255.0f, 119.0f / 255.0f, 37.0f / 255.0f });
    pointLight->SetRadius(8.0f);
    pointLight->SetIntensity(50.0f);

    playerEntity->AddComponent<SoundComponent>();

    /* Audio */
    {
        //playerEntity->AddComponent<AudioComponent>(appContext.assetManager->Get<AudioClip>("assets/audio/music/dashAction.ogg"));
        playerEntity->AddComponent<AudioComponent>(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/sword_parry_1.ogg"));
        auto characterAudio = playerEntity->AddComponent<CharacterAudioComponent>();
        characterAudio->SetIsPlayer(true);

        SoundLayer swingLayer;
        swingLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/swing_1.ogg"), 0.9f, 1.1f, 0.7f, 0.8f);
        swingLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/swing_2.ogg"), 0.9f, 1.1f, 0.7f, 0.8f);
        swingLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/swing_3.ogg"), 0.9f, 1.1f, 0.7f, 0.8f);
        swingLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/swing_4.ogg"), 0.9f, 1.1f, 0.7f, 0.8f);
        characterAudio->AddLightAttackSwingLayer(swingLayer);

        SoundLayer swordSwingLayer;
        swordSwingLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/sword_swing_1.ogg"), 0.7f, 0.9f, 1.25f, 1.45f);
        swordSwingLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/sword_swing_2.ogg"), 0.7f, 0.9f, 1.25f, 1.45f);
        swordSwingLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/sword_swing_3.ogg"), 0.7f, 0.9f, 1.25f, 1.45f);
        characterAudio->AddLightAttackSwingLayer(swordSwingLayer);

        SoundLayer squishLayer;
        squishLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/squish_1.ogg"), 0.65f, 0.85f, 0.95f, 1.05f);
        squishLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/squish_2.ogg"), 0.70f, 0.90f, 0.95f, 1.05f);
        squishLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/squish_3.ogg"), 0.70f, 0.90f, 0.93f, 1.07f);
        squishLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/squish_4.ogg"), 0.65f, 0.85f, 0.95f, 1.05f);
        squishLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/squish_5.ogg"), 0.65f, 0.85f, 0.95f, 1.05f);
        squishLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/squish_6.ogg"), 0.60f, 0.80f, 0.94f, 1.04f);

        SoundLayer boneLayer;
        boneLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/bone_1.ogg"), 0.45f, 0.60f, 0.94f, 1.06f);
        boneLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/bone_2.ogg"), 0.45f, 0.60f, 0.93f, 1.05f);
        boneLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/bone_3.ogg"), 0.45f, 0.60f, 0.95f, 1.05f);

        SoundLayer thudLayer;
        thudLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/LightAttack/thud_1.ogg"), 0.70f, 0.85f, 0.92f, 1.04f);

        characterAudio->AddLightAttackLayer(squishLayer);
        characterAudio->AddLightAttackLayer(boneLayer);
        characterAudio->AddLightAttackLayer(thudLayer);

        SoundLayer heavySwingLayer;
        heavySwingLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/heavy_swing_1.ogg"), 1.2f, 1.3f, 0.92f, 1.04f);

        SoundLayer heavyGoreLayer;
        heavyGoreLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/gore_1.ogg"), 0.75f, 0.95f, 0.93f, 1.05f);
        heavyGoreLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/gore_2.ogg"), 0.75f, 0.95f, 0.93f, 1.05f);
        heavyGoreLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/gore_3.ogg"), 0.70f, 0.90f, 0.95f, 1.05f);

        SoundLayer heavyHeavyGoreLayer;
        heavyHeavyGoreLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/heavy_gore_1.ogg"), 0.75f, 0.95f, 0.92f, 1.04f);

        SoundLayer heavyBoneLayer;
        heavyBoneLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/bone_1.ogg"), 0.65f, 0.80f, 0.92f, 1.03f);
        heavyBoneLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/bone_2.ogg"), 0.60f, 0.75f, 0.90f, 1.02f);

        SoundLayer heavyImpactLayer;
        heavyImpactLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/impact_1.ogg"), 0.80f, 1.00f, 0.92f, 1.04f);
        heavyImpactLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/impact_2.ogg"), 0.80f, 1.00f, 0.92f, 1.04f);
        heavyImpactLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/HeavyAttack/impact_3.ogg"), 0.80f, 1.00f, 0.92f, 1.04f);


        characterAudio->AddHeavyAttackSwingLayer(heavySwingLayer);
        characterAudio->AddHeavyAttackLayer(heavyGoreLayer);
        characterAudio->AddHeavyAttackLayer(heavyHeavyGoreLayer);
        characterAudio->AddHeavyAttackLayer(heavyBoneLayer);
        characterAudio->AddHeavyAttackLayer(heavyImpactLayer);


        SoundLayer footstepsLayer;
        footstepsLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/Footsteps_Mono/Male/Concrete/concrete_1.ogg"), 0.8f, 1.0f, 0.9f, 1.05f);
        footstepsLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/Footsteps_Mono/Male/Concrete/concrete_2.ogg"), 0.8f, 1.0f, 0.9f, 1.05f);
        footstepsLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/Footsteps_Mono/Male/Concrete/concrete_3.ogg"), 0.8f, 1.0f, 0.9f, 1.05f);
        footstepsLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/Footsteps_Mono/Male/Concrete/concrete_4.ogg"), 0.8f, 1.0f, 0.9f, 1.05f);
        footstepsLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/Footsteps_Mono/Male/Concrete/concrete_5.ogg"), 0.8f, 1.0f, 0.9f, 1.05f);
        footstepsLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/Footsteps_Mono/Male/Concrete/concrete_6.ogg"), 0.8f, 1.0f, 0.9f, 1.05f);
        characterAudio->AddFootstepLayer(footstepsLayer);

        SoundLayer attackShoutLayer;
        attackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_attack_01.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        attackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_attack_03.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        attackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_attack_04.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        attackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_attack_05.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        attackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_attack_06.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        attackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_attack_02.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        attackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_attack_07.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        attackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_attack_08.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        characterAudio->AddAttackShoutLayer(attackShoutLayer);

        SoundLayer heavyAttackShoutLayer;
        heavyAttackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_battle_shout_01.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        heavyAttackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_battle_shout_02.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        heavyAttackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_battle_shout_03.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        heavyAttackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_battle_shout_04.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        heavyAttackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_battle_shout_05.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        heavyAttackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_battle_shout_06.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        heavyAttackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_battle_shout_07.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        heavyAttackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_battle_shout_08.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        heavyAttackShoutLayer.AddSound(appContext.assetManager->Get<AudioClip>("assets/audio/female_a/voice_female_a_battle_shout_09.ogg"), 0.8f, 1.0f, 0.95f, 1.05f);
        characterAudio->AddHeavyAttackShoutLayer(heavyAttackShoutLayer);
    }

    /* Collisions */
    {
        auto playerCol = playerEntity->AddComponent<SphereCollisionComponent>();
        playerEntity->AddComponent<CameraComponent>();
        playerCol->SetLayer(CollisionLayer::PLAYER);
        playerCol->SetStatic(false);
        playerCol->SetRadius(0.5f);

        playerEntity->AddComponent<Health>();
        playerEntity->AddComponent<HurtboxComponent>();
    }

    /* Viewmodel */
    {
        Entity *viewEntity = outScene.AddEntity("viewmodel");
        auto *viewTransform = viewEntity->AddComponent<TransformComponent>();
        viewTransform->SetLocalPosition({ 0.0f, 0.0f, 0.0f });
        viewTransform->SetLocalScale({ 0.2f, 0.2f ,0.2f });
        viewTransform->SetLocalRotation({ 0.0f, 180.0f, 0.0f });
        ModelComponent *viewmodelModel = viewEntity->AddComponent<ModelComponent>();
        viewmodelModel->SetModel(appContext.assetManager->Get<Model>("assets/models/princess_viewmodel.glb"));
        XMFLOAT4 tint = UI::RGBA{162, 71, 63, 255};
        viewmodelModel->SetTint({tint.x, tint.y, tint.z});

        playerEntity->GetComponent<PlayerControllerComponent>()->SetRootTransform(viewTransform);

        AnimatorComponent *animator = viewEntity->AddComponent<AnimatorComponent>();
        animator->AddAnimations(viewmodelModel->GetModel()->GetAnimations());
        animator->SetSkeletonVisible(true);
        animator->Play("idle", true);
        playerEntity->GetComponent<PlayerControllerComponent>()->viewmodelAnimator = animator;

        viewEntity->SetParent(playerEntity);

        viewEntity->AddComponent<CameraComponent>();
        auto camera = viewEntity->GetComponent<CameraComponent>();

        Entity *cameraEntity = outScene.AddEntity("Camera");
        cameraEntity->AddComponent<CameraComponent>();
        cameraEntity->AddComponent<TransformComponent>();
        cameraEntity->AddComponent<ShakeComponent>();
        cameraEntity->AddComponent<ViewBobComponent>();
        cameraEntity->SetParent(viewEntity);

        playerEntity->GetComponent<PlayerControllerComponent>()->camera = cameraEntity->GetComponent<CameraComponent>();

        SocketComponent *cameraSocket = cameraEntity->AddComponent<SocketComponent>();
        cameraSocket->SetJointName("head");
        cameraSocket->SetOffsetPosition({ 0.0, 0.0f, 0.0f });
        cameraSocket->SetOffsetRotation({ -90.0, 90.0f, 0.0f });
        cameraSocket->SetOffsetScale({ 1.0f, 1.0f, 1.0f });

        Entity *swordEntity = outScene.AddEntity("sword");
        auto *swordTransform = swordEntity->AddComponent<TransformComponent>();
        ModelComponent *swordModel = swordEntity->AddComponent<ModelComponent>();
        swordModel->SetModel(appContext.assetManager->Get<Model>("assets/models/sword.glb"));
        SocketComponent *socket = swordEntity->AddComponent<SocketComponent>();
        socket->SetJointName("socket.r");
        socket->SetOffsetPosition({ -1.2f, 0.0f, 0.5f });
        socket->SetOffsetRotation({ -90.0f, -90.0f, 12.0f });
        socket->SetOffsetScale({ 1.0f, 1.0f, 1.0f });
        SwordTrailComponent *swordTrail = swordEntity->AddComponent<SwordTrailComponent>();
        swordTrail->SetPlayerController(playerController);
        swordEntity->SetParent(viewEntity);

        auto *collider = swordEntity->AddComponent<BoxCollisionComponent>();
        collider->SetLayer(CollisionLayer::PLAYER_ATTACK);
        collider->SetTrigger(true);
        collider->SetExtents({ 0.5f, 12.0f, 0.5f });
        collider->SetCenterOffset({ 0.0f, 7.5f, 0.0f });

        auto *swordHitBox = swordEntity->AddComponent<HitboxComponent>();
        swordHitBox->SetActive(false);
        swordHitBox->SetKnockbackForce(2.0f);
        playerEntity->GetComponent<PlayerControllerComponent>()->hitbox = swordHitBox;
    }

    return playerEntity;
}

static void CreatePauseSettings(Application *app, UI::Factory ui, Entity *group, float topY)
{
    // Volume
    UI::Factory::Place(
        ui.CreateTitle(group, "Volume", "volume_title"),
        UI::Anchor::TOP,
        { 0.0f, topY }
    )->SetAlignVertical(UI::AlignVertical::CENTRE);

    float masterVolume = app->GetMasterVolume();
    UITextComponent *masterLabel = UI::Factory::Place(
        ui.CreateText(group, std::format("{:.0f}%", masterVolume * 100), 0.0f, "master_label"),
        UI::Anchor::TOP,
        { -60.0f, topY + 75.0f }
    );
    masterLabel->SetAlignVertical(UI::AlignVertical::CENTRE);
    masterLabel->SetAlignHorizontal(UI::AlignHorizontal::RIGHT);
    UISliderComponent *masterDrag = UI::Factory::Place(
        ui.CreateSlider(group, 0.0f, 2.0f, masterVolume, [app, masterLabel](float value)
            {
                app->SetMasterVolume(value);
                masterLabel->SetText(std::format("{:.0f}%", value * 100));
            }, "master_slider"),
        UI::Anchor::TOP,
        { 200.0f, topY + 75.0f }
    );
    UI::Factory::Place(
        ui.CreateText(group, "Master", 0.0f, "master_text"),
        UI::Anchor::TOP_LEFT,
        { 600.0f, topY + 75.0f }
    )->SetAlignVertical(UI::AlignVertical::CENTRE);

    float soundVolume = app->GetSoundVolume();
    UITextComponent *soundLabel = UI::Factory::Place(
        ui.CreateText(group, std::format("{:.0f}%", soundVolume * 100), 0.0f, "sound_label"),
        UI::Anchor::TOP,
        { -60.0f, topY + 125.0f }
    );
    soundLabel->SetAlignVertical(UI::AlignVertical::CENTRE);
    soundLabel->SetAlignHorizontal(UI::AlignHorizontal::RIGHT);
    UISliderComponent *soundDrag = UI::Factory::Place(
        ui.CreateSlider(group, 0.0f, 2.0f, soundVolume, [app, soundLabel](float value)
            {
                app->SetSoundVolume(value);
                soundLabel->SetText(std::format("{:.0f}%", value * 100));
            }, "sound_slider"),
        UI::Anchor::TOP,
        { 200.0f, topY + 125.0f }
    );
    UI::Factory::Place(
        ui.CreateText(group, "Sound", 0.0f, "sound_text"),
        UI::Anchor::TOP_LEFT,
        { 600.0f, topY + 125.0f }
    )->SetAlignVertical(UI::AlignVertical::CENTRE);

    float musicVolume = app->GetMusicVolume();
    UITextComponent *musicLabel = UI::Factory::Place(
        ui.CreateText(group, std::format("{:.0f}%", musicVolume * 100), 0.0f, "music_label"),
        UI::Anchor::TOP,
        { -60.0f, topY + 175.0f }
    );
    musicLabel->SetAlignVertical(UI::AlignVertical::CENTRE);
    musicLabel->SetAlignHorizontal(UI::AlignHorizontal::RIGHT);
    UISliderComponent *musicDrag = UI::Factory::Place(
        ui.CreateSlider(group, 0.0f, 2.0f, musicVolume, [app, musicLabel](float value)
            {
                app->SetMusicVolume(value);
                musicLabel->SetText(std::format("{:.0f}%", value * 100));
            }, "music_slider"),
        UI::Anchor::TOP,
        { 200.0f, topY + 175.0f }
    );
    UI::Factory::Place(
        ui.CreateText(group, "Music", 0.0f, "music_text"),
        UI::Anchor::TOP_LEFT,
        { 600.0f, topY + 175.0f }
    )->SetAlignVertical(UI::AlignVertical::CENTRE);

    //Controls
    UI::Factory::Place(
        ui.CreateTitle(group, "Controls", "controls_title"),
        UI::Anchor::TOP,
        { 0.0f, topY + 250.0f }
    )->SetAlignVertical(UI::AlignVertical::CENTRE);

    float initialSens = app ? app->GetMouseSensitivity() : 0.1f;
    UITextComponent *sensLabel = UI::Factory::Place(
        ui.CreateText(group, std::format("{:.2f}", initialSens), 0.0f, "sens_label"),
        UI::Anchor::TOP,
        { -60.0f, topY + 325.0f }
    );
    sensLabel->SetAlignVertical(UI::AlignVertical::CENTRE);
    sensLabel->SetAlignHorizontal(UI::AlignHorizontal::RIGHT);
    UISliderComponent *sensSlider = UI::Factory::Place(
        ui.CreateSlider(group, 0.01f, 0.50f, initialSens, [app, sensLabel](float value)
            {
                if (app)
                    app->SetMouseSensitivity(value);
                sensLabel->SetText(std::format("{:.2f}", value));
            }, "sens_slider"),
        UI::Anchor::TOP,
        { 200.0f, topY + 325.0f }
    );
    sensSlider->SetRange(0.01f, 0.50f, 0.01f);
    UI::Factory::Place(
        ui.CreateText(group, "Sensitivity", 28.0f, "sens_text"),
        UI::Anchor::TOP_LEFT,
        { 600.0f, topY + 325.0f }
    )->SetAlignVertical(UI::AlignVertical::CENTRE);

    UICheckboxComponent *invertYCheckbox = UI::Factory::Place(
        ui.CreateCheckbox(group, "", app ? app->GetInvertY() : false, [app](bool isOn) {
            if (app)
                app->SetInvertY(isOn);
            }, "y_axis_checkbox"),
        UI::Anchor::TOP,
        { 200.0f, topY + 391.0f }
    );
    UI::Factory::Place(
        ui.CreateText(group, "Invert Y-Axis", 0.0f, "y_axis_label"),
        UI::Anchor::TOP_LEFT,
        { 600.0f, topY + 395.0f }
    )->SetAlignVertical(UI::AlignVertical::CENTRE);
}

static Entity* CreateHUD(
    Scene &outScene,
    const AppContext &appContext,
    Entity *player,
    WaveController *waveController,
    LevelComponent *level,
    EnemySpawnerComponent *enemySpawner
) {
    std::shared_ptr<Font> font = appContext.assetManager->Get<Font>("assets/fonts/YoungSerif-Regular.ttf");

    UI::Factory ui(outScene, font);
    SetDefaultStyle(ui.style, *appContext.assetManager);

    ui.style.panelTexture = appContext.assetManager->Get<Texture2D>("assets/images/ui/placeholder_button_hovered.png");
    ui.style.panelSliceBorder = 20.0f;
    ui.style.panelColour = {1.0f, 1.0f, 1.0f, 1.0f};
    ui.style.barColourFill = UI::RGBA{172, 47, 32, 255};
    ui.style.barColourTrail = {1.0f, 0.85f, 0.55f, 1.0f};

    Entity *hud = ui.CreateCanvas("hud");

    Health *health = player->GetComponent<Health>();

    UIBarComponent *healthBar = ui.CreateBar(hud, health->GetCurrentHealth(), health->GetMaxHealth(), "health_bar");
    UI::Factory::Place(healthBar, UI::Anchor::TOP_LEFT, {40.0f, 40.0f});

    UITextComponent *healthLabel = ui.CreateText(healthBar->GetOwner(), "", 22.0f, "health_label");
    UI::Factory::Place(healthLabel, UI::Anchor::STRETCH, {0.0f, 0.0f}, {0.0f, 0.0f});
    healthLabel->SetAlignment(UI::AlignHorizontal::CENTRE, UI::AlignVertical::CENTRE);
    healthLabel->SetOutline(2.0f, {0.0f, 0.0f, 0.0f, 1.0f});

    auto showHealthFunc = [healthBar, healthLabel](float current, float max) {
        healthBar->SetValue(current, max);
        healthLabel->SetText(std::format("{:.0f} / {:.0f}", current, max));
    };

    showHealthFunc(health->GetCurrentHealth(), health->GetMaxHealth());
    health->GetOnChangedHealth().Connect(showHealthFunc);

    PlayerControllerComponent *controller = player->GetComponent<PlayerControllerComponent>();

    ui.style.barSize = {210.0f, 20.0f};
    ui.style.barColourFill = UI::RGBA{15, 202, 119, 255};
    ui.style.barColourTrail = {0.0f, 0.0f, 0.0f, 0.0f};

    const float dashCooldownDuration = controller->GetDashCooldownDuration();

    UIBarComponent *dashBar = ui.CreateBar(hud, dashCooldownDuration, dashCooldownDuration, "dash_bar");
    UI::Factory::Place(dashBar, UI::Anchor::TOP_LEFT, {40.0f, 85.0f});

    controller->GetOnDashCooldownChanged().Connect([dashBar](float timer) {
        dashBar->SetValue(dashBar->GetMaxValue() - timer);
    });

    // Score
    Entity *scoreGroup = ui.CreateGroup(hud, { 50.0f, 150.0f }, "score_group");
    UI::Factory::Place(scoreGroup, UI::Anchor::TOP_RIGHT, { -205.0f, 250.0f });

    auto comboComponent = scoreGroup->AddComponent<ComboUIComponent>();

    /*UITextComponent *scoreLabel = ui.CreateText(scorePanel, "Score: 0", 22.0f, "combo_score_label");
    UI::Factory::Place(scoreLabel, UI::Anchor::TOP_LEFT, { 20.0f, 10.0f });
    //comboLabel->SetOutline(2.0f, { 0.0f, 0.0f, 0.0f, 1.0f });*/

    /*level->GetOnScoreChanged().Connect([scoreLabel](uint32_t score)
    {
        scoreLabel->SetText("Score: " + std::to_string(score));
    });*/

    /* UITextComponent *comboScoreLabel = ui.CreateText(scorePanel, "Combo Score: 0", 22.0f, "combo_score_label");
    UI::Factory::Place(comboScoreLabel, UI::Anchor::TOP_LEFT, { 20.0f, 40.0f });
    //comboLabel->SetOutline(2.0f, { 0.0f, 0.0f, 0.0f, 1.0f });

    controller->GetOnScoreChanged().Connect([comboScoreLabel](uint32_t score)
    {
        comboScoreLabel->SetText("Combo Score: " + std::to_string(score));
    });*/

    UITextComponent *comboLeftLabel = ui.CreateText(scoreGroup, "x", 30.0f, "combo_x_label");
    UI::Factory::Place(comboLeftLabel, UI::Anchor::TOP_LEFT, { 0.0f, 70.0f });
    comboLeftLabel->SetOutline(4.0f, { 0.0f, 0.0f, 0.0f, 1.0f });
    comboLeftLabel->SetAlignHorizontal(UI::AlignHorizontal::CENTRE);
    comboLeftLabel->SetAlignVertical(UI::AlignVertical::CENTRE);
    comboComponent->SetXLabel(comboLeftLabel);

    UITextComponent *comboLabel = ui.CreateText(scoreGroup, "3", 50.0f, "combo_label");
    UI::Factory::Place(comboLabel, UI::Anchor::TOP_RIGHT, { 20.0f, 35.0f });
    comboLabel->SetOutline(4.0f, { 0.0f, 0.0f, 0.0f, 1.0f });
    comboLabel->SetAlignHorizontal(UI::AlignHorizontal::CENTRE);
    comboLabel->SetAlignVertical(UI::AlignVertical::CENTRE);
    comboComponent->SetComboLabel(comboLabel);

    comboComponent->SetPlayerController(controller);

    /*UIBarComponent *comboBar = ui.CreateBar(scorePanel, 0.0f, controller->GetComboDuration(), "combo_bar");
    UI::Factory::Place(comboBar, UI::Anchor::TOP_LEFT, { 20.0f, 110.0f });

    controller->GetOnComboRemainingTimeChanged().Connect([comboBar](float comboTimeRemaining)
    {
        comboBar->SetValue(comboTimeRemaining);
    });*/

    ui.style.barSize = {420.0f, 15.0f};
    ui.style.barColourFill = UI::RGBA{243, 187, 130, 255};

    UIBarComponent *progressBar = ui.CreateBar(hud, 0.0f, 1.0f, "progress_bar");
    UI::Factory::Place(progressBar, UI::Anchor::TOP, {0.0f, 40.0f});

    waveController->GetOnFloorProgressChanged().Connect([progressBar] (float progress) {
        progressBar->SetValue(static_cast<float>(progress));
    });

    Entity *blocker = ui.CreateBlocker(hud);
    blocker->SetLocallyActive(false);

    CreatePauseSettings(appContext.application, ui, blocker, 200.0f);

    UIButtonComponent *continueButton = UI::Factory::Place(ui.CreateButton(blocker, "CONTINUE", nullptr, "btn_continue"), UI::Anchor::CENTRE, { 0.0f, 200.0f });
    continueButton->GetOnClick().Connect([controller]() {
        controller->SetIsPaused(false);
    });

    UIButtonComponent *quitButton = UI::Factory::Place(ui.CreateButton(blocker, "QUIT", nullptr, "btn_quit"), UI::Anchor::CENTRE, { 0.0f, 300.0f });
    quitButton->GetOnClick().Connect([appContext]() {
        appContext.sceneManager->RequestSceneChange("main_menu");
    });

    SetDefaultSounds(outScene, appContext, { continueButton, quitButton });

    controller->GetOnPaused().Connect([blocker]() {
        blocker->SetLocallyActive(true);
    });

    controller->GetOnUnpaused().Connect([blocker]() {
        blocker->SetLocallyActive(false);
    });

    // Tutorial Popup
    Entity *popupPanel = ui.CreatePanel(hud, {1000.0f, 130.0f}, "tutorial_popup");
    UI::Factory::Place(popupPanel, UI::Anchor::CENTRE, { 0.0f, -180.0f });

    UITextComponent *tutorialText = ui.CreateText(popupPanel, "", 30.0f, "tutorial_text");
    UI::Factory::Place(tutorialText, UI::Anchor::STRETCH, { 0.0f, 0.0f});
    tutorialText->SetAlignment(UI::AlignHorizontal::CENTRE, UI::AlignVertical::CENTRE);
    tutorialText->SetOutline(2.0f, { 0.0f, 0.0f, 0.0f, 1.0f });

    TutorialComponent *tutorialComp = hud->AddComponent<TutorialComponent>();
    tutorialComp->Setup(popupPanel, tutorialText, enemySpawner, controller, waveController);

    waveController->SetTutorialComponent(tutorialComp);

    return hud;
}

static Entity *CreateLevelEndScreen(
    Scene &outScene, 
    const AppContext 
    &appContext,
    float time,
    uint32_t score,
    uint32_t kills
)
{
    std::shared_ptr<Font> font = appContext.assetManager->Get<Font>("assets/fonts/YoungSerif-Regular.ttf");

    UI::Factory ui(outScene, font);
    SetDefaultStyle(ui.style, *appContext.assetManager);

    Entity *canvas = ui.CreateCanvas("level_end_screen_canvas");
    auto levelEndScreen = canvas->AddComponent<LevelEndscreenComponent>();

    UITextComponent *rubricLabel = ui.CreateText(canvas, "", 48.0f, "rubric_label");
    rubricLabel->SetText("FLOOR CLEARED");
    UI::Factory::Place(rubricLabel, UI::Anchor::CENTRE, { 0.0f, -128.0f }, { 0.0f, 0.0f });
    rubricLabel->SetAlignment(UI::AlignHorizontal::CENTRE, UI::AlignVertical::CENTRE);
    rubricLabel->SetOutline(2.0f, { 0.0f, 0.0f, 0.0f, 1.0f });
    levelEndScreen->SetRubricLabel(rubricLabel);

    float startY = 350.0f;

    std::array<std::string, 3> labelRubrics = 
    {
        "Time:",
        "Score:",
        "Kills:"
    };
    
    for (size_t i = 0; i < 3; ++i)
    {
        Entity *timeGroup = ui.CreateGroup(canvas, { 250, 350 }, "time_group");
        UI::Factory::Place(timeGroup, UI::Anchor::CENTRE, { 0.0f, startY + i * 64.0f });
        
        auto leftLabel = UI::Factory::Place(ui.CreateText(timeGroup, labelRubrics[i], 24.0f, "left_label"), UI::Anchor::TOP_LEFT, {0.0f, 125.0f});
        leftLabel->SetColour({ 1.0f, 0.92f, 0.8f, 0.8f });
        leftLabel->SetOutline(2.0f, { 0.0f, 0.0f, 0.0f, 1.0f });

        std::string rightStr = {};
        if (i == 0) {
            rightStr = std::to_string(time);
        }
        else if (i == 1) {
            rightStr = std::to_string(score);
        }
        else if (i == 2) {
            rightStr = std::to_string(kills);
        }

        auto rightLabel = UI::Factory::Place(ui.CreateText(timeGroup, rightStr, 32.0f, "right_label"), UI::Anchor::TOP_RIGHT, { 0.0f, 117.0f });
        rightLabel->SetAlignVertical(UI::AlignVertical::CENTRE);
        rightLabel->SetAlignHorizontal(UI::AlignHorizontal::RIGHT);
        rightLabel->SetColour({ 0.8f, 0.4f, 0.4f, 1.0f });
        rightLabel->SetOutline(2.0f, { 0.0f, 0.0f, 0.0f, 1.0f });

        if (i == 0) {
            levelEndScreen->SetTimeLabel(rightLabel);
        }
        else if (i == 1) {
            levelEndScreen->SetScoreLabel(rightLabel);
        }
        else if (i == 2) {
            levelEndScreen->SetKillsLabel(rightLabel);
        }
    }

    /* Continue */
    {
        UITextComponent *continueLabel = ui.CreateText(canvas, "", 24.0f, "ascend_label");
        continueLabel->SetText("Press Space to Ascend");
        UI::Factory::Place(continueLabel, UI::Anchor::BOTTOM_RIGHT, { -64.0f, -110.0f }, { 0.0f, 0.0f });
        continueLabel->SetAlignment(UI::AlignHorizontal::RIGHT, UI::AlignVertical::TOP);
        continueLabel->SetOutline(2.0f, { 0.0f, 0.0f, 0.0f, 1.0f });
        levelEndScreen->SetAscendLabel(continueLabel);
    }

    return canvas;
}

static void CreateCircularRoom(Scene &outScene, const AppContext &appContext, const DirectX::XMFLOAT3 &center, int numSegments)
{
    if (numSegments <= 0)
    {
        return;
    }

    auto wallModel = appContext.assetManager->Get<Model>("assets/models/wall.glb");
    auto torchModel = appContext.assetManager->Get<Model>("assets/models/torch.glb");
    auto floorModel = appContext.assetManager->Get<Model>("assets/models/floor.glb");
    auto ceilingModel = appContext.assetManager->Get<Model>("assets/models/ceiling.glb");
    auto flameTexture = appContext.assetManager->Get<Texture2D>("assets/images/flame_spritesheet.png");
    auto smokeTexture = appContext.assetManager->Get<Texture2D>("assets/images/smoke_spritesheet.png");
    auto fireSound = appContext.assetManager->Get<AudioClip>("assets/audio/music/egomassive__fire.ogg"); 

    const float wallLength = 10.0f;
    const float halfWallLength = wallLength * 0.5f;
    const float deltaAngle = DirectX::XM_2PI / static_cast<float>(numSegments);
    const float halfAngle = deltaAngle * 0.5f;
    // Apothem to the inner face of the wall segments so adjacent inner corners meet seamlessly
    const float innerRadius = halfWallLength / tanf(halfAngle);
    // Wall center is offset by half the wall thickness (0.5f)
    const float wallRadius = innerRadius + 0.5f;

    Entity *walls = outScene.AddEntity("Walls");

    // 1. Build walls (2 walls high) and torches
    for (int i = 0; i < numSegments; ++i)
    {
        float theta = static_cast<float>(i) * deltaAngle;
        float posX = center.x + wallRadius * cosf(theta);
        float posY = center.y + 2.5f;
        float posZ = center.z + wallRadius * sinf(theta);

        // Yaw in degrees such that local +X points inwards toward the center of the room
        float yaw = 180.0f - DirectX::XMConvertToDegrees(theta);

        // Lower wall segment entity
        std::string wallName = "Wall_" + std::to_string(i);
        Entity *wallEntity = outScene.AddEntity(wallName);
        wallEntity->SetParent(walls);

        TransformComponent *wallTransform = wallEntity->AddComponent<TransformComponent>();
        wallTransform->SetWorldPosition(posX, posY, posZ);
        wallTransform->SetWorldRotation(0.0f, yaw, 0.0f);

        BoxCollisionComponent *wallCol = wallEntity->AddComponent<BoxCollisionComponent>();
        wallCol->SetLayer(CollisionLayer::ENVIRONMENT);
        wallCol->SetStatic(true);
        wallCol->SetExtents({ 0.5f, 2.5f, halfWallLength });

        ModelComponent *wallModelComponent = wallEntity->AddComponent<ModelComponent>();
        wallModelComponent->SetModel(wallModel);
        wallModelComponent->SetTint({150.0f / 255.0f, 150.0f / 255.0f, 150.0f / 255.0f});
        wallModelComponent->SetIsHsvAdjusted(true);

        PaintableComponent* wallPaintable = wallEntity->AddComponent<PaintableComponent>();
        wallPaintable->SetPaintMask(appContext.assetManager->Get<Texture2D>("assets/images/blood_splat_mask.png"));
        wallPaintable->SetPaintMaskWidth(128);
        wallPaintable->SetPaintMaskHeight(128);

        // Upper wall segment entity stacked 1 wall higher directly on top
        std::string upperWallName = "Wall_Upper_" + std::to_string(i);
        Entity *upperWallEntity = outScene.AddEntity(upperWallName);
        upperWallEntity->SetParent(wallEntity);

        TransformComponent *upperWallTransform = upperWallEntity->AddComponent<TransformComponent>();
        upperWallTransform->SetLocalPosition(0.0f, 5.0f, 0.0f);
        upperWallTransform->SetLocalRotation(0.0f, 0.0f, 0.0f);

        BoxCollisionComponent *upperWallCol = upperWallEntity->AddComponent<BoxCollisionComponent>();
        upperWallCol->SetLayer(CollisionLayer::ENVIRONMENT);
        upperWallCol->SetStatic(true);
        upperWallCol->SetExtents({ 0.5f, 2.5f, halfWallLength });

        ModelComponent *upperWallModelComponent = upperWallEntity->AddComponent<ModelComponent>();
        upperWallModelComponent->SetModel(wallModel);
        upperWallModelComponent->SetTint({150.0f / 255.0f, 150.0f / 255.0f, 150.0f / 255.0f});
        upperWallModelComponent->SetIsHsvAdjusted(true);

        PaintableComponent* upperWallPaintable = upperWallEntity->AddComponent<PaintableComponent>();
        upperWallPaintable->SetPaintMask(appContext.assetManager->Get<Texture2D>("assets/images/blood_splat_mask.png"));
        upperWallPaintable->SetPaintMaskWidth(128);
        upperWallPaintable->SetPaintMaskHeight(128);

        // Torch entity attached to the lower wall
        std::string torchName = "Torch_" + std::to_string(i);
        Entity *torchEntity = outScene.AddEntity(torchName);
        torchEntity->SetParent(wallEntity);

        TransformComponent *torchTransform = torchEntity->AddComponent<TransformComponent>();
        // Positioned on the inner face of the wall segment at eye level, offset slightly outward so the torch holder is clearly visible
        torchTransform->SetLocalPosition(0.58f, 0.0f, 0.0f);
        torchTransform->SetLocalRotation(0.0f, 0.0f, 0.0f);
        torchTransform->SetLocalScale(2.0f, 2.0f, 2.0f);


        ModelComponent *torchModelComponent = torchEntity->AddComponent<ModelComponent>();
        torchModelComponent->SetModel(torchModel);
        torchModelComponent->SetIsHsvAdjusted(true);

        // Fire particle entity attached to the torch
        std::string fireName = "FireParticle_" + std::to_string(i);
        Entity *fireParticleEntity = outScene.AddEntity(fireName);
        fireParticleEntity->SetParent(torchEntity);

        TransformComponent *fireTransform = fireParticleEntity->AddComponent<TransformComponent>();
        fireTransform->SetLocalPosition(0.17f, 0.3f, 0.0f);

        // Flame particle system
        ParticleSystemComponent *flame = fireParticleEntity->AddComponent<ParticleSystemComponent>();
        flame->SetAtlasTexture(flameTexture);
        flame->SetAtlasSize({ 12.0f, 6.0f });
        flame->SetVelocity({ 0.0f, 0.0f, 0.0f });
        flame->SetAcceleration({ 0.0f, 0.0f, 0.0f });
        flame->SetLifeTime(2.0f);
        flame->SetStartScale(0.5f);
        flame->SetEndScale(0.5f);
        flame->SetIsAnimate(true);
        flame->SetIsAdditive(true);
        flame->SetSpawnRadius(0.01f);
        flame->SetAnimationSpeed(1.2f);
        flame->SetCount(2);

        // Smoke particle system
        ParticleSystemComponent *smoke = fireParticleEntity->AddComponent<ParticleSystemComponent>();
        smoke->SetAtlasTexture(smokeTexture);
        smoke->SetAtlasSize({ 13.0f, 1.0f });
        smoke->SetVelocity({ 0.0f, 0.1f, 0.0f });
        smoke->SetAcceleration({ 0.0f, 0.0f, 0.0f });
        smoke->SetLifeTime(5.0f);
        smoke->SetStartScale(0.2f);
        smoke->SetEndScale(0.5f);
        smoke->SetIsAnimate(true);
        smoke->SetIsAdditive(true);
        smoke->SetSpawnRadius(0.08f);
        smoke->SetCount(25);
        smoke->SetStartTint({ 150.0f / 255.0f, 150.0f / 255.0f, 150.0f / 255.0f });
        smoke->SetEndTint({ 0.0f, 0.0f, 0.0f });

        // Point light on torch
        PointLightComponent *pointLight = fireParticleEntity->AddComponent<PointLightComponent>();
        pointLight->SetColour({ 228.0f / 255.0f, 119.0f / 255.0f, 37.0f / 255.0f });

        pointLight->SetRadius(8.0f);
        pointLight->SetIntensity(50.0f);

        fireParticleEntity->AddComponent<LightFlickerComponent>();

        // 3D Fire audio
        AudioComponent *audioComponent = fireParticleEntity->AddComponent<AudioComponent>(fireSound);
        audioComponent->SetLooping(true);
        audioComponent->SetAudio3D(true);
        audioComponent->SetMinDistance(3.0f);
        audioComponent->SetMaxDistance(15.0f);
        audioComponent->SetVolume(0.8f);
        audioComponent->SetPlayOnStart(true);
    }

    // 2. Build floor and ceiling covering the area within the circumference of the wall circle
    const float tileSize = 10.0f;
    const float halfTile = tileSize * 0.5f;
    // Floor top surface is flush with the bottom of the lower walls (Y = center.y - 2.5f)
    const float floorY = center.y - 0.5f;
    // Ceiling bottom surface is flush with the top of the upper walls (Y = center.y + 7.5f)
    const float ceilingY = center.y + 10.5f;

    Entity *floorRoot = outScene.AddEntity("Floor");
    floorRoot->AddComponent<TransformComponent>()->SetWorldPosition(center.x, floorY, center.z);

    Entity *ceilingRoot = outScene.AddEntity("Ceiling");
    ceilingRoot->AddComponent<TransformComponent>()->SetWorldPosition(center.x, ceilingY, center.z);

    int tileIndex = 0;
    for (float x = -30.0f; x <= 30.0f; x += tileSize)
    {
        for (float z = -30.0f; z <= 30.0f; z += tileSize)
        {
            float closestX = (fabsf(x) <= halfTile) ? 0.0f : (fabsf(x) - halfTile);
            float closestZ = (fabsf(z) <= halfTile) ? 0.0f : (fabsf(z) - halfTile);
            float distSq = closestX * closestX + closestZ * closestZ;

            if (distSq < wallRadius * wallRadius)
            {
                // Floor tile
                std::string floorTileName = "FloorTile_" + std::to_string(tileIndex);
                Entity *floorTile = outScene.AddEntity(floorTileName);
                floorTile->SetParent(floorRoot);

                TransformComponent *floorTransform = floorTile->AddComponent<TransformComponent>();
                floorTransform->SetLocalPosition(x, 0.0f, z);

                ModelComponent *floorComp = floorTile->AddComponent<ModelComponent>();
                floorComp->SetModel(floorModel);
                floorComp->SetIsHsvAdjusted(true);

                BoxCollisionComponent *floorCollision = floorTile->AddComponent<BoxCollisionComponent>();
                floorCollision->SetStatic(true);
                floorCollision->SetLayer(CollisionLayer::ENVIRONMENT);
                floorCollision->SetExtents({ halfTile, 0.5f, halfTile });

                PaintableComponent* floorPaintable = floorTile->AddComponent<PaintableComponent>();
                floorPaintable->SetPaintMask(appContext.assetManager->Get<Texture2D>("assets/images/blood_splat_mask.png"));
                floorPaintable->SetPaintMaskWidth(128);
                floorPaintable->SetPaintMaskHeight(128);

                // Ceiling tile
                std::string ceilingTileName = "CeilingTile_" + std::to_string(tileIndex);
                Entity *ceilingTile = outScene.AddEntity(ceilingTileName);
                ceilingTile->SetParent(ceilingRoot);

                TransformComponent *ceilingTransform = ceilingTile->AddComponent<TransformComponent>();
                ceilingTransform->SetLocalPosition(x, 0.0f, z);

                BoxCollisionComponent *ceilingCol = ceilingTile->AddComponent<BoxCollisionComponent>();
                ceilingCol->SetLayer(CollisionLayer::ENVIRONMENT);
                ceilingCol->SetStatic(true);
                ceilingCol->SetExtents({ halfTile, 0.5f, halfTile });

                ModelComponent *ceilingComp = ceilingTile->AddComponent<ModelComponent>();
                ceilingComp->SetModel(ceilingModel);
                ceilingComp->SetIsHsvAdjusted(true);

                tileIndex++;
            }
        }
    }
}

static Entity *CreateFade(Scene &outScene, const AppContext &appContext)
{
    UI::Factory ui(outScene, nullptr);
    Entity *canvas = ui.CreateCanvas("fade_canvas");
    UIImageComponent *blackImage = UI::Factory::Place(ui.CreateImage(canvas, nullptr, "black"), UI::Anchor::STRETCH, { 0.0f, 0.0f }, { 0.0f, 0.0f });
    blackImage->SetColour({ 0.0f, 0.0f, 0.0f, 1.0f });
    blackImage->SetRaycastTarget(false);

    return canvas;
}

static Entity *CreateGibs(Scene &outScene, const AppContext &appContext)
{
    Entity *gibsEntity = outScene.AddEntity("gibs");
    gibsEntity->AddComponent<GibsComponent>();
    auto *gibPool = gibsEntity->AddComponent<PoolComponent>();

    gibPool->SetCreateEntityCallback([&outScene, &appContext]()
    {
        Entity *gib = outScene.AddEntity("gib");
        auto *gibTransform = gib->AddComponent<TransformComponent>();
        auto *modelComponent = gib->AddComponent<ModelComponent>();
        auto *gibParticle = gib->AddComponent<ParticleSystemComponent>();
        auto *projectileGib = gib->AddComponent<ProjectileComponent>();
        auto *sphereCollisionGib = gib->AddComponent<SphereCollisionComponent>();
        auto *splatterGib = gib->AddComponent<SplatterComponent>();
        auto *lifeTimeGib = gib->AddComponent<LifeTimeComponent>();

        gibParticle->SetSpawnRadius(RNG::Float(0.05f, 0.35f));
        gibParticle->SetCount(RNG::Int(8, 16));
        gibParticle->SetIsAnimate(true);
        gibParticle->SetIsGlobal(true);
        gibParticle->SetIsAdditive(false);
        gibParticle->SetStartTint({ 0.1f, 0.01f, 0.01f });
        gibParticle->SetEndTint({ 0.05f, 0.0f, 0.0f });
        gibParticle->SetAtlasTexture(appContext.assetManager->Get<Texture2D>("assets/images/detailed_smoke_spritesheet.png"));
        gibParticle->SetAtlasSize({ 8.0f, 8.0f });

        const float pAngle = RNG::Float(0.0f, DirectX::XM_2PI);
        const float pSpeed = RNG::Float(1.0f, 2.0f);
        gibParticle->SetVelocity({
            cosf(pAngle) * pSpeed,
            RNG::Float(0.1f, 0.3f),
            sinf(pAngle) * pSpeed
            });
        gibParticle->SetAcceleration({ 0.0f, -9.82f, 0.0f });

        const float pScale = RNG::Float(0.3f, 0.65f);
        gibParticle->SetStartScale(pScale);
        gibParticle->SetEndScale(pScale);

        projectileGib->SetGravity(9.82f);
        projectileGib->SetRestitution(RNG::Float(0.3f, 0.45f));
        projectileGib->SetIsMissile(false);

        sphereCollisionGib->SetRadius(RNG::Float(0.19f, 0.39f));
        sphereCollisionGib->SetLayer(CollisionLayer::GIBS);
        sphereCollisionGib->SetActive(true);
        sphereCollisionGib->SetStatic(false);
        sphereCollisionGib->SetTrigger(true);
        
        splatterGib->SetPaintSpacing(RNG::Float(0.4f, 1.0f));

        return gib;
    });

    return gibsEntity;
}

namespace Scenes 
{
    void Gameplay(Scene& outScene, const AppContext& appContext) 
    {
        //Entity *musicEntity = outScene.AddEntity("music_entity");
        //musicEntity->AddComponent<TransformComponent>();
        //MusicComponent *musicComponent = musicEntity->AddComponent<MusicComponent>(appContext.assetManager->Get<AudioClip>("assets/audio/music/Test2.ogg"));
        //musicComponent->SetVolume(0.2f);

        // Gibs
        Entity *gibsEntity = CreateGibs(outScene, appContext);

        // Enemy Spawner
        Entity* spawnerEntity = outScene.AddEntity("level_entity");
        spawnerEntity->AddComponent<TransformComponent>()->SetWorldPosition({ 0, 0, 0 });
        spawnerEntity->AddComponent<EnvironmentComponent>();
        spawnerEntity->AddComponent<EnemyDatabaseComponent>();
        EnemySpawnerComponent* enemySpawner = spawnerEntity->AddComponent<EnemySpawnerComponent>();
        enemySpawner->SetGibsComponent(gibsEntity->GetComponent<GibsComponent>());
        spawnerEntity->AddComponent<TrapSpawnerComponent>();
        WaveController *waveController = spawnerEntity->AddComponent<WaveController>();

        LevelComponent *level = spawnerEntity->AddComponent<LevelComponent>();
        
        auto *levelAudio = spawnerEntity->AddComponent<AudioComponent>(appContext.assetManager->Get<AudioClip>("assets/audio/sfx/end_level.ogg"));
        levelAudio->SetAudio3D(false);
        levelAudio->SetVolume(4.0f);
        levelAudio->SetPitch(1.0f);
        levelAudio->SetLooping(false);

        Entity *endScreen = CreateLevelEndScreen(outScene, appContext, 100.0f, 80, 500);
        endScreen->SetLocallyActive(false);
        level->SetEndScreen(endScreen);

        // Player
        Entity *playerEntity = CreatePlayer(outScene, appContext, spawnerEntity);
        Health *health = playerEntity->GetComponent<Health>();
        level->SetPlayerController(playerEntity->GetComponent<PlayerControllerComponent>());

        Entity *hud = CreateHUD(outScene, appContext, playerEntity, waveController, level, enemySpawner);
        level->SetHUDEntity(hud);

        Entity *lightEntity = outScene.AddEntity("light");
        AmbientLightComponent *ambientLight = lightEntity->AddComponent<AmbientLightComponent>();
        ambientLight->SetColour({0.86f, 0.59f, 0.47f});
        ambientLight->SetIntensity(0.2f);

        Entity *fadeCanvas = CreateFade(outScene, appContext);
        level->SetFadeEntity(fadeCanvas);

        /*lightEntity->AddComponent<DirectionalLightComponent>()->SetIntensity(3.0f);
        TransformComponent *transform = lightEntity->AddComponent<TransformComponent>();
        transform->SetWorldPosition(0.0f, 0.0f, 0.0f);
        transform->SetWorldRotation(30.0f, 0.0f, 0.0f);

        Entity *spotLightEntity = outScene.AddEntity("spot light");
        SpotLightComponent *spotLight = spotLightEntity->AddComponent<SpotLightComponent>();
        spotLight->SetIntensity(50.0f);
        spotLight->SetRange(30.0f);
        spotLight->SetColor({ 231.0f / 255.0f, 172.0f / 255.0f, 80.0f / 255.0f });
        spotLight->SetInnerAngle(XMConvertToRadians(60.0f));
        spotLight->SetOuterAngle(XMConvertToRadians(80.0f));
        TransformComponent *spotTransform = spotLightEntity->AddComponent<TransformComponent>();
        spotTransform->SetWorldPosition(-8.25f, -0.7f, 5.07f);
        spotTransform->SetWorldRotation(0, 116, 0);

        Entity *pointLightEntity = outScene.AddEntity("point light");
        PointLightComponent *pointLight = pointLightEntity->AddComponent<PointLightComponent>();
        pointLight->SetIntensity(220.0f);
        pointLight->SetRadius(10.0f);
        pointLight->SetCastShadows(false);
        TransformComponent *pointTransform = pointLightEntity->AddComponent<TransformComponent>();
        pointTransform->SetWorldPosition(-0.6f, -1.0f, 6.8f);*/

        CreateCircularRoom(outScene, appContext, { 0.0f, 0.0f, 0.0f }, 16);

        {
            //Entity *entity = outScene.AddEntity("animated_entity_twerk");
            //TransformComponent *transform = entity->AddComponent<TransformComponent>();
            //transform->SetWorldPosition(-1.0f, -2.5f, 8.0f);
            //transform->SetWorldRotation(0.0f, 180.0f, 0.0f);
            //transform->SetWorldScale(0.5f, 0.5f, 0.5f);
            //ModelComponent *modelComponent = entity->AddComponent<ModelComponent>();
            //std::shared_ptr<Model> model = appContext.assetManager->Get<Model>("assets/models/animations/princess_twerk.glb");
            //modelComponent->SetModel(model);
            //AnimatorComponent *animator = entity->AddComponent<AnimatorComponent>();
            //animator->AddAnimation("twerk", model->GetAnimations());
            //animator->SetSkeletonVisible(true);
            //animator->Play("twerk", true);
        }

        {
            //Entity *entity = outScene.AddEntity("animated_entity_gangnam_style");
            //TransformComponent *transform = entity->AddComponent<TransformComponent>();
            //transform->SetWorldPosition(1.0f, -2.5f, 8.0f);
            //transform->SetWorldRotation(0.0f, 180.0f, 0.0f);
            //ModelComponent *modelComponent = entity->AddComponent<ModelComponent>();
            //std::shared_ptr<Model> model = appContext.assetManager->Get<Model>("assets/models/animations/demo_gangnam_style.glb");
            //modelComponent->SetModel(model);
            //AnimatorComponent *animator = entity->AddComponent<AnimatorComponent>();
            //animator->AddAnimation("gangnam_style", model->GetAnimations());
            //animator->SetSkeletonVisible(true);
            //animator->Play("gangnam_style", true);
        }

        {
            //Entity *entity = outScene.AddEntity("animation_light");
            //TransformComponent *transform = entity->AddComponent<TransformComponent>();
            //transform->SetWorldPosition(0.0f, 0.5f, 8.0f);
            //PointLightComponent *pointLight = entity->AddComponent<PointLightComponent>();
            //pointLight->SetColour({ 228.0f / 255.0f, 119.0f / 255.0f, 37.0f / 255.0f });
            //pointLight->SetRadius(8.0f);
            //pointLight->SetIntensity(50.0f);
        }

        //outScene.PrintEntityHierarchy();
    }
}