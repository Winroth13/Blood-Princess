#include "assets/asset.hpp"
#include "core/logging.hpp"

void Asset::SetPath(const std::string &path)
{
	this->path = path;
}

std::string Asset::GetPath() const
{
	return this->path;
}
