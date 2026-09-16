#pragma once

#include "input/input.hpp"
#include "core/logging.hpp" 
#include <unordered_map>
#include <string>
#include <vector>

struct ActionBinding
{
	std::vector<Key> keys;
	std::vector<MouseButton> mouseButtons;

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

	static void AddAction(const std::string &name, Key key);
	static void AddAction(const std::string &name, MouseButton mouseButton);

	static bool IsActionDown(const std::string &name);
	static bool IsActionPressed(const std::string &name);
	static bool IsActionReleased(const std::string &name);
};