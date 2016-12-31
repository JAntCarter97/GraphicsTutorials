#include "BengineErrors.h"

#include <cstdlib>

#include <iostream>
#include <SDL/SDL.h>

namespace Bengine
{
//Simple Error Checking Function
void fatalError(std::string errorString)
{
	std::cout << errorString << std::endl;
	std::cout << "Enter any key to quit: ";
	char tmp;
	std::cin >> tmp;
	SDL_Quit();
	exit(69);
}
}