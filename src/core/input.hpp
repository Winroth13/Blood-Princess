#pragma once
#include "SDL3/SDL.h"
#include "SDL3/SDL_keyboard.h"
#include <string>
#include <iostream>
#include <array>
#include "logging.hpp" 
#include <DirectXMath.h>


enum class MouseMode {
	Visible,
	Hidden,
	Captured
};

struct MouseState
{
	DirectX::XMFLOAT2 position{0, 0};
	SDL_MouseButtonFlags buttonFlags = 0;
};

class Input {
private:
	static const bool* keyState;
	static std::array<bool, SDL_SCANCODE_COUNT> lastKeyState;

	static MouseMode mouseMode;
	static MouseState lastMouseState;

public:
	static void Initialize();
	static bool UpdateKeyStates(SDL_Window* window);

	static void Update(SDL_Window* window);

	static void Shutdown();

	static bool IsKeyPressed(SDL_Scancode scancode);
	static bool IsKeyReleased(SDL_Scancode scancode);
	static bool IsKeyDown(SDL_Scancode scancode);
	
	static DirectX::XMFLOAT2 GetMousePosition();
	static float GetMouseX();
	static float GetMouseY();

	static DirectX::XMFLOAT2 GetMouseDelta();
	static float GetMouseDeltaX();
	static float GetMouseDeltaY();

	static DirectX::XMFLOAT2 GetMouseWheel();
	static float GetMouseWheelX();
	static float GetMouseWheelY();

	static bool IsMouseButtonPressed(int button);
	static bool IsMouseButtonReleased(int button);
	

	static bool WasMouseButtonPressed(int button);
	static bool IsMouseJustPressed(int button);


	static MouseMode GetMouseMode();
	static void SetMouseMode(MouseMode mode);
};