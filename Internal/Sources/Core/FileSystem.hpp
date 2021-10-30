#pragma once

#include <string>

namespace Fluent::FileSystem
{
    void Init(char** argv);
    void SetShadersDirectory(const std::string& path);
    void SetTexturesDirectory(const std::string& path);

    const std::string& GetShadersDirectory();
    const std::string& GetTexturesDirectory(); 
} // namespace Fluent
