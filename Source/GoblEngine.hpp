#pragma once
#ifndef GOBLENGINE_H
#define GOBLENGINE_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

inline float lerp(float a, float b, float f) { return (a * (1.0f - f)) + (b * f); }

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

struct Vec2
{
    float x = 0, y = 0;

    Vec2(float _x, float _y)
    {
        x = _x;
        y = _y;
    }
    Vec2() = default;
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

    bool operator ==(const Color& b) const 
    {
        return (this->a == b.a && this->r == b.r && this->g == b.g && this->b == b.b);
    }

    bool operator !=(const Color& b) const { return !(*this == b); }
};

inline Uint32 ColorFromRGB(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 0xFF)
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

inline Color GetColorFromInt(const Uint32& col)
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
    float fDeltaTime = 0.0;

    void Tick()
    {
        uint32_t tick_time = SDL_GetTicks();
        delta = tick_time - last_tick_time;

        deltaTime = delta / 1000.0;
        fDeltaTime = static_cast<float>(deltaTime);

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
    class TextureManager
    {
    private:
        static std::vector<SDL_Texture*> textures;

    public:
        static SDL_Texture* GetTexture(int id) { return textures.at(id); }

        // FIXME: Make the texture manager store the texture instead of the sprite object
        static int CreateTexture(SDL_Texture* texture) 
        {
            textures.push_back(texture);

            return textures.size() - 1;
        }
    };

    struct RenderObject 
    {
        Color color{ 0xFF, 0xFF, 0xFF, 0xFF };
        int textureId = -1;
        SDL_Rect rect{}, sprRect{};

        RenderObject(SDL_Texture* texture, SDL_Rect rect, SDL_Rect sprRect) 
        {
            textureId = TextureManager::CreateTexture(texture);
            this->rect = rect;
            this->sprRect = sprRect;
        }
        RenderObject() = default;
    };

    struct RenderText 
    {
        std::string text = "";
        int size = 0, x = 0, y = 0;
        Uint8 r = 0, g = 0, b = 0;
        Uint16 outline = 0;

        RenderText() = default;
        RenderText(std::string _t, int _x, int _y, int _s, Uint8 _r, Uint8 _g, Uint8 _b)
        {
            text = _t;
            size = _s;
            x = _x;
            y = _y;
            r = _r;
            g = _g;
            b = _b;
        }

        RenderText(std::string _t, int _x, int _y, int _s, Uint8 _r, Uint8 _g, Uint8 _b, Uint16 _o)
        {
            text = _t;
            size = _s;
            x = _x;
            y = _y;
            r = _r;
            g = _g;
            b = _b;
            outline = _o;
        }
    };

    inline bool CriticalError(const char* out)
    {
        std::cout << "CRITICAL ERROR: " << out << std::endl;

        return false;
    }

    class InputManager
    {
    private:
        int mouseX = 0, mouseY = 0;
        Uint8 mouseButton = 0, prevButton = 0;

        Uint64 eatInput = 0;

        Sint32 mouseWheel = 0;
        Sint32 prevMouseWheel = 0;

        std::unordered_map<Sint32, KeyState> keyMap;

    public:
        static InputManager* instance;
        InputManager() { instance = this; }

        bool PollEvents();

        static bool GetKey(SDL_Keycode keycode) { return instance->keyMap[keycode] != KEY_NONE && instance->keyMap[keycode] != KEY_RELEASED; }
        static bool GetKeyPressed(SDL_Keycode keycode);
        static bool GetKeyReleased(SDL_Keycode keycode);
        static bool GetMouseButton(Uint8 b) { return instance->mouseButton == b; }
        static bool GetMouseButtonDown(Uint8 b) { return instance->mouseButton == b && instance->prevButton != b; }
        static bool GetMouseButtonUp(Uint8 b) { return instance->mouseButton != b && instance->prevButton == b; }
        static float GetMouseWheel() { return static_cast<float>(instance->mouseWheel - instance->prevMouseWheel); }
        static IntVec2 GetMouse() { return { instance->mouseX, instance->mouseY }; }

        // Functional stuff
        void SetEatInput(int amnt) { eatInput = amnt; }
        Uint64 GetEatInput() { return eatInput; }
    };

    struct Camera 
    {
    public:
        Vec2 pos{};
        float zoom = 0;

        SDL_Rect GetRect(SDL_Rect rect) 
        {
            rect.x -= static_cast<int>(pos.x);
            rect.y -= static_cast<int>(pos.y);

            rect.x *= static_cast<int>(1 + zoom);
            rect.y *= static_cast<int>(1 + zoom);
            rect.w *= static_cast<int>(1 + zoom);
            rect.h *= static_cast<int>(1 + zoom);

            return rect;
        }
    };

    class GoblRenderer
    {
    private:
        SDL_Window* m_window = NULL;
        SDL_Renderer* sdlRenderer = NULL;
        SDL_Texture* bgTex = NULL;
        Uint32* m_buffer = nullptr;
        bool shouldUpdateTexture = true;

        std::vector<RenderObject> renderObjects;
        std::vector<RenderText> strings;

        const char* windowTitle = "undef";
        const char* windowInfo = "";
        int WINDOW_WIDTH = 0, WINDOW_HEIGHT = 0;

        // Fonts
        std::string defaultFontName = "Fonts/Alkhemikal.ttf";
        TTF_Font* defaultFont;

    public:
        GoblRenderer() = default;
        ~GoblRenderer() { Close(); }

    public:
        bool Init();
        void Close();

    public:
        void SetWinTitle(const char* title) 
        {
            windowTitle = title;
            if (m_window != NULL) SDL_SetWindowTitle(m_window, title);
        }

        void SetWinInfo(const char* info)
        {
            windowInfo = info;
            std::string winowTitle = std::string(windowTitle) + " " + std::string(windowInfo);
            if (m_window != NULL) SDL_SetWindowTitle(m_window, winowTitle.c_str());
        }

        const char* GetWinInfo() { return windowInfo; }

        void ClearPresentation() { SDL_RenderClear(sdlRenderer); }
        void PresentBackground();
        void Present();
        void SetPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b);
        void ClearScreen(Color c = { 0, 0, 0, 0 });
        SDL_Texture* LoadTexture(const char* path, SDL_Rect& rect, SDL_Rect& sprRect);
        void DrawTexture(SDL_Texture* texture, SDL_Rect& rect, SDL_Rect& sprRect);
        void QueueTexture(SDL_Texture* texture, SDL_Rect& rect, SDL_Rect& sprRect);
        void QueueTexture(RenderObject ro);

        void QueueString(std::string text, int size, int x, int y, Uint8 r, Uint8 g, Uint8 b);
        void QueueString(RenderText t);

    private:
        void DrawStrings();
        void RenderSurfaces();

    public: // Public accessors
        SDL_Renderer* GetRenderer() { return sdlRenderer; }
        const int GetWindowWidth() { return WINDOW_WIDTH; }
        const int GetWindowHeight() { return WINDOW_HEIGHT; }
    };

    class Sprite
    {
    private:
        RenderObject renderObject{};

        GoblRenderer* renderer = nullptr;

    public:
        bool GetTextureExists() { return renderObject.textureId != -1; }

        void Draw();
        void DrawRelative(Camera* cam);
        void DrawImmediate();

        void SetAlpha(Uint8 alpha) { renderObject.color.a = alpha; }
        void SetColorMod(Color c) { renderObject.color = { c.r, c.g, c.b, c.a }; }

        void SetDimensions(int w, int h);
        void SetDimensions(IntVec2 d) { SetDimensions(d.x, d.y); }
        IntVec2 GetDimensions() { return { renderObject.sprRect.w, renderObject.sprRect.h }; }

        void SetSpriteIndex(int x, int y = 0);
        int GetSpriteIndex();

        bool Overlaps(int x, int y);
        bool Overlaps(IntVec2 pos) { return Overlaps(pos.x, pos.y); }

        void SetScale(int w, int h);
        void SetScale(float v);
        void ModScale(int w, int h);

        IntVec2 GetScale() { return { renderObject.rect.w, renderObject.rect.h }; }

        // Mutators
        void SetPosition(int x, int y);
        void SetPosition(IntVec2 pos) { SetPosition(pos.x, pos.y); }

        // Accessors
        IntVec2 GetPosition() { return { renderObject.rect.x, renderObject.rect.y }; }
        std::string GetRectDebugInfo();

    public:
        void LoadTexture(const char* path);
        void Create(GoblRenderer* _renderer, const char* path);

        Sprite() = default;
        Sprite(const Sprite&) = delete;
        Sprite(GoblRenderer* _renderer, const char* path = "") : renderer(_renderer)
        {
            if (path != "") LoadTexture(path);
        }

        // FIXME: This might not work...
        ~Sprite()
        { 
            if (renderObject.textureId != -1) 
                SDL_DestroyTexture(TextureManager::GetTexture(renderObject.textureId)); 
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
        Camera cam;

        Sprite* ngnLogo = nullptr;

        float splashTime = 3.0f;
        const float FRAME_TIME = 0.1f;
        float frameTime = FRAME_TIME;

    public:
        Clock time;
        bool debugging = false;

        void Launch()
        {
            InputManager inputManager{};
            Init();

            renderer.Init();
            renderer.ClearScreen();

            ngnLogo = new Sprite(&renderer, "Sprites/goblEngineLogo_Egg.png");
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

                if (InputManager::instance->PollEvents() == false) appRunning = false;

                if (Splash() == false) break;
                splashTime -= static_cast<float>(time.deltaTime);
                Debug();

                renderer.Present();
            }

            delete splash;

            while (appRunning)
            {
                if (Start() == false) break;

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
                    Debug();

                    time.Tick();
                }

                if (Exit() == true) break;
            }

            renderer.Close();
            SDL_Quit();
            IMG_Quit();
            TTF_Quit();
        }

        Sprite* CreateSpriteObject(const char* path) { return new Sprite(&renderer, path); }
        void CreateSpriteObject(Sprite& sprite, const char* path) { sprite.Create(&renderer, path); }

        Sprite* GetEngineLogo() { return ngnLogo; }

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
        virtual void Debug() 
        {
            if (InputManager::GetKeyPressed(SDLK_F3)) debugging = !debugging;

            if (debugging)
            {
                std::string value = "- FPS: " + std::to_string(time.GetFps());
                value += " delta: " + std::to_string(time.deltaTime);
                renderer.SetWinInfo(value.c_str());
            }
            else if (renderer.GetWinInfo() != "") renderer.SetWinInfo("");
        }
        virtual bool Exit() { return true; } // Return true to complete exit

    public:
        void SetTitle(const char* title) { renderer.SetWinTitle(title); }

        Camera* GetCameraObject() { return &cam; }

        Vec2 GetCamera() { return cam.pos; }
        void MoveCamera(float mX, float mY)
        {
            cam.pos.x += mX;
            cam.pos.y += mY;
        }
        void MoveZoom(float amnt) { cam.zoom += amnt; }

        Uint32 GetScreenWidth() { return renderer.GetWindowWidth(); }
        Uint32 GetScreenHeight() { return renderer.GetWindowHeight(); }

    public: // Draw functions
        GoblEngine& DrawString(std::string text, int x = 0, int y = 0, int size = 20, Uint8 r = 0xFF, Uint8 g = 0xFF, Uint8 b = 0xFF)
        {
            renderer.QueueString({ text, x, y, size, r, g, b });

            return *this;
        }

        GoblEngine& DrawOutlinedString(std::string text, int x = 0, int y = 0, int size = 20, Uint16 outlineSize = 1, Uint8 r = 0xFF, Uint8 g = 0xFF, Uint8 b = 0xFF)
        {
            renderer.QueueString({ text, x, y, size, r, g, b, outlineSize });

            return *this;
        }
    };

}

// UI stuff
namespace gobl
{
    class Switch 
    {
    private:
        bool active = false;
        Sprite spr;
        IntVec2 pos{};

    public:
        Sprite& GetSprite() { return spr; }

        const bool GetActive() { return active; }
        void Toggle() { active = !active; }
        void SetupSprite(GoblEngine* ge, const char* sprPath) { ge->CreateSpriteObject(spr, sprPath); }
        void SetPosition(IntVec2 pos) { this->pos = pos; }

        // FIXME: let the designer handle this stuff, don't hard code it
        void Draw() 
        {
            spr.SetDimensions(20, 12);
            spr.SetSpriteIndex(0);
            spr.SetScale(3.0f);
            spr.SetPosition(pos);
            spr.SetAlpha(100);
            spr.Draw();
            int w = spr.GetDimensions().x / 3;

            spr.SetSpriteIndex(1);
            spr.SetDimensions(13, 12);
            spr.SetScale(3.0f);
            spr.SetPosition({ pos.x + (active ? spr.GetScale().x - (w + 5) : -5), pos.y });
            spr.SetAlpha(255);
            spr.Draw();
        }

        void Update() 
        {
            spr.SetSpriteIndex(0);
            spr.SetScale(4.0f);
            spr.SetPosition(pos);

            if (spr.Overlaps(InputManager::GetMouse())) 
            {
                if (InputManager::GetMouseButtonUp(MB_LEFT)) 
                {
                    Toggle();
                }
            }

            Draw();
        }
    };
}

#endif // !GOBLENGINE_H