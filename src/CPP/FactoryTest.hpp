#ifndef _FACTORY_TEST_HPP
#define _FACTORY_TEST_HPP

#include <string>
#include <memory>
#include <iostream>

enum RENDER_TYPE {
    OPENGL = 1,
    VULKAN = 2,
};

#define _OPENGL_RENDER "OpenGL"
#define _VULKAN_RENDER "Vulkan"

class _Render {

};

// 类的继承默认是 Private !!! 需要手动设置public
class OpenGLRender : public _Render {
public:
    OpenGLRender() {
        std::cout<< _OPENGL_RENDER << std::endl;
    }
};

class VulkanRender : public _Render {
public:
    VulkanRender() {
        std::cout << _VULKAN_RENDER << std::endl;
    }

};


// 工厂函数 根据传入的值来确定
class FactoryRender {

public:
    static std::unique_ptr<_Render> createRenderer( std::string type  ) {
        if ( type == _OPENGL_RENDER ) return std::make_unique<OpenGLRender>();
        if ( type == _VULKAN_RENDER ) return std::make_unique<VulkanRender>();
        else return nullptr;
    }

};

#endif
