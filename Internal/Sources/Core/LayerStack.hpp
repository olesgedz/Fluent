
  
#pragma once

#include <list>
#include "Core/Layer.hpp"

namespace Fluent
{
    class LayerStack
    {
    private:
        std::list<Layer*> mLayers;
    public:
        void PushLayer(Layer* layer) noexcept
        {
            mLayers.push_front(layer);
        }

        void PushOverlay(Layer* layer) noexcept
        {
            mLayers.push_back(layer);
        }

        auto begin() noexcept
        {
           return mLayers.begin();
        }

        auto end() noexcept
        {
            return mLayers.end();
        }
    };
}