#include <Windows.h>
#include "SDL.h"

#pragma comment (lib, "sdl2.lib")
#pragma comment (lib, "sdl2main.lib")


/*
* main(argc, argv) - the standard C entry point for the program
*/
int main(int argc, char *argv[]) 
{
	// initialise out rendering context.
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
		return 0;

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_Window *sdl_window = SDL_CreateWindow("SDL2 + BGFX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);

	 // Main loop
    bool done = false;
    while (!done)
    {
		SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                done = true;
        }
        
        SDL_GL_SwapWindow(sdl_window);
    }


	// Cleanup
	SDL_DestroyWindow(sdl_window);

	SDL_Quit();

	return 0;
}

int CALLBACK WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    int         nCmdShow
    )
{
	return main(0, nullptr);
}
