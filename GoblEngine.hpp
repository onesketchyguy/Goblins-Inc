#pragma once
#ifndef ENGINE_H
#define ENGINE_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <fstream>
#include "libs/json.hpp"
using json = nlohmann::json;

float lerp(float a, float b, float f) { return (a * (1.0f - f)) + (b * f); }

struct IntVec2 
{
    int x = 0, y = 0;

    IntVec2(int _x, int _y)
    {
        x = _x;
        y = _y;
    }
    IntVec2() = default;
};

struct Color 
{
    Uint8 r = 0, g = 0, b = 0, a = 0;

    Color() = default;
    Color(Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a = 0xFF) 
    {
        r = _r;
        g = _g;
        b = _b;
        a = _a;
    }
};

Uint32 ColorFromRGB(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 0xFF)
{
    Uint32 color = 0;

    color += r;
    color <<= 8;
    color += g;
    color <<= 8;
    color += b;
    color <<= 8;
    color += a;

    return color;
}

Color GetColorFromInt(const Uint32& col)
{
    int r = (col & 0xFF0000) >> 16;
    int g = (col & 0x00FF00) >> 8;
    int b = (col & 0x0000FF);

    return Color(r, g, b, 0xFF);
}

enum MOUSE_BUTTON : Uint32
{
    NONE = 0,
    MB_LEFT = 1,
    MB_RIGHT = 4,
    MB_MIDDLE = 2
};

struct Clock
{
private:
    uint32_t last_tick_time = 0;
    uint32_t delta = 0;

    Uint32 fps = 30;
    Uint32 frames = 0;

    double frameTime = 0.0;

public:
    double deltaTime = 0.0;

    void Tick()
    {
        uint32_t tick_time = SDL_GetTicks();
        delta = tick_time - last_tick_time;

        deltaTime = delta / 1000.0;

        last_tick_time = tick_time;

        frames++;
        frameTime += deltaTime;
        if (frameTime >= 1.0f) // Real fps
        {
            fps = frames;
            frames = 0;
            frameTime = 0.0;
        }
    }

    Uint32 GetFps() { return fps; }
};

enum KeyState : Uint8
{
    KEY_NONE = 0,
    KEY_PRESSED = 1,
    KEY_HELD = 2,
    KEY_RELEASED = 3,
};

namespace gobl
{
    struct RenderText 
    {
        std::string text;
        int size, x, y;
        Uint8 r, g, b;

        RenderText(std::string _t, int _s, int _x, int _y, Uint8 _r, Uint8 _g, Uint8 _b)
        {
            text = _t;
            size = _s;
            x = _x;
            y = _y;
            r = _r;
            g = _g;
            b = _b;
        }
    };

    struct InitializationData
    {
        bool soundEnabled = true;
        int windowWidth = 1024;
        int windowHeight = 720;

        // Write JSON to file
        const static void WriteToJson(InitializationData& settings, std::string fileName = "Data/init.json")
        {
            std::ofstream o(fileName.c_str());

            json j;
            j["soundEnabled"] = settings.soundEnabled;
            j["windowWidth"] = settings.windowWidth;
            j["windowHeight"] = settings.windowHeight;
            o << j << std::endl;

            o.close();
        }

        // Read a JSON file
        const static bool LoadJsonData(InitializationData& settings, std::string fileName = "Data/init.json")
        {
            std::ifstream fs(fileName.c_str());

            if (fs.is_open() == false)
            {
                std::cout << "No settings file found. Returning default settings." << std::endl;
                return false;
            }

            std::cout << "Found file... ";
            std::string content;

            while (fs.eof() == false)
            {
                std::string lineContent;
                std::getline(fs, lineContent);

                content += lineContent;
            }

            fs.close();

            if (content.empty() == false)
            {
                auto j = json::parse(content);

                settings.soundEnabled = j.at("soundEnabled");
                settings.windowWidth = j.at("windowWidth");
                settings.windowHeight = j.at("windowHeight");
            }

            std::cout << "Settings loaded." << std::endl;

            return true;
        }
    };

    bool CriticalError(const char* out)
    {
        std::cout << out << std::endl;

        return false;
    }

    class InputManager
    {
    private:
        int mouseX = 0, mouseY = 0;
        Uint8 mouseButton = -1;
        Uint8 prevButton = -1;

        Uint64 eatInput = 0;

        std::unordered_map<Sint32, KeyState> keyMap;

    public:
        static InputManager* instance;
        InputManager() { instance = this; }

        bool PollEvents()
        {
            SDL_Event event;

            if (eatInput > 0) 
            {
                while (SDL_PollEvent(&event)) if (event.type == SDL_QUIT) return false;

                prevButton = 0;
                mouseButton = 0;

                eatInput--;

                return true;
            }

            while (SDL_PollEvent(&event))
            {
                switch (event.type) 
                {
                    case SDL_QUIT:
                        return false;

                    case SDL_KEYDOWN:
                        
                        if (keyMap[event.key.keysym.sym] != KEY_HELD)
                            keyMap[event.key.keysym.sym] = KEY_PRESSED;

                        break;
                    case SDL_KEYUP:

                        keyMap[event.key.keysym.sym] = KEY_RELEASED;

                        break;
                }
            }

            prevButton = mouseButton;
            mouseButton = SDL_GetMouseState(&mouseX, &mouseY);

            return true;
        }

        bool GetKey(SDL_Keycode keycode) { return keyMap[keycode] != KEY_NONE; }

        bool GetKeyPressed(SDL_Keycode keycode)
        {
            if (keyMap.find(keycode) == keyMap.end())
            {
                keyMap[keycode] = KEY_NONE;
                return false;
            }

            if (keyMap[keycode] == KEY_PRESSED)
            {
                keyMap[keycode] = KEY_HELD;
                return true;
            }

            return false;
        }

        bool GetKeyReleased(SDL_Keycode keycode)
        {
            if (keyMap.find(keycode) == keyMap.end())
            {
                keyMap[keycode] = KEY_NONE;
                return false;
            }

            if (keyMap[keycode] == KEY_RELEASED)
            {
                keyMap[keycode] = KEY_NONE;
                return true;
            }

            return false;
        }

        bool GetMouseButton(int b) { return mouseButton == b; }
        bool GetMouseButtonDown(int b) { return mouseButton == b && prevButton != b; }
        bool GetMouseButtonUp(int b) { return mouseButton != b && prevButton == b; }

        IntVec2 GetMouse() { return { mouseX, mouseY }; }

        // Functional stuff
        void SetEatInput(int amnt) { eatInput = amnt; }
        Uint64 GetEatInput() { return eatInput; }
    };

    InputManager* InputManager::instance = nullptr;

    class GoblRenderer
    {
    private:

        SDL_Window* m_window = NULL;
        SDL_Renderer* sdlRenderer = NULL;
        SDL_Texture* bgTex = NULL;
        Uint32* m_buffer = nullptr;
        bool shouldUpdateTexture = true;

        std::vector<SDL_Texture*> textures;
        std::vector<SDL_Rect> rects;
        std::vector<SDL_Rect> spriteRects;

        std::vector<RenderText> strings;

        const char* windowTitle = "undef";
        int WINDOW_WIDTH = 0, WINDOW_HEIGHT = 0;

        // Fonts
        std::string defaultFont = "Fonts/Alkhemikal.ttf";

    public:
        GoblRenderer() = default;
        ~GoblRenderer() { Close(); }

    public:
        bool Init() 
        {
            InitializationData settings;
            InitializationData::LoadJsonData(settings);

            // returns zero on success else non-zero
            if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
                printf("Error initializing SDL: %s\n", SDL_GetError());

                return CriticalError("Unable to init SDL");
            }

            m_window = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                settings.windowWidth, settings.windowHeight, SDL_WINDOW_VULKAN);

            if (m_window == NULL) return CriticalError("Window initialization failed!");

            Uint32 render_flags = SDL_RENDERER_ACCELERATED;
            sdlRenderer = SDL_CreateRenderer(m_window, -1, render_flags);
            if (sdlRenderer == NULL) return CriticalError("Coult not create SDL_Renderer!");

            bgTex = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGBA8888,
                SDL_TEXTUREACCESS_STATIC, settings.windowWidth, settings.windowHeight);
            if (bgTex == NULL) return CriticalError("Coult not create SDL_Texture!");

            m_buffer = new Uint32[settings.windowWidth * settings.windowHeight];
            if (m_buffer == NULL) return CriticalError("Unable to allocate memory to create buffer!");

            std::memset(m_buffer, 0, settings.windowWidth * settings.windowHeight * sizeof(Uint32)); // Clear the buffer

            WINDOW_WIDTH = settings.windowWidth;
            WINDOW_HEIGHT = settings.windowHeight;

            // Init image library
            if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) < 0)
            {
                std::cout << "Error initializing SDL_Image: " << IMG_GetError() << std::endl;
                return CriticalError("Unable to inititialize images!");
            }

            // Init fonts
            if (TTF_Init() < 0) 
            {
                std::cout << "Error initializing SDL_ttf: " << TTF_GetError() << std::endl;
                return CriticalError("Unable to inititialize fonts!");
            }

            return true;
        }

        void Close()
        {
            if (sdlRenderer != NULL) SDL_DestroyRenderer(sdlRenderer);
            if (bgTex != NULL) 
            {
                SDL_DestroyTexture(bgTex);

                if (m_buffer != nullptr)
                {
                    delete[] m_buffer;
                    m_buffer = nullptr;
                }
            }
            if (m_window != NULL) SDL_DestroyWindow(m_window);
        }

    public:
        void SetWinTitle(const char* title) 
        {
            windowTitle = title;
            if (m_window != NULL) SDL_SetWindowTitle(m_window, title);
        }

        void ClearPresentation() 
        {
            SDL_RenderClear(sdlRenderer); // Clear the renderer
        }

        void PresentBackground()
        {
            if (shouldUpdateTexture)
            {
                SDL_UpdateTexture(bgTex, NULL, m_buffer, WINDOW_WIDTH * sizeof(Uint32)); // Clear the texture
                shouldUpdateTexture = false;
            }

            SDL_RenderCopy(sdlRenderer, bgTex, NULL, NULL); // Move the texture to the renderer
        }

        void Present()
        {
            RenderSurfaces();
            DrawStrings();
            SDL_RenderPresent(sdlRenderer); // Show the renderer
        }

        void SetPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b)
        {
            if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT)  return;

            m_buffer[(y * WINDOW_WIDTH) + x] = ColorFromRGB(r, g, b);

            shouldUpdateTexture = true;
        }

        void ClearScreen(Color c = { 0, 0, 0, 0})
        {
            for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) m_buffer[i] = ColorFromRGB(c.r, c.g, c.b, c.a);

            shouldUpdateTexture = true;
        }

        SDL_Texture* LoadTexture(const char* path, SDL_Rect& rect, SDL_Rect& sprRect)
        {
            // Load image in a surface
            SDL_Surface* surface = IMG_Load(path);

            if (surface == nullptr) 
            {
                std::cout << "Unable to load image: " << path << std::endl;
                return nullptr;
            }

            // Create the texture
            SDL_Texture* texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
            SDL_FreeSurface(surface);

            // Rect initialization
            rect.x = rect.y = sprRect.y = sprRect.x = 0;

            // Set the dimensions and hook the rect
            SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
            sprRect.w = rect.w;
            sprRect.h = rect.h;

            return texture;
        }

        void DrawTexture(SDL_Texture* texture, SDL_Rect& rect, SDL_Rect& spriteRect)
        {
            // Move the texture to the renderer
            if (SDL_RenderCopy(sdlRenderer, texture, &spriteRect, &rect) < 0)
                std::cout << "ERROR: " << SDL_GetError() << std::endl;
        }

        void QueueTexture(SDL_Texture* texture, SDL_Rect& rect, SDL_Rect& spriteRect)
        {
            textures.push_back(texture);
            rects.push_back(rect);
            spriteRects.push_back(spriteRect);
        }

        void QueueString(std::string text, int size, int x, int y, Uint8 r, Uint8 g, Uint8 b)
        {
            strings.push_back({ text, size, x, y, r, g, b });
        }

    private:
        void DrawStrings()
        {
            for (auto str : strings) 
            {
                TTF_Font* font = TTF_OpenFont(defaultFont.c_str(), str.size);

                if (!font)
                {
                    std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
                    continue;
                }

                SDL_Color col = { str.r, str.g, str.b, 0xFF };
                SDL_Surface* textSurface = TTF_RenderText_Solid(font, str.text.c_str(), col);
                if (!textSurface)
                {
                    std::cout << "Failed to render text: " << TTF_GetError() << std::endl;
                    return;
                }

                SDL_Texture* text_texture;

                text_texture = SDL_CreateTextureFromSurface(sdlRenderer, textSurface);

                SDL_Rect dest = { str.x, str.y, textSurface->w, textSurface->h };

                SDL_RenderCopy(sdlRenderer, text_texture, NULL, &dest);

                SDL_DestroyTexture(text_texture);
                SDL_FreeSurface(textSurface);

                TTF_CloseFont(font);
            }

            strings.clear();
        }

        void RenderSurfaces()
        {
            for (size_t i = 0; i < textures.size(); i++)
            {
                // Move the texture to the renderer
                if (SDL_RenderCopy(sdlRenderer, textures.at(i), &spriteRects.at(i), &rects.at(i)) < 0) 
                    std::cout << "ERROR: " << SDL_GetError() << std::endl;
            }

            textures.clear();
            rects.clear();
            spriteRects.clear();
        }

    public: // Public accessors
        SDL_Renderer* GetRenderer() { return sdlRenderer; }
        const int GetWindowWidth() { return WINDOW_WIDTH; }
        const int GetWindowHeight() { return WINDOW_HEIGHT; }
    };

    class Sprite
    {
    private:
        SDL_Texture* texture = nullptr;
        SDL_Rect rect = { 0, 0, 0, 0 };
        SDL_Rect sprRect = { 0, 0, 0, 0 };

        GoblRenderer* renderer = nullptr;

    public:
        bool GetTextureExists() { return texture != nullptr; }

        void Draw()
        {
            if (GetTextureExists() == false) CriticalError("ERROR: Cannot render a NULL texture.");
            else renderer->QueueTexture(texture, rect, sprRect);
        }

        void DrawImmediate()
        {
            if (GetTextureExists() == false) CriticalError("ERROR: Cannot render a NULL texture.");
            else renderer->DrawTexture(texture, rect, sprRect);
        }

        void SetDimensions(int w, int h)
        {
            sprRect.w = w;
            sprRect.h = h;
            rect.w = w;
            rect.h = h;
        }

        void SetDimensions(IntVec2 d) { SetDimensions(d.x, d.y); }
        IntVec2 GetDimensions() { return { sprRect.w, sprRect.h }; }

        void SetSpriteIndex(int i) { sprRect.x = sprRect.w * i; }
        int GetSpriteIndex() { return (sprRect.x / sprRect.w); }

        bool Overlaps(int x, int y) { return (y >= rect.y && y <= rect.y + rect.h) && (x >= rect.x && x <= rect.x + rect.w); }

        void SetScale(int w, int h)
        {
            rect.w = w;
            rect.h = h;
        }

        void SetScale(float v)
        {
            rect.w = static_cast<int>(sprRect.w * v);
            rect.h = static_cast<int>(sprRect.h * v);
        }

        IntVec2 GetScale() { return { rect.w, rect.h }; }

        void SetPosition(int x, int y)
        {
            rect.x = x;
            rect.y = y;
        }

        void SetPosition(IntVec2 pos) { SetPosition(pos.x, pos.y); }

        IntVec2 GetPosition() { return { rect.x, rect.y }; }

        std::string GetRectDebugInfo() 
        {
            return "Rect: x" + std::to_string(rect.x) + " y" + std::to_string(rect.y) + " w" + 
                std::to_string(rect.w) + " h" + std::to_string(rect.h) + 
                " sprRect: x" + std::to_string(sprRect.x) + " y" + std::to_string(sprRect.x) +
                " w" + std::to_string(rect.w) + " h" + std::to_string(rect.h);
        }

    public:
        void LoadTexture(const char* path)
        {
            std::cout << "Loading texture... " << path << std::endl;
            texture = renderer->LoadTexture(path, rect, sprRect);
        }

        void Create(GoblRenderer* _renderer, const char* path)
        {
            renderer = _renderer;
            if (path != "") LoadTexture(path);
        }

        Sprite() = default;
        Sprite(const Sprite&) = delete;
        Sprite(GoblRenderer * _renderer, const char* path = "") : renderer(_renderer)
        {
            if (path != "") LoadTexture(path);
        }

        ~Sprite()
        {
            if (texture != NULL) SDL_DestroyTexture(texture);
        }
    };

    //class Object
    //{
    //public:
    //    // List components
    //};

    class GoblEngine
    {
    private:
        GoblRenderer renderer{};
        Sprite* splash = nullptr;
        float splashTime = 3.0f;
        const float FRAME_TIME = 0.1f;
        float frameTime = FRAME_TIME;

    public:
        Clock time;

        void Launch()
        {
            InputManager inputManager{};
            Init();

            renderer.Init();
            renderer.ClearScreen();

            splash = new Sprite(&renderer, "Sprites/gobleLogoAnim.png");
            splash->SetPosition(300, 200);
            splash->SetDimensions(64, 64);
            splash->SetScale(6.0f);

            bool appRunning = true;

            while (appRunning)
            {
                renderer.ClearPresentation();

                // Draw the current frame content
                Draw(renderer);

                time.Tick();

                // Allow the user to close the window and that's it
                inputManager.SetEatInput(1);
                if (InputManager::instance->PollEvents() == false) appRunning = false;

                if (Splash() == false) break;
                splashTime -= static_cast<float>(time.deltaTime);

                renderer.Present();
            }

            delete splash;

            Start();

            while (appRunning)
            {
                renderer.ClearPresentation();
                renderer.PresentBackground();

                // Get input for the next frame
                if (InputManager::instance->PollEvents() == false) break;
                if (Update() == false) break;

                // Draw the current frame content
                Draw(renderer);
                renderer.Present();

                time.Tick();
            }

            renderer.Close();
            SDL_Quit();
            IMG_Quit();
            TTF_Quit();
        }

        Sprite* CreateSpriteObject(const char* path) { return new Sprite(&renderer, path); }
        void CreateSpriteObject(Sprite& sprite, const char* path) { sprite.Create(&renderer, path); }

        InputManager& Input() { return *InputManager::instance; }

    protected:
        virtual void Init() { SetTitle("demo"); }
        virtual bool Splash() 
        {
            splash->Draw();

            if (splashTime <= 1.0f) 
            {
                if (frameTime <= 0.0f) 
                {
                    int sprIndex = splash->GetSpriteIndex();

                    if (sprIndex < 3)
                    {
                        splash->SetSpriteIndex(sprIndex + 1);
                        frameTime = FRAME_TIME;
                    }
                }
                else frameTime -= static_cast<float>(time.deltaTime);
            }

            return splashTime > 0.0f;
        }
        virtual bool Start() { return true; }
        virtual bool Update() { return true; }
        virtual void Draw(GoblRenderer& renderer) {}

    protected:
        void SetTitle(const char* title) { renderer.SetWinTitle(title); }

    public: // Draw functions
        GoblEngine& DrawString(std::string text, int size = 20, int x = 0, int y = 0, Uint8 r = 0xFF, Uint8 g = 0xFF, Uint8 b = 0xFF)
        {
            renderer.QueueString(text, size, x, y, r, g, b);

            return *this;
        }
    };

}
#endif // !ENGINE_H