#pragma once
#include "irenderer.hpp"
#include <QOpenGLFunctions>
class TriangleRender : protected QOpenGLFunctions, public IRenderer
{
public:
    TriangleRender();
    ~TriangleRender();

    virtual void initializeGL();
    virtual void initializeShader();
    virtual void resizeGL( int w, int h );
    virtual void render();
};


