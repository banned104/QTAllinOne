#ifndef RENDER_H
#define RENDER_H
#include <QOpenGLFunctions>
class Render : protected QOpenGLFunctions
{
public:
    Render();
    ~Render();

    virtual void initializeGL();
    virtual void initializeShader();
    virtual void resizeGL( int w, int h );
    virtual void render();
};

#endif // RENDER_H
