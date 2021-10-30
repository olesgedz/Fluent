#pragma once

#include <string>
#include <utility>

namespace Fluent
{
    class Event;
    
    class Layer
    {
    private:
        std::string mName;
    public:
        explicit Layer(std::string name) noexcept : mName(std::move(name)) {};
        virtual ~Layer() noexcept = default;

        virtual void OnAttach() = 0;
        virtual void OnLoad() = 0;
        virtual void OnUnload() = 0;
        virtual void OnDetach() = 0;
        virtual void OnUpdate(float deltaTime) = 0;
        
        const std::string& GetName() const noexcept { return mName; };
    };
}