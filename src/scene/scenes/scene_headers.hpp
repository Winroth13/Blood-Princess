#pragma once

#include "core/app_context.hpp"
#include "core/logging.hpp"

#include "scene/scene_loader.hpp"
#include "scene/entity.hpp"

#include "assets/asset_manager.hpp"
#include "assets/types/font.hpp"
#include "assets/types/model.hpp"
#include "assets/types/sound.hpp"
#include "assets/types/texture_2d.hpp"

#include "components/ai/enemy_controller_component.hpp"
#include "components/ai/enemy_spawner_component.hpp"

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

#include "components/gameplay/health_component.hpp"
#include "components/gameplay/main_menu_component.hpp"
#include "components/gameplay/paintable_component.hpp"
#include "components/gameplay/player_controller_component.hpp"
#include "components/gameplay/projectile_component.hpp"
#include "components/gameplay/shake_component.hpp"
#include "components/gameplay/splatter_component.hpp"

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