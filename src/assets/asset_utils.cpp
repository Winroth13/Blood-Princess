#include "asset_utils.hpp"
#include "core/logging.hpp"
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
        std::stringstream formattedName{};

        if (name.size() < 4)
        {
            LogWarn("Too short to format shader name: %s", name.c_str());
            return name;
        }

        // Capitalize first letter in actual shader name
        formattedName << static_cast<char>(std::toupper(name[3]));

        // Start after prefix
        for (size_t i = 4; i < name.size(); ++i)
        {
            if (static_cast<char>(name[i]) == '_')
            {
                if (name.size() >= i + 1)
                {
                    // Insert space and make next letter capital
                    formattedName << ' ' << static_cast<char>(std::toupper(name[i + 1]));
                    i += 1;
                }
            }
            else
            {
                formattedName << name[i];
            }
        }

        return formattedName.str();
	}
}