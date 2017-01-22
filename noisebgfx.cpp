#include <Windows.h>
#include "SDL.h"
#include "SDL_syswm.h"

#include "bx/bx.h"
#include "bx/fpumath.h"
#include "bx/timer.h"
#include "bgfx/platform.h" // it must be included after SDL to enable SDL
// integration code path.
#include "bgfx/bgfx.h"

#pragma comment (lib, "sdl2.lib")
#pragma comment (lib, "sdl2main.lib")

struct PosColorVertex
{
	float m_x;
	float m_y;
	float m_z;
	uint32_t m_abgr;

	static void init()
	{
		ms_decl
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();
	};

	static bgfx::VertexDecl ms_decl;
};

bgfx::VertexDecl PosColorVertex::ms_decl;

static PosColorVertex s_cubeVertices[] =
{
	{-1.0f,  1.0f,  1.0f, 0xff000000 },
	{ 1.0f,  1.0f,  1.0f, 0xff0000ff },
	{-1.0f, -1.0f,  1.0f, 0xff00ff00 },
	{ 1.0f, -1.0f,  1.0f, 0xff00ffff },
	{-1.0f,  1.0f, -1.0f, 0xffff0000 },
	{ 1.0f,  1.0f, -1.0f, 0xffff00ff },
	{-1.0f, -1.0f, -1.0f, 0xffffff00 },
	{ 1.0f, -1.0f, -1.0f, 0xffffffff },
};

static const uint16_t s_cubeTriList[] =
{
	0, 1, 2, // 0
	1, 3, 2,
	4, 6, 5, // 2
	5, 6, 7,
	0, 2, 4, // 4
	4, 2, 6,
	1, 5, 3, // 6
	5, 7, 3,
	0, 4, 1, // 8
	4, 5, 1,
	2, 3, 6, // 10
	6, 3, 7,
};

static const uint16_t s_cubeTriStrip[] =
{
	0, 1, 2,
	3,
	7,
	1,
	5,
	0,
	4,
	2,
	6,
	7,
	4,
	5,
};


inline bool sdlSetWindow(SDL_Window* _window)
{
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if ( !SDL_GetWindowWMInfo(_window, &wmi) )
	{
		return false;
	}

	bgfx::PlatformData pd;
#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	pd.ndt = wmi.info.x11.display;
	pd.nwh = (void*)(uintptr_t)wmi.info.x11.window;
#	elif BX_PLATFORM_OSX
	pd.ndt = NULL;
	pd.nwh = wmi.info.cocoa.window;
#	elif BX_PLATFORM_WINDOWS
	pd.ndt = NULL;
	pd.nwh = wmi.info.win.window;
#	elif BX_PLATFORM_STEAMLINK
	pd.ndt = wmi.info.vivante.display;
	pd.nwh = wmi.info.vivante.window;
#	endif // BX_PLATFORM_
	pd.context = NULL;
	pd.backBuffer = NULL;
	pd.backBufferDS = NULL;
	bgfx::setPlatformData(pd);

	return true;
}


/*
* main(argc, argv) - the standard C entry point for the program
*/
int main(int argc, char *argv[])
{
	// initialise out rendering context.
	if ( SDL_Init(SDL_INIT_EVERYTHING) < 0 )
		return 0;

	// Setup window
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	//SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_Window *sdl_window = SDL_CreateWindow("SDL2 + BGFX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	sdlSetWindow(sdl_window);

	bgfx::init();

	BX_UNUSED(s_cubeTriList, s_cubeTriStrip);

	uint32_t m_width = 1280;
	uint32_t m_height = 720;
	uint32_t m_debug = BGFX_DEBUG_TEXT;
	uint32_t m_reset = BGFX_RESET_VSYNC;

	bgfx::reset(m_width, m_height, m_reset);

	// Enable debug text.
	bgfx::setDebug(m_debug);

	// Set view 0 clear state.
	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
		, 0x303030ff
		, 1.0f
		, 0
	);

	// Create vertex stream declaration.
	PosColorVertex::init();

	// Create static vertex buffer.
	bgfx::VertexBufferHandle m_vbh = bgfx::createVertexBuffer(
		// Static data can be passed with bgfx::makeRef
		bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices))
		, PosColorVertex::ms_decl
	);

	// Create static index buffer.
	bgfx::IndexBufferHandle m_ibh = bgfx::createIndexBuffer(
		// Static data can be passed with bgfx::makeRef
		bgfx::makeRef(s_cubeTriStrip, sizeof(s_cubeTriStrip))
	);

	// Create program from shaders.
	//bgfx::ProgramHandle m_program = loadProgram("vs_cubes", "fs_cubes");

	int64_t m_timeOffset = bx::getHPCounter();

	// Main loop
	bool done = false;
	while ( !done )
	{
		SDL_Event event;
		while ( SDL_PollEvent(&event) )
		{
			if ( event.type == SDL_QUIT )
				done = true;
		}

		int64_t now = bx::getHPCounter();
		static int64_t last = now;
		const int64_t frameTime = now - last;
		last = now;
		const double freq = double(bx::getHPFrequency());
		const double toMs = 1000.0 / freq;

		float time = (float)((now - m_timeOffset) / double(bx::getHPFrequency()));

		// Use debug font to print information about this example.
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 1, 0x4f, "bgfx/examples/01-cube");
		bgfx::dbgTextPrintf(0, 2, 0x6f, "Description: Rendering simple static mesh.");
		bgfx::dbgTextPrintf(0, 3, 0x0f, "Frame: % 7.3f[ms]", double(frameTime)*toMs);

		float at[3] = { 0.0f, 0.0f,   0.0f };
		float eye[3] = { 0.0f, 0.0f, -35.0f };

		// Set view and projection matrix for view 0.
		const bgfx::HMD* hmd = bgfx::getHMD();
		if ( NULL != hmd && 0 != (hmd->flags & BGFX_HMD_RENDERING) )
		{
			float view[16];
			bx::mtxQuatTranslationHMD(view, hmd->eye[0].rotation, eye);
			bgfx::setViewTransform(0, view, hmd->eye[0].projection, BGFX_VIEW_STEREO, hmd->eye[1].projection);

			// Set view 0 default viewport.
			//
			// Use HMD's width/height since HMD's internal frame buffer size
			// might be much larger than window size.
			bgfx::setViewRect(0, 0, 0, hmd->width, hmd->height);
		}
		else
		{
			float view[16];
			bx::mtxLookAt(view, eye, at);

			float proj[16];
			bx::mtxProj(proj, 60.0f, float(m_width) / float(m_height), 0.1f, 100.0f);
			bgfx::setViewTransform(0, view, proj);

			// Set view 0 default viewport.
			bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
		}

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::touch(0);

		// Submit 11x11 cubes.
		/*for ( uint32_t yy = 0; yy < 11; ++yy )
		{
			for ( uint32_t xx = 0; xx < 11; ++xx )
			{
				float mtx[16];
				bx::mtxRotateXY(mtx, time + xx*0.21f, time + yy*0.37f);
				mtx[12] = -15.0f + float(xx)*3.0f;
				mtx[13] = -15.0f + float(yy)*3.0f;
				mtx[14] = 0.0f;

				// Set model matrix for rendering.
				bgfx::setTransform(mtx);

				// Set vertex and index buffer.
				bgfx::setVertexBuffer(m_vbh);
				bgfx::setIndexBuffer(m_ibh);

				// Set render states.
				bgfx::setState(0
					| BGFX_STATE_DEFAULT
					| BGFX_STATE_PT_TRISTRIP
				);

				// Submit primitive for rendering to view 0.
				bgfx::submit(0, m_program);
			}
		}*/

		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		bgfx::frame();
	}


	// Cleanup
	bgfx::destroyIndexBuffer(m_ibh);
	bgfx::destroyVertexBuffer(m_vbh);
	//bgfx::destroyProgram(m_program);

	// Shutdown bgfx.
	bgfx::shutdown();
	SDL_DestroyWindow(sdl_window);

	SDL_Quit();

	return 0;
}

#ifdef WIN32 // because win32 sucks
int CALLBACK WinMain(
	HINSTANCE   hInstance,
	HINSTANCE   hPrevInstance,
	LPSTR       lpCmdLine,
	int         nCmdShow
)
{
	return main(0, nullptr);
}
#endif // WIN32
