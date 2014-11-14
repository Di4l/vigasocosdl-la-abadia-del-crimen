// LinuxSDLFullScreen32bpp.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "LinuxSDLFullScreen32bpp.h"
#include "IPalette.h"

bool LinuxSDLFullScreen32bpp::init(const VideoInfo *vi, IPalette *pal)  
{
	_bpp = 32;
	_flags |= SDL_FULLSCREEN;

	_isInitialized = LinuxSDLBasicDrawPlugin::init(vi,pal);

	// gets a pointer to the game's palette
	_palette = (UINT32 *)pal->getRawPalette();

	return _isInitialized;
};

//drawing methods
void LinuxSDLFullScreen32bpp::render(bool throttle)
{
	SDL_Flip(screen);
};

void LinuxSDLFullScreen32bpp::setPixel(int x, int y, int color)
{
	/* Lock the screen for direct access to the pixels */
	if ( SDL_MUSTLOCK(screen) ) {
		if ( SDL_LockSurface(screen) < 0 ) {
			fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
			return;
		}
	}


	int bpp = screen->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to set */
	Uint8 *p = (Uint8 *)screen->pixels + y * screen->pitch + x * bpp;

	*(Uint32 *)p = _palette[color];

	if ( SDL_MUSTLOCK(screen) ) {
		SDL_UnlockSurface(screen);
	}
};
