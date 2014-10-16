#ifndef VIDEO_SDL_H
#define VIDEO_SDL_H

#include "bus.h"

class SDLVideoDevice : public IVideoDevice {
  public:
    SDLVideoDevice();
    ~SDLVideoDevice();

  public:
    void set_buffer(Framebuffer const& buffer) override;
    void on_frame() override;

  private:
    struct SDL_Window *window;
    void *glcon;
    uint32_t texture;
    Framebuffer _buffer;

};

#endif