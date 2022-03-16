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

float lerp(float a, float b, float f)
{
    return (a * (1.0 - f)) + (b * f);
}

struct IntVec2 
{
    int x = 0, y = 0;

    IntVec2(int _x, int _y)
    {
        x = _x;
        y = _y;
    }
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

namespace Engine
{
    struct RenderText 
    {
        std::string text;
        int size, x, y, r, g, b;

        RenderText(std::string _t, int _s, int _x, int _y, int _r, int _g, int _b) 
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
        int mouseButton = -1;

        std::unordered_map<Sint32, KeyState> keyMap;

    public:
        static InputManager* instance;

        InputManager()
        {
            instance = this;
        }

        bool PollEvents()
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                switch (event.type) 
                {
                    case SDL_QUIT:
                        return false;

                    case SDL_KEYDOWN:

                        if (keyMap[event.key.keysym.sym] == KEY_PRESSED)
                            keyMap[event.key.keysym.sym] = KEY_HELD;
                        else keyMap[event.key.keysym.sym] = KEY_PRESSED;

                        break;
                    case SDL_KEYUP:

                        keyMap[event.key.keysym.sym] = KEY_RELEASED;

                        break;
                }

                // FIXME: Add a released end state
            }

            mouseButton = SDL_GetMouseState(&mouseX, &mouseY);

            return true;
        }

        bool GetKey(SDL_Keycode keycode)
        {
            return keyMap[keycode] != KEY_NONE;
        }

        bool GetKeyPressed(SDL_Keycode keycode)
        {
            if (keyMap.find(keycode) == keyMap.end())
            {
                keyMap[keycode] = KEY_NONE;
                return false;
            }

            return keyMap[keycode] == KEY_PRESSED;
        }

        bool GetMouseButton(int b) { return mouseButton == b; }
        IntVec2 GetMouse() { return { mouseX, mouseY }; }
    };

    InputManager* InputManager::instance = nullptr;

    class EngineRenderer
    {
    private:

        SDL_Window* m_window = NULL;
        SDL_Renderer* m_renderer = NULL;
        SDL_Texture* m_texture = NULL;
        Uint32* m_buffer = nullptr;

        std::vector<SDL_Texture*> textures;
        std::vector<SDL_Rect> rects;
        std::vector<SDL_Rect> spriteRects;

        std::vector<RenderText> strings;

        const char* windowTitle = "undef";
        int WINDOW_WIDTH = 0, WINDOW_HEIGHT = 0;

        // Fonts
        std::string defaultFont = "Fonts/Alkhemikal.ttf";

    public:
        EngineRenderer() = default;
        ~EngineRenderer() { Close(); }

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
            m_renderer = SDL_CreateRenderer(m_window, -1, render_flags);
            if (m_renderer == NULL) return CriticalError("Coult not create SDL_Renderer!");

            m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ABGR8888,
                SDL_TEXTUREACCESS_STATIC, settings.windowWidth, settings.windowHeight);
            if (m_texture == NULL) return CriticalError("Coult not create SDL_Texture!");

            m_buffer = new Uint32[settings.windowWidth * settings.windowHeight];
            if (m_buffer == NULL) return CriticalError("Unable to allocate memory to create buffer!");

            std::memset(m_buffer, 0, settings.windowWidth * settings.windowHeight * sizeof(Uint32)); // Clear the buffer

            WINDOW_WIDTH = settings.windowWidth;
            WINDOW_HEIGHT = settings.windowHeight;

            // Init image library
            Uint32 flags = IMG_INIT_JPG | IMG_INIT_PNG;
            IMG_Init(flags);

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
            if (m_renderer != NULL) SDL_DestroyRenderer(m_renderer);
            if (m_texture != NULL) SDL_DestroyTexture(m_texture);
            if (m_window != NULL) SDL_DestroyWindow(m_window);
            if (m_buffer != nullptr) 
            {
                delete[] m_buffer;
                m_buffer = nullptr;
            }
        }

    public:

        void SetWinTitle(const char* title) { windowTitle = title; }

        void Present()
        {
            //ClearScreen(0.99);

            SDL_UpdateTexture(m_texture, NULL, m_buffer, WINDOW_WIDTH * sizeof(Uint32)); // Clear the texture
            SDL_RenderClear(m_renderer); // Clear the renderer
            SDL_RenderCopy(m_renderer, m_texture, NULL, NULL); // Move the texture to the renderer
            RenderSurfaces();
            DrawStrings();
            SDL_RenderPresent(m_renderer); // Show the renderer
        }

        void SetPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b)
        {
            if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT)  return;

            m_buffer[(y * WINDOW_WIDTH) + x] = ColorFromRGB(r, g, b);
        }

        // MUST BE CALLED BEFORE INITIALIZATION
        void SetScreen(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 0xFF)
        {
            for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++)
            {
                m_buffer[i] = ColorFromRGB(r, g, b, a);
            }
        }

        void ClearScreen(double trailLength)
        {
            if (trailLength > 1) trailLength = 1;
            else if (trailLength < 0) trailLength = 0;

            for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++)
            {
                Uint32 lastColor = m_buffer[i];
                Color c = GetColorFromInt(lastColor);

                m_buffer[i] = ColorFromRGB(c.r * trailLength, c.g * trailLength, c.b * trailLength);
                m_buffer[i] = 0;
            }
        }

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

        Color GetColorFromInt(const Uint32& col) {
            int r = (col & 0xFF0000) >> 16;
            int g = (col & 0x00FF00) >> 8;
            int b = (col & 0x0000FF);

            return Color(r, g, b, 0xFF);
        }

        SDL_Texture* LoadTexture(const char* path, IntVec2* spriteDimensions = nullptr) 
        {
            SDL_Surface* surface = IMG_Load(path);

            if (surface == nullptr) 
            {
                std::cout << "Unable to load image: " << path << std::endl;
                return nullptr;
            }

            if (spriteDimensions != nullptr)
            {
                spriteDimensions->x = surface->w;
                spriteDimensions->y = surface->h;
            }

            SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
            SDL_FreeSurface(surface);

            return texture;
        }

        void QueueTexture(SDL_Texture* texture, SDL_Rect rect, SDL_Rect spriteRect)
        {
            textures.push_back(texture);
            rects.push_back(rect);
            spriteRects.push_back(spriteRect);
        }

        void QueueString(std::string text, int size, int x, int y, int r, int g, int b) 
        {
            strings.push_back({ text, size, x, y, r, g, b });
        }

        void DrawText(std::string text, int size, int x, int y, int r, int g, int b)
        {
            TTF_Font* font = TTF_OpenFont(defaultFont.c_str(), size);
            if (!font)
            {
                std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
                return;
            }

            SDL_Color col = { r, g, b, 0xFF };
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), col);
            if (!textSurface)
            {
                std::cout << "Failed to render text: " << TTF_GetError() << std::endl;
                // Maybe free surface here? I don't know
                return;
            }

            SDL_Texture* text_texture;

            text_texture = SDL_CreateTextureFromSurface(m_renderer, textSurface);

            SDL_Rect dest = { x, y, textSurface->w, textSurface->h };

            SDL_RenderCopy(m_renderer, text_texture, NULL, &dest);

            SDL_DestroyTexture(text_texture);
            SDL_FreeSurface(textSurface);

            TTF_CloseFont(font);
        }

        const int GetWindowWidth() { return WINDOW_WIDTH; }
        const int GetWindowHeight() { return WINDOW_HEIGHT; }

    private:
        void DrawStrings()
        {
            for (auto str : strings) 
            {
                DrawText(str.text, str.size, str.x, str.y, str.r, str.g, str.b);
            }

            strings.clear();
        }

        void RenderSurfaces()
        {
            int i = 0;
            for (auto texture : textures)
            {
                SDL_RenderCopy(m_renderer, texture, &spriteRects.at(i), &rects.at(i)); // Move the texture to the renderer

                i++;
            }

            textures.clear();
            rects.clear();
            spriteRects.clear();
        }

        SDL_Renderer& GetRenderer() { return *m_renderer; }
    };

    class SpriteRenderer
    {
    private:
        SDL_Texture* texture = NULL;
        SDL_Rect rect{};
        SDL_Rect spriteSegment{};

        EngineRenderer* renderer = NULL;

    public:
        bool GetTextureExists() { return texture != NULL; }

        void FormatTexture()
        {
            Uint32* pFormat = new Uint32(SDL_PIXELFORMAT_RGBA32);
            SDL_QueryTexture(texture, pFormat, NULL, &rect.w, &rect.h);

            delete pFormat;
        }

        void LoadTexture(const char* path)
        {
            //std::cout << "Loading texture... " << path << std::endl;
            IntVec2 sprDimensions{ 0,0 };
            texture = renderer->LoadTexture(path, &sprDimensions);
            spriteSegment.w = sprDimensions.x;
            spriteSegment.h = sprDimensions.y;

            FormatTexture();
        }

        void Draw()
        {
            if (GetTextureExists() == false) 
            {
                CriticalError("ERROR: Cannot render a NULL texture.");
            }
            else renderer->QueueTexture(texture, rect, spriteSegment); 
        }

        void SetDimensions(int w, int h)
        {
            spriteSegment.w = w;
            spriteSegment.h = h;
            rect.w = w;
            rect.h = h;
        }

        IntVec2 GetDimensions()
        {
            return { spriteSegment.w, spriteSegment.h };
        }

        void SetSpriteIndex(int i)
        {
            spriteSegment.x = spriteSegment.w * i;
        }

        int GetSpriteIndex()
        {
            return (spriteSegment.x / spriteSegment.w);
        }

        bool Overlaps(int x, int y) 
        {
            return (y >= rect.y && y <= rect.y + rect.h) && (x >= rect.x && x <= rect.x + rect.w);
        }

        void SetScale(int w, int h)
        {
            rect.w = w;
            rect.h = h;
        }

        void SetScale(float v)
        {
            rect.w = spriteSegment.w * v;
            rect.h = spriteSegment.h * v;
        }

        IntVec2 GetScale()
        {
            return { rect.w, rect.h };
        }

        void SetPosition(int x, int y)
        {
            rect.x = x;
            rect.y = y;
        }

        void SetPosition(IntVec2 pos)
        {
            rect.x = pos.x;
            rect.y = pos.y;
        }

        IntVec2 GetPosition()
        {
            return { rect.x, rect.y };
        }

    public:
        SpriteRenderer() = default;
        SpriteRenderer(EngineRenderer * _renderer, const char* path = "") : renderer(_renderer)
        {
            if (path != "") LoadTexture(path);
        }

        ~SpriteRenderer()
        {
            if (texture != NULL) SDL_DestroyTexture(texture);
        }
    };

    //class Object
    //{
    //public:
    //    // List components
    //};

    class GameEngine
    {
    private:
        std::string appTitle;
        EngineRenderer renderer{};

    public:
        Clock time;

        void Launch()
        {
            InputManager inputManager{};
            Init();

            renderer.SetWinTitle(appTitle.c_str());
            renderer.Init();

            Start();

            while (1)
            {
                // Draw the current frame content
                Draw(renderer);
                renderer.Present();

                time.Tick();

                // Get input for the next frame
                if (InputManager::instance->PollEvents() == false) break;
                if (Update() == false) break;
            }

            renderer.Close();
            SDL_Quit();
            IMG_Quit();
            TTF_Quit();
        }

        // FIXME: Provide option to load a texture at creation time
        SpriteRenderer CreateSpriteObject(const char* path = "") 
        { return SpriteRenderer(&renderer, path); }

        InputManager& Input() { return *InputManager::instance; }

    protected:
        virtual void Init() { SetTitle("demo"); }
        virtual bool Start() { return true; }
        virtual bool Update() { return true; }
        virtual void Draw(EngineRenderer& renderer) {}

    protected:
        // Must be called during initialization
        void SetTitle(const char* title)
        {
            appTitle = title;
        }

    public: // Draw functions
        GameEngine& DrawString(std::string text, int size, int x, int y, int r, int g, int b) 
        {
            renderer.QueueString(text, size, x, y, r, g, b);

            return *this;
        }
    };

}
#endif // !ENGINE_H