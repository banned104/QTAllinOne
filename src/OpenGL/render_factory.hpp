#pragma once
#include "irenderer.hpp"
#include <memory>

enum class RenderType {
    Triangle,
    Cube,
    Custom,
};


class RenderFactory {
public:
    static std::unique_ptr<IRenderer> create( RenderType type ) {
        switch (type) {
        case RenderType::Triangle:
            return std::make_unique<TriangleRender>();
            break;
        default:
            return nullptr;
            break;
        }
    }
private:
    // 禁止实例化
    RenderFactory() = delete;
};
