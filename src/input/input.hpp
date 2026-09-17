#pragma once
#include "SDL3/SDL.h"
#include "SDL3/SDL_keyboard.h"
#include <string>
#include <iostream>
#include <array>
#include "core/logging.hpp" 
#include <DirectXMath.h>

enum class Key
{
	//Unknown key
	UNKNOWN = SDL_SCANCODE_UNKNOWN,
	
	// Letters
	A = SDL_SCANCODE_A, B = SDL_SCANCODE_B, C = SDL_SCANCODE_C, D = SDL_SCANCODE_D,
	E = SDL_SCANCODE_E, F = SDL_SCANCODE_F, G = SDL_SCANCODE_G, H = SDL_SCANCODE_H,
	I = SDL_SCANCODE_I, J = SDL_SCANCODE_J, K = SDL_SCANCODE_K, L = SDL_SCANCODE_L,
	M = SDL_SCANCODE_M, N = SDL_SCANCODE_N, O = SDL_SCANCODE_O, P = SDL_SCANCODE_P,
	Q = SDL_SCANCODE_Q, R = SDL_SCANCODE_R, S = SDL_SCANCODE_S, T = SDL_SCANCODE_T,
	U = SDL_SCANCODE_U, V = SDL_SCANCODE_V, W = SDL_SCANCODE_W, X = SDL_SCANCODE_X,
	Y = SDL_SCANCODE_Y, Z = SDL_SCANCODE_Z,

	// Number row
	NUM_0 = SDL_SCANCODE_0, NUM_1 = SDL_SCANCODE_1, NUM_2 = SDL_SCANCODE_2,
	NUM_3 = SDL_SCANCODE_3, NUM_4 = SDL_SCANCODE_4, NUM_5 = SDL_SCANCODE_5,
	NUM_6 = SDL_SCANCODE_6, NUM_7 = SDL_SCANCODE_7, NUM_8 = SDL_SCANCODE_8,
	NUM_9 = SDL_SCANCODE_9,

	// Function keys
	F1 = SDL_SCANCODE_F1, F2 = SDL_SCANCODE_F2, F3 = SDL_SCANCODE_F3,
	F4 = SDL_SCANCODE_F4, F5 = SDL_SCANCODE_F5, F6 = SDL_SCANCODE_F6,
	F7 = SDL_SCANCODE_F7, F8 = SDL_SCANCODE_F8, F9 = SDL_SCANCODE_F9,
	F10 = SDL_SCANCODE_F10, F11 = SDL_SCANCODE_F11, F12 = SDL_SCANCODE_F12,

	// Modifiers
	LEFT_SHIFT = SDL_SCANCODE_LSHIFT, RIGHT_SHIFT = SDL_SCANCODE_RSHIFT,
	LEFT_CONTROL = SDL_SCANCODE_LCTRL, RIGHT_CONTROL = SDL_SCANCODE_RCTRL,
	LEFT_ALT = SDL_SCANCODE_LALT, RIGHT_ALT = SDL_SCANCODE_RALT,

	// Editing and navigation
	SPACE = SDL_SCANCODE_SPACE, ENTER = SDL_SCANCODE_RETURN,
	ESCAPE = SDL_SCANCODE_ESCAPE, TAB = SDL_SCANCODE_TAB,
	BACKSPACE = SDL_SCANCODE_BACKSPACE, DELETE_KEY = SDL_SCANCODE_DELETE,
	INSERT = SDL_SCANCODE_INSERT, HOME = SDL_SCANCODE_HOME, END = SDL_SCANCODE_END,
	PAGE_UP = SDL_SCANCODE_PAGEUP, PAGE_DOWN = SDL_SCANCODE_PAGEDOWN,

	// Arrows
	LEFT = SDL_SCANCODE_LEFT, RIGHT = SDL_SCANCODE_RIGHT,
	UP = SDL_SCANCODE_UP, DOWN = SDL_SCANCODE_DOWN,

	// Numpad
	NUMPAD_0 = SDL_SCANCODE_KP_0, NUMPAD_1 = SDL_SCANCODE_KP_1,
	NUMPAD_2 = SDL_SCANCODE_KP_2, NUMPAD_3 = SDL_SCANCODE_KP_3,
	NUMPAD_4 = SDL_SCANCODE_KP_4, NUMPAD_5 = SDL_SCANCODE_KP_5,
	NUMPAD_6 = SDL_SCANCODE_KP_6, NUMPAD_7 = SDL_SCANCODE_KP_7,
	NUMPAD_8 = SDL_SCANCODE_KP_8, NUMPAD_9 = SDL_SCANCODE_KP_9,
	NUMPAD_PLUS = SDL_SCANCODE_KP_PLUS, NUMPAD_MINUS = SDL_SCANCODE_KP_MINUS,
	NUMPAD_MULTIPLY = SDL_SCANCODE_KP_MULTIPLY, NUMPAD_DIVIDE = SDL_SCANCODE_KP_DIVIDE,
	NUMPAD_ENTER = SDL_SCANCODE_KP_ENTER, NUMPAD_PERIOD = SDL_SCANCODE_KP_PERIOD
};

enum class MouseMode
{
	VISIBLE,
	HIDDEN,
	CAPTURED
};

struct MouseState
{
	DirectX::XMFLOAT2 position = {0, 0};
	DirectX::XMFLOAT2 wheel = {0, 0};
	DirectX::XMFLOAT2 delta = {0, 0};
	SDL_MouseButtonFlags buttonFlags = 0;
	SDL_MouseButtonFlags previousButtonFlags = 0;
};

enum class MouseButton
{
	UNKNOWN = 0,
	LEFT = SDL_BUTTON_LEFT,
	RIGHT = SDL_BUTTON_RIGHT,
	MIDDLE = SDL_BUTTON_MIDDLE,
	X1 = SDL_BUTTON_X1,
	X2 = SDL_BUTTON_X2
};


class Input {
private:
	static std::array<bool, SDL_SCANCODE_COUNT> currentKeys;
	static std::array<bool, SDL_SCANCODE_COUNT> previousKeys;

	static MouseMode mouseMode;
	static MouseState lastMouseState;
public:
	static void Initialize();

	static void Update();

	static void Shutdown();
	static void HandleEvent(const SDL_Event& event);

	static bool IsKeyPressed(Key key);
	static bool IsKeyReleased(Key key);
	static bool IsKeyDown(Key key);

	static DirectX::XMFLOAT2 GetMousePosition();
	static float GetMouseX();
	static float GetMouseY();

	static DirectX::XMFLOAT2 GetMouseDelta();
	static float GetMouseDeltaX();
	static float GetMouseDeltaY();


	static DirectX::XMFLOAT2 GetMouseWheel();
	static float GetMouseWheelX();
	static float GetMouseWheelY();

	static bool IsMouseButtonDown(MouseButton button);
	static bool IsMouseButtonPressed(MouseButton button);
	static bool IsMouseButtonReleased(MouseButton button);

	static MouseMode GetMouseMode();
	static void SetMouseMode(MouseMode mode, SDL_Window* window);

	static const char *GetKeyName(Key key);
	static Key GetKeyFromName(const std::string &name);
	static const char *GetMouseButtonName(MouseButton button);
	static MouseButton GetMouseButtonFromName(const std::string &name);
};