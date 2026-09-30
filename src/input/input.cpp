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
SDL_Window *Input::sdlWindow;

std::array<bool, SDL_SCANCODE_COUNT> Input::savedCurrentKeys;
std::array<bool, SDL_SCANCODE_COUNT> Input::savedPreviousKeys;
MouseState Input::savedMouseState;

void Input::Initialize(SDL_Window *window)
{
	currentKeys.fill(false);
	previousKeys.fill(false);

	sdlWindow = window;

	LogInfo("Input initialized..");
}

void Input::Update()
{
	lastMouseState.wheel = {0.0f, 0.0f};
	int numKeys = 0;
	const bool* state = SDL_GetKeyboardState(&numKeys);

	previousKeys = currentKeys;
	std::copy_n(state, std::min(numKeys, static_cast<int>(SDL_SCANCODE_COUNT)), currentKeys.begin());

	SDL_MouseButtonFlags buttonFlags = SDL_GetMouseState(NULL, NULL);
	lastMouseState.previousButtonFlags = lastMouseState.buttonFlags;
	lastMouseState.buttonFlags = buttonFlags;

	SDL_GetRelativeMouseState(&lastMouseState.delta.x, &lastMouseState.delta.y);
}

void Input::HandleEvent(const SDL_Event &event)
{
	switch (event.type)
	{
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

const char *Input::GetKeyName(Key key)
{
	return SDL_GetScancodeName(static_cast<SDL_Scancode>(key));
}

Key Input::GetKeyFromName(const std::string &name)
{
	const SDL_Scancode scancode = SDL_GetScancodeFromName(name.c_str());

	if (scancode == SDL_SCANCODE_UNKNOWN)
	{
		return Key::UNKNOWN;
	}

	return static_cast<Key>(scancode);
}

const char *Input::GetMouseButtonName(MouseButton button)
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

MouseButton Input::GetMouseButtonFromName(const std::string &name)
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
