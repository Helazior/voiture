/*background.c*/

// TODO :

// Si souris hors de l'écran, gérer le cas de la position. (garder l'ancienne)

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "../include/ia.h"
#include "../include/background.h"
#include "../include/create_map.h"

typedef struct visible_sitting{
	char* name;
	int* int_variable;
	float* float_variable;
	float min;
	float max;
	Type_of_settings type;
}Visible_setting;

static int init_setting(
        Setting settings[][NB_SETTINGS],
        Visible_setting sub_settings[NB_PAGES][NB_SETTINGS],
        SDL_Renderer *renderer,
        TTF_Font* font,
        SDL_Color fg_color,
        SDL_Color bg_color,
        int toolbar_y
		){

    int toolbar_y_numepage;
    for (int num_page = 0; num_page < NB_PAGES; ++num_page) {
        toolbar_y_numepage = toolbar_y;
        for (int num_var = 0; num_var < NB_SETTINGS; num_var++){
            settings[num_page][num_var].int_variable = sub_settings[num_page][num_var].int_variable;
            settings[num_page][num_var].float_variable = sub_settings[num_page][num_var].float_variable;
            settings[num_page][num_var].type = sub_settings[num_page][num_var].type;
            settings[num_page][num_var].min = sub_settings[num_page][num_var].min;
            settings[num_page][num_var].max = sub_settings[num_page][num_var].max;
            SDL_Surface* text = TTF_RenderText_Shaded(font, sub_settings[num_page][num_var].name, fg_color, bg_color);
            if (!text){
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", TTF_GetError());
                return EXIT_FAILURE;
            }
            int tex_size_w;
            int tex_size_h;
            if (!(settings[num_page][num_var].texture = SDL_CreateTextureFromSurface(renderer, text))){ // settings[num_var].texture
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", TTF_GetError());
                return EXIT_FAILURE;
            }
            SDL_QueryTexture(settings[num_page][num_var].texture, NULL, NULL, &tex_size_w, &tex_size_h);
            settings[num_page][num_var].tex_size.y = toolbar_y + 20 + toolbar_y_numepage;
            toolbar_y_numepage += settings[num_page][num_var].type == Line ? 70 : 30;
            settings[num_page][num_var].tex_size.w = tex_size_w;
            settings[num_page][num_var].tex_size.h = tex_size_h;
            SDL_FreeSurface(text);
        }
    }
	return EXIT_SUCCESS;
}

int init_toolbar(Toolbar* toolbar, SDL_Renderer *renderer, Entity* car, Road* road, Ia* ia, Camera* cam, Background* bg, Callback* callback){
	toolbar->size.w = WIDTH_TOOLBAR;
	toolbar->size.y = 0;
    toolbar->top_h = 30;
    toolbar->pos_click_x = 0;

	toolbar->select_var_int = NULL;
	toolbar->select_var_float = NULL;
	toolbar->is_selecting = False;

    toolbar->num_page = 0;
    toolbar->num_setting = 0; // by default "-" and "+" change the first var

    //init font
    TTF_Font* font = TTF_OpenFont(FONT, FONT_SIZE);
    if (!font){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", TTF_GetError());
        return EXIT_FAILURE;
    }
    SDL_Color fg_color = { WHITE };
    SDL_Color bg_color = { COLOR_TOOLBAR };
    Visible_setting null_setting = {" ", NULL, NULL, 0, 0, Empty};
    Visible_setting sub_settings[][NB_SETTINGS] = {
            {
                    {"IA:", (int*)&ia->active, NULL, 0, 1, Checkbox},
                    {"IA drift:", (int*)&ia->drift, NULL, 0, 1, Checkbox},
                    {"IA show simu traj:", (int*)&ia->show_simu_traj, NULL, 0, 1, Checkbox},
                    {"Cam follow car:", (int*)&cam->follow_car, NULL, 0, 1, Checkbox},
                    {"Show the background:", (int*)&bg->show, NULL, 0, 1, Checkbox},
                    {"road->size", &road->size, NULL, 0, 2000, Line},
                    {"car->turn", NULL, &car->turn, 0.1f, 30, Line},
                    {"car->acceleration", NULL, &car->acceleration, 0.1f, 30, Line},
                    {"cam->zoom", NULL, &cam->zoom, 0.02f, 2, Line},
                    null_setting,
                    null_setting,
                    null_setting,
                    null_setting,
                    null_setting,
                    null_setting,
            },
            {
                    {"Generate Map:", (int *) &callback->create_road, NULL, 0, 1, Button},
                    {"Generate Continuously:", (int *) &road->generation.generate_continuously, NULL, 0, 1, Checkbox},
                    {"Nb max of CP:", (int *) &road->generation.nb_cp_max, NULL, 4, NB_MAX_SQUARES, Line},
                    {"CP's size angle to remove", NULL, &road->generation.cp_size_angle_to_remove, 0.f, 30.f, Line},
                    {"Dist between CP:", (int *) &road->generation.dist_cp, NULL, 200, 2000, Line},
                    {"Nb loops to uncross segments:", (int *) &road->generation.nb_loops, NULL, 0, 100, Line},
                    {"Algo set up CP:", (int *) &road->generation.set_up_cp, NULL, 0, 1, Checkbox},
                    {"Algo greedy:", (int *) &road->generation.greedy, NULL, 0, 1, Checkbox},
                    {"Algo uncross segments:", (int *) &road->generation.uncross_and_remove, NULL, 0, 1, Checkbox},
                    {"Cam follow car:", (int *) &cam->follow_car, NULL, 0, 1, Checkbox},
                    null_setting,
                    null_setting,
                    null_setting,
                    null_setting,
                    null_setting,
            }
    };

    // init struct Toolbar:
	if (init_setting(toolbar->settings, sub_settings, renderer, font, fg_color, bg_color, toolbar->top_h) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

	TTF_CloseFont(font);
	return EXIT_SUCCESS;
}

//check if the user click in a box of setting
static Bool is_in(int x, int y, SDL_Rect* tex_size, SDL_Rect* toolbar_size, Type_of_settings type) {
    // TODO: revoir
//	return x >= x_mean - SIZE_LINE_TOOLBAR / 2 - 10 && x <= x_mean + SIZE_LINE_TOOLBAR / 2 + 10 && y > size->y - 5 && y < size->y + 2 * size->h;
    if (type == Line)
        return x >= toolbar_size->x && x <= toolbar_size->x + toolbar_size->w && y > tex_size->y - 5 && y < tex_size->y + 2 * tex_size->h + 10;
    if (type == Button || type == Checkbox)
        return x >= toolbar_size->x && x <= toolbar_size->x + toolbar_size->w && y > tex_size->y - 5 && y < tex_size->y + tex_size->h + 5;
    return False;
}

void click_toolbar(Toolbar* toolbar) {
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
	toolbar->pos_click_x = mouse_x;
    // on top to change the page
    if (mouse_y <= toolbar->top_h) {
        // TODO: faire un truc générique
        // TODO: initialiser tout, par exemple les var sélectionnées doivent être désélectionnées.
        toolbar->num_page = (mouse_x > toolbar->size.x + toolbar->size.w / 2);
        return;
    }
	// click on something ?
	int i;
	for (i = 0; i < NB_SETTINGS; i++){
        //TODO C'est pas la bonne hitbox
        if (is_in(toolbar->pos_click_x, mouse_y, &toolbar->settings[toolbar->num_page][i].tex_size, &toolbar->size, toolbar->settings[toolbar->num_page][i].type)){
			//put the corresponding variable in select_var
			toolbar->select_var_int = toolbar->settings[toolbar->num_page][i].int_variable;
			toolbar->select_var_float = toolbar->settings[toolbar->num_page][i].float_variable;
			toolbar->is_selecting = True;
			toolbar->num_setting = i;
			break;
		}
	}
}

/** @return If one of the parameter of the road has changed, the map generate continuously */
Bool has_road_var_changed(Road* road) {
    return (road->generation.has_changed.cp_size_angle_to_remove
            || road->generation.has_changed.nb_loops_uncross_segments
            || road->generation.has_changed.dist_cp
            || road->generation.has_changed.nb_cp_max)
           && road->generation.generate_continuously;
//           && (road->generation.greedy
//               || road->generation.uncross_and_remove
//               || road->generation.remove_hairpin_turns);
}

static void road_has_changed_int(Road* road, int* selected_var_int) {
    if (selected_var_int == &road->generation.nb_cp_max)
        road->generation.has_changed.nb_cp_max = True;
    else if (selected_var_int == &road->generation.dist_cp)
        road->generation.has_changed.dist_cp = True;
    else if (selected_var_int == &road->generation.nb_loops)
        road->generation.has_changed.nb_loops_uncross_segments = True;
}

static void road_has_changed_float(Road* road, float* selected_var_float) {
    if (selected_var_float == &road->generation.cp_size_angle_to_remove)
        road->generation.has_changed.cp_size_angle_to_remove = True;
}

void change_variable(Toolbar* toolbar, Road* road){
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
	// Line
	if (toolbar->settings[toolbar->num_page][toolbar->num_setting].type == Line){
		// int variable
		if (toolbar->select_var_int){
			*(toolbar->select_var_int) += (mouse_x - toolbar->pos_click_x) * (int)(toolbar->settings[toolbar->num_page][toolbar->num_setting].max - toolbar->settings[toolbar->num_page][toolbar->num_setting].min) / SIZE_LINE_TOOLBAR;
			if (*(toolbar->select_var_int) > (int)toolbar->settings[toolbar->num_page][toolbar->num_setting].max){
				*(toolbar->select_var_int) = (int)toolbar->settings[toolbar->num_page][toolbar->num_setting].max;
			} else if (*(toolbar->select_var_int) < (int)toolbar->settings[toolbar->num_page][toolbar->num_setting].min){
				*(toolbar->select_var_int) = (int)toolbar->settings[toolbar->num_page][toolbar->num_setting].min;
			} else {
                road_has_changed_int(road, toolbar->select_var_int);
            }
			// float variable
		} else if (toolbar->select_var_float) {
			*(toolbar->select_var_float) += (float)(mouse_x - toolbar->pos_click_x) * (toolbar->settings[toolbar->num_page][toolbar->num_setting].max - toolbar->settings[toolbar->num_page][toolbar->num_setting].min) / SIZE_LINE_TOOLBAR;
			if (*(toolbar->select_var_float) > toolbar->settings[toolbar->num_page][toolbar->num_setting].max){
				*(toolbar->select_var_float) = toolbar->settings[toolbar->num_page][toolbar->num_setting].max;
			} else if (*(toolbar->select_var_float) < toolbar->settings[toolbar->num_page][toolbar->num_setting].min){
				*(toolbar->select_var_float) = toolbar->settings[toolbar->num_page][toolbar->num_setting].min;
			} else {
                road_has_changed_float(road, toolbar->select_var_float);
            }
		} else {
			printf("Error: select_var NULL\n");
		}
		// TODO : vérifier que le clique est dans la fenêtre !
		toolbar->pos_click_x = mouse_x;
	}
}


static int minor_by_1(int a) {
    return (a < 1) ? 1 : a;
}

static float minor_by_1f(float a) {
    return (a < 1.f) ? 1.f : a;
}

void change_variable_keys(Toolbar* toolbar, short add, Road* road){
    if (toolbar->select_var_int) {
        *(toolbar->select_var_int) += add * minor_by_1((int)((toolbar->settings[toolbar->num_page][toolbar->num_setting].max - toolbar->settings[toolbar->num_page][toolbar->num_setting].min) / SIZE_LINE_TOOLBAR));
        if (*(toolbar->select_var_int) > (int)toolbar->settings[toolbar->num_page][toolbar->num_setting].max){
            *(toolbar->select_var_int) = (int)toolbar->settings[toolbar->num_page][toolbar->num_setting].max;
        } else if (*(toolbar->select_var_int) < (int)toolbar->settings[toolbar->num_page][toolbar->num_setting].min){
            *(toolbar->select_var_int) = (int)toolbar->settings[toolbar->num_page][toolbar->num_setting].min;
        } else {
            // TODO : nul de devoir rajouter road de partout :/
            road_has_changed_int(road, toolbar->select_var_int);
        }
    } else if (toolbar->select_var_float) {
        *(toolbar->select_var_float) += (float)add * minor_by_1f((toolbar->settings[toolbar->num_page][toolbar->num_setting].max - toolbar->settings[toolbar->num_page][toolbar->num_setting].min) / SIZE_LINE_TOOLBAR);
        if (*(toolbar->select_var_float) > toolbar->settings[toolbar->num_page][toolbar->num_setting].max){
            *(toolbar->select_var_float) = toolbar->settings[toolbar->num_page][toolbar->num_setting].max;
        } else if (*(toolbar->select_var_float) < toolbar->settings[toolbar->num_page][toolbar->num_setting].min){
            *(toolbar->select_var_float) = toolbar->settings[toolbar->num_page][toolbar->num_setting].min;
        } else {
            road_has_changed_float(road, toolbar->select_var_float);
        }
    } else {
        perror("Error: select_var NULL\n");
    }
}

static void recover_old_road(Road* road, int num_road_saved) { // TODO: changer le nom
    road->len_tab_cp = road->generation.len_tab_cp[num_road_saved];
    for (int i = 0; i < road->len_tab_cp; ++i) {
        road->tab_cp[i] = road->generation.tab_cp_at_step[num_road_saved][i];
    }
}

static void init_has_changed(Road* road) {
    road->generation.has_changed.cp_size_angle_to_remove = False;
    road->generation.has_changed.nb_loops_uncross_segments = False;
    road->generation.has_changed.dist_cp = False;
    road->generation.has_changed.nb_cp_max = False;
}

void regenerate_map(Road* road, Camera* cam, Player* player, Callback* callback) {
    // TODO : réinitialiser les IA

    if (road->generation.set_up_cp) {
        travelling_set_up_cp(road);
    } else if (road->generation.greedy) {
        recover_old_road(road, 0);
    }
    if (road->generation.greedy) {
        greedy(road);
    } else if (!road->generation.set_up_cp){ // TODO: changer les conditions en un truc clean
        recover_old_road(road, 1);
    }
    if (road->generation.uncross_and_remove) {
        uncross_and_remove(road, player);
    } else {
        if (road->generation.has_changed.dist_cp && !(road->generation.greedy || road->generation.uncross_and_remove)) {
            // TODO : éloigner x et y selon l'ancienne dist et la nouvelle + changer les sauvegardes intermédiaires
        }
        for (int i = 0; i < NB_OF_PLAYERS; ++i) {
            player[i].car.pos_initx = (float)road->tab_cp[0].x - 200;
            player[i].car.pos_inity = (float)road->tab_cp[0].y + 100.f * (float)i;
            player[i].car.posx = player[i].car.pos_initx;
            player[i].car.posy = player[i].car.pos_inity;
            player[i].car.frame.x = (int)(player[i].car.posx);
            player[i].car.frame.y = (int)(player[i].car.posy);
        }
    }
    reinitialize_map(player, road, cam);
    callback->create_road = False;
    init_has_changed(road);
}

void manage_selected_toolbar(Toolbar* toolbar, Road* road, Camera* cam, Callback* callback, Player* player) {
    // TODO : revoir

    // TODO : au lieu de le faire là, faire une function que refait la map selon les variables et qui sera appelé à chaque fois
    // Comme ça dès qu'on modifie la variable on refait la map directement
    toolbar->is_selecting = False;
    if (((toolbar->settings[toolbar->num_page][toolbar->num_setting].type == Checkbox
         || toolbar->settings[toolbar->num_page][toolbar->num_setting].type == Button)
        && toolbar->select_var_int == toolbar->settings[toolbar->num_page][toolbar->num_setting].int_variable)) {

        *toolbar->settings[toolbar->num_page][toolbar->num_setting].int_variable =
                (*toolbar->settings[toolbar->num_page][toolbar->num_setting].int_variable + 1) % 2;

        // the box has just been checked
        if (*toolbar->settings[toolbar->num_page][toolbar->num_setting].int_variable == True) {
            // the box is ia->active
            if (toolbar->select_var_int == (int *) &player[0].ia->active) {
                // TODO : Utile ?
                init_ia(player[0].ia, road, &player[0].car, &player[0].cp);
            }
            // TODO : à mettre dans une fonction
            // function to create road
            if (toolbar->select_var_int == (int *) &callback->create_road) {
                // TODO : faire un thread pour pas avoir de freeze
                regenerate_map(road, cam, player, callback);
            }
            // the box has just been unchecked
        } else {
            // the box is ia->active
            if (toolbar->settings[toolbar->num_page][toolbar->num_setting].int_variable ==
                (int *) &player[0].ia->active) {
                // the ia change keys, so we need to fixe them to False
                release_the_keys(&player[0].key);
            }
        }
        toolbar->select_var_int = NULL;
    }
}

void move_screen(Camera* cam, Toolbar* toolbar){
	// TODO à reprendre
    // TODO voir le signal du process quand la fenêtre est resize
	SDL_GetMouseState(&cam->cursor_x, &cam->cursor_y);
	if (cam->cursor_x < 20){
		cam->x -= (300.f / FRAMES_PER_SECONDE);
	} else if (cam->cursor_x > cam->winSize_w + toolbar->size.w - 20){
		cam->x += (300.f / FRAMES_PER_SECONDE);
	}
	if (cam->cursor_y < 20){
		cam->y -= (300.f / FRAMES_PER_SECONDE);
	} else if (cam->cursor_y > cam->winSize_h - 20){
		cam->y += (300.f / FRAMES_PER_SECONDE);
	}
}

void render_number(SDL_Renderer* renderer, Background* bg, int number, int pos_x, int pos_y) {
    // TODO : faire float number
    bg->number_size.x = pos_x;
    bg->number_size.y = pos_y;
    int rest = number;
    uint8_t digit;
    while (rest > 0) {
        digit = rest % 10;
        rest = rest / 10;
        bg->number_size.x -= 10;
        SDL_RenderCopy(renderer, bg->number[digit], NULL, &bg->number_size);
    }
}

void render_toolbar(SDL_Renderer *renderer, Toolbar* toolbar){
    //TODO découper en fonctions
	// choose color
	SDL_SetRenderDrawColor(renderer, COLOR_TOOLBAR);
    SDL_RenderFillRect(renderer, &toolbar->size);
	// display toolbar
	SDL_RenderFillRect(renderer, &toolbar->size);
    // display the top of the toolbar
    SDL_SetRenderDrawColor(renderer, COLOR_TOP_TOOLBAR);
    SDL_Rect rect_top = {
            toolbar->size.x + (1 - toolbar->num_page) * toolbar->size.w / NB_PAGES,
            0,
            toolbar->size.w / NB_PAGES,
            toolbar->top_h
    };
    SDL_RenderFillRect(renderer, &rect_top);

	// display text
	SDL_SetRenderDrawColor(renderer, WHITE);
	int x_mean_line;
	int y_line;
	int min;
	int max;
	SDL_Rect rect = {0, 0, 4, 10};
	int i;
	for(i = 0; i < NB_SETTINGS; i++){ //for each seatting
		toolbar->settings[toolbar->num_page][i].tex_size.x = toolbar->size.x + 50;
		//text
		SDL_RenderCopy(renderer, toolbar->settings[toolbar->num_page][i].texture, NULL, &(toolbar->settings[toolbar->num_page][i].tex_size));
		//line
        switch (toolbar->settings[toolbar->num_page][i].type) {
            case Line:
//                x_mean_line = toolbar->settings[toolbar->num_page][i].tex_size.x + toolbar->settings[toolbar->num_page][i].tex_size.w / 2;
                x_mean_line = toolbar->size.x + toolbar->size.w / 2;
                y_line = toolbar->settings[toolbar->num_page][i].tex_size.y + 2 * toolbar->settings[toolbar->num_page][i].tex_size.h;
                // draw the line
                SDL_RenderDrawLine(renderer,  x_mean_line- SIZE_LINE_TOOLBAR / 2, y_line, toolbar->size.x + toolbar->size.w /2 + SIZE_LINE_TOOLBAR / 2, y_line);
                // cursor
                min = (int)toolbar->settings[toolbar->num_page][i].min;
                max = (int)toolbar->settings[toolbar->num_page][i].max;
                if (toolbar->settings[toolbar->num_page][i].int_variable){
                    rect.x = x_mean_line - SIZE_LINE_TOOLBAR / 2 + (*(toolbar->settings[toolbar->num_page][i].int_variable) - min) * SIZE_LINE_TOOLBAR / (max - min);//calculer la pos du curseur
                } else if (toolbar->settings[toolbar->num_page][i].float_variable) {
                    rect.x = x_mean_line - SIZE_LINE_TOOLBAR / 2 + (int)(*(toolbar->settings[toolbar->num_page][i].float_variable) - (float)min) * SIZE_LINE_TOOLBAR / (max - min);//calculer la pos du curseur
                }
                rect.y = y_line - rect.h/2;
                SDL_RenderFillRect(renderer, &rect);
                break;
            case Checkbox:
                rect.h = toolbar->settings[toolbar->num_page][i].tex_size.h;
                rect.w = toolbar->settings[toolbar->num_page][i].tex_size.h;
                rect.x = toolbar->settings[toolbar->num_page][i].tex_size.x + toolbar->settings[toolbar->num_page][i].tex_size.w + 10;
                rect.y = toolbar->settings[toolbar->num_page][i].tex_size.y;
                if (toolbar->select_var_int == toolbar->settings[toolbar->num_page][i].int_variable){
                    SDL_SetRenderDrawColor(renderer, CP_SELECTED_COLOR); // checkbox selected
                    SDL_RenderFillRect(renderer, &rect);
                } else if (*toolbar->settings[toolbar->num_page][i].int_variable == True){
                    SDL_SetRenderDrawColor(renderer, NEXT_CP_COLOR); // green checkbox
                    SDL_RenderFillRect(renderer, &rect);
                } else if (*toolbar->settings[toolbar->num_page][i].int_variable == False){
                    SDL_SetRenderDrawColor(renderer, CP_TAKEN_COLOR); // empty checkbox
                    SDL_RenderDrawRect(renderer, &rect);
                }

                // Put back as before
                SDL_SetRenderDrawColor(renderer, WHITE);
                rect.h = 10;
                rect.w = 4;
                break;
            case Button:
                rect.h = toolbar->settings[toolbar->num_page][i].tex_size.h;
                rect.w = toolbar->settings[toolbar->num_page][i].tex_size.h;
                rect.x = toolbar->settings[toolbar->num_page][i].tex_size.x + toolbar->settings[toolbar->num_page][i].tex_size.w + 10;
                rect.y = toolbar->settings[toolbar->num_page][i].tex_size.y;
                if (toolbar->select_var_int == toolbar->settings[toolbar->num_page][i].int_variable){
                    SDL_SetRenderDrawColor(renderer, 50, 0, 0, 255); // checkbox selected
                    SDL_RenderFillRect(renderer, &rect);
                } else if (*toolbar->settings[toolbar->num_page][i].int_variable == True){
                    SDL_SetRenderDrawColor(renderer, 25, 25, 75, 255);
                    SDL_RenderFillRect(renderer, &rect);
                } else if (*toolbar->settings[toolbar->num_page][i].int_variable == False){
                    SDL_SetRenderDrawColor(renderer, CP_TAKEN_COLOR); // empty checkbox
                    SDL_RenderDrawRect(renderer, &rect);
                }

                // Put back as before
                SDL_SetRenderDrawColor(renderer, WHITE);
                rect.h = 10;
                rect.w = 4;
                break;
            case Empty:
                break;

            default:
                fprintf(stderr, "Error: bad toolbar->settings[%d].type. Must be Line, Checkbox, Button or Empty. \n", i);
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
	// drift
	if (key->drift == drift_left){
		SDL_SetRenderDrawColor(renderer, CP_SELECTED_COLOR);
		SDL_RenderFillRect(renderer, &key_square);
	// active
	} else if (key->left == True) {
		SDL_SetRenderDrawColor(renderer, BLACK);
		SDL_RenderFillRect(renderer, &key_square);
	// unactive
	} else {
		SDL_SetRenderDrawColor(renderer, BLACK);
		SDL_RenderDrawRect(renderer, &key_square);
	}

	// down
	key_square.x += key_square.w + 2;
	// active
	if (key->down == True) {
		SDL_SetRenderDrawColor(renderer, BLACK);
		SDL_RenderFillRect(renderer, &key_square);
	// unactive
	} else {
		SDL_SetRenderDrawColor(renderer, BLACK);
		SDL_RenderDrawRect(renderer, &key_square);
	}

	// up
	key_square.y -= key_square.h + 2;
	// active
	if (key->up == True) {
		SDL_SetRenderDrawColor(renderer, BLACK);
		SDL_RenderFillRect(renderer, &key_square);
	// unactive
	} else {
		SDL_SetRenderDrawColor(renderer, BLACK);
		SDL_RenderDrawRect(renderer, &key_square);
	}

	// right
	key_square.y += key_square.h + 2;
	key_square.x += key_square.w + 2;
	// drift
	if (key->drift == drift_right){
		SDL_SetRenderDrawColor(renderer, CP_SELECTED_COLOR);
		SDL_RenderFillRect(renderer, &key_square);
	// active
	} else if (key->right == True) {
		SDL_SetRenderDrawColor(renderer, BLACK);
		SDL_RenderFillRect(renderer, &key_square);
	// unactive
	} else {
		SDL_SetRenderDrawColor(renderer, BLACK);
		SDL_RenderDrawRect(renderer, &key_square);
	}

}

/**
 * Initialize the font and texture for the 10 digits.
 * Can be used for the FPS
 * @param renderer
 * @param bg
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int init_number_tex(SDL_Renderer* renderer, Background* bg) {

    //init font
    TTF_Font* font = TTF_OpenFont(FONT, FONT_SIZE);
    if (!font){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", TTF_GetError());
        return EXIT_FAILURE;
    }
    SDL_Color fg_color = { WHITE };
    SDL_Color bg_color = { COLOR_TOOLBAR };

    char number[] = "0";
    for (int i = 0; i < 10; i++){ // 0 to 9
        number[0] = (char)('0' + i);
        SDL_Surface* text = TTF_RenderText_Shaded(font, number, fg_color, bg_color);
        if (!text){
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", TTF_GetError());
            return EXIT_FAILURE;
        }
        if (!(bg->number[i] = SDL_CreateTextureFromSurface(renderer, text))){ // settings[i].texture
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", TTF_GetError());
            return EXIT_FAILURE;
        }
        SDL_QueryTexture(bg->number[0], NULL, NULL, &bg->number_size.w, &bg->number_size.h);
        SDL_FreeSurface(text);
    }

    TTF_CloseFont(font);
    return EXIT_SUCCESS;
}

void destroy_texture(Background* bg){
#if 1 // TODO : utile ?
	for (int i = 0; i < NB_COLORS_BG; i++){
		if(NULL != bg->texture[i])
			SDL_DestroyTexture(bg->texture[i]);
	}
#endif
    for (int i = 0; i < 10; i++) {
        if (NULL != bg->number[i])
            SDL_DestroyTexture(bg->number[i]);
    }
}

#if 1
#define NB_PT_X 150
#define NB_PT_Y 220
int init_background(SDL_Renderer* renderer, Background* bg){
    if (EXIT_FAILURE == init_number_tex(renderer, bg))
        return EXIT_FAILURE;

    /*bg->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, NB_PT_X, NB_PT_Y);*/
    /*SDL_Rect rect = {0, 0, SIZE_PT_X, SIZE_PT_Y};*/
#if 0
    for (int i = 0; i < NB_COLORS_BG; i++){
        bg->texture[i] = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, NB_PT_X, NB_PT_Y);

        if (bg->texture[i] == NULL){
            fprintf(stderr, "Error SDL_CreateTexture : %s\n", SDL_GetError());
            return EXIT_FAILURE;
        }
    }
    SDL_SetRenderTarget(renderer, bg->texture[0]);
    SDL_SetRenderDrawColor(renderer, CP_START_COLOR);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, bg->texture[1]);
    SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, bg->texture[2]);
    SDL_SetRenderDrawColor(renderer, NEXT_CP_COLOR);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, bg->texture[3]);
    SDL_SetRenderDrawColor(renderer, CP_COLOR);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, bg->texture[4]);
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, bg->texture[5]);
    SDL_SetRenderDrawColor(renderer, 255, 50, 0, 255);
    SDL_RenderClear(renderer);
	SDL_SetRenderTarget(renderer, NULL);
#endif
	return EXIT_SUCCESS;
}
#endif

#if 0
static int det(int x1, int x2, int y1, int y2) {
    return x1*y2 - x2*y1;
}

static int vect_x(int x1, int x2) {
    return x2 - x1;
}

static int vect_y(int y1, int y2) {
    return y2 - y1;
}

void fill_background(SDL_Renderer* renderer, Background* bg, Road* road, Camera* cam) {
    if (bg->show){
        // first verif
        int is_in_edge;

        int size_pt_y = cam->winSize_h / NB_GRID_ROW;
        int size_pt_x = cam->winSize_w / NB_GRID_COLUMN;
        // draw on the texture
        SDL_Rect rect = {0, 0, size_pt_x, size_pt_y};
        SDL_Rect mini_rect = {0, 0, size_pt_x / bg->nb_sq_fill + 1 , size_pt_y / bg->nb_sq_fill + 1};
        int vect_road_x;
        int vect_road_y;
        int vect_pt_x;
        int vect_pt_y;
        int det_vect;
        for (int row = 0; row < NB_GRID_ROW; row++) {
            for (int column = 0; column < NB_GRID_COLUMN; column++) {
                // mettre une table de SDL_Rect !
                // avec SDL_RenderDrawRects
                /*SDL_RenderDrawPoint(renderer, i, j);*/
                rect.x = size_pt_x * column;
                rect.y = size_pt_y * row;
                is_in_edge = road->nb_pts_collision[row][column];
                if (is_in_edge) {
                    if (road->nb_pts_collision[row][column] <= 2) {
                        if (!road->pt_in_road[row][column]) {
                            SDL_RenderCopy(renderer, bg->texture[2], NULL, &rect);
                        } else {
                            SDL_RenderCopy(renderer, bg->texture[1], NULL, &rect);
                        }
                    } else {
                        // TODO : pour l'instant on va juste faire avec tous les points sans opti, déjà on va faire avec un vect du dernier - premier point (au lieu de faire chaque point)
                        // TODO : en suite faudra regarder la valeur du déterminant et si le point est loin, mettre un gros carré, et ne calculer ses voisins que s'il est prêt, on peut faire ça
                        // TODO : par récurrence, ie / 2 la taille des zones à chaque fois
                        // TODO : on peut aussi gagner 4 fois les perfs en divisant une première fois, et selon si le carré touche le centre ou pas on peut en déduire pas mal de choses
                        for (int i = 0; i < bg->nb_sq_fill; i++) {
                            for (int j = 0; j < bg->nb_sq_fill; j++) {
                                mini_rect.x = size_pt_x * (bg->nb_sq_fill * column + i) / bg->nb_sq_fill;
                                mini_rect.y = size_pt_y * (bg->nb_sq_fill * row + j) / bg->nb_sq_fill;
                                vect_road_x = vect_x(
                                        road->collision_grid[row][column][road->nb_pts_collision[row][column] - 2].x,
                                        road->collision_grid[row][column][0].x);
                                vect_road_y = vect_y(
                                        road->collision_grid[row][column][road->nb_pts_collision[row][column] - 2].y,
                                        road->collision_grid[row][column][0].y);
                                vect_pt_x = vect_x(mini_rect.x + size_pt_x / (2 * bg->nb_sq_fill),
                                                   road->collision_grid[row][column][0].x);
                                vect_pt_y = vect_x(mini_rect.y + size_pt_x / (2 * bg->nb_sq_fill),
                                                   road->collision_grid[row][column][0].y);

                                det_vect = det(
                                        vect_road_x,
                                        vect_pt_x,
                                        vect_road_y,
                                        vect_pt_y
                                );
                                //printf("%s : %d, %d | %d, %d -> %d\n", road->edge_right[row][column]? "droite":"gauche" , vect_road_x, vect_road_y, vect_pt_x, vect_pt_y, det_vect);
                                if ((det_vect >= 0 && road->edge_right[row][column])) {
                                    //SDL_RenderCopy(renderer, bg->texture[0], NULL, &mini_rect);
                                } else if ((det_vect < 0 && !road->edge_right[row][column])) {
                                    //SDL_RenderCopy(renderer, bg->texture[4], NULL, &mini_rect);
                                } else if (det_vect >= 0 && !road->edge_right[row][column]) {
                                    // TODO : bug sur l'arrêt de ça !
                                    //SDL_RenderCopy(renderer, bg->texture[5], NULL, &mini_rect);
                                    SDL_RenderCopy(renderer, bg->texture[2], NULL, &mini_rect);
                                } else if (det_vect < 0 && road->edge_right[row][column]) {
                                    //SDL_RenderCopy(renderer, bg->texture[3], NULL, &mini_rect);
                                    SDL_RenderCopy(renderer, bg->texture[2], NULL, &mini_rect);
                                }
                            }
                        }
                    }
                } else if (road->pt_in_road[row][column]) { // really in the road
                    // SDL_RenderCopy(renderer, bg->texture[1], NULL, &rect);
                } else {
                    SDL_RenderCopy(renderer, bg->texture[2], NULL, &rect);
                }
            }
        }
    }
}
#endif
