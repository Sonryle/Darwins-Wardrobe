#ifndef ANDROID_OUTFISUAL_RENDERER
#define ANDROID_OUTFISUAL_RENDERER

#include <EGL/egl.h>
#include <GLES3/gl3.h>

struct android_app;

struct Renderer
{
    /*!
     * @param pApp the android_app this Renderer belongs to, needed to configure OpenGL
     */
    explicit Renderer(android_app* pApp) :
            app_(pApp),
            display_(EGL_NO_DISPLAY),
            surface_(EGL_NO_SURFACE),
            context_(EGL_NO_CONTEXT),
            width_(-1),
            height_(-1) {
        initRenderer();
        initTriangle();
    }

    /*!
     * Renders a red screen (and a temporary triangle)
     */
    void render();

private:

    /*!
     * Initialises & configures EGL and OpenGLES 3
     */
    void initRenderer();

    /*!
     * // Sets up shaders and sends a triangle to the GPU for rendering with OpenGL
     */
    void initTriangle();

    android_app *app_;
    EGLDisplay display_;
    EGLSurface surface_;
    EGLContext context_;
    EGLint width_;
    EGLint height_;

    GLuint VAO;
    GLuint VBO;
    GLuint shader_program;

    int time_at_start;
};

#endif