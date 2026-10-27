#include "input/input.hpp"
#include "SDL3/SDL.h"

#include <algorithm>

#undef min
#undef max

// Allocation and definition of static member variables
MouseState Input::lastMouseState;
MouseMode Input::mouseMode = MouseMode::VISIBLE;
std::array<bool, SDL_SCANCODE_COUNT> Input::currentKeys;
std::array<bool, SDL_SCANCODE_COUNT> Input::previousKeys;
SDL_Window* Input::sdlWindow;

std::array<bool, SDL_SCANCODE_COUNT> Input::savedCurrentKeys;
std::array<bool, SDL_SCANCODE_COUNT> Input::savedPreviousKeys;
MouseState Input::savedMouseState;

SDL_Gamepad* Input::activeGamepad = nullptr;
std::array<bool, SDL_GAMEPAD_BUTTON_COUNT + 2> Input::currentGamepadButtons = {};
std::array<bool, SDL_GAMEPAD_BUTTON_COUNT + 2> Input::previousGamepadButtons = {};
float Input::deadzone = 0.15f; // 15% stick deadzone


void Input::Initialize(SDL_Window* window)
{
	currentKeys.fill(false);
	previousKeys.fill(false);

	sdlWindow = window;

	LogInfo("Input initialized..");
}

void Input::Update()
{
	lastMouseState.wheel = { 0.0f, 0.0f };
	int numKeys = 0;
	const bool* state = SDL_GetKeyboardState(&numKeys);

	previousKeys = currentKeys;
	std::copy_n(state, std::min(numKeys, static_cast<int>(SDL_SCANCODE_COUNT)), currentKeys.begin());

	SDL_MouseButtonFlags buttonFlags = SDL_GetMouseState(NULL, NULL);
	lastMouseState.previousButtonFlags = lastMouseState.buttonFlags;
	lastMouseState.buttonFlags = buttonFlags;

	SDL_GetRelativeMouseState(&lastMouseState.delta.x, &lastMouseState.delta.y);


	previousGamepadButtons = currentGamepadButtons;
	if (activeGamepad)
	{
		for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT + 2; ++i)
		{
			if (i < SDL_GAMEPAD_BUTTON_COUNT)
			{
				currentGamepadButtons[i] = SDL_GetGamepadButton(activeGamepad, static_cast<SDL_GamepadButton>(i));
			}
			/* Update Trigger Buttons */
			else
			{
				if (i == static_cast<int>(GamepadButton::LEFT_TRIGGER))
				{
					currentGamepadButtons[i] = SDL_GetGamepadAxis(activeGamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > 0.5;
				}
				else if (i == static_cast<int>(GamepadButton::RIGHT_TRIGGER))
				{
					currentGamepadButtons[i] = SDL_GetGamepadAxis(activeGamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.5;
				}
			}
		}
	}
	else
	{
		currentGamepadButtons.fill(false);
	}

}

void Input::HandleEvent(const SDL_Event& event)
{
	switch (event.type)
	{
	case SDL_EVENT_GAMEPAD_ADDED:
		if (!activeGamepad)
		{
			activeGamepad = SDL_OpenGamepad(event.gdevice.which);
			LogInfo("Gamepad connected: %s", SDL_GetGamepadName(activeGamepad));
		}
		break;

	case SDL_EVENT_GAMEPAD_REMOVED:
		if (activeGamepad && SDL_GetGamepadID(activeGamepad) == event.gdevice.which)
		{
			LogInfo("Gamepad disconnected.");
			SDL_CloseGamepad(activeGamepad);
			activeGamepad = nullptr;
			currentGamepadButtons.fill(false);
			previousGamepadButtons.fill(false);
		}
		break;

	case SDL_EVENT_MOUSE_WHEEL:
		// Several wheel events can arrive in one frame, so they are summed
		lastMouseState.wheel.x += event.wheel.x;
		lastMouseState.wheel.y += event.wheel.y;
		break;
	default:
		break;
	}
}

void Input::Shutdown()
{
	LogUnindent();
	LogInfo("Input shutdown..");

	if (activeGamepad)
	{
		SDL_CloseGamepad(activeGamepad);
		activeGamepad = nullptr;
	}

}


bool Input::IsKeyDown(Key key)
{
	return currentKeys[static_cast<int>(key)];
}

bool Input::IsKeyPressed(Key key)
{
	return currentKeys[static_cast<int>(key)] && !previousKeys[static_cast<int>(key)];
}

bool Input::IsKeyReleased(Key key)
{
	return !currentKeys[static_cast<int>(key)] && previousKeys[static_cast<int>(key)];
}


DirectX::XMFLOAT2 Input::GetMousePosition()
{
	DirectX::XMFLOAT2 position = { 0, 0 };
	SDL_GetMouseState(&position.x, &position.y);
	return position;
}

DirectX::XMFLOAT2 Input::GetMousePositionInPixels()
{
	DirectX::XMFLOAT2 position = Input::GetMousePosition();

	int width = 0, height = 0, pixelWidth = 0, pixelHeight = 0;
	if (!sdlWindow || !SDL_GetWindowSize(sdlWindow, &width, &height) || !SDL_GetWindowSizeInPixels(sdlWindow, &pixelWidth, &pixelHeight))
		return position;

	if (width > 0 && height > 0)
	{
		position.x *= static_cast<float>(pixelWidth) / width;
		position.y *= static_cast<float>(pixelHeight) / height;
	}

	return position;
}

float Input::GetMouseX()
{
	return GetMousePosition().x;
}

float Input::GetMouseY()
{
	return GetMousePosition().y;
}

DirectX::XMFLOAT2 Input::GetMouseDelta()
{
	return lastMouseState.delta;
}

float Input::GetMouseDeltaX()
{
	return lastMouseState.delta.x;
}

float Input::GetMouseDeltaY()
{
	return lastMouseState.delta.y;
}

DirectX::XMFLOAT2 Input::GetMouseWheel()
{
	return lastMouseState.wheel;
}

float Input::GetMouseWheelX()
{
	return lastMouseState.wheel.x;
}

float Input::GetMouseWheelY()
{
	return lastMouseState.wheel.y;
}

MouseMode Input::GetMouseMode()
{
	return mouseMode;
}

void Input::SetMouseMode(MouseMode mode)
{
	// Set the mouse mode and update the cursor visibility and relative mouse mode accordingly
	//Window rect is used to define the area where the mouse can move when captured
	mouseMode = mode;
	int width, height;

	SDL_GetWindowSize(sdlWindow, &width, &height);
	SDL_Rect rect;
	rect.x = width / 2;
	rect.y = height / 2;
	rect.w = 1;
	rect.h = 1;
	switch (mouseMode)
	{
	case MouseMode::VISIBLE:
		SDL_SetWindowMouseRect(sdlWindow, nullptr);
		SDL_ShowCursor();
		SDL_SetWindowRelativeMouseMode(sdlWindow, false);
		break;
	case MouseMode::HIDDEN:
		SDL_SetWindowMouseRect(sdlWindow, nullptr);
		SDL_HideCursor();
		SDL_SetWindowRelativeMouseMode(sdlWindow, true);
		break;
	case MouseMode::CAPTURED:
		SDL_WarpMouseInWindow(sdlWindow, width / 2.0f, height / 2.0f);
		SDL_SetWindowMouseRect(sdlWindow, &rect);
		SDL_HideCursor();
		SDL_SetWindowRelativeMouseMode(sdlWindow, true);
		break;
	default:
		break;
	}
}

bool Input::IsMouseButtonDown(MouseButton button)
{
	return (lastMouseState.buttonFlags & SDL_BUTTON_MASK(static_cast<int>(button))) != 0;
}

bool Input::IsMouseButtonPressed(MouseButton button)
{
	return (lastMouseState.buttonFlags & SDL_BUTTON_MASK(static_cast<int>(button))) && !(lastMouseState.previousButtonFlags & SDL_BUTTON_MASK(static_cast<int>(button)));
}

bool Input::IsMouseButtonReleased(MouseButton button)
{
	return !(lastMouseState.buttonFlags & SDL_BUTTON_MASK(static_cast<int>(button))) && (lastMouseState.previousButtonFlags & SDL_BUTTON_MASK(static_cast<int>(button)));
}

const char* Input::GetKeyName(Key key)
{
	return SDL_GetScancodeName(static_cast<SDL_Scancode>(key));
}

Key Input::GetKeyFromName(const std::string& name)
{
	const SDL_Scancode scancode = SDL_GetScancodeFromName(name.c_str());

	if (scancode == SDL_SCANCODE_UNKNOWN)
	{
		return Key::UNKNOWN;
	}

	return static_cast<Key>(scancode);
}

const char* Input::GetMouseButtonName(MouseButton button)
{

	switch (button)
	{
	case MouseButton::LEFT:
		return "LEFT";

	case MouseButton::RIGHT:
		return "RIGHT";

	case MouseButton::MIDDLE:
		return "MIDDLE";

	case MouseButton::X1:
		return "X1";

	case MouseButton::X2:
		return "X2";
	default:
		return "UNKNOWN";
	}
}

MouseButton Input::GetMouseButtonFromName(const std::string& name)
{
	if (name == "LEFT")
	{
		return MouseButton::LEFT;
	}
	if (name == "RIGHT")
	{
		return MouseButton::RIGHT;
	}

	if (name == "MIDDLE")
	{
		return MouseButton::MIDDLE;
	}
	if (name == "X1")
	{
		return MouseButton::X1;
	}
	if (name == "X2")
	{
		return MouseButton::X2;
	}
	return MouseButton::UNKNOWN;
}

void Input::ClearState()
{
	std::fill(currentKeys.begin(), currentKeys.end(), false);
	std::fill(previousKeys.begin(), previousKeys.end(), false);
	lastMouseState = {};
}

void Input::SaveState()
{
	savedCurrentKeys = currentKeys;
	savedPreviousKeys = previousKeys;
	savedMouseState = lastMouseState;
}

void Input::LoadState()
{
	lastMouseState = savedMouseState;
	currentKeys = savedCurrentKeys;
	previousKeys = savedPreviousKeys;
}



bool Input::IsGamepadConnected()
{
	return activeGamepad != nullptr;
}

bool Input::IsGamepadButtonDown(GamepadButton button)
{
	if (!activeGamepad || button == GamepadButton::UNKNOWN) return false;
	return currentGamepadButtons[static_cast<int>(button)];
}

bool Input::IsGamepadButtonPressed(GamepadButton button)
{
	if (!activeGamepad || button == GamepadButton::UNKNOWN) return false;
	int idx = static_cast<int>(button);
	return currentGamepadButtons[idx] && !previousGamepadButtons[idx];
}

bool Input::IsGamepadButtonReleased(GamepadButton button)
{
	if (!activeGamepad || button == GamepadButton::UNKNOWN) return false;
	int idx = static_cast<int>(button);
	return !currentGamepadButtons[idx] && previousGamepadButtons[idx];
}

DirectX::XMFLOAT2 Input::GetGamepadLeftStick()
{
	return { GetGamepadAxis(GamepadAxis::LEFT_X), -GetGamepadAxis(GamepadAxis::LEFT_Y) };
}

DirectX::XMFLOAT2 Input::GetGamepadRightStick()
{
	return { GetGamepadAxis(GamepadAxis::RIGHT_X), -GetGamepadAxis(GamepadAxis::RIGHT_Y) };
}

float Input::GetGamepadAxis(GamepadAxis axis)
{
	if (!activeGamepad) return 0.0f;

	Sint16 rawValue = SDL_GetGamepadAxis(activeGamepad, static_cast<SDL_GamepadAxis>(axis));

	// Triggers range from 0 to 32767; thumbsticks range from -32768 to 32767
	if (axis == GamepadAxis::LEFT_TRIGGER || axis == GamepadAxis::RIGHT_TRIGGER)
	{
		return static_cast<float>(rawValue) / 32767.0f;
	}

	float normalized = (rawValue < 0) ? (static_cast<float>(rawValue) / 32768.0f)
		: (static_cast<float>(rawValue) / 32767.0f);

	if (std::abs(normalized) < deadzone) return 0.0f;

	return normalized;
}

const char* Input::GetGamepadButtonName(GamepadButton button)
{
	return SDL_GetGamepadStringForButton(static_cast<SDL_GamepadButton>(button));
}

GamepadButton Input::GetGamepadButtonFromName(const std::string& name)
{
	std::string lower = name;
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
	if (lower == "south" || lower == "a" || lower == "cross") return GamepadButton::SOUTH;
	if (lower == "east" || lower == "b" || lower == "circle") return GamepadButton::EAST;
	if (lower == "west" || lower == "x" || lower == "square") return GamepadButton::WEST;
	if (lower == "north" || lower == "y" || lower == "triangle") return GamepadButton::NORTH;
	SDL_GamepadButton btn = SDL_GetGamepadButtonFromString(lower.c_str());
	if (btn == SDL_GAMEPAD_BUTTON_INVALID)
	{
		return GamepadButton::UNKNOWN;
	}
	return static_cast<GamepadButton>(btn);
}


void Input::UpdateGamepadCursor(float deltaTime, float speed)
{
	if (mouseMode != MouseMode::VISIBLE || !activeGamepad || !sdlWindow)
		return;

	DirectX::XMFLOAT2 stick = GetGamepadLeftStick();
	if (std::abs(stick.x) < 0.05f && std::abs(stick.y) < 0.05f)
		return;

	DirectX::XMFLOAT2 currentPos = GetMousePosition();

	currentPos.x += stick.x * speed * deltaTime;
	currentPos.y += -stick.y * speed * deltaTime;

	int winWidth = 0, winHeight = 0;
	SDL_GetWindowSize(sdlWindow, &winWidth, &winHeight);

	currentPos.x = std::clamp(currentPos.x, 0.0f, static_cast<float>(winWidth));
	currentPos.y = std::clamp(currentPos.y, 0.0f, static_cast<float>(winHeight));

	SDL_WarpMouseInWindow(sdlWindow, currentPos.x, currentPos.y);
}


Key Input::GetAnyKeyPressed()
{
	for (int i = 1; i < static_cast<int>(SDL_SCANCODE_COUNT); ++i)
	{
		Key key = static_cast<Key>(i);
		if (IsKeyPressed(key))
		{
			return key;
		}
	}
	return Key::UNKNOWN;
}

MouseButton Input::GetAnyMouseButtonPressed()
{
	const MouseButton buttons[] = {
		MouseButton::LEFT,
		MouseButton::RIGHT,
		MouseButton::MIDDLE,
		MouseButton::X1,
		MouseButton::X2
	};
	for (MouseButton b : buttons)
	{
		if (IsMouseButtonPressed(b))
		{
			return b;
		}
	}
	return MouseButton::UNKNOWN;
}

GamepadButton Input::GetAnyGamepadButtonPressed()
{
	if (!activeGamepad)
		return GamepadButton::UNKNOWN;

	for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; ++i)
	{
		GamepadButton btn = static_cast<GamepadButton>(i);
		if (IsGamepadButtonPressed(btn))
		{
			return btn;
		}
	}
	return GamepadButton::UNKNOWN;
}

const char* Input::GetGamepadAxisName(GamepadAxis axis)
{
	return SDL_GetGamepadStringForAxis(static_cast<SDL_GamepadAxis>(axis));
}

GamepadAxis Input::GetGamepadAxisFromName(const std::string& name)
{
	std::string lower = name;
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
	SDL_GamepadAxis axis = SDL_GetGamepadAxisFromString(lower.c_str());
	return static_cast<GamepadAxis>(axis);
}
