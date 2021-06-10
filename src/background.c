/*background.c*/

// TODO :

// si souris hors de l'écran, gérer le cas de la position. (garder l'ancienne)

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <math.h>
#include "../include/ia.h"
#include "../include/background.h"

typedef struct visible_sitting{
	char* name;
	int* int_variable;
	float* float_variable;
	float min;
	float max;
	Type_of_settings type;
}Visible_sitting;

static void init_setting(
		Setting settings[NB_SETTINGS],
		Visible_sitting sub_sittings[NB_SETTINGS],
		SDL_Renderer *renderer,
		TTF_Font* font,
		SDL_Color fg_color,
		SDL_Color bg_color
		){

	for (int num_var = 0; num_var < NB_SETTINGS; num_var++){
		if (sub_sittings[num_var].int_variable != NULL){
			settings[num_var].int_variable = sub_sittings[num_var].int_variable;
		} else if (sub_sittings[num_var].float_variable != NULL) {
			settings[num_var].float_variable = sub_sittings[num_var].float_variable;
		} else {
			printf("Error: no variable in the setting %d\n", num_var);
		}
		settings[num_var].type = sub_sittings[num_var].type;
		settings[num_var].min = sub_sittings[num_var].min;
		settings[num_var].max = sub_sittings[num_var].max;
		SDL_Surface* text = TTF_RenderText_Shaded(font, sub_sittings[num_var].name, fg_color, bg_color);
		if (!text){
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", TTF_GetError());
		}
		int tex_size_w;
		int tex_size_h;
		if (!(settings[num_var].texture = SDL_CreateTextureFromSurface(renderer, text))){ // settings[num_var].texture
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", TTF_GetError());
		}
		SDL_QueryTexture(settings[num_var].texture, NULL, NULL, &tex_size_w, &tex_size_h);
		settings[num_var].tex_size.y = 100 + 200 * num_var;
		settings[num_var].tex_size.w = tex_size_w;
		settings[num_var].tex_size.h = tex_size_h;
		SDL_FreeSurface(text);
	}
}

int init_toolbar(Toolbar* toolbar, SDL_Renderer *renderer, Entity* car, Road* road, Ia* ia){
	toolbar->size.w = 300;
	toolbar->size.y = 0;
	
	toolbar->select_var_int = NULL;
	toolbar->select_var_float = NULL;
	toolbar->is_selecting = False;
	
	//init font
	TTF_Font* font = TTF_OpenFont(FONT, FONT_SIZE);
	if (!font){
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", TTF_GetError());
		return EXIT_FAILURE;
	}
	SDL_Color fg_color = { WHITE };
	SDL_Color bg_color = { COLOR_TOOLBAR };
	
	// init struct Toolbar:
	Visible_sitting sub_sittings[NB_SETTINGS] = {
		{"IA :", (int*)&ia->active, NULL, 0, 1, Checkbox},
		{"road->size", &road->size, NULL, 0, 2000, Line},
		{"car->turn", NULL, &car->turn, 0.1, 30, Line},
		{"car->acceleration", NULL, &car->acceleration, 0.1, 30, Line}
	};

	init_setting(toolbar->settings, sub_sittings, renderer, font, fg_color, bg_color);

	TTF_CloseFont(font);
	return EXIT_SUCCESS;
}

void click_toolbar(Toolbar* toolbar, SDL_Event* event){
	toolbar->pos_click_x = event->button.x;
	// click on something ?
	int i;
	for (i = 0; i < NB_SETTINGS; i++){
		if (is_in(toolbar->pos_click_x, event->button.y , &(toolbar->settings[i].tex_size))){
			//put the carresponding variable in select_var
			toolbar->select_var_int = toolbar->settings[i].int_variable;
			toolbar->select_var_float = toolbar->settings[i].float_variable;
			toolbar->is_selecting = True;
			toolbar->num_setting = i;
			break;
		}
	}
}

Bool is_in(int x, int y, SDL_Rect* size){
	int x_mean;
	x_mean = size->x + size->w / 2;
	return x >= x_mean - SIZE_LINE_TOOLBAR / 2 - 10 && x <= x_mean + SIZE_LINE_TOOLBAR / 2 + 10 && y > size->y && y < size->y + 3 * size->h;
}

void change_variable(Toolbar* toolbar, SDL_Event* event){
	// Line
	if (toolbar->settings[toolbar->num_setting].type == Line){
		// int variable
		if (toolbar->select_var_int){
			*(toolbar->select_var_int) += (event->button.x - toolbar->pos_click_x) * (int)(toolbar->settings[toolbar->num_setting].max - toolbar->settings[toolbar->num_setting].min) / SIZE_LINE_TOOLBAR;
			if (*(toolbar->select_var_int) > (int)toolbar->settings[toolbar->num_setting].max){
				*(toolbar->select_var_int) = (int)toolbar->settings[toolbar->num_setting].max;
			} else if (*(toolbar->select_var_int) < (int)toolbar->settings[toolbar->num_setting].min){
				*(toolbar->select_var_int) = (int)toolbar->settings[toolbar->num_setting].min;
			} 
			// float variable
		} else if (toolbar->select_var_float) {
			*(toolbar->select_var_float) += (float)(event->button.x - toolbar->pos_click_x) * (toolbar->settings[toolbar->num_setting].max - toolbar->settings[toolbar->num_setting].min) / SIZE_LINE_TOOLBAR;	
			if (*(toolbar->select_var_float) > toolbar->settings[toolbar->num_setting].max){
				*(toolbar->select_var_float) = toolbar->settings[toolbar->num_setting].max;
			} else if (*(toolbar->select_var_float) < toolbar->settings[toolbar->num_setting].min){
				*(toolbar->select_var_float) = toolbar->settings[toolbar->num_setting].min;
			}
		} else {
			printf("Error: select_var NULL\n"); 
		}
		// TODO : vérifier que le clique est dans la fenêtre !
		toolbar->pos_click_x = event->button.x;
	}
}


void change_variable_keys(Toolbar* toolbar, short add){
	if (toolbar->select_var_int){
		*(toolbar->select_var_int) += add * (int)(toolbar->settings[toolbar->num_setting].max - toolbar->settings[toolbar->num_setting].min) / SIZE_LINE_TOOLBAR;
		if (*(toolbar->select_var_int) > (int)toolbar->settings[toolbar->num_setting].max){
			*(toolbar->select_var_int) = (int)toolbar->settings[toolbar->num_setting].max;
		}else if (*(toolbar->select_var_int) < (int)toolbar->settings[toolbar->num_setting].min){
			*(toolbar->select_var_int) = (int)toolbar->settings[toolbar->num_setting].min;
		}
	} else if (toolbar->select_var_float) {
		*(toolbar->select_var_float) += (float)add * (toolbar->settings[toolbar->num_setting].max - toolbar->settings[toolbar->num_setting].min) / SIZE_LINE_TOOLBAR;
		if (*(toolbar->select_var_float) > toolbar->settings[toolbar->num_setting].max){
			*(toolbar->select_var_float) = toolbar->settings[toolbar->num_setting].max;
		}else if (*(toolbar->select_var_float) < toolbar->settings[toolbar->num_setting].min){
			*(toolbar->select_var_float) = toolbar->settings[toolbar->num_setting].min;
		}
	} else {
		printf("Error: select_var NULL\n"); 
	}
}

void render_toolbar(SDL_Renderer *renderer, Toolbar* toolbar){
	// choose color
	SDL_SetRenderDrawColor(renderer, COLOR_TOOLBAR);
	// display menu
	SDL_RenderFillRect(renderer, &toolbar->size);
	// display text
	SDL_SetRenderDrawColor(renderer, WHITE);
	int x_mean_line;
	int y_line;
	int min;
	int max;
	SDL_Rect rect = {0, 0, 4, 10};
	int i;
	for(i = 0; i < NB_SETTINGS; i++){ //for each seatting
		toolbar->settings[i].tex_size.x = toolbar->size.x + 100;
		//text
		SDL_RenderCopy(renderer, toolbar->settings[i].texture, NULL, &(toolbar->settings[i].tex_size));
		//line
		if (toolbar->settings[i].type == Line){
			x_mean_line = toolbar->settings[i].tex_size.x + toolbar->settings[i].tex_size.w / 2;
			y_line = toolbar->settings[i].tex_size.y + 2 * toolbar->settings[i].tex_size.h;
			// draw the line
			SDL_RenderDrawLine(renderer, x_mean_line - SIZE_LINE_TOOLBAR / 2, y_line, x_mean_line + SIZE_LINE_TOOLBAR / 2, y_line);
			// cursor
			min = toolbar->settings[i].min;
			max = toolbar->settings[i].max;
			if (toolbar->settings[i].int_variable){
				rect.x = x_mean_line - SIZE_LINE_TOOLBAR / 2 + (*(toolbar->settings[i].int_variable) - min) * SIZE_LINE_TOOLBAR / (max - min);//calculer la pos du curseur
			} else if (toolbar->settings[i].float_variable) {
				rect.x = x_mean_line - SIZE_LINE_TOOLBAR / 2 + (int)(*(toolbar->settings[i].float_variable) - min) * SIZE_LINE_TOOLBAR / (max - min);//calculer la pos du curseur
			}
			rect.y = y_line - rect.h/2;
			SDL_RenderFillRect(renderer, &rect);
		} else if (toolbar->settings[i].type == Checkbox){
			rect.h = toolbar->settings[i].tex_size.h;
			rect.w = toolbar->settings[i].tex_size.h;
			rect.x = toolbar->settings[i].tex_size.x + toolbar->settings[i].tex_size.w + 10;
			rect.y = toolbar->settings[i].tex_size.y;
			if (toolbar->select_var_int == toolbar->settings[i].int_variable){
				SDL_SetRenderDrawColor(renderer, CP_SELECTED_COLOR); // checkboxe selected
				SDL_RenderFillRect(renderer, &rect);
			} else if (*toolbar->settings[i].int_variable == True){
				SDL_SetRenderDrawColor(renderer, NEXT_CP_COLOR); // green checkboxe
				SDL_RenderFillRect(renderer, &rect);
			} else if (*toolbar->settings[i].int_variable == False){
				SDL_SetRenderDrawColor(renderer, CP_TAKEN_COLOR); // empty checkboxe
				SDL_RenderDrawRect(renderer, &rect);
			}

			// Put back as before
				SDL_SetRenderDrawColor(renderer, WHITE);
			rect.h = 10;
			rect.w = 4;
		} else {
			printf("Error: bad toolbar->settings[%d].type. Must be Line or Checkbox. \n", i);
		}
	}
}