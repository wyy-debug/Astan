#include "AssetManager.h"

#include <filesystem>

namespace Astan
{
    std::filesystem::path AssetManager::getFullPath(const std::string& relative_path) const
    {
        // TODO filesystem
        return std::filesystem::absolute("/home");
    }
} // namespace Piccolo