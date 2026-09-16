#include "input/action.hpp"

std::unordered_map<std::string, ActionBinding> Action::actions;

void Action::Initialize() 
{
	actions.clear();
	LogInfo("Action System initialized...");
}


void Action::Shutdown() 
{
	actions.clear();
	LogInfo("Action System Shutdown...");
}

// Must run after event polling, so Input has current data for this frame.
// wasActive is saved before isActive is recalculated, which is what makes
// IsActionPressed fire exactly once even when an action has several keys bound.
void Action::Update()
{

	for (auto &[name, binding] : actions)
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

	}
}

void Action::AddAction(const std::string &name, Key key) 
{
	ActionBinding &binding = actions[name];


	if (std::find(binding.keys.begin(), binding.keys.end(), key) != binding.keys.end())
	{
		return;
	}

	binding.keys.push_back(key);
}

void Action::AddAction(const std::string &name, MouseButton mouseButton)
{
	ActionBinding& binding = actions[name];

	if (std::find(binding.mouseButtons.begin(), binding.mouseButtons.end(), mouseButton) != binding.mouseButtons.end())
	{
		return;
	}

	binding.mouseButtons.push_back(mouseButton);
}

bool Action::IsActionDown(const std::string &name)
{
	auto it = actions.find(name);

	if (it == actions.end()) 
	{
		return false;
	}

	return it->second.isActive;
}

bool Action::IsActionPressed(const std::string &name)
{
	auto it = actions.find(name);

	if (it == actions.end())
	{
		return false;
	}
	
	
	return it->second.isActive && !it->second.wasActive;
}

bool Action::IsActionReleased(const std::string& name)
{
	auto it = actions.find(name);

	if (it == actions.end())
	{
		return false;
	}
	
	
	return !it->second.isActive && it->second.wasActive;

}