#pragma once

#include <DirectXMath.h>
#include <memory>

class Texture2D;
class Font;
class AssetManager;

namespace UI {
    struct Style {
        // Text //
        std::shared_ptr<Font> font = nullptr;
        float fontSizeRegular = 32.0f;
        float fontSizeTitle = 48.0f;

        DirectX::XMFLOAT4 textColour = {1.0f, 1.0f, 1.0f, 1.0f};
        DirectX::XMFLOAT4 textColourDisabled = {0.6f, 0.6f, 0.6f, 0.6f};
        float textOutlineThickness = 0.0f;
        DirectX::XMFLOAT4 textOutlineColour = {0.0f, 0.0f, 0.0f, 1.0f};

        // Panels //
        std::shared_ptr<Texture2D> panelTexture = nullptr;
        DirectX::XMFLOAT4 panelColour = {0.4f, 0.4f, 0.4f, 1.0f};
        float panelSliceBorder = 16.0f;

        // Buttons //
        std::shared_ptr<Texture2D> buttonTextureNormal = nullptr;
        std::shared_ptr<Texture2D> buttonTextureHovered = nullptr;
        std::shared_ptr<Texture2D> buttonTexturePressed = nullptr;
        std::shared_ptr<Texture2D> buttonTextureDisabled = nullptr;
        float buttonSliceBorder = 16.0f;

        DirectX::XMFLOAT4 buttonColourNormal = {0.8f, 0.8f, 0.8f, 1.0f};
        DirectX::XMFLOAT4 buttonColourHovered = {1.0f, 1.0f, 1.0f, 1.0f};
        DirectX::XMFLOAT4 buttonColourPressed = {0.6f, 0.6f, 0.6f, 1.0f};
        DirectX::XMFLOAT4 buttonColourDisabled = {0.4f, 0.4f, 0.4f, 0.5f};

        float buttonFadeDuration = 0.5f;

        DirectX::XMFLOAT2 buttonTextPadding = {16.0f, 16.0f};
        DirectX::XMFLOAT2 buttonMinSize = {0.0f, 0.0f};

        // Checkboxes //
        std::shared_ptr<Texture2D> checkmarkTexture = nullptr;
        DirectX::XMFLOAT4 checkmarkColour = {1.0f, 1.0f, 1.0f, 1.0f};
        float checkboxSize = 48.0f;
        float checkboxSpacing = 12.0f;

        // Slider //
        DirectX::XMFLOAT2 sliderSize = {420.0f, 36.0f};
        DirectX::XMFLOAT2 sliderHandleSize = {28.0f, 44.0f};
        DirectX::XMFLOAT4 sliderFillColour = {0.6f, 0.6f, 0.6f, 1.0f};
        float sliderPadding = 5.0f;
        float sliderSliceScale = 0.5f;

        // Bars //
        DirectX::XMFLOAT2 barSize = {420.0f, 36.0f};
        std::shared_ptr<Texture2D> barFillTexture = nullptr;
        float barFillSliceBorder = 0.0f;
        DirectX::XMFLOAT4 barColourFill = {0.6f, 0.6f, 0.6f, 1.0f};
        DirectX::XMFLOAT4 barColourTrail = {0.8f, 0.8f, 0.8f, 1.0f};
        float barPadding = 5.0f;
        float barSliceScale = 0.5f;

        // Blocker // 
        DirectX::XMFLOAT4 blockerColour = {0.0f, 0.0f, 0.0f, 0.5f};

        Style(AssetManager &assetManager); // Default font
        Style(std::shared_ptr<Font> font); // Other font
    };
}