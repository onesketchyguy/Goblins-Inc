#include "GoblEngine.hpp"

// Input manager
namespace gobl
{
    bool InputManager::PollEvents()
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

        prevMouseWheel = mouseWheel;

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
            case SDL_MOUSEWHEEL:

                mouseWheel = event.wheel.y;

                break;

            default:
                break;
            }
        }

        prevButton = mouseButton;
        mouseButton = SDL_GetMouseState(&mouseX, &mouseY);

        return true;
    }

    bool InputManager::GetKeyPressed(SDL_Keycode keycode)
    {
        if (instance->keyMap.find(keycode) == instance->keyMap.end())
        {
            instance->keyMap[keycode] = KEY_NONE;
            return false;
        }

        if (instance->keyMap[keycode] == KEY_PRESSED)
        {
            instance->keyMap[keycode] = KEY_HELD;
            return true;
        }

        return false;
    }

    bool InputManager::GetKeyReleased(SDL_Keycode keycode)
    {
        if (instance->keyMap.find(keycode) == instance->keyMap.end())
        {
            instance->keyMap[keycode] = KEY_NONE;
            return false;
        }

        if (instance->keyMap[keycode] == KEY_RELEASED)
        {
            instance->keyMap[keycode] = KEY_NONE;
            return true;
        }

        return false;
    }

	InputManager* InputManager::instance = nullptr;

}

// Renderer
namespace gobl 
{
    bool GoblRenderer::Init()
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

        defaultFont = TTF_OpenFont(defaultFontName.c_str(), 20);
        if (!defaultFont)
        {
            std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
            return CriticalError("Unable to load default fonts!");
        }

        return true;
    }

    void GoblRenderer::Close()
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

        TTF_CloseFont(defaultFont);
    }

    void GoblRenderer::PresentBackground()
    {
        if (shouldUpdateTexture)
        {
            SDL_UpdateTexture(bgTex, NULL, m_buffer, WINDOW_WIDTH * sizeof(Uint32)); // Clear the texture
            shouldUpdateTexture = false;
        }

        SDL_RenderCopy(sdlRenderer, bgTex, NULL, NULL); // Move the texture to the renderer
    }

    void GoblRenderer::Present()
    {
        RenderSurfaces();
        DrawStrings();
        SDL_RenderPresent(sdlRenderer); // Show the renderer
    }

    void GoblRenderer::SetPixel(int x, int y, Uint8 r, Uint8 g, Uint8 b)
    {
        if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT)  return;

        m_buffer[(y * WINDOW_WIDTH) + x] = ColorFromRGB(r, g, b);

        shouldUpdateTexture = true;
    }

    void GoblRenderer::ClearScreen(Color c)
    {
        for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) m_buffer[i] = ColorFromRGB(c.r, c.g, c.b, c.a);

        shouldUpdateTexture = true;
    }

    SDL_Texture* GoblRenderer::LoadTexture(const char* path, SDL_Rect& rect, SDL_Rect& sprRect)
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

    void GoblRenderer::DrawTexture(SDL_Texture* texture, SDL_Rect& rect, SDL_Rect& sprRect)
    {
        // Move the texture to the renderer
        if (SDL_RenderCopy(sdlRenderer, texture, &sprRect, &rect) < 0)
            std::cout << "ERROR: " << SDL_GetError() << std::endl;
    }

    void GoblRenderer::QueueTexture(SDL_Texture* texture, SDL_Rect& rect, SDL_Rect& sprRect)
    {
        renderObjects.push_back({texture, rect, sprRect});
    }
    void GoblRenderer::QueueTexture(RenderObject ro) { renderObjects.push_back(ro); }

    void GoblRenderer::QueueString(std::string text, int size, int x, int y, Uint8 r, Uint8 g, Uint8 b)
    { 
        strings.push_back({ text, size, x, y, r, g, b }); 
    }
    void GoblRenderer::QueueString(RenderText t) 
    { 
        strings.push_back(t); 
    }

    void GoblRenderer::DrawStrings()
    {
        for (auto str : strings)
        {
            if (str.text.length() < 1) continue;

            TTF_SetFontSize(defaultFont, str.size);

            SDL_Color col = { str.r, str.g, str.b, 0xFF };
            SDL_Surface* textSurface = TTF_RenderText_Solid(defaultFont, str.text.c_str(), col);
            if (!textSurface)
            {
                std::cout << "Failed to render text: " << TTF_GetError() << std::endl;
                return;
            }

            SDL_Texture* text_texture = SDL_CreateTextureFromSurface(sdlRenderer, textSurface);
            SDL_Rect dest = { str.x, str.y, textSurface->w, textSurface->h };

            if (str.outline > 0)
            {
                SDL_SetTextureColorMod(text_texture, 0, 0, 0);

                dest.x -= str.outline / 2;
                dest.y -= str.outline / 2;

                for (Uint16 i = 0; i < str.outline; i++)
                {
                    dest.x += i;
                    dest.y += i;

                    SDL_RenderCopy(sdlRenderer, text_texture, NULL, &dest);
                }

                dest = { str.x, str.y, textSurface->w, textSurface->h };
                SDL_SetTextureColorMod(text_texture, str.r, str.g, str.b);
            }

            SDL_RenderCopy(sdlRenderer, text_texture, NULL, &dest);

            SDL_DestroyTexture(text_texture);
            SDL_FreeSurface(textSurface);
        }

        strings.clear();
    }

    void GoblRenderer::RenderSurfaces()
    {
        SDL_Texture* lastT = nullptr;
        Color lastC{ 0xFF, 0xFF, 0xFF, 0xFF };

        for (size_t i = 0; i < renderObjects.size(); i++)
        {
            auto r = renderObjects.at(i).rect;
            r.w += 1;
            r.h += 1;
            r.x -= 1;
            r.y -= 1;

            Color c = renderObjects.at(i).color;

            if (c != lastC || lastT != renderObjects.at(i).texture)
            {
                SDL_SetTextureColorMod(renderObjects.at(i).texture, c.r, c.g, c.b);
                SDL_SetTextureAlphaMod(renderObjects.at(i).texture, c.a);

                lastC = c;
                lastT = renderObjects.at(i).texture;
            }

            // Move the texture to the renderer
            if (SDL_RenderCopy(sdlRenderer, lastT, &renderObjects.at(i).sprRect, &r) < 0)
                std::cout << "ERROR: " << SDL_GetError() << std::endl;
        }

        renderObjects.clear();
    }
}

// Sprite
namespace gobl 
{
    void TranslateRect(Camera* cam, RenderObject& ro)
    {
        ro.rect = cam->GetRect(ro.rect);
    }

    void Sprite::Draw()
    {
        if (GetTextureExists() == false) CriticalError("ERROR: Cannot render a NULL texture.");
        else renderer->QueueTexture(renderObject);
    }

    void Sprite::DrawRelative(Camera* cam) 
    {
        if (GetTextureExists() == false) CriticalError("ERROR: Cannot render a NULL texture.");
        else
        {
            RenderObject ro = RenderObject(renderObject);
            TranslateRect(cam, ro); // FIXME: This is a bandaid to a greater problem.
            renderer->QueueTexture(ro);
        }
    }

    void Sprite::DrawImmediate()
    {
        if (GetTextureExists() == false) CriticalError("ERROR: Cannot render a NULL texture.");
        else renderer->DrawTexture(renderObject.texture, renderObject.rect, renderObject.sprRect);
    }

    void Sprite::SetSpriteIndex(int x, int y) 
    {
        renderObject.sprRect.x = renderObject.sprRect.w * x;
        renderObject.sprRect.y = renderObject.sprRect.h * y;
    }
    int Sprite::GetSpriteIndex() { return (renderObject.sprRect.x / renderObject.sprRect.w); }

    void Sprite::SetDimensions(int w, int h)
    {
        renderObject.sprRect.w = w;
        renderObject.sprRect.h = h;
        renderObject.rect.w = w;
        renderObject.rect.h = h;
    }

    bool Sprite::Overlaps(int x, int y)
    {
        // FIXME: Ask for a camera relative position
        auto r = renderObject.rect; //GetRect();
        return (y >= r.y && y <= r.y + r.h) && (x >= r.x && x <= r.x + r.w);
    }

    void Sprite::SetScale(int w, int h)
    {
        renderObject.rect.w = w;
        renderObject.rect.h = h;
    }

    void Sprite::SetScale(float v)
    {
        renderObject.rect.w = static_cast<int>(renderObject.sprRect.w * v);
        renderObject.rect.h = static_cast<int>(renderObject.sprRect.h * v);
    }

    void Sprite::ModScale(int w, int h)
    {
        renderObject.rect.w += static_cast<int>(w);
        renderObject.rect.h += static_cast<int>(h);
    }

    void Sprite::SetPosition(int x, int y)
    {
        renderObject.rect.x = x;
        renderObject.rect.y = y;
    }

    std::string Sprite::GetRectDebugInfo()
    {
        return "Rect: x" + std::to_string(renderObject.rect.x) + " y" + std::to_string(renderObject.rect.y) + " w" +
            std::to_string(renderObject.rect.w) + " h" + std::to_string(renderObject.rect.h) +
            " sprRect: x" + std::to_string(renderObject.sprRect.x) + " y" + std::to_string(renderObject.sprRect.x) +
            " w" + std::to_string(renderObject.rect.w) + " h" + std::to_string(renderObject.rect.h);
    }

    void Sprite::LoadTexture(const char* path)
    {
        std::cout << "Loading texture... " << path << std::endl;
        renderObject.texture = renderer->LoadTexture(path, renderObject.rect, renderObject.sprRect);
    }

    void Sprite::Create(GoblRenderer* _renderer, const char* path)
    {
        renderer = _renderer;
        if (path != "") LoadTexture(path);
    }
}