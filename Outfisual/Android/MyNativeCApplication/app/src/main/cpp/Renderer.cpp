#include "Renderer.h"

#include "AndroidOut.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>

void Renderer::render()
{
    // Bind shader program & VAO
    glUseProgram(shader_program);
    glBindVertexArray(VAO);

    // Update shader uniforms
    struct timeval tv;
    gettimeofday(&tv, NULL);
    float time_in_secs = (float)(tv.tv_sec - time_at_start) + ((float)tv.tv_usec / 1000000.0f);
    aout << "time: " << time_in_secs << std::endl;
    glUniform1f(glGetUniformLocation(shader_program, "time"), time_in_secs);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    eglSwapBuffers(display_, surface_);
}

void Renderer::initRenderer()
{
    // TEMPORARY
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_at_start = tv.tv_sec;

    // Initialize EGL and get the default display associated with Android
    display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display_, nullptr, nullptr);

    // Configuration attributes for our EGL context
    constexpr EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    // Determine how many EGL configs match the requested attributes
    EGLint numberOfConfigs;
    eglChooseConfig(display_, attribs, nullptr, 0, &numberOfConfigs);
    if (numberOfConfigs == 0)
        aout << "ERROR: No matching EGL config found. Check attribute values for correctness." << std::endl;

    // Retrieve all matching configs and select the first one
    auto supportedConfigs = new EGLConfig[numberOfConfigs];
    eglChooseConfig(display_, attribs, supportedConfigs, numberOfConfigs, &numberOfConfigs);
    EGLConfig config = supportedConfigs[0]; // select the first config from the list
    delete[] supportedConfigs;

    // Create the proper window surface
    EGLint format;
    eglGetConfigAttrib(display_, config, EGL_NATIVE_VISUAL_ID, &format);
    surface_ = eglCreateWindowSurface(display_, config, app_->window, nullptr);

    // Create an OpenGL ES 3 context
    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    context_ = eglCreateContext(display_, config, nullptr, contextAttribs);
    eglMakeCurrent(display_, surface_, surface_, context_);

    // Set up any other OpenGL related global states
    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
}

void Renderer::initTriangle()
{
    // Vertex data for triangle
    float triangle_vertex[] = {
            -0.7f, -0.25f, 0.0f,
            0.7f, -0.25f, 0.0f,
            0.0f,  0.25f, 0.0f
    };

    // Generate VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Generate VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertex), triangle_vertex, GL_STATIC_DRAW);

    // Create Vertex Shader
    const char* vs = "#version 300 es\n"
                     "layout (location = 0) in vec3 aPos;\n"

                     "uniform float time;\n"

                     "out vec4 vertex_colour;\n"

                     "void main()\n"
                     "{\n"
                     "    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"

                     "    if (aPos.x ==  0.0f && aPos.y >=  0.1f)\n"
                     "    {\n"
                     "        vertex_colour = vec4(1.0f, 0.3f, 0.3f, 1.0f);\n"  // red
                     "        gl_Position = vec4(cos(time), sin(time) / 2.35f, aPos.z, 1.0);\n"
                     "    }\n"
                     "    if (aPos.x <= -0.1f && aPos.y <= -0.1f)\n"
                     "    {\n"
                     "        vertex_colour = vec4(0.3f, 1.0f, 0.3f, 1.0f);\n"  // green
                     "        gl_Position = vec4(cos(time+radians(60.0f)), sin(time+radians(120.0f)) / 2.35f, aPos.z, 1.0);\n"
                     "    }\n"
                     "    if (aPos.x >=  0.1f && aPos.y <= -0.1f)\n"
                     "    {\n"
                     "        vertex_colour = vec4(0.3f, 0.3f, 1.0f, 1.0f);\n"  // blue
                     "        gl_Position = vec4(cos(time+radians(120.0f)), sin(time+radians(240.0f)) / 2.35f, aPos.z, 1.0);\n"
                     "    }\n"
                     "}\n";
    GLuint vertex_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vs, NULL);
    glCompileShader(vertex_shader);

    // Log any compilation errors
    int  success;
    char infoLog[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
        aout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Create Fragment Shader
    const char* fs = "#version 300 es\n"
                     "precision mediump float;"
                     "out vec4 FragColor;\n"

                     "in vec4 vertex_colour;\n"

                     "void main()\n"
                     "{\n"
                     "    FragColor = vertex_colour;\n"
                     "}\n";
    GLuint fragment_shader;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fs, NULL);
    glCompileShader(fragment_shader);

    // Log any compilation errors
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
        aout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Create Shader Program
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    // Log any compilation errors
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        aout << "ERROR::SHADER_PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Delete shader objects (they have been copied into the shader program and are no longer needed)
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // Set up vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}