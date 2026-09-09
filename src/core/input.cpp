#include "input.hpp"
// Allocation and definition of static member variables
const bool* Input::keyState;
MouseState Input::lastMouseState;
MouseMode Input::mouseMode = MouseMode::VISIBLE;
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

	lastMouseState.wheel = { 0.0f, 0.0f };
	int numKeys = 0;
	auto keyState = SDL_GetKeyboardState(&numKeys);

	for (int i = 0; i < SDL_SCANCODE_COUNT; ++i)
	{
		lastKeyState[i] = keyState[i];
	}

	SDL_MouseButtonFlags buttonFlags = SDL_GetMouseState(NULL, NULL);
	lastMouseState.previousButtonFlags = lastMouseState.buttonFlags;
	lastMouseState.buttonFlags = buttonFlags;

	SDL_GetRelativeMouseState(&lastMouseState.delta.x, &lastMouseState.delta.y);

}

void Input::HandleEvent(const SDL_Event& event)
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


bool Input::IsKeyPressed(SDL_Scancode scancode)
{
	int numKeys = 0;
	auto keyState = SDL_GetKeyboardState(&numKeys);
	return keyState[scancode] && !lastKeyState[scancode];
}

bool Input::IsKeyReleased(SDL_Scancode scancode)
{
	int numKeys = 0;
	auto keyState = SDL_GetKeyboardState(&numKeys);
	return !keyState[scancode] && lastKeyState[scancode];
}

bool Input::IsKeyDown(SDL_Scancode scancode)
{
	int numKeys = 0;
	auto keyState = SDL_GetKeyboardState(&numKeys);
	return keyState[scancode];
}

bool Input::IsKeyPressed(SDL_Keycode keycode)
{
	SDL_Scancode scancode = SDL_GetScancodeFromKey(keycode, NULL);
	int numKeys = 0;
	auto keyState = SDL_GetKeyboardState(&numKeys);
	return keyState[scancode] && !lastKeyState[scancode];
}

bool Input::IsKeyReleased(SDL_Keycode keycode)
{
	SDL_Scancode scancode = SDL_GetScancodeFromKey(keycode, NULL);
	int numKeys = 0;
	auto keyState = SDL_GetKeyboardState(&numKeys);
	return !keyState[scancode] && lastKeyState[scancode];
}

bool Input::IsKeyDown(SDL_Keycode keycode)
{
	SDL_Scancode scancode = SDL_GetScancodeFromKey(keycode, NULL);
	int numKeys = 0;
	auto keyState = SDL_GetKeyboardState(&numKeys);
	return keyState[scancode];
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

void Input::OnMouseWheelEvent(float x, float y)
{
	lastMouseState.wheel.x += x;
	lastMouseState.wheel.y += y;
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

void Input::SetMouseMode(MouseMode mode, SDL_Window* window)
{

	// Set the mouse mode and update the cursor visibility and relative mouse mode accordingly
	//Window rect is used to define the area where the mouse can move when captured
	mouseMode = mode;
	int width, height;

	SDL_GetWindowSize(window, &width, &height);
	SDL_Rect rect;
	rect.x = width / 2;
	rect.y = height / 2;
	rect.w = 1;
	rect.h = 1;
	switch (mouseMode)
	{
	case MouseMode::VISIBLE:
		SDL_SetWindowMouseRect(window, nullptr);
		SDL_ShowCursor();
		SDL_SetWindowRelativeMouseMode(window, false);
		break;
	case MouseMode::HIDDEN:
		SDL_SetWindowMouseRect(window, nullptr);
		SDL_HideCursor();
		SDL_SetWindowRelativeMouseMode(window, false);
		break;
	case MouseMode::CAPTURED:
		SDL_SetWindowMouseRect(window, &rect);
		SDL_HideCursor();
		SDL_SetWindowRelativeMouseMode(window, true);
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