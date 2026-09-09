#pragma once
#include <string>

class Asset
{
protected:
	std::string path;

public:
	Asset() = default;
	~Asset() = default;

	void SetPath(const std::string &path);
	std::string GetPath() const;
};