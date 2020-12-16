/*background.c*/

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <math.h>
#include "jeu.h"
#include "ia.h"
#include "background.h"


void init_toolbar(Toolbar* toolbar, SDL_Renderer *renderer){
	// TODO: enlever text et font de la struct, faire un tab de struct de { texture, posx, poy }
	toolbar->size.w = 300;
	toolbar->size.y = 0;
	//init font
	toolbar->font = TTF_OpenFont("akaashNormal.ttf", FONT_SIZE);
	if (!toolbar->font){
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", TTF_GetError());
	}
	SDL_Color color = { WHITE };
	if (ANTIALIASING){
		toolbar->text = TTF_RenderText_Blended(toolbar->font, "Hello World", color);
	} else {
		toolbar->text = TTF_RenderText_Solid(toolbar->font, "Hello World", color);
	}
	if (!toolbar->text){
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", TTF_GetError());
	}
	if (!(toolbar->texture = SDL_CreateTextureFromSurface(renderer, toolbar->text))){ // texture
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", TTF_GetError());
	}
	int tex_w;
	int tex_h;
	SDL_QueryTexture(toolbar->texture, NULL, NULL, &tex_w, &tex_h);
	toolbar->tex_size.y = 100;
	toolbar->tex_size.w = tex_w;
	toolbar->tex_size.h = tex_h;

	SDL_FreeSurface(toolbar->text); // deviendra une structure
	TTF_CloseFont(toolbar->font);
}

void render_toolbar(SDL_Renderer *renderer, Toolbar* toolbar){
	// choose color
	SDL_SetRenderDrawColor(renderer, ORANGE);
	// display menu
	SDL_RenderFillRect(renderer, &toolbar->size);
	// display text
	SDL_RenderCopy(renderer, toolbar->texture, NULL, &(toolbar->tex_size));

}

void free_font(Toolbar* toolbar){
	SDL_DestroyTexture(toolbar->texture);
	TTF_Quit();
}

