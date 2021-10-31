#include <filesystem>
#include "Core/Base.hpp"
#include "Core/FileSystem.hpp"

namespace Fluent::FileSystem
{
    std::string absoluteExecutablePath;
    std::string shadersDirectory;
    std::string texturesDirectory;

    void Init(char** argv)
    {
        absoluteExecutablePath = std::filesystem::weakly_canonical(std::filesystem::path(argv[0])).parent_path().string();
        LOG_INFO(absoluteExecutablePath);
    }

    void SetShadersDirectory(const std::string& path)
    {
        shadersDirectory = absoluteExecutablePath + path;
    }

    void SetTexturesDirectory(const std::string& path)
    {
        texturesDirectory = absoluteExecutablePath + path;
    }

    const std::string& GetShadersDirectory()
    {
        return shadersDirectory;
    }

    const std::string& GetTexturesDirectory()
    {
        return texturesDirectory;
    }
} // namespace Fluent::FileSystem
