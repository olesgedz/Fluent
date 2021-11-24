#include <filesystem>
#include "Core/Base.hpp"
#include "Core/FileSystem.hpp"

namespace Fluent::FileSystem
{
    static std::string absoluteExecutablePath;
    static std::string shadersDirectory;
    static std::string texturesDirectory;
    static std::string modelsDirectory;

    void Init(char** argv)
    {
        absoluteExecutablePath = std::filesystem::weakly_canonical(std::filesystem::path(argv[0])).parent_path().string() + "/";
    }

    void SetShadersDirectory(const std::string& path)
    {
        shadersDirectory = absoluteExecutablePath + path;
    }

    void SetTexturesDirectory(const std::string& path)
    {
        texturesDirectory = absoluteExecutablePath + path;
    }

    void SetModelsDirectory(const std::string& path)
    {
        modelsDirectory = absoluteExecutablePath + path;
    }

    const std::string& GetShadersDirectory()
    {
        return shadersDirectory;
    }

    const std::string& GetTexturesDirectory()
    {
        return texturesDirectory;
    }

    const std::string& GetModelsDirectory()
    {
        return modelsDirectory;
    }
} // namespace Fluent::FileSystem
