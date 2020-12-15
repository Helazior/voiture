/*background.c*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include "jeu.h"
#include "ia.h"
#include "background.h"

void init_toolbar(Toolbar* toolbar){
	toolbar->size.w = 300;
	toolbar->size.y = 0;
}

void render_toolbar(SDL_Renderer *renderer, Toolbar* toolbar){
	SDL_SetRenderDrawColor(renderer, ORANGE); // normal checkpoint
	SDL_RenderFillRect(renderer, &toolbar->size);
}

