// LinuxSDLFullScreen32bpp.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "LinuxSDLFullScreen32bpp.h"
#include "IPalette.h"

#ifdef DEBUG
#include <stdio.h>
#define DEBUG_FAIL_FUNC printf("%s\n",__func__);
#else
#define DEBUG_FAIL_FUNC 
#endif

void PrintFlags(Uint32 flags);
	bool LinuxSDLFullScreen32bpp::init(const VideoInfo *vi, IPalette *pal)  
	{
		if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
#ifdef DEBUG
			fprintf(stderr,
                	"Couldn't initialize SDL: %s\n", SDL_GetError());
#endif
			return false;
		}

		screen = SDL_SetVideoMode(vi->width, vi->height, 32, SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_FULLSCREEN);
		if ( screen == NULL ) {
#ifdef DEBUG
		        fprintf(stderr, "Couldn't set 640x480x32 video mode: %s\n",
                        SDL_GetError());
#endif
		        return false;
	    	}
		PrintFlags(screen->flags);

		surface = SDL_CreateRGBSurface(SDL_HWSURFACE,screen->w, screen->h,32, 0, 0, 0, 0);

	        // gets a pointer to the game's palette
	        _palette = (UINT32 *)pal->getRawPalette();

		_isInitialized = true;

		return true;
	};

	void LinuxSDLFullScreen32bpp::end()  { LinuxSDLBasicDrawPlugin::end(); };

	// no se porque el linkador se queja sin esto, ?no deberia tomar la virtual de LinuxSDLBasicDrawPlugin ?
	bool LinuxSDLFullScreen32bpp::isInitialized() const  { return LinuxSDLBasicDrawPlugin::isInitialized(); };


	bool LinuxSDLFullScreen32bpp::isFullScreen() const  { DEBUG_FAIL_FUNC };

	// bitmap creation/destruction
	int LinuxSDLFullScreen32bpp::createBitmap(int width, int height) { DEBUG_FAIL_FUNC };  
	void LinuxSDLFullScreen32bpp::destroyBitmap(int bitmap) { DEBUG_FAIL_FUNC };  

	// bitmap methods
	void LinuxSDLFullScreen32bpp::setActiveBitmap(int bitmap) { DEBUG_FAIL_FUNC };  
	void LinuxSDLFullScreen32bpp::compose(int bitmap, int mode, int attr) { DEBUG_FAIL_FUNC };  
	void LinuxSDLFullScreen32bpp::getDimensions(int &width, int &height) const   { DEBUG_FAIL_FUNC };

	// clipping methods
	const Rect *LinuxSDLFullScreen32bpp::getClipArea() const   { DEBUG_FAIL_FUNC };
	void LinuxSDLFullScreen32bpp::setClipArea(int x, int y, int width, int height) { DEBUG_FAIL_FUNC };  
	void LinuxSDLFullScreen32bpp::setNoClip() { DEBUG_FAIL_FUNC };  

	// drawing methods
	void LinuxSDLFullScreen32bpp::render(bool throttle)
	{
		// SDL_UpdateRect(screen, 0, 0, 0, 0);
		SDL_BlitSurface(surface, NULL, screen, NULL);
		SDL_Flip(screen);
	};

	void LinuxSDLFullScreen32bpp::setPixel(int x, int y, int color)
	{
		/* Lock the screen for direct access to the pixels */ /*
		if ( SDL_MUSTLOCK(surface) ) {
			if ( SDL_LockSurface(surface) < 0 ) {
#ifdef DEBUG
		            fprintf(stderr, "Can't lock surface: %s\n", SDL_GetError());
#endif
		            return;
		        }
		} */


		int bpp = surface->format->BytesPerPixel;
    		/* Here p is the address to the pixel we want to set */
		Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

		*(Uint32 *)p = _palette[color];
/*
		if ( SDL_MUSTLOCK(surface) ) {
			SDL_UnlockSurface(surface);
		} */
	};

	void LinuxSDLFullScreen32bpp::drawLine(int x0, int y0, int x1, int y1, int color) { DEBUG_FAIL_FUNC };  
	void LinuxSDLFullScreen32bpp::drawRect(Rect *rect, int color) { DEBUG_FAIL_FUNC };  
	void LinuxSDLFullScreen32bpp::drawRect(int x0, int y0, int width, int height, int color) { DEBUG_FAIL_FUNC };  
	void LinuxSDLFullScreen32bpp::drawCircle(int x, int y, int radius, int color) { DEBUG_FAIL_FUNC };  
	void LinuxSDLFullScreen32bpp::drawEllipse(int x, int y, int a, int b, int color) { DEBUG_FAIL_FUNC };  

	void LinuxSDLFullScreen32bpp::fillRect(Rect *rect, int color) { DEBUG_FAIL_FUNC };  
	void LinuxSDLFullScreen32bpp::fillRect(int x0, int y0, int width, int height, int color) { DEBUG_FAIL_FUNC };  
	void LinuxSDLFullScreen32bpp::fillCircle(int x, int y, int radius, int color) { DEBUG_FAIL_FUNC };  
	void LinuxSDLFullScreen32bpp::fillEllipse(int x, int y, int a, int b, int color) { DEBUG_FAIL_FUNC };  

	void LinuxSDLFullScreen32bpp::drawGfx(GfxElement *gfx, int code, int color, int x, int y, int attr) { DEBUG_FAIL_FUNC };  
	void LinuxSDLFullScreen32bpp::drawGfxClip(GfxElement *gfx, int code, int color, int x, int y, int attr) { DEBUG_FAIL_FUNC };  
	void LinuxSDLFullScreen32bpp::drawGfxTrans(GfxElement *gfx, int code, int color, int x, int y, int attr, int transData) { DEBUG_FAIL_FUNC };  
	void LinuxSDLFullScreen32bpp::drawGfxClipTrans(GfxElement *gfx, int code, int color, int x, int y, int attr, int transData) { DEBUG_FAIL_FUNC };  

	// access to custom plugin properties
	const std::string *LinuxSDLFullScreen32bpp::getProperties(int *num) const  { DEBUG_FAIL_FUNC }; 
	const int *LinuxSDLFullScreen32bpp::getPropertiesType() const   { DEBUG_FAIL_FUNC };
	void LinuxSDLFullScreen32bpp::setProperty(std::string prop, int data) { DEBUG_FAIL_FUNC };  
	void LinuxSDLFullScreen32bpp::setProperty(std::string prop, int index, int data) { DEBUG_FAIL_FUNC };  
	int LinuxSDLFullScreen32bpp::getProperty(std::string prop) const   { DEBUG_FAIL_FUNC };
	int LinuxSDLFullScreen32bpp::getProperty(std::string prop, int index) const   { DEBUG_FAIL_FUNC };

