#pragma once

#include "input/input.hpp"
#include <unordered_map>
#include <string>
#include <vector>

enum class InputDeviceType
{
	NONE = 0,
	KEYBOARD,
	MOUSE
};

struct ActionSlot
{
	InputDeviceType type = InputDeviceType::NONE;
	int code = 0;

	bool IsNone() const { return type == InputDeviceType::NONE; }
	bool IsKey() const { return type == InputDeviceType::KEYBOARD; }
	bool IsMouse() const { return type == InputDeviceType::MOUSE; }

	Key AsKey() const { return IsKey() ? static_cast<Key>(code) : Key::UNKNOWN; }
	MouseButton AsMouse() const { return IsMouse() ? static_cast<MouseButton>(code) : MouseButton::UNKNOWN; }

	static ActionSlot FromKey(Key k) {
		ActionSlot s;
		if (k != Key::UNKNOWN) {
			s.type = InputDeviceType::KEYBOARD;
			s.code = static_cast<int>(k);
		}
		return s;
	}

	static ActionSlot FromMouse(MouseButton mb) {
		ActionSlot s;
		if (mb != MouseButton::UNKNOWN) {
			s.type = InputDeviceType::MOUSE;
			s.code = static_cast<int>(mb);
		}
		return s;
	}

	bool operator==(const ActionSlot& other) const {
		return type == other.type && code == other.code;
	}
};

struct ActionBinding
{
	ActionSlot input1;
	ActionSlot input2;
	GamepadButton controllerInput = GamepadButton::UNKNOWN;

	std::vector<Key> keys;
	std::vector<MouseButton> mouseButtons;
	std::vector<GamepadButton> gamepadButtons;

	bool isActive = false;
	bool wasActive = false;
};

class Action
{
private:
	static std::unordered_map<std::string, ActionBinding> actions;
	static std::vector<std::string> actionOrder;

	static void SyncVectors(ActionBinding& binding);

public:
	static void Initialize();
	static void Shutdown();

	static void Update();

	static void AddAction(const std::string& name, Key key);
	static void AddAction(const std::string& name, MouseButton mouseButton);
	static void AddAction(const std::string& name, GamepadButton button);

	static bool IsActionDown(const std::string& name);
	static bool IsActionPressed(const std::string& name);
	static bool IsActionReleased(const std::string& name);

	static const std::vector<std::string>& GetActionNames();
	static ActionBinding* GetBinding(const std::string& name);

	static void SetInput1(const std::string& name, const ActionSlot& slot);
	static void SetInput2(const std::string& name, const ActionSlot& slot);
	static void SetControllerInput(const std::string& name, GamepadButton button);

	static void ResetToDefaults();

	static std::string GetSlotDisplayName(const ActionSlot& slot);
	static std::string GetGamepadButtonDisplayName(GamepadButton button);
	static std::string GetActionDisplayName(const std::string& actionName);

	static bool SaveBindings(const std::string& filepath);
	static bool LoadBindings(const std::string& filepath);
};
