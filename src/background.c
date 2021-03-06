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

static int init_setting(
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
			return EXIT_FAILURE;
		}
		int tex_size_w;
		int tex_size_h;
		if (!(settings[num_var].texture = SDL_CreateTextureFromSurface(renderer, text))){ // settings[num_var].texture
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", TTF_GetError());
			return EXIT_FAILURE;
		}
		SDL_QueryTexture(settings[num_var].texture, NULL, NULL, &tex_size_w, &tex_size_h);
		settings[num_var].tex_size.y = 100 + 100 * num_var;
		settings[num_var].tex_size.w = tex_size_w;
		settings[num_var].tex_size.h = tex_size_h;
		SDL_FreeSurface(text);
	}
	return EXIT_SUCCESS;
}

int init_toolbar(Toolbar* toolbar, SDL_Renderer *renderer, Entity* car, Road* road, Ia* ia, Camera* cam){
	toolbar->size.w = WIDTH_TOOLBAR;
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
		{"IA drift:", (int*)&ia->drift, NULL, 0, 1, Checkbox},
		{"IA show simu traj :", (int*)&ia->show_simu_traj, NULL, 0, 1, Checkbox},
		{"cam follow car :", (int*)&cam->follow_car, NULL, 0, 1, Checkbox},
		{"road->size", &road->size, NULL, 0, 2000, Line},
		{"car->turn", NULL, &car->turn, 0.1, 30, Line},
		{"car->acceleration", NULL, &car->acceleration, 0.1, 30, Line}
	};

	if (init_setting(toolbar->settings, sub_sittings, renderer, font, fg_color, bg_color) == EXIT_FAILURE){
		return EXIT_FAILURE;
	}

	TTF_CloseFont(font);
	return EXIT_SUCCESS;
}

//check if the user click in a box of setting
static Bool is_in(int x, int y, SDL_Rect* size){
	int x_mean;
	x_mean = size->x + size->w / 2;
	return x >= x_mean - SIZE_LINE_TOOLBAR / 2 - 10 && x <= x_mean + SIZE_LINE_TOOLBAR / 2 + 10 && y > size->y && y < size->y + 3 * size->h;
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

void move_screen(Camera* cam, Toolbar* toolbar){
	// TODO à reprendre
	SDL_GetMouseState(&cam->cursor_x, &cam->cursor_y);
	if (cam->cursor_x < 20){
		cam->x -= (180. / FRAMES_PER_SECONDE);
	} else if (cam->cursor_x > cam->winSize_w + toolbar->size.w - 20){
		cam->x += (180. / FRAMES_PER_SECONDE);
	}
	if (cam->cursor_y < 20){
		cam->y -= (180. / FRAMES_PER_SECONDE);
	} else if (cam->cursor_y > cam->winSize_h - 20){
		cam->y += (180. / FRAMES_PER_SECONDE);
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

void render_keys(SDL_Renderer *renderer, Keys_pressed* key, Camera* cam){
	// 3/4 of the screen
	SDL_Rect key_square = {
		.x = 3 * cam->winSize_w / 4,
		.y = cam->winSize_h - 200,
		.w = 75,
		.h = 75
	};
	// left
	// dift
	if (key->drift == drift_left){
		SDL_SetRenderDrawColor(renderer, CP_SELECTED_COLOR);
		SDL_RenderFillRect(renderer, &key_square);
	// active
	} else if (key->left == True) {
		SDL_SetRenderDrawColor(renderer, CP_TAKEN_COLOR);
		SDL_RenderFillRect(renderer, &key_square);
	// unactive	
	} else {
		SDL_SetRenderDrawColor(renderer, CP_TAKEN_COLOR);
		SDL_RenderDrawRect(renderer, &key_square);
	}
	
	// down
	key_square.x += key_square.w + 2;
	// active
	if (key->down == True) {
		SDL_SetRenderDrawColor(renderer, CP_TAKEN_COLOR);
		SDL_RenderFillRect(renderer, &key_square);
	// unactive	
	} else {
		SDL_SetRenderDrawColor(renderer, CP_TAKEN_COLOR);
		SDL_RenderDrawRect(renderer, &key_square);
	}

	// up
	key_square.y -= key_square.h + 2;
	// active
	if (key->up == True) {
		SDL_SetRenderDrawColor(renderer, CP_TAKEN_COLOR);
		SDL_RenderFillRect(renderer, &key_square);
	// unactive	
	} else {
		SDL_SetRenderDrawColor(renderer, CP_TAKEN_COLOR);
		SDL_RenderDrawRect(renderer, &key_square);
	}


	// right
	key_square.y += key_square.h + 2;
	key_square.x += key_square.w + 2;
	// dift
	if (key->drift == drift_right){
		SDL_SetRenderDrawColor(renderer, CP_SELECTED_COLOR);
		SDL_RenderFillRect(renderer, &key_square);
	// active
	} else if (key->right == True) {
		SDL_SetRenderDrawColor(renderer, CP_TAKEN_COLOR);
		SDL_RenderFillRect(renderer, &key_square);
	// unactive	
	} else {
		SDL_SetRenderDrawColor(renderer, NEXT_CP_COLOR);
		SDL_RenderDrawRect(renderer, &key_square);
	}

}

void destroy_texture(Background* bg){
	for (int i = 0; i < 3; i++){
		if(NULL != bg->texture[i])
			SDL_DestroyTexture(bg->texture[i]);

	} 
}

#define NB_PT_X 150
#define NB_PT_Y 220
int init_background(SDL_Renderer* renderer, Background* bg){
	/*bg->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, NB_PT_X, NB_PT_Y);*/
	/*SDL_Rect rect = {0, 0, SIZE_PT_X, SIZE_PT_Y};*/
	for (int i = 0; i < 3; i++){
		bg->texture[i] = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, NB_PT_X, NB_PT_Y);

		if (bg->texture[i] == NULL){
			fprintf(stderr, "Erreur SDL_CreateTexture : %s\n", SDL_GetError());
			return EXIT_FAILURE;
		}
	}
	SDL_SetRenderTarget(renderer, bg->texture[0]);
	SDL_SetRenderDrawColor(renderer, CP_START_COLOR);
	SDL_RenderClear(renderer);
	SDL_SetRenderTarget(renderer, bg->texture[1]);
	SDL_SetRenderDrawColor(renderer, CP_SELECTED_COLOR);
	SDL_RenderClear(renderer);
	SDL_SetRenderTarget(renderer, bg->texture[2]);
	SDL_SetRenderDrawColor(renderer, NEXT_CP_COLOR);
	SDL_RenderClear(renderer);

	SDL_SetRenderTarget(renderer, NULL);
	return EXIT_SUCCESS;
}

/*SDL_MapRGBA*/
void fill_background(SDL_Renderer* renderer, Background* bg, Road* road, Camera* cam){
#if 1
	int size_pt_x = cam->winSize_w / NB_PT_X;
	int size_pt_y = cam->winSize_h / NB_PT_Y;
	// draw on the texture
	SDL_Rect dst = {0, 0, size_pt_x, size_pt_y};
	/*SDL_Rect scr = {0, 0, 1000, 1000};*/
	float coll_grid_x = 0.;
	float coll_grid_y = 0.;
	int is_in_road = 0;
	for (int i = 0; i < NB_PT_X; i++){
		for (int j = 0; j < NB_PT_Y; j++){
			// mettre un table de SDL_Rect !
			// avec SDL_RenderDrawRects
			/*SDL_RenderDrawPoint(renderer, i, j);*/
			dst.x = size_pt_x * i;
			dst.y = size_pt_y * j;
			/*printf("dst.y / cam->winSize_h = %d, %d \n", NB_GRID_ROW * dst.y / cam->winSize_h, NB_GRID_COLUMN * dst.x / cam->winSize_w);*/
			/*printf("road->collision_grid[NB_GRID_ROW * dst.y / cam->winSize_h][NB_GRID_COLUMN * dst.x / cam->winSize_w].x = %d \n",*/
					/*(int)road->collision_grid[NB_GRID_ROW * dst.y / cam->winSize_h][NB_GRID_COLUMN * dst.x / cam->winSize_w].x);*/
			coll_grid_x = road->collision_grid[NB_GRID_ROW * dst.y / cam->winSize_h][NB_GRID_COLUMN * dst.x / cam->winSize_w].x;
			coll_grid_y = road->collision_grid[NB_GRID_ROW * dst.y / cam->winSize_h][NB_GRID_COLUMN * dst.x / cam->winSize_w].y;

			is_in_road = coll_grid_x != 0.;
			/*if (is_in_road > 2 || is_in_road < 0){*/
				/*printf("%d\n",is_in_road);*/
				/*continue;*/
			/*}*/
			if (is_in_road){
				/*printf("dst.x, dst.y, coll_grid_x, coll_grid_y = %d, %d, %f, %f \n",dst.x, dst.y, coll_grid_x, coll_grid_y ); */
				/*printf("dist = %f > %f \n", distance((float)dst.x, (float)dst.y, coll_grid_x, coll_grid_y), (float)road->size / 2.); */
			}
			if (is_in_road && distance((float)dst.x, (float)dst.y, coll_grid_x, coll_grid_y) > 90.){
				SDL_RenderCopy(renderer, bg->texture[2], NULL, &dst);
				continue;
			}
			if (is_in_road == 0){	
				SDL_RenderCopy(renderer, bg->texture[2 - 2 * is_in_road], NULL, &dst);
			}
		}
	}
/*SDL_QueryTexture(bg->texture, NULL, NULL, &dst.w, &dst.h);*/
#endif
}
