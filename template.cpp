// Template, major revision 3, beta
// IGAD/NHTV - Jacco Bikker - 2006-2009

// Note:
// This version of the template attempts to setup a rendering surface in system RAM
// and copies it to VRAM using DMA. On recent systems, this yields extreme performance,
// and flips are almost instant. For older systems, there is a fall-back path that
// uses a more conventional approach offered by SDL. If your system uses this, the
// window caption will indicate this. In this case, you may want to tweak the video
// mode setup code for optimal performance.

#pragma warning (disable : 4530) // complaint about exception handler
#pragma warning (disable : 4273)
#pragma warning (disable : 4311) // pointer truncation from HANDLE to long

#include <windows.h>

extern "C" 
{ 
#include "glew.h" 
}
#include "gl.h"
#include "io.h"
#include <ios>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "SDL.h"
#include "SDL_syswm.h"
#include "wglext.h"

#include "../include/headers.h"
#include "fcntl.h"

int SCRWIDTH = -1;
int SCRHEIGHT = -1;

namespace Tmpl8 { 
void NotifyUser( char* s )
{
	HWND hApp = FindWindow( NULL, "Template" );
	MessageBox( hApp, s, "ERROR", MB_OK );
	exit( 0 );
}
}

using namespace Tmpl8;
using namespace std;

PFNGLGENBUFFERSPROC glGenBuffers = 0;
PFNGLBINDBUFFERPROC glBindBuffer = 0;
PFNGLBUFFERDATAPROC glBufferData = 0;
PFNGLMAPBUFFERPROC glMapBuffer = 0;
PFNGLUNMAPBUFFERPROC glUnmapBuffer = 0;
typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)(int);
PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;
unsigned int framebufferTexID[2];
GLuint fbPBO[2];
unsigned char* framedata = 0;

static int SCRPITCH = 0;
int ACTWIDTH, ACTHEIGHT;
static bool FULLSCREEN = false, firstframe = true;

Surface* surface = 0;
Game* game = 0;
double lastftime = 0;
LARGE_INTEGER lasttime, ticksPS;

bool createFBtexture()
{
	glGenTextures( 2, framebufferTexID );
	if (glGetError()) return false;
	for ( int i = 0; i < 2; i++ ) 
	{
		glBindTexture( GL_TEXTURE_2D, framebufferTexID[i] );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, SCRWIDTH, SCRHEIGHT, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL );
		glBindTexture(GL_TEXTURE_2D, 0);
		if (glGetError()) return false;
	}
	const int sizeMemory = 4 * SCRWIDTH * SCRHEIGHT;
	glGenBuffers( 2, fbPBO );
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, fbPBO[0] );	
	glBufferData( GL_PIXEL_UNPACK_BUFFER_ARB, sizeMemory, NULL, GL_STREAM_DRAW_ARB );
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, fbPBO[1] );	
	glBufferData( GL_PIXEL_UNPACK_BUFFER_ARB, sizeMemory, NULL, GL_STREAM_DRAW_ARB );
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, 0 );
	glBindTexture( GL_TEXTURE_2D, framebufferTexID[0] );
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, fbPBO[0] );
	framedata = (unsigned char*)glMapBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB );
	if (!framedata) return false;
	memset( framedata, 0, SCRWIDTH * SCRHEIGHT * 4 );
	return (glGetError() == 0);
}

bool init()
{
	fbPBO[0] = fbPBO[1] = -1;
	glGenBuffers =  (PFNGLGENBUFFERSPROC)wglGetProcAddress( "glGenBuffersARB" );
	glBindBuffer =  (PFNGLBINDBUFFERPROC)wglGetProcAddress( "glBindBufferARB" );
	glBufferData =  (PFNGLBUFFERDATAPROC)wglGetProcAddress( "glBufferDataARB" );
	glMapBuffer  =  (PFNGLMAPBUFFERPROC)wglGetProcAddress( "glMapBufferARB" );
	glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)wglGetProcAddress( "glUnmapBufferARB" );
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress( "wglSwapIntervalEXT" );
	if ((!glGenBuffers) || (!glBindBuffer) || (!glBufferData) || (!glMapBuffer) || (!glUnmapBuffer)) return false;
	if (glGetError()) return false;
	glViewport( 0, 0, SCRWIDTH, SCRHEIGHT );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
 	glOrtho( 0, 1, 0, 1, -1, 1 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glEnable( GL_TEXTURE_2D );
	glShadeModel( GL_SMOOTH );
	if (!createFBtexture()) return false;
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glClear( GL_COLOR_BUFFER_BIT );
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);
	if (wglSwapIntervalEXT) wglSwapIntervalEXT( 0 );
	QueryPerformanceFrequency( &ticksPS );
	surface = new Surface( SCRWIDTH, SCRHEIGHT, 0, SCRWIDTH );
	surface->InitCharset();
	return true;
}

void redirectIO()
{
	static const WORD MAX_CONSOLE_LINES = 500;
	int hConHandle;
	long lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;
	AllocConsole();
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
	&coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),
	coninfo.dwSize);
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );
	ios::sync_with_stdio();
}

void swap()
{
	static int index = 0;
	int nextindex;
	glUnmapBuffer( GL_PIXEL_UNPACK_BUFFER_ARB );
	glBindTexture( GL_TEXTURE_2D, framebufferTexID[index] );
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, fbPBO[index] );
	glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, SCRWIDTH, SCRHEIGHT, GL_BGRA, GL_UNSIGNED_BYTE, 0 ); 
    nextindex = (index + 1) % 2;
	index = (index + 1) % 2;
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, fbPBO[nextindex] );	
	framedata = (unsigned char*)glMapBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB );
    glColor3f( 1.0f, 1.0f, 1.0f );
    glBegin( GL_QUADS );
	glNormal3f( 0, 0, 1 );
	glTexCoord2f( 0.0f, 0.0f );
	glVertex2f  ( 0.0f, 1.0f );
	glTexCoord2f( 1.0f, 0.0f );
	glVertex2f  ( 1.0f, 1.0f );
	glTexCoord2f( 1.0f, 1.0f );
	glVertex2f  ( 1.0f, 0.0f );
	glTexCoord2f( 0.0f, 1.0f );
	glVertex2f  ( 0.0f, 0.0f );
    glEnd();
    glBindTexture( GL_TEXTURE_2D, 0 );	
   	SDL_GL_SwapBuffers(); 
}

int main( int argc, char **argv ) 
{  
	redirectIO();
	SDL_Init( SDL_INIT_VIDEO );
	int scr_width = GetSystemMetrics(SM_CXSCREEN);
	int scr_height = GetSystemMetrics(SM_CYSCREEN);

	if(scr_width < 1024 || scr_height < 768) {
		MessageBox(NULL, "You need a resolution of at least 1024x768 to run this game.", "GameOfLife v1.0", MB_OK);
		exit(0);
	}

	if (MessageBox(NULL, "Run in fullscreen?", "GameOfLife v1.0", MB_YESNO) == IDYES) {
		SCRWIDTH = scr_width;
		SCRHEIGHT = scr_height;
		SDL_SetVideoMode(SCRWIDTH, SCRHEIGHT, 32, SDL_OPENGL | SDL_FULLSCREEN);
	} else {
		SCRHEIGHT = scr_height-75-(scr_height-75)%32;
		SCRWIDTH = (scr_height/3)*4;
		SDL_SetVideoMode(SCRWIDTH, SCRHEIGHT, 32, SDL_OPENGL);
	}

	SDL_EnableKeyRepeat( 0, 0 );
	bool vbo = true;
	if (!init()) 
	{
		SDL_SetVideoMode( SCRWIDTH, SCRHEIGHT, 32, SDL_HWSURFACE|SDL_DOUBLEBUF );
		SDL_Surface* s = SDL_GetVideoSurface();
		SCRHEIGHT = scr_height-75-(scr_height-75)%32;
		SCRWIDTH = (scr_height/3)*4;
		surface = new Surface( SCRWIDTH, SCRHEIGHT, (Pixel*)s->pixels, s->pitch );
		surface->InitCharset();
		vbo = false;
		SDL_WM_SetCaption( "GameOfLife v1.0 by Alexander Dzhoganov", NULL );
	}
	else SDL_WM_SetCaption( "GameOfLife v1.0 by Alexander Dzhoganov", NULL );
	SDL_EnableUNICODE(true);
	int exitapp = 0;
	game = new Game();
	game->SetTarget( surface );
	while (!exitapp) 
	{
		if (vbo) // frame buffer swapping for VBO mode
		{
			swap();
			surface->SetBuffer( (Pixel*)framedata );
		}
		else	 // frame buffer swapping for fall-back path
		{
			SDL_Surface* s = SDL_GetVideoSurface();
			SDL_UpdateRect( s, 0, 0, 0, 0 );
			framedata = (unsigned char*)s->pixels;
			surface->SetPitch( s->pitch / 4 );
			surface->SetBuffer( (Pixel*)framedata );
		}
		if (firstframe)
		{
			game->Init();
			firstframe = false;
		}
		// calculate frame time and pass it to game->Tick
		LARGE_INTEGER start, end;
		QueryPerformanceCounter( &start );
		game->Tick( (float)lastftime );
		QueryPerformanceCounter( &end );
		lastftime = float( end.QuadPart - start.QuadPart ) / float( ticksPS.QuadPart / 1000 );
		// event loop
		SDL_Event event;
		while (SDL_PollEvent( &event )) 
		{
			switch (event.type)
			{
			case SDL_QUIT:
				exitapp = 1;
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) 
				{
					exitapp = 1;
					// find other keys here: http://sdl.beuc.net/sdl.wiki/SDLKey
				}
				game->KeyDown( event.key.keysym.unicode, event.key.keysym.scancode );
				break;
			case SDL_KEYUP:
				game->KeyUp( event.key.keysym.unicode, event.key.keysym.scancode );
				break;
			case SDL_MOUSEMOTION:
				game->MouseMove( event.motion.x, event.motion.y );
				break;
			case SDL_MOUSEBUTTONUP:
				game->MouseUp( event.button.button );
				break;
			case SDL_MOUSEBUTTONDOWN:
				game->MouseDown( event.button.button );
				break;
			default:
				// more info on events in SDL: http://sdl.beuc.net/sdl.wiki/SDL_Event
				break;
			}
		}
	}
	SDL_Quit();
	return 1;
}