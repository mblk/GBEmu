#pragma once

#include <memory> // unique_ptr
#include <string>

#include <SDL_rect.h>

struct SDL_Renderer;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Window SDL_Window;
typedef struct _TTF_Font TTF_Font;

namespace GBEmu
{

class SdlDisplayBitmap;
class SdlSound;

class SdlHelper
{
public:
    SdlHelper();
    virtual ~SdlHelper();

    int Initialize();
    int Shutdown();

    void Clear();
    void Present();
    
    SDL_Renderer *GetRenderer() const { return renderer_; }
    const SDL_Rect& GetWindowRect() const { return windowRect_; }

    SdlDisplayBitmap& GetDisplayBitmap() { return *sdlDisplayBitmap_; }
    SdlSound &GetSound() { return *sdlSound_; }

    void RenderTextTopLeft(int x, int y, const char *text);
    void RenderTextCenter(int x, int y, const char *text);

private:
    const std::string fontFileName_;
    const int displayZoomFactor_;

    SDL_Window *window_;
    SDL_Rect windowRect_;
    SDL_Renderer *renderer_;
    TTF_Font *font_;

    std::unique_ptr<SdlDisplayBitmap> sdlDisplayBitmap_;
    std::unique_ptr<SdlSound> sdlSound_;


};


}