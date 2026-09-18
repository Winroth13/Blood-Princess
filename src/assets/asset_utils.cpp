#include "asset_utils.hpp"
#include <filesystem>

namespace AssetUtils
{
	std::string GetNameFromPath(const std::string &path)
	{
		std::filesystem::path filePath = path;
		return filePath.filename().replace_extension("").string();
	}

	std::string FormatShaderName(const std::string &name)
	{
		std::string shaderName = name.substr(3, name.size());
		shaderName[0] = std::toupper(shaderName[0]);

		return shaderName;
	}
}