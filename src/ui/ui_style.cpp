#include "ui_style.hpp"
#include "assets/asset_manager.hpp"
#include "assets/types/font.hpp"
#include "components/ui/ui_text_component.hpp"

namespace UI {
    Style::Style(AssetManager &assetManager) : font(assetManager.Get<Font>(UITextComponent::DEFAULT_FONT_PATH)) {}
    Style::Style(std::shared_ptr<Font> font) : font(font) {}
}