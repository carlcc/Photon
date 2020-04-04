//
// Created by carl on 20-4-1.
//

#include "MediaPlayer.h"
#include "PlayerEvent.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"
#include <SDL2/SDL.h>
#include <SSBase/Buffer.h>
#include <functional>
#include <glad/glad.h>
#include <iostream>
#include <memory>
#include <mutex>

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
    uvs[0] = vec2(0.0f, 1.0f);
    uvs[1] = vec2(1.0f, 0.0f);
    uvs[2] = vec2(0.0f, 0.0f);
    uvs[3] = vec2(0.0f, 1.0f);
    uvs[4] = vec2(1.0f, 1.0f);
    uvs[5] = vec2(1.0f, 0.0f);

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
    float y = texture(yTex, texCoord). r - 16.0/256;
    float u = texture(uTex, texCoord).r - 0.5;
    float v = texture(vTex, texCoord).r - 0.5;
    float r = y -0.001 * u + 1.404*v;
    float g = y - 0.3441 * u - 0.7141*v;
    float b = y + 1.772 * u - 0.001*v;
    OutColor = vec4(r,g,b, 1.0);
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
        glUseProgram(glProgram_);
        glUniform1i(glGetUniformLocation(glProgram_, "yTex"), 0);
        CHECK_GL_ERROR();
        glUniform1i(glGetUniformLocation(glProgram_, "uTex"), 1);
        CHECK_GL_ERROR();
        glUniform1i(glGetUniformLocation(glProgram_, "vTex"), 2);
        CHECK_GL_ERROR();

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glGenTextures(3, yuvTextures);
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
        if (frame == nullptr) {
            return;
        }

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
        glBindTexture(GL_TEXTURE_2D, yuvTextures[0]);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, yStride);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, y);
        glGenerateMipmap(GL_TEXTURE_2D);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, yuvTextures[1]);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, uStride);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, u);
        glGenerateMipmap(GL_TEXTURE_2D);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, yuvTextures[2]);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, vStride);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, v);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    void Draw()
    {
        glUseProgram(glProgram_);
        glBindVertexArray(VAO);
        for (int i = 0; i < 3; ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, yuvTextures[i]);
        }
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
    GLuint glProgram_ { 0 };
    GLuint yuvTextures[3] { 0 };
    GLuint VAO { 0 };
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

    explicit Impl(const Config* config)
    {
        config_ = *config;
    }

    ~Impl()
    {
        Stop();
    }

    void Stop()
    {
        isRunning_ = false;
    }

    bool Init()
    {
        static SDLInitHelper sSDLInitHelper;

        window_ = SDL_CreateWindow(config_.title, config_.x, config_.y, config_.width, config_.height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
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
        isRunning_ = true;
        Init();

        while (isRunning_) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
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

        SDL_CloseAudio();
        Clean();
        return 0;
    }

    void HandleUserEvent(SDL_UserEvent& event)
    {
        playerDisplayShader_->SetTextureData(frameData_.get());
    }

    void RenderImage()
    {
        if (frameData_ == nullptr) {
            return;
        }
        ImGuiIO& io = ImGui::GetIO();
        auto imgW = frameData_->Width();
        auto imgH = frameData_->Height();
        float imgAspect = 1.0f * imgH / imgW;
        auto winW = io.DisplaySize.x;
        auto winH = io.DisplaySize.y;
        float winAspect = 1.0f * winH / winW;

        int x, y;
        int w, h;
        if (winAspect < imgAspect) {
            w = winH / imgAspect;
            h = winH;
            x = (winW - w) / 2;
            y = 0;
        } else {
            w = winW;
            h = winW * imgAspect;
            x = 0;
            y = (winH - h) / 2;
        }

        glViewport(x, y, w, h);
        playerDisplayShader_->Draw();
    }

    void RepaintWindow()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window_);
        ImGui::NewFrame();

        if (onUpdate_) {
            onUpdate_();
        }

        // mWindowSystem->emit(EventType::Window::GuiRenderEvent, dummy); // gen draw calls
        ImGui::Render();

        ImGuiIO& io = ImGui::GetIO();
        glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        RenderImage();

        glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
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

    bool SetAudioFormat(const MicConf& format)
    {
        SDL_CloseAudio();

        // Audio
        SDL_AudioSpec wanted_spec, got;
        wanted_spec.freq = format.sampleRate;
        wanted_spec.format = AUDIO_S16SYS;
        wanted_spec.channels = format.channelCount;
        wanted_spec.silence = 0;
        wanted_spec.samples = 1024;
        wanted_spec.callback = [](void* userdata, Uint8* stream, int len) {
            Impl* self = (Impl*)userdata;
            if (len > self->audioBuffer_.Size()) {
                len = self->audioBuffer_.Size();
            }
            std::unique_lock<std::mutex> lck(self->audioBufferMutex_);
            //            SDL_MixAudio(stream, self->audioBuffer_.GetData<uint8_t>(), len, SDL_MIX_MAXVOLUME);
            self->audioBuffer_.ReadData(stream, len);
            self->audioBuffer_.Skip(len);
        };
        wanted_spec.userdata = this;

        if (SDL_OpenAudio(&wanted_spec, &got) < 0) {
            std::cerr << "can't open audio: " << SDL_GetError() << std::endl;
            micConf_ = MicConf {};
            return false;
        }
        micConf_ = format;
        SDL_PauseAudio(0);
        return true;
    }

    uint32_t PushAudioFrames(const int16_t* data, uint32_t count)
    {
        std::unique_lock<std::mutex> lck(audioBufferMutex_);
        return audioBuffer_.TryPushData(data, count * sizeof(int16_t) * micConf_.channelCount);
    }

private:
    bool isRunning_ { false };
    Config config_ {};

    // These members are only alive while Run() is not return
    SDL_Window* window_ { nullptr };
    SDL_GLContext glContext_ { nullptr };
    ImGuiContext* imguiContext_ { nullptr };
    PlayerDisplayShader* playerDisplayShader_ { nullptr };
    std::shared_ptr<IVideoFrame> frameData_ { nullptr };
    std::function<void()> onUpdate_;
    ss::Buffer<256 * 1024> audioBuffer_ {}; // 256kB audio buffer
    std::mutex audioBufferMutex_;
    MicConf micConf_;

    friend class MediaPlayer;
};

MediaPlayer::MediaPlayer(const Config* config)
    : impl_(new Impl(config))
{
    impl_->onUpdate_ = [this]() {
        UpdateUI();
    };
}

MediaPlayer::~MediaPlayer()
{
    delete impl_;
}

bool MediaPlayer::Run()
{
    return impl_->Run();
}

void MediaPlayer::Stop()
{
    return impl_->Stop();
}

void MediaPlayer::SetVideoFrame(const std::shared_ptr<IVideoFrame>& frame)
{
    impl_->SetVideoFrame(frame);
}

void MediaPlayer::SetAudioFormat(const MicConf& format)
{
    impl_->SetAudioFormat(format);
}

uint32_t MediaPlayer::PushAudioFrames(const int16_t* data, uint32_t count)
{
    return impl_->PushAudioFrames(data, count);
}