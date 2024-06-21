#pragma once
enum class AssetStatus
{
    NONE,
    WORKING,
    READY,
    FAILED,
};
template <typename AssetType>
struct AssetContainer
{
        AssetStatus Status;
        AssetType Asset;
        std::string Path;
};
