#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_events.h>

#include <XChip/Media/SDLMedia/SdlRender.h>
#include <XChip/Utility/Log.h>
#include <XChip/Utility/ScopeExit.h>
#include <XChip/Utility/Assert.h>

#define _SDLRENDER_INITIALIZED_ASSERT_() ASSERT_MSG(_initialized == true, "SdlRender is not initialized")

namespace xchip {

extern SDL_Event g_sdlEvent;

SdlRender::SdlRender() noexcept
	: SdlSystem(System::Render)

{
	utility::LOG("Creating SdlRenderer object...");
}


SdlRender::~SdlRender()
{
	utility::LOG("Destroying SdlRenderer object...");
	if (_initialized)
		this->Dispose();
}





bool SdlRender::Initialize(const int width, const int height) noexcept
{
	if (_initialized)
		this->Dispose();

	else if (!this->InitSubSystem())
		return false;

	const auto scope = utility::make_scope_exit([this]() noexcept 
	{
		using namespace xchip::utility::literals;
		if (!this->_initialized) {
			utility::LOGerr("Couldn't initialize SdlRender. SDL ERROR MSG: "_s + SDL_GetError());
			this->Dispose();
		}
	});

	_pitch = width * 4;
	_window = SDL_CreateWindow("Chip8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                   width * 4, height * 6, SDL_WINDOW_RESIZABLE);

	if (!_window) 
		return false;

	_rend = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

	if (!_rend)
		return false;

	_texture = SDL_CreateTexture(_rend, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

	if (!_texture)
		return false;

	if(SDL_SetTextureColorMod(_texture, 255, 255, 255))
		return false;

	
	SDL_RenderClear(_rend);
	SDL_RenderPresent(_rend);

	_initialized = true;

	return true;
}


void SdlRender::Dispose() noexcept
{
	SDL_DestroyTexture(_texture);
	SDL_DestroyRenderer(_rend);
	SDL_DestroyWindow(_window);
	_window = nullptr;
	_buffer = nullptr;
	_closeClbk = nullptr;
	_resizeClbk = nullptr;
	_initialized = false;
}





bool SdlRender::UpdateEvents() noexcept
{
	_SDLRENDER_INITIALIZED_ASSERT_();
	PollEvent();	
	if (g_sdlEvent.type == SDL_WINDOWEVENT)
	{
		switch (g_sdlEvent.window.event)
		{
			case SDL_WINDOWEVENT_RESIZED: /* fall */
			case SDL_WINDOWEVENT_RESTORED: 
				if (_resizeClbk) 
					_resizeClbk(_resizeClbkArg);  
				
				break;
			
			case SDL_WINDOWEVENT_CLOSE: 
				if (_closeClbk) 
					_closeClbk(_closeClbkArg); 
				
				break;
		}

		return true;
	}

	
	return false;
}


bool SdlRender::SetColorFilter(const utility::Color& color) noexcept
{
	_SDLRENDER_INITIALIZED_ASSERT_();
	if(SDL_SetTextureColorMod(_texture, color.r, color.g, color.b))
	{
		xchip::utility::LOGerr(SDL_GetError());
		return false;
	}
	return true;
}





bool SdlRender::SetResolution(const utility::Resolution& res) noexcept
{

	_SDLRENDER_INITIALIZED_ASSERT_();
	
	if( res.w <= 0 || res.h <= 0 )
	{
		xchip::utility::LOGerr("SetResolution: w and h must be greater than 0.");
		return false;
	}


	SDL_DisplayMode displayMode;

	if( SDL_GetWindowDisplayMode(_window, &displayMode) )
	{
		xchip::utility::LOGerr(SDL_GetError());
		return false;
	}

	displayMode.w = res.w;
	displayMode.h = res.h;

	if( SDL_SetWindowDisplayMode(_window, &displayMode) )
	{
		xchip::utility::LOGerr(SDL_GetError());
		return false;
	}

	// Get the actual evaluated width and height:
	if( SDL_GetWindowDisplayMode(_window, &displayMode) )
	{
		xchip::utility::LOGerr(SDL_GetError());
		return false;
	}

	// set window size
	SDL_SetWindowSize(_window, displayMode.w, displayMode.h);

	return true;
}


bool SdlRender::SetFullScreen(const bool val) noexcept
{
	using namespace utility::literals;

	if(val)
	{
		if(SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN_DESKTOP))
		{
			utility::LOGerr("Error while setting SdlRender Fullscreen: "_s + SDL_GetError());
			return false;
		}
	}

	else
	{

		if(SDL_SetWindowFullscreen(_window, 0))
		{
			utility::LOGerr("Error while setting SdlRender Windowed: "_s + SDL_GetError());
			return false;
		}
	}


	return true;
}



void SdlRender::DrawBuffer() noexcept
{
	_SDLRENDER_INITIALIZED_ASSERT_();
	ASSERT_MSG(_buffer != nullptr, "attempt to draw null buffer");

	SDL_UpdateTexture(_texture, nullptr, _buffer, _pitch);
	SDL_RenderCopy(_rend, _texture, nullptr, nullptr);
	SDL_RenderPresent(_rend);
}


void SdlRender::HideWindow() noexcept
{
	_SDLRENDER_INITIALIZED_ASSERT_();
	SDL_HideWindow(_window);
}

void SdlRender::ShowWindow() noexcept
{
	_SDLRENDER_INITIALIZED_ASSERT_();
	SDL_ShowWindow(_window);
}



void SdlRender::SetWinCloseCallback(const void* arg, WinCloseCallback callback) noexcept
{
	_closeClbkArg = arg;
	_closeClbk = callback;
}




void SdlRender::SetWinResizeCallback(const void* arg, WinResizeCallback callback) noexcept 
{
	_resizeClbkArg = arg;
	_resizeClbk = callback;
}




utility::Color SdlRender::GetColorFilter() const noexcept
{
	_SDLRENDER_INITIALIZED_ASSERT_();
	utility::Color color;
	SDL_GetTextureColorMod(_texture, &color.r, &color.g, &color.b);
	return color;

}




utility::Resolution SdlRender::GetResolution() const noexcept
{
	_SDLRENDER_INITIALIZED_ASSERT_();
	
	SDL_DisplayMode displayMode;

	if( SDL_GetWindowDisplayMode(_window, &displayMode) )
	{
		xchip::utility::LOGerr(SDL_GetError());
		return {0, 0};
	}

	return {displayMode.w, displayMode.h};
}






}
