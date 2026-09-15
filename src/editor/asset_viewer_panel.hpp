#pragma once
#include <memory>

class Asset;

class AssetViewerPanel
{
	std::weak_ptr<Asset> viewingAsset;

public:
	AssetViewerPanel() = default;
	~AssetViewerPanel() = default;

	void ImGuiRender();
	void DrawAssetNode(Asset *asset);

	void SetViewingAsset(std::shared_ptr<Asset> viewingAsset) { this->viewingAsset = viewingAsset; }
};