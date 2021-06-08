/*jeu.c*/

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <math.h>
#include "jeu.h"
#include "ia.h"
#include "background.h"

static unsigned int startLapTime;

int init(SDL_Window **window, SDL_Renderer **renderer, int w, int h){
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

int setWindowColor(SDL_Renderer *renderer, SDL_Color color){
	if(SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a) < 0){
		printf("Erreur SDL_SetRenderDrawColor : %s", SDL_GetError());
        return -1;
	}
    if(SDL_RenderClear(renderer) < 0){
		printf("Erreur SDL_SetRenderDrawColor : %s", SDL_GetError());
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

void init_car(Entity* car, SDL_Renderer *renderer){
	car->speed = 0.;//pixels per frame
	car->angle = 0.;
	car->angle_drift = 0.;
	car->pos_initx = 700.;
	car->pos_inity = 700.;
	car->posx = car->pos_initx;
	car->posy = car->pos_inity;
	car->frame.x = (int)(car->posx);
	car->frame.y = (int)(car->posy);
	car->frame.w = 128;
	car->frame.h = 52;
	car->tex = loadTexture(renderer, "image/car.png");//car image
	car->pos_tab = 0;
	car->count_pos_tab = 0;
	car->acceleration = ACCELERATION;
	car->frottement = FROTTEMENT;
	car->turn = TURN;
	car->turn_drift = TURN_DRIFT;
}

float distance(float x1, float y1, float x2, float y2){
	return sqrt(pow((float)x1 - (float)x2, 2) + pow(((float)y1 - (float)y2), 2));
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

void move_car(Entity* car, Keys_pressed* key, Camera* cam) {
	manage_skid_marks(car, key);
	//keys_to_struct
	if (car->speed < 0.5 && key->drift){
		key->drift = none;
		car->angle += car->angle_drift;
		car->angle_drift = 0.;
	}
	car->angle_drift += ((key->drift == drift_left) - 2*(key->drift == drift_right)) * car->turn_drift * (60. / FRAMES_PER_SECONDE) / 320.;
	car->speed += ((float)(key->up) - (float)(key->down)/2.) * car->acceleration * (60. / FRAMES_PER_SECONDE) / 20.;
	if (fabs(car->speed) > 3.){
		car->angle += (double)(car->turn * (60. / FRAMES_PER_SECONDE) * ((double)(key->left) - (double)(key->right))) / (10 * (1. - 2. * (double)((car->speed) < 0.)) * 6 * sqrt(fabs(car->speed)));
	}
	else if (fabs(car->speed) <= 3.){
		car->angle += (double)((double)(key->left) - (double)(key->right)) * (car->speed) * car->turn * (60. / FRAMES_PER_SECONDE) / 1280;
	}
	
	//struct_to_moveCar
	car->posx += car->speed * (float)cos(car->angle);
	car->posy -= car->speed * (float)sin(car->angle);
	car->frame.x = (int)(car->posx);
	car->frame.y = (int)(car->posy);
	car->speed += ((float)(((car->speed) < 0.) - ((car->speed) > 0.))) * (1. + (fabs(car->speed))) * car->frottement / 640.;
	//manage cam
	float new_cam_x = car->posx - (float)cam->winSize_w / 2 + REAR_CAMERA * car->speed * ( FRAMES_PER_SECONDE / 60. ) * cos(car->angle);
	float new_cam_y = car->posy - (float)cam->winSize_h / 2 - REAR_CAMERA * car->speed * ( FRAMES_PER_SECONDE / 60. ) * sin(car->angle);
	cam->x = (int)((9.*(float)cam->x + new_cam_x) / 10.);
	cam->y = (int)((9.*(float)cam->y + new_cam_y) / 10.);
}

void manage_key(SDL_Event* event, Keys_pressed* key, Bool stat, Entity* car, Camera* cam, Road* road, Toolbar* toolbar){
	short add_to_var;
	add_to_var = 1;
	switch(event->key.keysym.sym){
		case SDLK_UP:
			key->up = stat;
			if (key->drift && stat){
				key->drift = none;
				car->angle += car->angle_drift;
				car->angle_drift = 0.;
			}
			break;
		case SDLK_LCTRL:
		case SDLK_DOWN:
			key->down = stat;
			if (stat == True && (key->left ^ key->right) && key->up == False && car->speed > 1.)
			{
				key->drift = drift_left * (key->left) + drift_right * (key->right);
			}
			break;
		case SDLK_RIGHT:
			key->right = stat;
			if (key->drift == drift_right && stat == False)
			{
				key->drift = none;
				car->angle += car->angle_drift;
				car->angle_drift = 0.;
			}
			break;
		case SDLK_LEFT:
			key->left = stat;
			if (key->drift == drift_left && stat == False)
			{
				key->drift = none;
				car->angle += car->angle_drift;
				car->angle_drift = 0.;
			}
			break;
		case SDLK_ESCAPE:
			car->posx = car->pos_initx;
			car->posy = car->pos_inity;
			car->speed = 0.;
			reset_valid_tab(road);
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
			if (toolbar->select_var){
				change_variable_keys(toolbar, add_to_var);
			}
			break;
		case SDLK_KP_MINUS:
		case SDLK_MINUS:
		case SDLK_KP_LESS:
		case SDLK_LESS:
		case SDLK_6:
			if (toolbar->select_var){
				change_variable_keys(toolbar, -add_to_var);
			}
			break;



		default:
			break;
	}
}

//add a checkpoint:
void add_checkPoint(Road* road, SDL_Event* event, Camera* cam, Entity* car){
	if (road->long_tab_checkPoints < NB_SQUARE){
		road->tab_checkPoints[road->long_tab_checkPoints].x = event->button.x - (float)road->square_width / 2 + cam->x + (event->button.x + cam->x - car->frame.x) * (float)(-1. + 1/cam->zoom);
	   	road->tab_checkPoints[road->long_tab_checkPoints].y = event->button.y - (float)road->square_width / 2 + cam->y + (event->button.y + cam->y - car->frame.y) * (float)(-1. + 1/cam->zoom);
		road->tab_checkPoints[road->long_tab_checkPoints].w = road->square_width;
		road->tab_checkPoints[road->long_tab_checkPoints].h = road->square_width;	
		road->tab_valid_checkPoints[road->long_tab_checkPoints] = False;
		startLapTime = SDL_GetTicks();

		//printf("%d	%d		\n", road->tab_checkPoints[road->long_tab_checkPoints].x, road->tab_checkPoints[road->long_tab_checkPoints].y);
	
	road->long_tab_checkPoints++;
	}
}

//found the closest checkpoint to the clic:
void closest_checkpoint(Road* road, SDL_Event* event, Camera* cam, Entity* car){
	int dist = 0;
	int pos_clique_x = event->button.x - (float)road->square_width / 2 + cam->x + (event->button.x + cam->x - car->frame.x) * (float)(-1. + 1/cam->zoom);
	int pos_clique_y = event->button.y - (float)road->square_width / 2 + cam->y + (event->button.y + cam->y - car->frame.y) * (float)(-1. + 1/cam->zoom);
	road->num_clos_check = 0;
	int min_dist = distance((float)pos_clique_x, (float)pos_clique_y, (float)road->tab_checkPoints[0].x, (float)road->tab_checkPoints[0].y);
	int i;
	for (i = 1; i < road->long_tab_checkPoints; i++){
		dist = distance((float)pos_clique_x, (float)pos_clique_y, (float)road->tab_checkPoints[i].x, (float)road->tab_checkPoints[i].y);
		if (dist < min_dist)
		{
			road->num_clos_check = i;
			min_dist = dist;
		}
	}	 
	road->selectx = road->tab_checkPoints[road->num_clos_check].x - pos_clique_x;
	road->selecty = road->tab_checkPoints[road->num_clos_check].y - pos_clique_y;
}

void reset_valid_tab(Road* road){
	if (road->long_tab_checkPoints >= 4 && road->nb_valid_checkPoints == road->long_tab_checkPoints){
		printf("Lap time: %.2f\n", ((float)SDL_GetTicks() - (float)startLapTime) / 1000.);
		//startLapTime = SDL_GetTicks();
	}
	int i;
	for (i = 0; i < road->long_tab_checkPoints; i++){
		road->tab_valid_checkPoints[i] = 0;
	}
	road->nb_valid_checkPoints = 0;
}	

//manage a checkpoint:
void manage_checkpoint(Road* road, SDL_Event* event, Camera* cam, Entity* car){
	road->select = True;
	closest_checkpoint(road, event, cam, car);
}

//del a checkpoint:
void del_checkPoint(Road* road, SDL_Event* event, Camera* cam, Entity* car){
	closest_checkpoint(road, event, cam, car);
	int i;
	if (road->tab_valid_checkPoints[road->num_clos_check] != False){
		road->nb_valid_checkPoints--;
		if (road->tab_valid_checkPoints[road->num_clos_check] == Start){
			reset_valid_tab(road);
		}
	}
	for(i = road->num_clos_check; i < road->long_tab_checkPoints - 1; i++){
		road->tab_checkPoints[i] = road->tab_checkPoints[i+1];
		road->tab_valid_checkPoints[i] = road->tab_valid_checkPoints[i+1];
	}
	road->long_tab_checkPoints--;
}

void clear(SDL_Renderer *renderer){
	SDL_RenderClear(renderer);
}

void render_car(SDL_Renderer *renderer, Entity* car, Camera* cam){
	SDL_Rect src;
	src.x = 0;
	src.y = 0;
	src.w = 641;
	src.h = 258;
	
	int w_prev = car->frame.w;
	int h_prev = car->frame.h;
	car->frame.w *= cam->zoom;
	car->frame.h *= cam->zoom;
	SDL_Point center;
	center.x = car->frame.w/2;
	center.y = car->frame.h/2;

	double angle = car->angle + car->angle_drift;
	car->frame.x -= cam->x;
	car->frame.y -= cam->y;
	SDL_RenderCopyEx(renderer, car->tex, &src, &car->frame, 360. * (1. - angle / (2. * PI)), &center, SDL_FLIP_NONE);
	car->frame.y += cam->y;
	car->frame.x += cam->x;
	car->frame.h = h_prev;
	car->frame.w = w_prev;
}

void render_checkPoints(SDL_Renderer *renderer, Road* road, Camera* cam, Entity* car, SDL_Event* event, Ia* ia){
	int i;
	int x;
	int y;
	int x_prev;
	int y_prev;
	int w_prev;
	int h_prev;
	int centre_x = car->frame.x - cam->x;
	int centre_y = car->frame.y - cam->y;
	int square_w = road->square_width * cam->zoom;
	if (road->select && event->button.x < 20000 && event->button.y < 20000){
		int pos_clique_x = event->button.x - (float)road->square_width / 2 + cam->x + (event->button.x + cam->x - car->frame.x) * (float)(-1. + 1/cam->zoom);
		int pos_clique_y = event->button.y - (float)road->square_width / 2 + cam->y + (event->button.y + cam->y - car->frame.y) * (float)(-1. + 1/cam->zoom);
		road->tab_checkPoints[road->num_clos_check].x = pos_clique_x + road->selectx;
		road->tab_checkPoints[road->num_clos_check].y = pos_clique_y + road->selecty;
	}
	for (i = 0; i < road->long_tab_checkPoints; i++){
		x_prev = road->tab_checkPoints[i].x;
		y_prev = road->tab_checkPoints[i].y;
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
		if (x + square_w > 0 && x - square_w < cam->winSize_w && y + square_w > 0 && y - square_w < cam->winSize_h){
			//check if collision between car and CP:
			
			if (road->long_tab_checkPoints > 1 && (distance(x_prev, y_prev, car->posx, car->posy) <= (float)road->size / 2 + (float)car->frame.h / 2)){	
				if (road->tab_valid_checkPoints[i] == False){ // CP not already valid
					road->nb_valid_checkPoints++;
					road->tab_valid_checkPoints[i] = True;
					if (IA_MODE && road->long_tab_checkPoints >= 4){
						calcul_next_cp(road, ia);	
					}

					if (road->nb_valid_checkPoints == 1){//Start
						road->tab_valid_checkPoints[i] = Start;
						startLapTime = SDL_GetTicks();
					}
					
				}else if (road->nb_valid_checkPoints >= road->long_tab_checkPoints && road->tab_valid_checkPoints[i] == Start){
					reset_valid_tab(road);
				}
			}	
			//display: 
			if (road->select && road->num_clos_check == i){
				SDL_SetRenderDrawColor(renderer, CP_SELECTED_COLOR); // checkpoint selected
				SDL_RenderFillRect(renderer, &road->tab_checkPoints[i]);
			}
			else if (road->tab_valid_checkPoints[i] == True){
				SDL_SetRenderDrawColor(renderer, CP_TAKEN_COLOR); // checkpoint taken
				SDL_RenderDrawRect(renderer, &road->tab_checkPoints[i]);
			}
			else if (ia->mode >= 1 && ia->num_next_cp == i){
				SDL_SetRenderDrawColor(renderer, NEXT_CP_COLOR); // checkpoint taken
				SDL_RenderFillRect(renderer, &road->tab_checkPoints[i]);
			}else if (road->tab_valid_checkPoints[i] == False){
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

void render_drift(SDL_Renderer *renderer, Entity* car, Camera* cam){
	int w, h;
	w = car->frame.w * cam->zoom;
	h = car->frame.h * cam->zoom;
	float dist;
	dist = (sqrt(pow((float)w / 2.5, 2) + pow((float)h/3, 2)));
	
	int x[4] = {0};
	int y[4] = {0};

	SDL_SetRenderDrawColor(renderer, DRIFT_COLOR);//drift's black pixel
	
	int centre_x = car->frame.x + w / 2 - cam->x;
	int centre_y = car->frame.y + h / 2 - cam->y;
	
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

void calcul_spline(Entity* car, Camera* cam, Road* road, float* x, float* y, float* pt, short* draw){
	float t = *pt; 
	int centre_x = car->frame.x - cam->x;
	int centre_y = car->frame.y - cam->y;
	int p0, p1, p2, p3;	
	p0 = (int)t % road->long_tab_checkPoints;
	p1 = (p0 + 1) % road->long_tab_checkPoints;
	p2 = (p1 + 1) % road->long_tab_checkPoints;
	
	//if checkpoint not in the screen, go to next
	int s = road->size * cam->zoom / 2;
	int cx = road->tab_checkPoints[p1].x - cam->x;
	cx = (1 - cam->zoom) * centre_x + cam->zoom * cx;
	int cy = road->tab_checkPoints[p1].y - cam->y;
	cy = (1 - cam->zoom) * centre_y + cam->zoom * cy;
	int cx2 = road->tab_checkPoints[p2].x - cam->x;
	cx2 = (1 - cam->zoom) * centre_x + cam->zoom * cx2;
	int cy2 = road->tab_checkPoints[p2].y - cam->y;
	cy2 = (1 - cam->zoom) * centre_y + cam->zoom * cy2;
	if ((cx + s < 0 && cx2 + s < 0) || (cy + s < 0 && cy2 + s < 0) || (cx - s > cam->winSize_w && cx2 - s > cam->winSize_w) || (cy - s > cam->winSize_h && cy2 - s > cam->winSize_h)){
		*draw = 0;// then don't draw
		*pt = (int)t + 1;
		return;	
	}//end

	p3 = (p2 + 1) % road->long_tab_checkPoints;
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
}

void calcul_road(Camera* cam, Road* road, float* x, float* y, float* prevx, float* prevy, float* tabx, float* taby){
	float distx = *prevx - *x;
	float disty = *prevy - *y;
	float dist = 2 * distance(*prevx, *prevy, *x, *y);
	tabx[2] = *x + (disty * road->size * cam->zoom) / dist;
	taby[2] = *y - (distx * road->size * cam->zoom) / dist;
	tabx[3] = *x - (disty * road->size * cam->zoom) / dist;
	taby[3] = *y + (distx * road->size * cam->zoom) / dist;
}

void render_road(Entity* car, SDL_Renderer *renderer, Camera* cam, Road* road){
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
	SDL_SetRenderDrawColor(renderer, LINE_ROAD_COLOR);
	while (t < (float)road->long_tab_checkPoints - 3.0f * (road->long_tab_checkPoints <= 3)){
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
				//fill
				SDL_RenderDrawLine(renderer, (int)tabx[0], (int)taby[0], (int)tabx[2], (int)taby[2]);
				SDL_RenderDrawLine(renderer, (int)tabx[1], (int)taby[1], (int)tabx[3], (int)taby[3]);
				//SDL_RenderDrawLine(renderer, (int)prevx, (int)prevy, (int)x, (int)y);
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


void display(SDL_Renderer *renderer, Entity* car, Road* road, Camera* cam, SDL_Event* event, Ia* ia, Toolbar* toolbar){
	//____spline display____
	render_road(car, renderer, cam, road);

	//____road display____
	render_checkPoints(renderer, road, cam, car, event, ia);

	//_____drift display_____
	render_drift(renderer, car, cam);
	
	//_____toolbar display___
	render_toolbar(renderer, toolbar);

	//default color background
	SDL_SetRenderDrawColor(renderer, BACKGROUND_COLOR);
	//_____car display____
	render_car(renderer, car, cam);
	SDL_RenderPresent(renderer); 
}
