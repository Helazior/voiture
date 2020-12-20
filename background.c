/*background.c*/

// TODO :
// mettre les barres sous le text
// mettre un curseur
// faire marcher "-" et "+" dans jeu.c
// faire une fonction pour créé les différents texts
// faire fonctionner les variables float
//

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
	
	toolbar->select_var = NULL;
	toolbar->is_selecting = False;
	
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
		toolbar->settings[i].max = 2000;

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
	toolbar->pos_click_x = event->button.x;
	// click on something ?
	int i;
	for (i = 0; i < NB_SETTINGS; i++){
		if (is_in(toolbar->pos_click_x, event->button.y , &(toolbar->settings[i].tex_size))){
			//put the carresponding variable in select_var
			toolbar->select_var = toolbar->settings[i].variable;
			toolbar->is_selecting = True;
			toolbar->num_setting = i;
			break;
		}
	}
}

Bool is_in(int x, int y, SDL_Rect* size){
	return x > size->x && x < size->x + size->w && y > size->y && y < size->y + size->h + 50; // + 50 for the line
}

void change_variable(Toolbar* toolbar, SDL_Event* event){
	*(toolbar->select_var) += (event->button.x - toolbar->pos_click_x) * (toolbar->settings[toolbar->num_setting].max - toolbar->settings[toolbar->num_setting].min) / SIZE_LINE_TOOLBAR;
	if (*(toolbar->select_var) > toolbar->settings[toolbar->num_setting].max){
		//TODO : Retenir la position pour ne pas décaler tant qu'on n'y revient pas
		*(toolbar->select_var) = toolbar->settings[toolbar->num_setting].max;
	}else if (*(toolbar->select_var) < toolbar->settings[toolbar->num_setting].min){
			*(toolbar->select_var) = toolbar->settings[toolbar->num_setting].min;
	}
	toolbar->pos_click_x = event->button.x;
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

