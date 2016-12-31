#include "Window.h"
#include "BengineErrors.h"

namespace Bengine
{
Window::Window()
{
}


Window::~Window()
{
}


int Window::create(std::string windowName, int screenWidth, int screenHeight, unsigned int currentFlags)
{
	Uint32 flags = SDL_WINDOW_OPENGL;

	if (currentFlags & INVISIBLE)
	{
		flags |= SDL_WINDOW_HIDDEN;
	}
	if (currentFlags & FULLSCREEN)
	{
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	if (currentFlags & BORDERLESS)
	{
		flags |= SDL_WINDOW_BORDERLESS;
	}

	//Initialize private window variable with the SDL create window function
	_sdlWindow = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, flags);

	//Checks if _window did not properly initialize and catches error with our error checking function
	if (_sdlWindow == nullptr)
	{
		fatalError("SDL Window could not be created!");
	}

	//Initializing OpenGL with an SDL context, then checks if context is properly initialized with our error function
	SDL_GLContext glContext = SDL_GL_CreateContext(_sdlWindow);
	if (glContext == nullptr)
	{
		fatalError("SDL_GL context could not be created!");
	}

	//Initializes GLEW and catches any errors
	//Uncomment this code if crashes occur: glewExperimental = true;
	GLenum error = glewInit();
	if (error != GLEW_OK)
	{
		fatalError("Could not initialize glew!");
	}

	//Check the OpenGL version
	std::printf("*** OpenGL Version: %s  ***\n", glGetString(GL_VERSION));

	//Set background color
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

	//Set VSYNC
	SDL_GL_SetSwapInterval(0);

	// Enable alpha blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return 0;
}

void Window::swapBuffer()
{
	SDL_GL_SwapWindow(_sdlWindow);
}
}