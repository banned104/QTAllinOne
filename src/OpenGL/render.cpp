#include "render.hpp"

void Render::initializeGL() {
    initializeOpenGLFunctions();
    initializeShader();
}

void Render::initializeShader() {

}

void Render::resizeGL( int w, int h ) {
    // vec2 position + vec2 size
    glViewport(0, 0, w, h);
}

void Render::render() {

}
