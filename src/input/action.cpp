#include "input/action.hpp"
#include <fstream>
#include <sstream>
#include "core/logging.hpp" 

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

//Example to create action: Action::AddAction( "Jump", Key::SPACE)
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
	ActionBinding &binding = actions[name];

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
		LogWarn("Unknown action: %s", name.c_str());
		return false;
	}

	return it->second.isActive;
}

bool Action::IsActionPressed(const std::string &name)
{
	auto it = actions.find(name);

	if (it == actions.end())
	{
		LogWarn("Unknown action: %s", name.c_str());
		return false;
	}


	return it->second.isActive && !it->second.wasActive;
}

bool Action::IsActionReleased(const std::string &name)
{
	auto it = actions.find(name);

	if (it == actions.end())
	{
		LogWarn("Unknown action: %s", name.c_str());
		return false;
	}


	return !it->second.isActive && it->second.wasActive;

}

bool Action::SaveBindings(const std::string &path)
{

	std::ofstream file(path);
	
	if (!file)
	{
		LogError("Could not open %s for writing", path.c_str());		
		return false;
	}
	
	file << "# action key KEYNAME\n";
	file << "# action mouse BUTTONNAME\n\n";


	// Sorted so the file stays readable, not really necessary but git reasons 
	std::vector<std::string> sortedNames;
	sortedNames.reserve(actions.size());

	for (const auto &[name, binding] : actions)
	{
		sortedNames.push_back(name);
	}



	for (auto &[name, binding] : actions)
	{
		for (const Key key : binding.keys)
		{
			file << name << " key " << Input::GetKeyName(key) << "\n";
		}

		for (const MouseButton mouseButton : binding.mouseButtons)
		{
			file << name << " mouse " << Input::GetMouseButtonName(mouseButton) << "\n";
		}
	}
	
	return true;
}

bool Action::LoadBindings(const std::string &path)
{
	std::ifstream file(path);

	if (!file)
	{
		LogError("Could not open %s for reading", path.c_str());
		return false;
	}


	actions.clear(); //Remove when file custom time so temporary

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

		else
		{
			LogWarn("Line %d: unknown type \"%s\"", lineNumber, type.c_str());
		}
	}

	return true;
}