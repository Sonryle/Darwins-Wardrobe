#include "Renderer.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <GLES3/gl3.h>
#include <memory>
#include <vector>
#include <android/imagedecoder.h>

#include "Shader.h"
#include "Utility.h"
#include "TextureAsset.h"

static const char* vertex = "#version 300 es\n"
                            "in vec3 inPosition;\n"
                            "in vec2 inUV;\n"

                            "out vec2 fragUV;\n"

                            "uniform mat4 uProjection;\n"

                            "void main() {\n"
                            "    fragUV = inUV;\n"
                            "    gl_Position = uProjection * vec4(inPosition, 1.0);\n"
                            "}\n";

static const char* fragment = "#version 300 es\n"
                              "precision mediump float;\n"

                              "in vec2 fragUV;\n"

                              "uniform sampler2D uTexture;\n"

                              "out vec4 outColor;\n"

                              "void main() {\n"
                              "    outColor = texture(uTexture, fragUV);\n"
                              "}\n";

Renderer::~Renderer() {
    if (display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context_ != EGL_NO_CONTEXT) {
            eglDestroyContext(display_, context_);
            context_ = EGL_NO_CONTEXT;
        }
        if (surface_ != EGL_NO_SURFACE) {
            eglDestroySurface(display_, surface_);
            surface_ = EGL_NO_SURFACE;
        }
        eglTerminate(display_);
        display_ = EGL_NO_DISPLAY;
    }
}

void Renderer::render() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (const auto &model: models_)
        shader_->drawModel(model);

    eglSwapBuffers(display_, surface_);
}

void Renderer::initRenderer() {
    // Choose your render attributes
    constexpr EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    // The default display is probably what you want on Android
    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);

    // figure out how many configs there are
    EGLint numConfigs;
    eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

    // get the list of configurations
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);

    // Find a config we like.
    // Could likely just grab the first if we don't care about anything else in the config.
    // Otherwise hook in your own heuristic
    auto config = *std::find_if(
            supportedConfigs.get(),
            supportedConfigs.get() + numConfigs,
            [&display](const EGLConfig &config) {
                EGLint red, green, blue, depth;
                if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red)
                    && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green)
                    && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue)
                    && eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth)) {

                    //aout << "Found config with " << red << ", " << green << ", " << blue << ", "
                    //     << depth << std::endl;
                    return red == 8 && green == 8 && blue == 8 && depth == 24;
                }
                return false;
            });

    // create the proper window surface
    EGLint format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    EGLSurface surface = eglCreateWindowSurface(display, config, app_->window, nullptr);

    // Create a GLES 3 context
    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);

    // get some window metrics
    eglMakeCurrent(display, surface, surface, context);

    display_ = display;
    surface_ = surface;
    context_ = context;

    shader_ = std::unique_ptr<Shader>(
            Shader::loadShader(vertex, fragment, "inPosition", "inUV", "uProjection"));
    assert(shader_);
    shader_->activate();

    glClearColor(100 / 255.f, 149 / 255.f, 237 / 255.f, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // get some demo models into memory
    createModels();

    // Get dimensions of screen and yada yada
    EGLint width;
    EGLint height;
    eglQuerySurface(display_, surface_, EGL_WIDTH, &width);
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &height);
    glViewport(0, 0, width, height);
    width_ = width;
    height_ = height;

    // Send projection matrix to shader
    float projectionMatrix[16] = {0};
    Utility::buildOrthographicMatrix(projectionMatrix,2,float(width_) / height_,-1,1);
    shader_->setProjectionMatrix(projectionMatrix);
}

/**
 * @brief Create any demo models we want for this demo.
 */
void Renderer::createModels() {
    /*
     * This is a square:
     * 0 --- 1
     * | \   |
     * |  \  |
     * |   \ |
     * 3 --- 2
     */
    std::vector<Vertex> crafting_table_vertices = {
            Vertex(Vector3{0.5f, 1.75f, 0}, Vector2{0, 0}), // 0
            Vertex(Vector3{-0.5f, 1.75f, 0}, Vector2{1, 0}), // 1
            Vertex(Vector3{-0.5f, 0.75f, 0}, Vector2{1, 1}), // 2
            Vertex(Vector3{0.5f, 0.75f, 0}, Vector2{0, 1}) // 3
    };
    std::vector<Index> crafting_table_indices = {
            0, 1, 2, 0, 2, 3
    };

    std::vector<Vertex> furnace_vertices = {
            Vertex(Vector3{0.5f, -0.75f, 0}, Vector2{0, 0}), // 0
            Vertex(Vector3{-0.5f, -0.75f, 0}, Vector2{1, 0}), // 1
            Vertex(Vector3{-0.5f, -1.75f, 0}, Vector2{1, 1}), // 2
            Vertex(Vector3{0.5f, -1.75f, 0}, Vector2{0, 1}) // 3
    };
    std::vector<Index> furnace_indices = {
            0, 1, 2, 0, 2, 3
    };

    std::vector<Vertex> blast_furnace_vertices = {
            Vertex(Vector3{0.5f, 0.5f, 0}, Vector2{0, 0}), // 0
            Vertex(Vector3{-0.5f, 0.5f, 0}, Vector2{1, 0}), // 1
            Vertex(Vector3{-0.5f, -0.5f, 0}, Vector2{1, 1}), // 2
            Vertex(Vector3{0.5f, -0.5f, 0}, Vector2{0, 1}) // 3
    };
    std::vector<Index> blast_furnace_indices = {
            0, 1, 2, 0, 2, 3
    };

    // loads an image and assigns it to the square.
    //
    // Note: there is no texture management in this sample, so if you reuse an image be careful not
    // to load it repeatedly. Since you get a shared_ptr you can safely reuse it in many models.
    auto assetManager = app_->activity->assetManager;
    auto crafting_table_texture = TextureAsset::loadAsset(assetManager, "crafting_table.png");
    auto furnace_texture = TextureAsset::loadAsset(assetManager, "furnace.png");
    auto blast_furnace_texture = TextureAsset::loadAsset(assetManager, "blast_furnace.png");

    // Create a model and put it in the back of the render list.
    models_.emplace_back(crafting_table_vertices, crafting_table_indices, crafting_table_texture);
    models_.emplace_back(furnace_vertices, furnace_indices, furnace_texture);
    models_.emplace_back(blast_furnace_vertices, blast_furnace_indices, blast_furnace_texture);
}

void Renderer::handleInput() {
    // handle all queued inputs
    auto *inputBuffer = android_app_swap_input_buffers(app_);
    if (!inputBuffer) {
        // no inputs yet.
        return;
    }

    // handle motion events (motionEventsCounts can be 0).
    for (auto i = 0; i < inputBuffer->motionEventsCount; i++) {
        auto &motionEvent = inputBuffer->motionEvents[i];
        auto action = motionEvent.action;

        // Find the pointer index, mask and bitshift to turn it into a readable value.
        auto pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

        // get the x and y position of this event if it is not ACTION_MOVE.
        auto &pointer = motionEvent.pointers[pointerIndex];
        auto x = GameActivityPointerAxes_getX(&pointer);
        auto y = GameActivityPointerAxes_getY(&pointer);

        // determine the action type and process the event accordingly.
        switch (action & AMOTION_EVENT_ACTION_MASK) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                //aout << "(" << pointer.id << ", " << x << ", " << y << ") "
                //     << "Pointer Down";
                break;

            case AMOTION_EVENT_ACTION_CANCEL:
                // treat the CANCEL as an UP event: doing nothing in the app, except
                // removing the pointer from the cache if pointers are locally saved.
                // code pass through on purpose.
            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
                //aout << "(" << pointer.id << ", " << x << ", " << y << ") "
                //     << "Pointer Up";
                break;

            case AMOTION_EVENT_ACTION_MOVE:
                // There is no pointer index for ACTION_MOVE, only a snapshot of
                // all active pointers; app needs to cache previous active pointers
                // to figure out which ones are actually moved.
                for (auto index = 0; index < motionEvent.pointerCount; index++) {
                    pointer = motionEvent.pointers[index];
                    x = GameActivityPointerAxes_getX(&pointer);
                    y = GameActivityPointerAxes_getY(&pointer);
                }
                //aout << "Pointer Move";
                break;
            default:
                //aout << "Unknown MotionEvent Action: " << action;
        }
        //aout << std::endl;
    }
    // clear the motion input count in this buffer for main thread to re-use.
    android_app_clear_motion_events(inputBuffer);

    // handle input key events.
    for (auto i = 0; i < inputBuffer->keyEventsCount; i++) {
        auto &keyEvent = inputBuffer->keyEvents[i];
        //aout << "Key: " << keyEvent.keyCode <<" ";
        switch (keyEvent.action) {
            case AKEY_EVENT_ACTION_DOWN:
                //aout << "Key Down";
                break;
            case AKEY_EVENT_ACTION_UP:
                //aout << "Key Up";
                break;
            case AKEY_EVENT_ACTION_MULTIPLE:
                // Deprecated since Android API level 29.
                //aout << "Multiple Key Actions";
                break;
            default:
                //aout << "Unknown KeyEvent Action: " << keyEvent.action;
        }
        //aout << std::endl;
    }
    // clear the key input count too.
    android_app_clear_key_events(inputBuffer);
}