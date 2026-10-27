#include "input/action.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include "core/logging.hpp" 

std::unordered_map<std::string, ActionBinding> Action::actions;
std::vector<std::string> Action::actionOrder;

void Action::Initialize()
{
	actions.clear();
	actionOrder.clear();
	LogInfo("Action System initialized...");
}


void Action::Shutdown()
{
	actions.clear();
	actionOrder.clear();
	LogInfo("Action System Shutdown...");
}

void Action::SyncVectors(ActionBinding& binding)
{
	binding.keys.clear();
	binding.mouseButtons.clear();
	binding.gamepadButtons.clear();
	binding.gamepadAxes.clear();

	auto addSlot = [&](const ActionSlot& slot) {
		if (slot.IsKey() && slot.AsKey() != Key::UNKNOWN)
		{
			binding.keys.push_back(slot.AsKey());
		}
		else if (slot.IsMouse() && slot.AsMouse() != MouseButton::UNKNOWN)
		{
			binding.mouseButtons.push_back(slot.AsMouse());
		}
	};

	addSlot(binding.input1);
	addSlot(binding.input2);

	if (binding.controllerInput != GamepadButton::UNKNOWN)
	{
		binding.gamepadButtons.push_back(binding.controllerInput);
	}
}

// Must run after event polling, so Input has current data for this frame.
// wasActive is saved before isActive is recalculated, which is what makes
// IsActionPressed fire exactly once even when an action has several keys bound.
void Action::Update()
{

	for (auto& [name, binding] : actions)
	{
		binding.wasActive = binding.isActive;
		binding.isActive = false;

		for (const Key key : binding.keys)
		{
			if (Input::IsKeyDown(key))
			{
				binding.isActive = true;
				break;
			}
		}

		if (!binding.isActive)
		{
			for (const MouseButton mouseButton : binding.mouseButtons)
			{
				if (Input::IsMouseButtonDown(mouseButton))
				{
					binding.isActive = true;
					break;
				}
			}

		}

		if (!binding.isActive)
		{
			for (const GamepadButton button : binding.gamepadButtons)
			{
				if (Input::IsGamepadButtonDown(button))
				{
					binding.isActive = true;
					break;
				}
			}
		}

		if (!binding.isActive)
		{
			for (const auto& axisBind : binding.gamepadAxes)
			{
				float val = Input::GetGamepadAxis(axisBind.axis);

				bool isTriggered = (axisBind.threshold >= 0.0f)
					? (val >= axisBind.threshold)
					: (val <= axisBind.threshold);

				if (isTriggered)
				{
					binding.isActive = true;
					break;
				}
			}
		}
	}
}

//Example to create action: Action::AddAction( "Jump", Key::SPACE)
void Action::AddAction(const std::string& name, Key key)
{
	if (std::find(actionOrder.begin(), actionOrder.end(), name) == actionOrder.end())
	{
		actionOrder.push_back(name);
	}

	ActionBinding& binding = actions[name];


	if (binding.input1.IsNone())
	{
		binding.input1 = ActionSlot::FromKey(key);
	}
	else if (binding.input2.IsNone() && !(binding.input1 == ActionSlot::FromKey(key)))
	{
		binding.input2 = ActionSlot::FromKey(key);
	}

	if (std::find(binding.keys.begin(), binding.keys.end(), key) != binding.keys.end())
	{
		return;
	}

	binding.keys.push_back(key);
}

void Action::AddAction(const std::string& name, MouseButton mouseButton)
{
	if (std::find(actionOrder.begin(), actionOrder.end(), name) == actionOrder.end())
	{
		actionOrder.push_back(name);
	}

	ActionBinding& binding = actions[name];

	if (binding.input1.IsNone())
	{
		binding.input1 = ActionSlot::FromMouse(mouseButton);
	}
	else if (binding.input2.IsNone() && !(binding.input1 == ActionSlot::FromMouse(mouseButton)))
	{
		binding.input2 = ActionSlot::FromMouse(mouseButton);
	}

	if (std::find(binding.mouseButtons.begin(), binding.mouseButtons.end(), mouseButton) != binding.mouseButtons.end())
	{
		return;
	}

	binding.mouseButtons.push_back(mouseButton);
}

void Action::AddAction(const std::string& name, GamepadButton button)
{
	if (std::find(actionOrder.begin(), actionOrder.end(), name) == actionOrder.end())
	{
		actionOrder.push_back(name);
	}

	ActionBinding& binding = actions[name];
	binding.controllerInput = button;

	if (std::find(binding.gamepadButtons.begin(), binding.gamepadButtons.end(), button) != binding.gamepadButtons.end())
	{
		return;
	}
	binding.gamepadButtons.push_back(button);
}

void Action::AddAction(const std::string& name, GamepadAxis axis, float threshold)
{
	if (std::find(actionOrder.begin(), actionOrder.end(), name) == actionOrder.end())
	{
		actionOrder.push_back(name);
	}

	ActionBinding& binding = actions[name];

	// Avoid duplicate bindings for the same axis
	auto it = std::find_if(binding.gamepadAxes.begin(), binding.gamepadAxes.end(),
		[axis](const GamepadAxisBinding& b) { return b.axis == axis; });

	if (it == binding.gamepadAxes.end())
	{
		binding.gamepadAxes.push_back({ axis, threshold });
	}
}

bool Action::IsActionDown(const std::string& name)
{
	auto it = actions.find(name);

	if (it == actions.end())
	{
		LogWarn("Unknown action: %s", name.c_str());
		return false;
	}

	return it->second.isActive;
}

bool Action::IsActionPressed(const std::string& name)
{
	auto it = actions.find(name);

	if (it == actions.end())
	{
		LogWarn("Unknown action: %s", name.c_str());
		return false;
	}


	return it->second.isActive && !it->second.wasActive;
}

bool Action::IsActionReleased(const std::string& name)
{
	auto it = actions.find(name);

	if (it == actions.end())
	{
		LogWarn("Unknown action: %s", name.c_str());
		return false;
	}


	return !it->second.isActive && it->second.wasActive;

}

const std::vector<std::string>& Action::GetActionNames()
{
	return actionOrder;
}

ActionBinding* Action::GetBinding(const std::string& name)
{
	auto it = actions.find(name);
	if (it != actions.end())
	{
		return &it->second;
	}
	return nullptr;
}

void Action::SetInput1(const std::string& name, const ActionSlot& slot)
{
	ActionBinding& binding = actions[name];
	binding.input1 = slot;
	SyncVectors(binding);
}

void Action::SetInput2(const std::string& name, const ActionSlot& slot)
{
	ActionBinding& binding = actions[name];
	binding.input2 = slot;
	SyncVectors(binding);
}

void Action::SetControllerInput(const std::string& name, GamepadButton button)
{
	ActionBinding& binding = actions[name];
	binding.controllerInput = button;
	SyncVectors(binding);
}

void Action::ResetToDefaults()
{
	actions.clear();
	actionOrder.clear();

	AddAction("MoveForward", Key::W);
	AddAction("MoveBackward", Key::S);
	AddAction("MoveLeft", Key::A);
	AddAction("MoveRight", Key::D);

	AddAction("Dash", Key::LEFT_SHIFT);
	AddAction("Dash", Key::SPACE);
	AddAction("Dash", GamepadButton::LEFT_SHOULDER);

	AddAction("LightAttack", MouseButton::LEFT);
	AddAction("LightAttack", GamepadButton::RIGHT_TRIGGER);

	AddAction("HeavyAttack", MouseButton::RIGHT);
	AddAction("HeavyAttack", GamepadButton::RIGHT_SHOULDER);

	AddAction("Parry", Key::F);
	AddAction("Parry", GamepadButton::LEFT_TRIGGER);
}

std::string Action::GetSlotDisplayName(const ActionSlot& slot)
{
	if (slot.IsKey())
	{
		Key k = slot.AsKey();
		if (k == Key::UNKNOWN) return "---";
		const char* name = Input::GetKeyName(k);
		return name ? name : "---";
	}
	else if (slot.IsMouse())
	{
		MouseButton mb = slot.AsMouse();
		switch (mb)
		{
		case MouseButton::LEFT: return "Left Mouse";
		case MouseButton::RIGHT: return "Right Mouse";
		case MouseButton::MIDDLE: return "Middle Mouse";
		case MouseButton::X1: return "Mouse 4";
		case MouseButton::X2: return "Mouse 5";
		default: return "---";
		}
	}
	return "---";
}

std::string Action::GetGamepadButtonDisplayName(GamepadButton button)
{
	switch (button)
	{
	case GamepadButton::SOUTH: return "A / Cross";
	case GamepadButton::EAST: return "B / Circle";
	case GamepadButton::WEST: return "X / Square";
	case GamepadButton::NORTH: return "Y / Triangle";
	case GamepadButton::LEFT_SHOULDER: return "LB / L1";
	case GamepadButton::RIGHT_SHOULDER: return "RB / R1";
	case GamepadButton::LEFT_STICK: return "LS Click";
	case GamepadButton::RIGHT_STICK: return "RS Click";
	case GamepadButton::START: return "Start / Menu";
	case GamepadButton::BACK: return "Back / View";
	case GamepadButton::GUIDE: return "Guide";
	case GamepadButton::DPAD_UP: return "D-Pad Up";
	case GamepadButton::DPAD_DOWN: return "D-Pad Down";
	case GamepadButton::DPAD_LEFT: return "D-Pad Left";
	case GamepadButton::DPAD_RIGHT: return "D-Pad Right";
	case GamepadButton::LEFT_TRIGGER: return GetGamepadAxisDisplayName(GamepadAxis::LEFT_TRIGGER);
	case GamepadButton::RIGHT_TRIGGER: return GetGamepadAxisDisplayName(GamepadAxis::RIGHT_TRIGGER);
	default: return "---";
	}
}

std::string Action::GetActionDisplayName(const std::string& actionName)
{
	if (actionName == "MoveForward") return "Move Forward";
	if (actionName == "MoveBackward") return "Move Backward";
	if (actionName == "MoveLeft") return "Move Left";
	if (actionName == "MoveRight") return "Move Right";
	if (actionName == "Dash") return "Dash";
	if (actionName == "LightAttack") return "Light Attack";
	if (actionName == "HeavyAttack") return "Heavy Attack";
	if (actionName == "Parry") return "Parry";
	return actionName;
}

std::string Action::GetGamepadAxisDisplayName(GamepadAxis axis)
{
	switch (axis)
	{
	case GamepadAxis::LEFT_TRIGGER:  return "LT / L2";
	case GamepadAxis::RIGHT_TRIGGER: return "RT / R2";
	case GamepadAxis::LEFT_X:        return "Left Stick X";
	case GamepadAxis::LEFT_Y:        return "Left Stick Y";
	case GamepadAxis::RIGHT_X:       return "Right Stick X";
	case GamepadAxis::RIGHT_Y:       return "Right Stick Y";
	default:                         return "---";
	}
}

bool Action::SaveBindings(const std::string& path)
{
	std::ofstream file(path);

	if (!file)
	{
		LogWarn("Could not open %s for writing", path.c_str());
		return false;
	}

	file << "# action key KEYNAME\n";
	file << "# action mouse BUTTONNAME\n";
	file << "# action gamepad BUTTONNAME\n";
	file << "# action axis AXISNAME\n\n";

	for (const std::string& name : actionOrder)
	{
		auto it = actions.find(name);
		if (it == actions.end()) continue;
		const ActionBinding& binding = it->second;

		auto writeSlot = [&](const ActionSlot& slot) {
			if (slot.IsKey() && slot.AsKey() != Key::UNKNOWN)
			{
				file << name << " key " << Input::GetKeyName(slot.AsKey()) << "\n";
			}
			else if (slot.IsMouse() && slot.AsMouse() != MouseButton::UNKNOWN)
			{
				file << name << " mouse " << Input::GetMouseButtonName(slot.AsMouse()) << "\n";
			}
			};

		writeSlot(binding.input1);
		if (!binding.input2.IsNone() && !(binding.input2 == binding.input1))
		{
			writeSlot(binding.input2);
		}

		if (binding.controllerInput != GamepadButton::UNKNOWN)
		{
			const char* buttonName = Input::GetGamepadButtonName(binding.controllerInput);
			if (buttonName)
			{
				file << name << " gamepad " << buttonName << "\n";
			}
		}

		for (const auto& axisBind : binding.gamepadAxes)
		{
			const char* axisName = Input::GetGamepadAxisName(axisBind.axis);
			if (axisName && axisName[0] != '\0')
			{
				file << name << " axis " << axisName << "\n";
			}
		}
	}

	return true;
}
bool Action::LoadBindings(const std::string& path)
{
	std::ifstream file(path);

	if (!file)
	{
		LogInfo("Could not open %s for reading", path.c_str());
		return false;
	}


	actions.clear(); //Remove when file custom time so temporary
	actionOrder.clear();

	int lineNumber = 0;
	std::string line;

	while (getline(file, line))
	{
		lineNumber++;

		if (line.empty() || line[0] == '#')
		{
			continue;
		}

		std::istringstream stream(line);
		std::string actionName;
		std::string type;

		stream >> actionName; //Example: "Crouch"
		stream >> type; //Examole: "Key"

		std::string inputName;
		stream >> std::ws;
		std::getline(stream, inputName);

		if (actionName.empty() || type.empty() || inputName.empty())
		{
			LogWarn("Line %d: expected three fields", lineNumber);
			continue;
		}

		if (type == "key")
		{
			Key key = Input::GetKeyFromName(inputName);

			if (key == Key::UNKNOWN)
			{
				LogWarn("Line %d: unknown key name \"%s\"", lineNumber, inputName.c_str());
				continue;
			}

			AddAction(actionName, key);
		}

		else if (type == "mouse")
		{
			MouseButton mouseButton = Input::GetMouseButtonFromName(inputName);

			if (mouseButton == MouseButton::UNKNOWN)
			{
				LogWarn("Line %d: unknown mousebutton name \"%s\"", lineNumber, inputName.c_str());
				continue;
			}
			AddAction(actionName, mouseButton);
		}

		else if (type == "gamepad")
		{
			GamepadButton btn = Input::GetGamepadButtonFromName(inputName);
			if (btn == GamepadButton::UNKNOWN)
			{
				LogWarn("Line %d: unknown gamepad button \"%s\"", lineNumber, inputName.c_str());
				continue;
			}
			AddAction(actionName, btn);
		}
		else if (type == "axis")
		{
			GamepadAxis axis = Input::GetGamepadAxisFromName(inputName);
			if (axis != static_cast<GamepadAxis>(SDL_GAMEPAD_AXIS_INVALID))
			{
				AddAction(actionName, axis);
			}
			else
			{
				LogWarn("Line %d: unknown gamepad axis \"%s\"", lineNumber, inputName.c_str());
			}
		}
		else
		{
			LogWarn("Line %d: unknown type \"%s\"", lineNumber, type.c_str());
		}
	}

	return true;
}
