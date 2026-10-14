#pragma once

#include "core/app_context.hpp"
#include "core/application.hpp"
#include "core/logging.hpp"

#include "scene/scene_loader.hpp"
#include "scene/entity.hpp"

#include "assets/asset_manager.hpp"
#include "assets/types/font.hpp"
#include "assets/types/model.hpp"
#include "assets/types/sound.hpp"
#include "assets/types/texture_2d.hpp"

#include "ui/ui_factory.hpp"
#include "ui/ui_style.hpp"
#include "ui/ui_text_layout.hpp"
#include "ui/ui_types.hpp"

#include "components/ai/enemy_controller_component.hpp"
#include "components/ai/enemy_spawner_component.hpp"

#include "components/traps/trap_component.hpp"
#include "components/traps/trap_spawner_component.hpp"

#include "components/audio/audio_component.hpp"
#include "components/audio/character_audio_component.hpp"
#include "components/audio/listener_component.hpp"
#include "components/audio/music_component.hpp"
#include "components/audio/sound_component.hpp"

#include "components/collisions/box_collision_component.hpp"
#include "components/collisions/hit_box_component.hpp"
#include "components/collisions/hurt_box_component.hpp"
#include "components/collisions/sphere_collision_component.hpp"

#include "components/core/camera_component.hpp"
#include "components/core/lifetime_component.hpp"
#include "components/core/model_component.hpp"
#include "components/core/particle_system_component.hpp"
#include "components/core/transform_component.hpp"
#include "components/core/pool_component.hpp"

#include "components/gameplay/health_component.hpp"
#include "components/gameplay/main_menu_component.hpp"
#include "components/gameplay/paintable_component.hpp"
#include "components/gameplay/player_controller_component.hpp"
#include "components/gameplay/projectile_component.hpp"
#include "components/gameplay/shake_component.hpp"
#include "components/gameplay/splatter_component.hpp"
#include "components/gameplay/gibs_component.hpp"

#include "components/lights/ambient_light_component.hpp"
#include "components/lights/directional_light_component.hpp"
#include "components/lights/light_flicker_component.hpp"
#include "components/lights/point_light_component.hpp"
#include "components/lights/spot_light_component.hpp"

#include "components/skeletal/animator_component.hpp"
#include "components/skeletal/socket_component.hpp"

#include "components/ui/ui_bar_component.hpp"
#include "components/ui/ui_button_component.hpp"
#include "components/ui/ui_canvas_component.hpp"
#include "components/ui/ui_checkbox_component.hpp"
#include "components/ui/ui_image_component.hpp"
#include "components/ui/ui_interactable_component.hpp"
#include "components/ui/ui_rect_component.hpp"
#include "components/ui/ui_slider_component.hpp"
#include "components/ui/ui_text_component.hpp"

#include <algorithm>
#include <cmath>
#include <format>

inline void SetDefaultStyle(UI::Style &style, AssetManager &assetManager) {
    style.fontSizeRegular = 28.0f;
    style.fontSizeTitle = 44.0f;
    style.textColour = {1.0f, 0.92f, 0.8f, 1.0f};

    style.panelTexture = assetManager.Get<Texture2D>("assets/images/ui/placeholder_panel.png");
    style.panelColour = {1.0f, 1.0f, 1.0f, 1.0f};
    style.panelSliceBorder = 14.0f;

    style.buttonTextureNormal = assetManager.Get<Texture2D>("assets/images/ui/placeholder_button_grey.png");
    style.buttonSliceBorder = 14.0f;
    style.buttonColourNormal = {0.55f, 0.42f, 0.4f, 1.0f};
    style.buttonColourHovered = {0.9f, 0.7f, 0.6f, 1.0f};
    style.buttonColourPressed = {0.8f, 0.2f, 0.22f, 1.0f};
    style.buttonColourDisabled = {0.4f, 0.4f, 0.4f, 0.5f};
    style.buttonFadeDuration = 0.1f;
    style.buttonTextPadding = {24.0f, 18.0f};

    style.checkmarkTexture = assetManager.Get<Texture2D>("assets/images/ui/placeholder_checkmark.png");
    style.checkmarkColour = {1.0f, 0.92f, 0.8f, 1.0f};

    style.sliderFillColour = {0.75f, 0.08f, 0.12f, 1.0f};
    style.barColourFill = {0.75f, 0.08f, 0.12f, 1.0f};
    style.barColourTrail = {1.0f, 0.85f, 0.55f, 1.0f};
}