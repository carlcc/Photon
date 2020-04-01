//
// Created by carl on 20-4-1.
//

#include "MediaPlayer.h"
#include "PlayerEvent.h"
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <iostream>
#include <memory>

#define PHOTON_ABORT(msg)                                                                \
    do {                                                                                 \
        std::cerr << __FILE__ << ":" << __LINE__ << ":" << msg << "Abort!" << std::endl; \
        abort();                                                                         \
    } while (false)

#define PHOTON_ASSERT(condition, msg) \
    do {                              \
        if (condition)                \
            break;                    \
        PHOTON_ABORT(#msg << msg);    \
    } while (false)

#define CHECK_GL_ERROR()                                                                           \
    do {                                                                                           \
        auto code = glGetError();                                                                  \
        if (code != GL_NO_ERROR) {                                                                 \
            std::cerr << "GLERROR " << code << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        }                                                                                          \
    } while (false)

const char* kPlayerDisplayVS = R"(#version 330 core
out vec2 texCoord;

void main() {
    vec2 vertices[6];
    vertices[0] = vec2(-1.0f, -1.0f);
    vertices[1] = vec2(1.0f, 1.0f);
    vertices[2] = vec2(-1.0f, 1.0f);
    vertices[3] = vec2(-1.0f, -1.0f);
    vertices[4] = vec2(1.0f, -1.0f);
    vertices[5] = vec2(1.0f, 1.0f);

    vec2 uvs[6];
    uvs[0] = vec2(0.0f, 0.0f);
    uvs[1] = vec2(1.0f, 1.0f);
    uvs[2] = vec2(0.0f, 1.0f);
    uvs[3] = vec2(0.0f, 0.0f);
    uvs[4] = vec2(1.0f, 0.0f);
    uvs[5] = vec2(1.0f, 1.0f);

    gl_Position = vec4(vertices[gl_VertexID], 0.0f, 1.0f);
    texCoord = uvs[gl_VertexID];
}
)";
const char* kPlayerDisplayPS = R"(#version 330 core

in vec2 texCoord;

uniform sampler2D yTex;
uniform sampler2D uTex;
uniform sampler2D vTex;

out vec4 OutColor;

void main() {
    const mat4 yuv2rgb = mat4(
        0.00456, 0.00456, 0.00456, 0.00000,
        0.00000, -0.00153, 0.00791, 0.00000,
        0.00626, -0.00319, 0.00000, 0.00000,
        -0.87416, 0.53133, -1.08599, 1.00000);

    OutColor = vec4(1.0f,0.0f, 0.0f, 1.0f);
}
)";

class PlayerDisplayShader {
public:
    PlayerDisplayShader()
    {
        glProgram_ = createProgram(kPlayerDisplayVS, kPlayerDisplayPS);
        if (glProgram_ == 0) {
            return;
        }
        glUniform1i(glGetUniformLocation(glProgram_, "yTex"), 0);
        glUniform1i(glGetUniformLocation(glProgram_, "uTex"), 1);
        glUniform1i(glGetUniformLocation(glProgram_, "vTex"), 2);

        glGenTextures(3, yuvTextures);

        glGenVertexArrays(1, &VAO);
    }

    ~PlayerDisplayShader()
    {
        if (glProgram_ != 0) {
            glDeleteProgram(glProgram_);
            glProgram_ = 0;
        }
        for (int i = 0; i < 3; ++i) {
            if (yuvTextures[i] != 0) {
                glDeleteTextures(1, yuvTextures + i);
            }
        }
    }

    bool IsValid()
    {
        return glProgram_ != 0 && yuvTextures[0] != 0 && yuvTextures[1] != 0 && yuvTextures[2] != 0 && VAO != 0;
    }

    void SetTextureData(const IVideoFrame* frame)
    {
        GLsizei width = frame->Width();
        GLsizei height = frame->Height();
        const uint8_t* y = frame->Y();
        const uint8_t* u = frame->U();
        const uint8_t* v = frame->V();
        const uint32_t yStride = frame->YStride();
        const uint32_t uStride = frame->UStride();
        const uint32_t vStride = frame->VStride();


        bool needResize = width != currentWidth || height != currentHeight;
        if (needResize) {
            currentWidth = width;
            currentHeight = height;
        }

        glActiveTexture(GL_TEXTURE0);
        if (needResize) {
            glTexStorage2D(GL_TEXTURE_2D, 0, GL_RED, width, height);
        }
        glPixelStorei(GL_UNPACK_ROW_LENGTH, yStride);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, y);

        glActiveTexture(GL_TEXTURE1);
        if (needResize) {
            glTexStorage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2);
        }
        glPixelStorei(GL_UNPACK_ROW_LENGTH, uStride);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RED, GL_UNSIGNED_BYTE, u);

        glActiveTexture(GL_TEXTURE2);
        if (needResize) {
            glTexStorage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2);
        }
        glPixelStorei(GL_UNPACK_ROW_LENGTH, vStride);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RED, GL_UNSIGNED_BYTE, v);
    }

    void Draw()
    {
        glBindVertexArray(VAO);
        glUseProgram(glProgram_);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    static GLuint createProgram(const std::string& vsSource, const std::string& psSource)
    {
        GLuint vs = loadShader(GL_VERTEX_SHADER, vsSource);
        if (vs == 0) {
            glDeleteShader(vs);
            return 0;
        }
        GLuint ps = loadShader(GL_FRAGMENT_SHADER, psSource);
        if (ps == 0) {
            glDeleteShader(vs);
            glDeleteShader(ps);
            return 0;
        }

        std::shared_ptr<void> defer(nullptr, [vs, ps](void*) {
            glDeleteShader(vs);
            glDeleteShader(ps);
        });

        GLuint program = glCreateProgram();
        if (program == 0) {
            return 0;
        }

        // link program
        glAttachShader(program, vs);
        glAttachShader(program, ps);
        glLinkProgram(program);

        int linked, length;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (linked) {
            return program;
        }

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        auto* buffer = new char[length + 1];
        int outLength;
        glGetProgramInfoLog(program, length, &outLength, buffer);
        glDeleteProgram(program);

        std::cerr << "Linke program failed!: " << buffer << std::endl;
        delete[] buffer;

        return 0;
    }

    static GLuint loadShader(GLenum shaderType, const std::string& source)
    {
        GLuint name = glCreateShader(shaderType);
        if (name == 0) {
            CHECK_GL_ERROR();
            return 0;
        }

        const std::string& shaderSource = source;

        const auto* shaderCStr = shaderSource.c_str();
        glShaderSource(name, 1, &shaderCStr, nullptr);
        glCompileShader(name);

        int compiled, length;
        glGetShaderiv(name, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            glGetShaderiv(name, GL_INFO_LOG_LENGTH, &length);
            auto* buffer = new char[length + 1];
            int outLength;
            glGetShaderInfoLog(name, length, &outLength, buffer);
            glDeleteShader(name);
            name = 0;
            std::cerr << "Compile shader failed: " << buffer << std::endl;
            delete[] buffer;

            glDeleteShader(name);
            return 0;
        }

        return name;
    }

private:
    GLuint glProgram_;
    GLuint yuvTextures[3];
    GLuint VAO;
    GLsizei currentWidth { 0 };
    GLsizei currentHeight { 0 };
};

class MediaPlayer::Impl {
public:
    class SDLInitHelper {
    public:
        SDLInitHelper()
        {
            PHOTON_ASSERT(0 == SDL_Init(SDL_INIT_EVERYTHING), "Failed to init SDL2.");

#if __APPLE__
            PHOTON_ASSERT(0 == SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG), "Set attrib failed."); // Always required on Mac
            PHOTON_ASSERT(0 == SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE), "Set attrib failed.");
            PHOTON_ASSERT(0 == SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3), "Set attrib failed.");
            PHOTON_ASSERT(0 == SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2), "Set attrib failed.");
#else
            PHOTON_ASSERT(0 == SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3), "Set attrib failed.");
            PHOTON_ASSERT(0 == SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3), "Set attrib failed.");
            PHOTON_ASSERT(0 == SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE), "Set attrib failed.");
#endif
            PHOTON_ASSERT(0 == SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1), "Set attrib failed.");
        }
    };

    explicit Impl(Config* config)
    {
        config_ = *config;
    }

    ~Impl()
    {
        Stop();
    }

    bool Start()
    {
        if (playerThread_ != nullptr) {
            std::cerr << "The player is already started." << std::endl;
            return true;
        }
        isRunning_ = true;
        // clang-format off
        playerThread_ = SDL_CreateThread([](void* data) -> int {
            auto* self = static_cast<Impl*>(data);
            return self->Run();
        },"PlayerThread", this);
        // clang-format on
        return playerThread_ != nullptr;
    }

    void Stop(bool waitUntilThreadStop = true)
    {
        isRunning_ = false;
        if (waitUntilThreadStop) {
            if (playerThread_ != nullptr) {
                SDL_WaitThread(playerThread_, nullptr);
                playerThread_ = nullptr;
            }
        }
    }

    bool Init()
    {
        static SDLInitHelper sSDLInitHelper;

        window_ = SDL_CreateWindow(config_.title, config_.x, config_.y, config_.width, config_.height, SDL_WINDOW_OPENGL);
        PHOTON_ASSERT(window_ != nullptr, "Create SDL2 window failed.");

        glContext_ = SDL_GL_CreateContext(window_);
        PHOTON_ASSERT(glContext_ != nullptr, "SDL create ogl context failed.");

        PHOTON_ASSERT(SDL_GL_MakeCurrent(window_, glContext_) == 0, "Make context current failed");
        PHOTON_ASSERT(gladLoadGL() != 0, "Initialize opengl failed.");

#if __APPLE__
        const char* glsl_version = "#version 150"; // GL 3.2 Core + GLSL 150
#else
        const char* glsl_version = "#version 130"; // GL 3.0 + GLSL 130
#endif

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        imguiContext_ = ImGui::CreateContext();
        PHOTON_ASSERT(imguiContext_ != nullptr, "Create ImGui context failed.");

        ImGuiIO& io = ImGui::GetIO();
        // io.Fonts->AddFontFromFileTTF("./assets/fonts/wqydkwmh.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
        (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        // ImGui::StyleColorsDark();
        // ImGui::StyleColorsClassic();

        // Setup Platform/Renderer bindings
        PHOTON_ASSERT(ImGui_ImplSDL2_InitForOpenGL(window_, glContext_), "Init failed.");
        PHOTON_ASSERT(ImGui_ImplOpenGL3_Init(glsl_version), "Init failed.");

        playerDisplayShader_ = new PlayerDisplayShader;
        PHOTON_ASSERT(playerDisplayShader_->IsValid(), "Init PlayerDisplayShader failed.");

        return true;
    }

    void Clean()
    {
        if (playerDisplayShader_ != nullptr) {
            delete playerDisplayShader_;
            playerDisplayShader_ = nullptr;
        }
        if (imguiContext_) {
            ImGui::DestroyContext(imguiContext_);
            imguiContext_ = nullptr;
        }
        if (glContext_) {
            SDL_GL_DeleteContext(glContext_);
            glContext_ = nullptr;
        }
        if (window_) {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }
        frameData_ = nullptr;
    }

    int Run()
    {
        Init();

        while (isRunning_) {
            SDL_Event event;
            if (SDL_PollEvent(&event)) {
                if (ImGui_ImplSDL2_ProcessEvent(&event)) {
                    // This event was handled by imgui
                } else {
                    if (event.type == SDL_QUIT) {
                        isRunning_ = false;
                    }
                    if (event.type == SDL_USEREVENT) {
                        HandleUserEvent(event.user);
                    }
                }
            }
            //            else {
            //                PHOTON_ABORT("SDL Wait event error");
            //            }
            RepaintWindow();
        }

        Clean();
        return 0;
    }

    void HandleUserEvent(SDL_UserEvent& event)
    {
        playerDisplayShader_->SetTextureData(frameData_.get());
    }

    void Update()
    {
        if (ImGui::Begin("MainWindow")) {
            ImGui::Text("Text");
        }
        ImGui::End();
    }

    void RenderWindow()
    {
        playerDisplayShader_->Draw();
    }

    void RepaintWindow()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window_);
        ImGui::NewFrame();

        Update();

        // mWindowSystem->emit(EventType::Window::GuiRenderEvent, dummy); // gen draw calls
        ImGui::Render();

        ImGuiIO& io = ImGui::GetIO();
        glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        RenderWindow();

        // mWindowSystem->emit(EventType::Window::RenderEvent, dummy); // do 3d render
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // do actual draw call

        SDL_GL_SwapWindow(window_);
    }

    void SetVideoFrame(const std::shared_ptr<IVideoFrame>& frame)
    {
        SDL_Event event;
        event.type = SDL_USEREVENT;
        event.user.windowID = SDL_GetWindowID(window_);

        frameData_ = frame;

        SDL_PushEvent(&event);
    }

private:
    bool isRunning_ { false };
    SDL_Thread* playerThread_ { nullptr };
    Config config_ {};

    // These members are only alive while Run() is not return
    SDL_Window* window_ { nullptr };
    SDL_GLContext glContext_ { nullptr };
    ImGuiContext* imguiContext_ { nullptr };
    PlayerDisplayShader* playerDisplayShader_ { nullptr };
    std::shared_ptr<IVideoFrame> frameData_ { nullptr };
};

MediaPlayer::MediaPlayer(Config* config)
    : impl_(new Impl(config))
{
}

MediaPlayer::~MediaPlayer()
{
    delete impl_;
}

bool MediaPlayer::Start()
{
    return impl_->Start();
}

void MediaPlayer::Stop(bool waitUntilThreadStop)
{
    return impl_->Stop(waitUntilThreadStop);
}

void MediaPlayer::SetVideoFrame(const std::shared_ptr<IVideoFrame>& frame)
{
    impl_->SetVideoFrame(frame);
}
