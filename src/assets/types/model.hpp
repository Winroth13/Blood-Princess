#pragma once
#include "assets/asset.hpp"

#define TINYGLTF3_ENABLE_FS
#include "tinygltf/tiny_gltf_v3.h"

class Model : public Asset
{
public:
	Model();
	~Model();
};