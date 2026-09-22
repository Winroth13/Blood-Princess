#pragma once

#include <string>

namespace AssetUtils
{
	std::string GetNameFromPath(const std::string &path);
	std::string FormatShaderName(const std::string &name);
}