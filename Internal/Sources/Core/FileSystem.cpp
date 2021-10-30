#include <filesystem>
#include "Core/FileSystem.hpp"

namespace Fluent::FileSystem
{
    std::string absoluteExecutablePath;
    std::string shadersDirectory;
    std::string texturesDirectory;

    void Init(char** argv)
    {
        std::string relativeExecutablePath = std::string(argv[0]);
        relativeExecutablePath = relativeExecutablePath.substr(0, relativeExecutablePath.find_last_of('/'));
        absoluteExecutablePath = std::filesystem::current_path().string() + "/" + relativeExecutablePath + "/";
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
