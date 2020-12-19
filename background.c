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


void init_toolbar(Toolbar* toolbar, SDL_Renderer *renderer, Camera* cam, Road* road){
	// TODO: enlever text et font de la struct, faire un tab de struct de { texture, posx, poy }
	toolbar->size.w = 300;
	toolbar->size.y = 0;
	
	
	//init font
	toolbar->font = TTF_OpenFont("akaashNormal.ttf", FONT_SIZE);
	if (!toolbar->font){
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", TTF_GetError());
	}
	SDL_Color fg_color = { WHITE };
	SDL_Color bg_color = { COLOR_TOOLBAR };
	
	// init struct Toolbar:
	int tex_w;
	int tex_h;
	
	int i;
	for(i = 0; i < NB_SETTINGS; i++){
		toolbar->settings[i].variable = &(road->size);
		toolbar->settings[i].type = Line;
		toolbar->settings[i].min = 0;
		toolbar->settings[i].max = 400;

		toolbar->settings[i].text = TTF_RenderText_Shaded(toolbar->font, "Hello World", fg_color, bg_color);
		if (!toolbar->settings[i].text){
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", TTF_GetError());
		}
		if (!(toolbar->settings[i].texture = SDL_CreateTextureFromSurface(renderer, toolbar->settings[i].text))){ // texture
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", TTF_GetError());
		}
		SDL_QueryTexture(toolbar->settings[i].texture, NULL, NULL, &tex_w, &tex_h);
		toolbar->settings[i].tex_size.y = 100 + 200 * i;
		toolbar->settings[i].tex_size.w = tex_w;
		toolbar->settings[i].tex_size.h = tex_h;
		SDL_FreeSurface(toolbar->settings[i].text); // deviendra une structure
	}

	TTF_CloseFont(toolbar->font);
}

void click_toolbar(Toolbar* toolbar, SDL_Event* event){
	// TODO : Check if cliqué sur quelque chose
	toolbar->pos_click_x = event->button.x;
	toolbar->pos_click_y = event->button.y;
	// TODO : mettre la variable correspondante dans select_var
	printf("%d | %d\n", toolbar->pos_click_x, toolbar->pos_click_y);	
}

void render_toolbar(SDL_Renderer *renderer, Toolbar* toolbar){
	// choose color
	SDL_SetRenderDrawColor(renderer, COLOR_TOOLBAR);
	// display menu
	SDL_RenderFillRect(renderer, &toolbar->size);
	// display text
	int i;
	for(i = 0; i < NB_SETTINGS; i++){
		toolbar->settings[i].tex_size.x = toolbar->size.x + 50;
		SDL_RenderCopy(renderer, toolbar->settings[i].texture, NULL, &(toolbar->settings[i].tex_size));
	}
}

void free_font(Toolbar* toolbar){
	int i;
	for(i = 0; i < NB_SETTINGS; i++){
		SDL_DestroyTexture(toolbar->settings[i].texture);
	}
	TTF_Quit();
}

