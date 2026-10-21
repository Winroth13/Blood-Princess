#pragma once

#include "input/input.hpp"
#include <unordered_map>
#include <string>
#include <vector>

struct ActionBinding
{
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

	static bool SaveBindings(const std::string& filepath);
	static bool LoadBindings(const std::string& filepath);
};