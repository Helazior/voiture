/*jeu.c*/


#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <math.h>
#include "../include/jeu.h"
#include "../include/ia.h"
#include "../include/background.h"
#include "../include/create_map.h"

static unsigned int startLapTime;


int init(SDL_Window **window, SDL_Renderer **renderer, int w, int h)      {
	if(0 != SDL_Init(SDL_INIT_VIDEO)){
		fprintf(stderr, "Erreur SDL_Init : %s", SDL_GetError());
		return -1;
	}
	if (!(IMG_Init(IMG_INIT_PNG))){
		fprintf(stderr, "Erreur IMG_Init : %s", SDL_GetError());
        return -1;
	}
	if (TTF_Init() < 0){ // to write text
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[DEBUG] > %s", SDL_GetError());
		return EXIT_FAILURE;
	}
	// SDL_WINDOW_MAXIMIZED -> not fullscreen
	// SDL_WINDOW_FULLSCREEN_DESKTOP -> fs
	if(0 != SDL_CreateWindowAndRenderer(w, h, SDL_WINDOW_FULLSCREEN_DESKTOP, window, renderer)){
        fprintf(stderr, "Erreur SDL_CreateWindowAndRenderer : %s", SDL_GetError());
        return -1;
    }
    return 0;
}

__attribute__((unused)) int setWindowColor(SDL_Renderer *renderer, SDL_Color color){
	if(SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a) < 0){
		fprintf(stderr, "Erreur SDL_SetRenderDrawColor : %s", SDL_GetError());
        return -1;
	}
    if(SDL_RenderClear(renderer) < 0){
		fprintf(stderr, "Erreur SDL_SetRenderDrawColor : %s", SDL_GetError());
        return -1;
	}
    return 0;
}

SDL_Texture* loadTexture(SDL_Renderer *renderer, const char* p_filePath){
	SDL_Texture* texture = NULL;
	texture = IMG_LoadTexture(renderer, p_filePath);

	if (texture == NULL)
        fprintf(stderr, "Failed to load texture. Error: %s", SDL_GetError());
	return texture;
}

void pause(){
	// TODO : ne pas compter le temps du chrono
    // TODO : corriger les bugs dans "next_steps.txt"
	SDL_Event event;
	do{
		SDL_WaitEvent(&event);
	} while(event.type != SDL_KEYDOWN);
}

void init_car(Entity* car, SDL_Renderer *renderer, uint8_t num){
	car->speed = 0.f;//pixels per frame
	car->angle = 0.f;
	car->angle_drift = 0.f;
	car->pos_initx = 0.f;
	car->pos_inity = 0.f + 100.f * (float)num;
	car->posx = car->pos_initx;
	car->posy = car->pos_inity;
	car->frame.x = (int)(car->posx);
	car->frame.y = (int)(car->posy);
	car->frame.w = 128;
	car->frame.h = 52;
    if (num == 0) {
        car->tex = loadTexture(renderer, "image/first_car.png");//car image
    } else {
        car->tex = loadTexture(renderer, "image/car.png");//car image
    }
	car->pos_tab = 0;
	car->count_pos_tab = 0;
	car->acceleration = ACCELERATION;
	car->frottement = FROTTEMENT;
	car->turn = TURN;
	car->turn_drift = TURN_DRIFT;
}

void init_player_cp(PlayerCP* cp, int len_tab_checkPoints) {
    for (int i = 0; i < len_tab_checkPoints; i++) {
        cp->tab_valid_checkPoints[i] = False;
    }
}

void init_cam(Camera* cam, Entity* car){
	cam->x = (float)car->pos_initx - (float)cam->winSize_w / 2.;
	cam->y = (float)car->pos_inity - (float)cam->winSize_h / 2.;
}

void free_players(Player* player) {
    for (int i = 0; i < NB_OF_PLAYERS; ++i) {
        free(player[i].ia);
    }
}

float distance(float x1, float y1, float x2, float y2){
	return sqrtf(pow((float)x1 - (float)x2, 2) + pow(((float)y1 - (float)y2), 2)); // TODO : faire x*x au lieu de pow(x,2) pour opti
}

float dist2(float x1, float y1, float x2, float y2){
	return pow((float)x1 - (float)x2, 2) + pow(((float)y1 - (float)y2), 2); // TODO : faire x*x au lieu de pow(x,2) pour opti
}

void manage_skid_marks(Entity* car, Keys_pressed* key){
	if (key->drift){
		car->tab_skid_marks_x[car->pos_tab] = car->posx;
		car->tab_skid_marks_y[car->pos_tab] = car->posy;
		car->tab_skid_marks_angle[car->pos_tab] = car->angle + car->angle_drift;
		car->count_pos_tab += (int)(car->count_pos_tab < NB_PIX_DRIFT - 1);
		car->pos_tab ++;
		car->pos_tab *= (int)(car->pos_tab < NB_PIX_DRIFT);
	}
}

void move_car(Entity* car, Keys_pressed* key, Camera* cam, Bool first_car) {
	manage_skid_marks(car, key);
	//keys_to_struct
	if (car->speed < 1 && key->drift){
		key->drift = none;
		car->angle += car->angle_drift;
		car->angle_drift = 0.;
	}
	car->angle_drift += ((key->drift == drift_left) - 2*(key->drift == drift_right)) * car->turn_drift * (60. / FRAMES_PER_SECONDE) / 320.;
	car->speed += ((float)(key->up) - (float)(key->down)/2.) * car->acceleration * (60. / FRAMES_PER_SECONDE) / 20.;
	if (fabsf(car->speed) > 3.){
		car->angle += (double)(car->turn * (60. / FRAMES_PER_SECONDE) * ((double)(key->left) - (double)(key->right))) / (10 * (1. - 2. * (double)((car->speed) < 0.)) * 6 * sqrtf(fabsf(car->speed)));
	}
	else if (fabsf(car->speed) <= 3.){
		car->angle += (double)((double)(key->left) - (double)(key->right)) * (car->speed) * car->turn * (60. / FRAMES_PER_SECONDE) / 1280;
	}

	//struct_to_moveCar
	float old_posx = car->posx;
	float old_posy = car->posy;
	car->posx += car->speed * (float)cos(car->angle);
	car->posy -= car->speed * (float)sin(car->angle);
	car->frame.x = (int)(car->posx);
	car->frame.y = (int)(car->posy);
	car->speed += ((float)(((car->speed) < 0.) - ((car->speed) > 0.))) * (1. + (fabsf(car->speed))) * car->frottement / 640.;
	//manage cam
	//TODO : si on est au bout de la piste, ne pas aller plus loin !
    if (first_car) {
        if (cam->follow_car) {
            float new_cam_x = car->posx - (float) cam->winSize_w / 2 +
                              REAR_CAMERA * car->speed * (FRAMES_PER_SECONDE / 60.) * cos(car->angle);
            float new_cam_y = car->posy - (float) cam->winSize_h / 2 -
                              REAR_CAMERA * car->speed * (FRAMES_PER_SECONDE / 60.) * sin(car->angle);
            cam->x = (int) ((9. * (float) cam->x + (float) new_cam_x) / 10.);
            cam->y = (int) ((9. * (float) cam->y + (float) new_cam_y) / 10.);
        } else {
            cam->x += (1 - cam->zoom) * (car->posx - old_posx);
            cam->y += (1 - cam->zoom) * (car->posy - old_posy);
        }
    }
}

void manage_key(SDL_Event* event, Keys_pressed* key, Bool status, Camera* cam, Road* road, Toolbar* toolbar, Player* player, uint8_t num_player){
	// TODO : faire un enum !
	short add_to_var;
	add_to_var = 1;
	switch(event->key.keysym.sym){
		case SDLK_UP:
			key->up = status;
			if (key->drift && status){
				key->drift = none;
				player[num_player].car.angle += player[num_player].car.angle_drift;
				player[num_player].car.angle_drift = 0.;
			}
			break;
		case SDLK_LCTRL:
		case SDLK_DOWN:
			key->down = status;
			if (status == True && (key->left ^ key->right) && key->up == False && player[num_player].car.speed > 3.){
				key->drift = drift_left * (key->left) + drift_right * (key->right);
			}
			break;
		case SDLK_RIGHT:
			key->right = status;
			if (key->drift == drift_right && status == False){
				key->drift = none;
				player[num_player].car.angle += player[num_player].car.angle_drift;
				player[num_player].car.angle_drift = 0.;
			}
			break;
		case SDLK_LEFT:
			key->left = status;
			if (key->drift == drift_left && status == False){
				key->drift = none;
				player[num_player].car.angle += player[num_player].car.angle_drift;
				player[num_player].car.angle_drift = 0.;
			}
			break;
		case SDLK_ESCAPE:
            for (int i = 0; i < NB_OF_PLAYERS; ++i) {
                player[i].car.posx = player[i].car.pos_initx;
                player[i].car.posy = player[i].car.pos_inity;
                player[i].car.speed = 0.;
                reset_valid_tab(road, &player[i].cp, i == 0);
                if (player[i].ia->active)
                    init_ia(player[i].ia, road, &player[i].car, &player[i].cp);
            }
            if (cam->follow_car == False)
                init_cam(cam, &player[0].car);
            break;
        case SDLK_p:
            cam->zoom *= 1.1;
			break;
		case SDLK_o:
			cam->zoom /= 1.1;
			break;
		case SDLK_KP_PLUS:
		case SDLK_PLUS:
		case SDLK_EQUALS:
			if (toolbar->settings[toolbar->num_setting].type == Line && (toolbar->select_var_int || toolbar->select_var_float)){
				change_variable_keys(toolbar, add_to_var);
			}
			break;
		case SDLK_KP_MINUS:
		case SDLK_MINUS:
		case SDLK_KP_LESS:
		case SDLK_LESS:
		case SDLK_6:
			if (toolbar->settings[toolbar->num_setting].type == Line && (toolbar->select_var_int || toolbar->select_var_float)){
				change_variable_keys(toolbar, -add_to_var);
			}
			break;
		case SDLK_SPACE:
			pause();
		break;
		default:
			break;
	}
}

//add a checkpoint:
void add_checkPoint(Road* road, SDL_Event* event, Camera* cam, Entity* car, Player* player){
	if (road->len_tab_checkPoints < NB_SQUARE){
		road->tab_checkPoints[road->len_tab_checkPoints].x = event->button.x - (float)road->square_width / 2 + cam->x + (event->button.x + cam->x - car->frame.x) * (float)(-1. + 1/cam->zoom);
		road->tab_checkPoints[road->len_tab_checkPoints].y = event->button.y - (float)road->square_width / 2 + cam->y + (event->button.y + cam->y - car->frame.y) * (float)(-1. + 1/cam->zoom);
		road->tab_checkPoints[road->len_tab_checkPoints].w = road->square_width;
		road->tab_checkPoints[road->len_tab_checkPoints].h = road->square_width;
        road->len_tab_checkPoints++;
        for (int i = 0; i < NB_OF_PLAYERS; ++i) {
            player[i].cp.tab_valid_checkPoints[road->len_tab_checkPoints - 1] = False;
            if (road->len_tab_checkPoints == 4){
                calcul_next_cp(road, player[i].ia, &player[i].cp, car);
            }
        }
		startLapTime = SDL_GetTicks();

		//printf("%d	%d		\n", road->tab_checkPoints[road->len_tab_checkPoints].x, road->tab_checkPoints[road->len_tab_checkPoints].y);

	}
}

//found the closest checkpoint to the clic:
void closest_checkpoint(Road* road, SDL_Event* event, Camera* cam, Entity* car) {
	int dist;
	int pos_clique_x = event->button.x - (float)road->square_width / 2 + cam->x + (event->button.x + cam->x - car->frame.x) * (float)(-1. + 1/cam->zoom);
	int pos_clique_y = event->button.y - (float)road->square_width / 2 + cam->y + (event->button.y + cam->y - car->frame.y) * (float)(-1. + 1/cam->zoom);
    road->num_clos_check = 0;
	int min_dist = distance((float)pos_clique_x, (float)pos_clique_y, (float)road->tab_checkPoints[0].x, (float)road->tab_checkPoints[0].y);
	int i;
	for (i = 1; i < road->len_tab_checkPoints; i++){
		dist = distance((float)pos_clique_x, (float)pos_clique_y, (float)road->tab_checkPoints[i].x, (float)road->tab_checkPoints[i].y);
		if (dist < min_dist){
			road->num_clos_check = i;
			min_dist = dist;
		}
	}
	road->selectx = road->tab_checkPoints[road->num_clos_check].x - pos_clique_x;
	road->selecty = road->tab_checkPoints[road->num_clos_check].y - pos_clique_y;
}

void reset_valid_tab(Road* road, PlayerCP* cp, bool first_player) {
	if (road->len_tab_checkPoints >= 4 && cp->nb_valid_checkPoints == road->len_tab_checkPoints && first_player) {
        // TODO: faire le classement lorsqu'on passe le start
        // TODO: faire un chrono par voiture
		printf("Lap time: %.2f\n", ((float)SDL_GetTicks() - (float)startLapTime) / 1000.);
		//startLapTime = SDL_GetTicks();
	}
	for (int i = 0; i < road->len_tab_checkPoints; i++){
		cp->tab_valid_checkPoints[i] = 0;
	}
	cp->nb_valid_checkPoints = 0;
}

//manage a checkpoint:
void manage_checkpoint(Road* road, SDL_Event* event, Camera* cam, Entity* car) {
	road->select = True;
	closest_checkpoint(road, event, cam, car);
}

//del a checkpoint:
void del_checkPoint(Road* road, SDL_Event* event, Camera* cam, Player* player){
	closest_checkpoint(road, event, cam, &player[0].car);
    for (int num = 0; num < NB_OF_PLAYERS; ++num) {
        if (player[num].cp.tab_valid_checkPoints[road->num_clos_check] != False){
            player[num].cp.nb_valid_checkPoints--;
            if (player[num].cp.tab_valid_checkPoints[road->num_clos_check] == Start){
                reset_valid_tab(road, &player[num].cp, num == 0);
            }
        }
        for(int i = road->num_clos_check; i < road->len_tab_checkPoints - 1; i++){
            road->tab_checkPoints[i] = road->tab_checkPoints[i+1];
            player[num].cp.tab_valid_checkPoints[i] = player[num].cp.tab_valid_checkPoints[i+1];
        }
    }
	road->len_tab_checkPoints--;
}

void clear(SDL_Renderer *renderer){
	SDL_RenderClear(renderer);
}

static void render_car(SDL_Renderer *renderer, Player* player, Camera* cam) {
    // TODO : tout mettre avec des floats pour éviter les tremblements ?
    int w_prev = player[0].car.frame.w;
    int h_prev = player[0].car.frame.h;
    player[0].car.frame.w *= cam->zoom;
    player[0].car.frame.h *= cam->zoom;
    SDL_Point center;
    center.x = player[0].car.frame.w / 2;
    center.y = player[0].car.frame.h / 2;

    double angle = player[0].car.angle + player[0].car.angle_drift;
    player[0].car.frame.x -= cam->x;
    player[0].car.frame.y -= cam->y;
    // TODO essayer de mettre NULL à la place de &src
    SDL_RenderCopyEx(
            renderer,
            player[0].car.tex,
            NULL,
            &player[0].car.frame,
            360. * (1. - angle / (2. * PI)),
            &center,
            SDL_FLIP_NONE);

    player[0].car.frame.y += cam->y;
    player[0].car.frame.x += cam->x;
    player[0].car.frame.h = h_prev;
    player[0].car.frame.w = w_prev;
    //____________________________________________________________________________
    // Other players
    int x;
    int y;
    int x_prev;
    int y_prev;

    float centre_x = player[0].car.posx - cam->x;
    float centre_y = player[0].car.posy - cam->y;
    int square_w = w_prev * cam->zoom;

    // TODO : factoriser avec l'affichage de checkpoint
    for (int i = 1; i < NB_OF_PLAYERS; i++){
        x_prev = player[i].car.frame.x;
        y_prev = player[i].car.frame.y;
        player[i].car.frame.x -= cam->x;
        player[i].car.frame.x = (1 - cam->zoom) * centre_x + cam->zoom * player[i].car.frame.x;
        player[i].car.frame.y -= cam->y;
        player[i].car.frame.y = (1 - cam->zoom) * centre_y + cam->zoom * player[i].car.frame.y;
        player[i].car.frame.w *= cam->zoom;
        player[i].car.frame.h *= cam->zoom;
        x = player[i].car.frame.x;
        y = player[i].car.frame.y;
        if (x + square_w > 0 && x - square_w < cam->winSize_w && y + square_w > 0 && y - square_w < cam->winSize_h){
            angle = player[i].car.angle + player[i].car.angle_drift;
            //display:
            SDL_RenderCopyEx(
                    renderer,
                    player[i].car.tex,
                    NULL,
                    &player[i].car.frame,
                    360. * (1. - angle / (2. * PI)),
                    &center,
                    SDL_FLIP_NONE);
        }
        player[i].car.frame.h = h_prev;
        player[i].car.frame.w = w_prev;
        player[i].car.frame.x = x_prev;
        player[i].car.frame.y = y_prev;
    }
}

static void render_checkPoints(SDL_Renderer *renderer, Road* road, Camera* cam, Player* player){
	int i;
	int x;
	int y;
	int x_prev;
	int y_prev;
	int w_prev;
	int h_prev;
    int mouse_x, mouse_y;

    SDL_GetMouseState(&mouse_x, &mouse_y);

    float centre_x = player[0].car.posx - cam->x;
	float centre_y = player[0].car.posy - cam->y;
	int square_w = road->square_width * cam->zoom;

    // TODO : changer en un truc mieux
	if (road->select && mouse_x < 20000 && mouse_y < 20000){
		int pos_clique_x = mouse_x - (float)road->square_width / 2 + cam->x + (mouse_x + cam->x - player[0].car.frame.x) * (float)(-1. + 1/cam->zoom);
		int pos_clique_y = mouse_y - (float)road->square_width / 2 + cam->y + (mouse_y + cam->y - player[0].car.frame.y) * (float)(-1. + 1/cam->zoom);
		road->tab_checkPoints[road->num_clos_check].x = pos_clique_x + road->selectx;
		road->tab_checkPoints[road->num_clos_check].y = pos_clique_y + road->selecty;
	}

	for (i = 0; i < road->len_tab_checkPoints; i++){
		x_prev = road->tab_checkPoints[i].x;
		y_prev = road->tab_checkPoints[i].y;
        // TODO : à mettre en dehors du for
		w_prev = road->tab_checkPoints[i].w;
		h_prev = road->tab_checkPoints[i].h;
		road->tab_checkPoints[i].x -= cam->x;
		road->tab_checkPoints[i].x = (1 - cam->zoom) * centre_x + cam->zoom * road->tab_checkPoints[i].x;
		road->tab_checkPoints[i].y -= cam->y;
		road->tab_checkPoints[i].y = (1 - cam->zoom) * centre_y + cam->zoom * road->tab_checkPoints[i].y;
		road->tab_checkPoints[i].w *= cam->zoom;
		road->tab_checkPoints[i].h *= cam->zoom;
		x = road->tab_checkPoints[i].x;
		y = road->tab_checkPoints[i].y;

        //check if collision between car and CP:
        // TODO optimiser ça !!!
        for (int num_player = 0; num_player < NB_OF_PLAYERS; ++num_player) {
            if (road->len_tab_checkPoints > 1 && (distance(x_prev, y_prev, player[num_player].car.posx, player[num_player].car.posy)
                                                  <= (float) road->size / 2 + (float) player[num_player].car.frame.h / 2)) {
                if (player[num_player].cp.tab_valid_checkPoints[i] == False) { // CP not already valid
                    player[num_player].cp.nb_valid_checkPoints++;
                    player[num_player].cp.tab_valid_checkPoints[i] = True;
                    if (player[num_player].ia->active && road->len_tab_checkPoints >= 4) {
                        // the
                        road->tab_checkPoints[i].x = x_prev;
                        road->tab_checkPoints[i].y = y_prev;
                        calcul_next_cp(road, player[num_player].ia, &player[num_player].cp, &player[num_player].car);
                    }

                    if (player[num_player].cp.nb_valid_checkPoints == 1) {//Start
                        player[num_player].cp.tab_valid_checkPoints[i] = Start;
                        if (!num_player) {
                            startLapTime = SDL_GetTicks();
                        }
                    }

                } else if (player[num_player].cp.nb_valid_checkPoints >= road->len_tab_checkPoints &&
                           player[num_player].cp.tab_valid_checkPoints[i] == Start) {
                    reset_valid_tab(road, &player[num_player].cp, true);
                }
            }
        }
        if (x + square_w > 0 && x - square_w < cam->winSize_w && y + square_w > 0 && y - square_w < cam->winSize_h){

			//display:
			if (road->select && road->num_clos_check == i){
				SDL_SetRenderDrawColor(renderer, CP_SELECTED_COLOR); // checkpoint selected
				SDL_RenderFillRect(renderer, &road->tab_checkPoints[i]);
			}
			else if (player[0].cp.tab_valid_checkPoints[i] == True){
				SDL_SetRenderDrawColor(renderer, CP_TAKEN_COLOR); // checkpoint taken
				SDL_RenderDrawRect(renderer, &road->tab_checkPoints[i]);
			}
			else if (player[0].ia->active >= True && player[0].ia->num_next_cp == i){
				SDL_SetRenderDrawColor(renderer, NEXT_CP_COLOR); // next checkpoint
				SDL_RenderFillRect(renderer, &road->tab_checkPoints[i]);
			}else if (player[0].cp.tab_valid_checkPoints[i] == False){
				SDL_SetRenderDrawColor(renderer, CP_COLOR); // normal checkpoint
				SDL_RenderFillRect(renderer, &road->tab_checkPoints[i]);
			}else{
				SDL_SetRenderDrawColor(renderer, CP_START_COLOR); // start checkpoint
				SDL_RenderFillRect(renderer, &road->tab_checkPoints[i]);
			}

		}
		road->tab_checkPoints[i].h = h_prev;
		road->tab_checkPoints[i].w = w_prev;
		road->tab_checkPoints[i].x = x_prev;
		road->tab_checkPoints[i].y = y_prev;
	}
}

static void render_drift(SDL_Renderer *renderer, Entity* car, Camera* cam){
	int w, h;
	w = car->frame.w * cam->zoom;
	h = car->frame.h * cam->zoom;
	float dist;
	dist = (sqrt(pow((float)w / 2.5, 2) + pow((float)h/3, 2)));

	int x[4] = {0};
	int y[4] = {0};

	SDL_SetRenderDrawColor(renderer, DRIFT_COLOR);//drift's black pixel

	float centre_x = car->posx + (float)w / 2 - cam->x;
	float centre_y = car->posy + (float)h / 2 - cam->y;

	int marks_x, marks_y;
	double angle;
	unsigned int i;
	unsigned int j;
	double angle_marks;
	for (i = 0; i < car->count_pos_tab; i++){
		marks_x = car->tab_skid_marks_x[i] + w / 2;
		marks_y = car->tab_skid_marks_y[i] + h / 2;
		angle = car->tab_skid_marks_angle[i];
		//devil eyes
		for (j = 0; j < 4; j ++){
			angle_marks = angle - 0.2 + (j >= 2) * 0.5 - (j == 1 || j == 3) * 0.1;
			x[j] = (int)(marks_x - dist * cos(angle_marks) / cam->zoom);
			x[j] -= cam->x;
			x[j] = (1 - cam->zoom) * centre_x + cam->zoom * x[j];
			y[j] = (int)(marks_y + dist * sin(angle_marks) / cam->zoom);
			y[j] -= cam->y;
			y[j] = (1 - cam->zoom) * centre_y + cam->zoom * y[j];
			if (x[j] > 0 && x[j] < cam->winSize_w && y[j] > 0 && y[j] < cam->winSize_h)
				SDL_RenderDrawPoint(renderer, x[j], y[j]);
		}
	}
}

/*
__attribute__((unused)) static void fill_collision_grid(Road* road, int row, int column, const float* x, const float* y) {
// TODO : à voir si c'est problématique de mettre juste *x ou pas
road->collision_grid[row][column][road->nb_pts_collision[row][column]].x = *x;
road->collision_grid[row][column][road->nb_pts_collision[row][column]].y = *y;
road->nb_pts_collision[row][column] ++;
// TODO : temporaire, à rendre plus beau
if (road->nb_pts_collision[row][column] > NB_PTS_COLL) {
    road->nb_pts_collision[row][column] --;
}
}
 */

static void calcul_spline(Entity* car, Camera* cam, Road* road, float* x, float* y, float* pt, short* draw){
	float t = *pt;
	float centre_x = car->posx - cam->x;
	float centre_y = car->posy - cam->y;
	int p0, p1, p2, p3;
	p0 = (int)t % road->len_tab_checkPoints;
	p1 = (p0 + 1) % road->len_tab_checkPoints;
	p2 = (p1 + 1) % road->len_tab_checkPoints;

	//if checkpoint not in the screen, go to next
	int s = road->size * cam->zoom / 2;
	float cx = road->tab_checkPoints[p1].x - cam->x;
	cx = (1 - cam->zoom) * centre_x + cam->zoom * cx;
	float cy = road->tab_checkPoints[p1].y - cam->y;
	cy = (1 - cam->zoom) * centre_y + cam->zoom * cy;
	float cx2 = road->tab_checkPoints[p2].x - cam->x;
	cx2 = (1 - cam->zoom) * centre_x + cam->zoom * cx2;
	float cy2 = road->tab_checkPoints[p2].y - cam->y;
	cy2 = (1 - cam->zoom) * centre_y + cam->zoom * cy2;
	if ((cx + s < 0 && cx2 + s < 0) || (cy + s < 0 && cy2 + s < 0) || (cx - s > cam->winSize_w && cx2 - s > cam->winSize_w) || (cy - s > cam->winSize_h && cy2 - s > cam->winSize_h)){
		*draw = 0;// then don't draw
		*pt = (int)t + 1;
		return;
	}//end

	p3 = (p2 + 1) % road->len_tab_checkPoints;
	t = t - (int)t;
	*draw += 3* (*draw == 0)  - (*draw == 2) - (*draw == 3);
	float tt = t * t;
	float ttt = tt * t;

	float q1 = -ttt + 2.0f * tt - t;
	float q2 = 3.0f * ttt - 5.0f*tt + 2.0f;
	float q3 = -3.0f * ttt + 4.0f * tt + t;
	float q4 = ttt - tt;

	*x = 0.5f * (road->tab_checkPoints[p0].x * q1 + road->tab_checkPoints[p1].x * q2 + road->tab_checkPoints[p2].x * q3 + road->tab_checkPoints[p3].x * q4);
	*y = 0.5f * (road->tab_checkPoints[p0].y * q1 + road->tab_checkPoints[p1].y * q2 + road->tab_checkPoints[p2].y * q3 + road->tab_checkPoints[p3].y * q4);

	*x -= cam->x - (float)road->square_width / 2;
	*x = (1 - cam->zoom) * centre_x + cam->zoom * *x;
	*y -= cam->y - (float)road->square_width / 2;
	*y = (1 - cam->zoom) * centre_y + cam->zoom * *y;

	// TODO : Mettre un set pour plus tard
	// TODO Mettre *x et *y
	// TODO Rajouter un point spécial pour le centre (comme ça pas de calcul)
    //int row = NB_GRID_ROW * (*y) / (float)cam->winSize_h;
    //int column = NB_GRID_COLUMN * (*x) / (float)cam->winSize_w;
	//if (*x >= 0. && *x <= cam->winSize_w && *y >= 0. && *y <= cam->winSize_h){
		// TODO à revoir :
        //  road->collision_grid[(unsigned int)(NB_GRID_ROW * (*y) / (float)cam->winSize_h)][(unsigned int)(NB_GRID_COLUMN * (*x) / (float)cam->winSize_w)][(unsigned int)(t*NB_PTS*cam->zoom)].x = *x;
        // TODO : plus tard mettre un truc qui dit que c'est forcément dedans, donc pas besoin d'enregistrer *x et *y, juste changer la première valeur par : true
        //road->pt_in_road[row][column] = true;
		//printf("-> %u \n", road->nb_pts_collision[row][column]);
		/*printf("(int)*y / cam->winSize_h = %d, %d\n",NB_GRID_ROW *  (int)*y / cam->winSize_h, NB_GRID_COLUMN * (int)*x / cam->winSize_w);*/
	//}
}

static void calcul_road(Camera* cam, Road* road, const float* x, const float* y, const float* prevx, const float* prevy, float* tabx, float* taby){
	float distx = *prevx - *x;
	float disty = *prevy - *y;
	float dist = 2 * distance(*prevx, *prevy, *x, *y);
	tabx[2] = *x + (disty * road->size * cam->zoom) / dist;
	taby[2] = *y - (distx * road->size * cam->zoom) / dist;
	tabx[3] = *x - (disty * road->size * cam->zoom) / dist;
	taby[3] = *y + (distx * road->size * cam->zoom) / dist;

    /*
   int row;
   int column;
    // si dans la fenêtre
	if (tabx[2] >= 0. && tabx[2] <= cam->winSize_w && taby[2] >= 0. && taby[2] <= cam->winSize_h){
        row = NB_GRID_ROW * taby[2] / (float)cam->winSize_h;
        column = NB_GRID_COLUMN * tabx[2] / (float)cam->winSize_w;
        fill_collision_grid(road, row, column, &tabx[2], &taby[2]);
        // left of the road
        road->edge_right[row][column] = false;
	}

    // si dans la fenêtre
	if (tabx[3] >= 0. && tabx[3] <= cam->winSize_w && taby[3] >= 0. && taby[3] <= cam->winSize_h){
        row = NB_GRID_ROW * taby[3] / (float)cam->winSize_h;
        column = NB_GRID_COLUMN * tabx[3] / (float)cam->winSize_w;
        fill_collision_grid(road, row, column, &tabx[3], &taby[3]);
        road->edge_right[row][column] = true;
	}
     */
}

static int max (int a, int b) {
    return a < b ? b : a;
}

static void fill_road(SDL_Renderer* renderer, float tabx[4], float taby[4]) {
    // TODO : remplacer le 10 par la distance
    int n = max((int)distance(tabx[0], taby[0], tabx[2], taby[2]), (int)distance(tabx[1], taby[1], tabx[3], taby[3])) + 1;
    for (int i = 0; i < n; ++i) {
        SDL_RenderDrawLine(renderer, (int)((n - i) * tabx[0] + i *tabx[2]) / n, (int)((n - i) * taby[0] + i * taby[2]) / n, (int)((n - i) * tabx[1] + i * tabx[3]) / n, (int)((n - i) * taby[1] + i * taby[3]) / n);
        SDL_RenderDrawLine(renderer, (int)((n - i) * tabx[0] + i *tabx[2]) / n + 1, (int)((n - i) * taby[0] + i * taby[2]) / n, (int)((n - i) * tabx[1] + i * tabx[3]) / n + 1, (int)((n - i) * taby[1] + i * taby[3]) / n);
    }
}

static void render_road(Entity* car, SDL_Renderer *renderer, Camera* cam, Road* road, Background* bg){
	// Draw Spline
	float x, y;
	x = 0.;
	y = 0.;
	float prevx = 0, prevy = 0;
	float tabx[4] = {0., 0.};
    float taby[4] = {0., 0.};
    short draw = 0;
    float t;
    t = 0;

    if (bg->show)
        SDL_SetRenderDrawColor(renderer, LINE_ROAD_COLOR);
    else
        SDL_SetRenderDrawColor(renderer, BLACK);

    while (t < (float)road->len_tab_checkPoints - 3.0f * (road->len_tab_checkPoints <= 3)){
        calcul_spline(car, cam, road, &x, &y, &t, &draw);

        if (x  + road->size * cam->zoom / 2 >= 0 && x < cam->winSize_w  + road->size * cam->zoom / 2 && y + road->size * cam->zoom / 2 >= 0 && y < cam->winSize_h + road->size * cam->zoom / 2){
            //SDL_RenderDrawPoint(renderer, (int)x, (int)y);//original spline
            calcul_road(cam, road, &x, &y, &prevx, &prevy, tabx, taby);//calcul 2 points of the road
            //display road:
            //test
            //SDL_RenderDrawPoint(renderer, (int)tabx[2], (int)taby[2]);//original road
            //SDL_RenderDrawPoint(renderer, (int)tabx[3], (int)taby[3]);
            //SDL_RenderDrawLine(renderer, (int)tabx[2], (int)taby[2], (int)tabx[3], (int)taby[3]);
            if (draw == 1){
                // TODO SDL_RenderDrawLines plus opti
                //fill
                if (!bg->show) {
                    SDL_RenderDrawLine(renderer, (int) tabx[0], (int) taby[0], (int) tabx[2], (int) taby[2]);
                    SDL_RenderDrawLine(renderer, (int) tabx[1], (int) taby[1], (int) tabx[3], (int) taby[3]);
                    //SDL_RenderDrawLine(renderer, (int)prevx, (int)prevy, (int)x, (int)y);
                } else {
                    fill_road(renderer, tabx, taby);
                }
            }
            prevx = x; prevy = y;

            //end
            tabx[0] = tabx[2]; taby[0] = taby[2];
            tabx[1] = tabx[3]; taby[1] = taby[3];
        } else {
            draw = 0;
        }
        t += 1. / (NB_PTS * cam->zoom);
    }
}


void display(SDL_Renderer *renderer, Player* player, Road* road, Camera* cam, Toolbar* toolbar, Background* bg, int nb_fps){
	//____background display_____

	//____spline display____
	render_road(&player[0].car, renderer, cam, road, bg);

	//____road display____
	render_checkPoints(renderer, road, cam, player);

	//_____drift display_____
//    for (int i = 0; i < NB_OF_PLAYERS; i++) {
        render_drift(renderer, &player[0].car, cam);
//    }

	//_____toolbar display___
	render_toolbar(renderer, toolbar);

    //_____fps display ______
    render_number(renderer, bg, nb_fps, cam->winSize_w + toolbar->size.w - 5, 5);

	//_____key display_______
	render_keys(renderer, &player[0].key, cam);

	//default color background
    if (bg->show) {
        SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR);
    } else {
        SDL_SetRenderDrawColor(renderer, WHITE);
    }
	//_____car display____
    render_car(renderer, player, cam);
	SDL_RenderPresent(renderer);
}
