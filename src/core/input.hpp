#pragma once
#include "SDL3/SDL.h"
#include "SDL3/SDL_keyboard.h"
#include <string>
#include <iostream>
#include <array>
#include "logging.hpp" 
#include <DirectXMath.h>

enum class MouseMode 
{
	Visible,
	Hidden,
	Captured
};

struct MouseState 
{
	DirectX::XMFLOAT2 position = { 0, 0 };
	DirectX::XMFLOAT2 wheel = { 0, 0 };
	DirectX::XMFLOAT2 delta = { 0, 0 };
	SDL_MouseButtonFlags buttonFlags = 0;
	SDL_MouseButtonFlags previousButtonFlags = 0;
};

enum class MouseButton 
{
	Left = SDL_BUTTON_LEFT,
	Right = SDL_BUTTON_RIGHT,
	Middle = SDL_BUTTON_MIDDLE,
	X1 = SDL_BUTTON_X1,
	X2 = SDL_BUTTON_X2
};


class Input {
private:
	static const bool* keyState;
	static std::array<bool, SDL_SCANCODE_COUNT> lastKeyState;

	static MouseMode mouseMode;
	static MouseState lastMouseState;
public:
	static void Initialize();

	static void Update(SDL_Window* window);

	static void Shutdown();
	static void HandleEvent(const SDL_Event& event);
	static bool IsKeyPressed(SDL_Scancode scancode);
	static bool IsKeyReleased(SDL_Scancode scancode);
	static bool IsKeyDown(SDL_Scancode scancode);

	static bool IsKeyPressed(SDL_Keycode keycode);
	static bool IsKeyReleased(SDL_Keycode keycode);
	static bool IsKeyDown(SDL_Keycode keycode);

	
	static DirectX::XMFLOAT2 GetMousePosition();
	static float GetMouseX();
	static float GetMouseY();

	static DirectX::XMFLOAT2 GetMouseDelta();
	static float GetMouseDeltaX();
	static float GetMouseDeltaY();

	static void OnMouseWheelEvent(float x, float y);
	static DirectX::XMFLOAT2 GetMouseWheel();
	static float GetMouseWheelX();
	static float GetMouseWheelY();

	static bool IsMouseButtonDown(MouseButton button);
	static bool IsMouseButtonPressed(MouseButton button);
	static bool IsMouseButtonReleased(MouseButton button);

	static MouseMode GetMouseMode();
	static void SetMouseMode(MouseMode mode, SDL_Window* window);
};