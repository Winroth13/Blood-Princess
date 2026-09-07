#include "input.hpp"

// Allocation and definition of static member variables
const bool* Input::keyState;
MouseState Input::lastMouseState;
MouseMode Input::mouseMode = MouseMode::Visible;
std::array<bool, SDL_SCANCODE_COUNT> Input::lastKeyState;



void Input::Initialize()
{
	keyState = SDL_GetKeyboardState(NULL);

	LogInfo("Input initialized..");
	LogIndent();

	for (int i = 0; i < SDL_SCANCODE_COUNT; ++i)
	{
		lastKeyState[i] = false;
	}
	LogUnindent();
}

void Input::Update(SDL_Window* window)
{
	int numKeys = 0;
	auto keyState = SDL_GetKeyboardState(&numKeys);

	for (int i = 0; i < SDL_SCANCODE_COUNT; ++i)
	{
		lastKeyState[i] = keyState[i];
	}
	SDL_MouseButtonFlags buttonFlags = SDL_GetMouseState(&lastMouseState.position.x, &lastMouseState.position.y);
	lastMouseState.buttonFlags = buttonFlags;


	int width, height;
	SDL_GetWindowSize(window, &width, &height);
	switch (mouseMode)
	{
	case MouseMode::Visible:
		SDL_ShowCursor();
		SDL_SetWindowRelativeMouseMode(window, false);
		break;
	case MouseMode::Hidden:
		SDL_HideCursor();
		SDL_SetWindowRelativeMouseMode(window, false);
		break;
	case MouseMode::Captured:
		SDL_WarpMouseInWindow(window, width / 2.0f, height / 2.0f);
		SDL_HideCursor();
		SDL_SetWindowRelativeMouseMode(window, true);
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


bool Input::IsKeyReleased(SDL_Scancode scancode)
{
	int numKeys = 0;
	auto keyState = SDL_GetKeyboardState(&numKeys);
	return !keyState[scancode] && lastKeyState[scancode];
}

bool Input::IsKeyPressed(SDL_Scancode keycode)
{
	int numKeys = 0;
	auto keyState = SDL_GetKeyboardState(&numKeys);
	return keyState[keycode] && !lastKeyState[keycode];
}

bool Input::IsKeyDown(SDL_Scancode keycode)
{
	int numKeys = 0;
	auto keyState = SDL_GetKeyboardState(&numKeys);
	return keyState[keycode];
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
	DirectX::XMFLOAT2 lastPosition = lastMouseState.position;
	DirectX::XMFLOAT2 currentPosition = GetMousePosition();
	
	return {currentPosition.x - lastPosition.x, currentPosition.y - lastPosition.y};
}

float Input::GetMouseDeltaX()
{
	return GetMouseDelta().x;
}

float Input::GetMouseDeltaY()
{
	return GetMouseDelta().y;
}


MouseMode Input::GetMouseMode()
{
	return mouseMode;
}

void Input::SetMouseMode(MouseMode mode)
{
	mouseMode = mode;
}

