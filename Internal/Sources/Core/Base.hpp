#pragma once

#include <memory>
#include "Core/Log.hpp"

namespace Fluent 
{
    using Handle = void*;
    
    template <typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T>
    using Ref = std::shared_ptr<T>;

    template<typename T, typename... Args>
    Scope<T> CreateScope(Args&&... args)
    {
        return std::make_unique<T>(args...);
    }

    template<typename T, typename... Args>
    Ref<T> CreateRef(Args&&... args)
    {
        return std::make_shared<T>(args...);
    }
}