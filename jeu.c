#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include "jeu.h"

#define PI 3.141592653589

#define BLACK 0, 0, 0, 255
#define ORANGE 150, 50, 0, 255
#define RED 100, 0, 0, 255
#define GREEN 50, 100, 0, 255
#define WHITE 255, 200, 175, 255
#define YELLOW 200, 150, 0, 255

#define NB_PIX_DRIFT 800
#define NB_SQUARE 400

#define ACCELERATION 10.
#define FROTTEMENT 8.
#define TURN 8.
#define TURN_DRIFT 7.
#define REAR_CAMERA 20.

#define NB_PTS 50.

int init(SDL_Window **window, SDL_Renderer **renderer, int w, int h)
{
    if(0 != SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "Erreur SDL_Init : %s", SDL_GetError());
        return -1;
    }
	if (!(IMG_Init(IMG_INIT_PNG)))
	{
		fprintf(stderr, "Erreur IMG_Init : %s", SDL_GetError());
        return -1;
	}
	if(0 != SDL_CreateWindowAndRenderer(w, h, SDL_WINDOW_MAXIMIZED, window, renderer))
    {
        fprintf(stderr, "Erreur SDL_CreateWindowAndRenderer : %s", SDL_GetError());
        return -1;
    }
    return 0;
}

int setWindowColor(SDL_Renderer *renderer, SDL_Color color)
{
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

SDL_Texture* loadTexture(SDL_Renderer *renderer, const char* p_filePath)
{
	SDL_Texture* texture = NULL;
	texture = IMG_LoadTexture(renderer, p_filePath);

	if (texture == NULL)
        fprintf(stderr, "Failed to load texture. Error: %s", SDL_GetError());
	return texture;
}

float distance(float x1, float y1, float x2, float y2)
{
	return sqrt(pow((float)x1 - (float)x2, 2) + pow(((float)y1 - (float)y2), 2));
}

void manage_skid_marks(struct Entity* car, struct Keys_pressed* key)
{
	if (key->drift)
	{
		car->tab_skid_marks_x[car->pos_tab] = car->posx;
		car->tab_skid_marks_y[car->pos_tab] = car->posy;
		car->tab_skid_marks_angle[car->pos_tab] = car->angle + car->angle_drift;
		car->count_pos_tab += (int)(car->count_pos_tab < NB_PIX_DRIFT - 1);
		car->pos_tab ++;
		car->pos_tab *= (int)(car->pos_tab < NB_PIX_DRIFT);
	}
}

void move_car(struct Entity *car, struct Keys_pressed* key, struct Camera* cam) 
{
	manage_skid_marks(car, key);
	//keys_to_struct
	if (car->speed < 0.5 && key->drift)
	{
		key->drift = none;
		car->angle += car->angle_drift;
		car->angle_drift = 0.;
	}
	car->angle_drift += ((key->drift == left) - 2*(key->drift == right)) * TURN_DRIFT / 320.;
	car->speed += ((float)(key->up) - (float)(key->down)/2.) * ACCELERATION / 20.;
	if (abs(car->speed) > 3.)
	{
		car->angle += (double)(TURN *((double)(key->left) - (double)(key->right))) / (10 * (1. - 2. * (double)((car->speed) < 0.)) * 6 * sqrt(abs(car->speed)));
	}
	else if (abs(car->speed) <= 3.)
	{
		car->angle += (double)((double)(key->left) - (double)(key->right)) * (car->speed) * TURN / 1280;
	}
	
	//struct_to_moveCar
	car->posx += car->speed * (float)cos(car->angle);
	car->posy -= car->speed * (float)sin(car->angle);
	car->frame.x = (int)(car->posx);
	car->frame.y = (int)(car->posy);
	car->speed += ((float)(((car->speed) < 0.) - ((car->speed) > 0.))) * (1. + (abs(car->speed))) * FROTTEMENT / 640.;
	//manage cam
	float new_cam_x = car->posx - cam->winSize_w / 2 + REAR_CAMERA * car->speed * cos(car->angle);
	float new_cam_y = car->posy - cam->winSize_h / 2 - REAR_CAMERA * car->speed * sin(car->angle);
	cam->x = (int)((9.*(float)cam->x + new_cam_x) / 10.);
	cam->y = (int)((9.*(float)cam->y + new_cam_y) / 10.);
}

void manage_key(SDL_Event* event, struct Keys_pressed* key, Bool stat, struct Entity* car, struct Camera* cam)
{
	switch(event->key.keysym.sym)
	{
		case SDLK_UP:
			key->up = stat;
			if (key->drift && stat)
			{
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
				key->drift = left * (key->left) + right * (key->right);
			}
			break;
		case SDLK_RIGHT:
			key->right = stat;
			if (key->drift == right && stat == False)
			{
				key->drift = none;
				car->angle += car->angle_drift;
				car->angle_drift = 0.;
			}
			break;
		case SDLK_LEFT:
			key->left = stat;
			if (key->drift == left && stat == False)
			{
				key->drift = none;
				car->angle += car->angle_drift;
				car->angle_drift = 0.;
			}
			break;
		case SDLK_ESCAPE:
			car->posx = car->pos_initx;
			car->posy = car->pos_inity;
			break;
		case SDLK_p:
			cam->zoom *= 1.1;
			break;
		case SDLK_o:
			cam->zoom /= 1.1;
			break;

		default:
			break;
	}
}

//add a checkpoint:
void add_checkPoint(struct Road* road, SDL_Event* event, struct Camera* cam, struct Entity* car){
	if (road->long_tab_checkPoints < NB_SQUARE)
	{
		road->tab_checkPoints[road->long_tab_checkPoints].x = event->button.x - road->square_width / 2 + cam->x + (event->button.x + cam->x - car->frame.x) * (float)(-1. + 1/cam->zoom);
	   	road->tab_checkPoints[road->long_tab_checkPoints].y = event->button.y - road->square_width / 2 + cam->y + (event->button.y + cam->y - car->frame.y) * (float)(-1. + 1/cam->zoom);
		road->tab_checkPoints[road->long_tab_checkPoints].w = road->square_width;
		road->tab_checkPoints[road->long_tab_checkPoints].h = road->square_width;	
		road->tab_valid_checkPoints[road->long_tab_checkPoints] = False;
		startLapTime = SDL_GetTicks();

		//printf("%d	%d		\n", road->tab_checkPoints[road->long_tab_checkPoints].x, road->tab_checkPoints[road->long_tab_checkPoints].y);
	
	road->long_tab_checkPoints++;
	}
}
//found the closest checkpoint to the clic:
void closest_checkpoint(struct Road* road, SDL_Event* event, struct Camera* cam, struct Entity* car){
	int dist = 0;
	int pos_clique_x = event->button.x - road->square_width / 2 + cam->x + (event->button.x + cam->x - car->frame.x) * (float)(-1. + 1/cam->zoom);
	int pos_clique_y = event->button.y - road->square_width / 2 + cam->y + (event->button.y + cam->y - car->frame.y) * (float)(-1. + 1/cam->zoom);
	road->num_clos_check = 0;
	int min_dist = distance((float)pos_clique_x, (float)pos_clique_y, (float)road->tab_checkPoints[0].x, (float)road->tab_checkPoints[0].y);
	int i;
	for (i = 1; i < road->long_tab_checkPoints; i++)
	{
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

void reset_valid_tab(struct Road* road){
	int i;
	for (i = 0; i < road->long_tab_checkPoints; i++){
		road->tab_valid_checkPoints[i] = 0;
	}
	road->nb_valid_checkPoints = 0;
	if (road->long_tab_checkPoints >= 4){
		printf("Lap time: %.2f\n", ((float)SDL_GetTicks() - (float)startLapTime) / 1000.);
		startLapTime = SDL_GetTicks();
	}
}	

//manage a checkpoint:
void manage_checkpoint(struct Road* road, SDL_Event* event, struct Camera* cam, struct Entity* car){
	road->select = True;
	closest_checkpoint(road, event, cam, car);
}

//del a checkpoint:
void del_checkPoint(struct Road* road, SDL_Event* event, struct Camera* cam, struct Entity* car){
	closest_checkpoint(road, event, cam, car);
	int i;
	if (road->tab_valid_checkPoints[road->num_clos_check] != False){
		road->nb_valid_checkPoints--;
		if (road->tab_valid_checkPoints[road->num_clos_check] == Start){
			reset_valid_tab(road);
		}
	}
	for(i = road->num_clos_check; i < road->long_tab_checkPoints - 1; i++)
	{
		road->tab_checkPoints[i] = road->tab_checkPoints[i+1];
		road->tab_valid_checkPoints[i] = road->tab_valid_checkPoints[i+1];
	}
	road->long_tab_checkPoints--;
}

void clear(SDL_Renderer *renderer)
{
	SDL_RenderClear(renderer);
}

void render_car(SDL_Renderer *renderer, struct Entity* car, struct Camera* cam)
{
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

void render_checkPoints(SDL_Renderer *renderer, struct Road* road, struct Camera* cam, struct Entity* car, SDL_Event* event){
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
	if (event->button.x < 20000 && event->button.y < 20000 && road->select){
		int pos_clique_x = event->button.x - road->square_width / 2 + cam->x + (event->button.x + cam->x - car->frame.x) * (float)(-1. + 1/cam->zoom);
		int pos_clique_y = event->button.y - road->square_width / 2 + cam->y + (event->button.y + cam->y - car->frame.y) * (float)(-1. + 1/cam->zoom);
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
		if (x + square_w > 0 && x - square_w < cam->winSize_w && y + square_w > 0 && y - square_w < cam->winSize_h)
		{
			//check if collision:
			
			if (road->long_tab_checkPoints > 1 && (distance(x_prev, y_prev, car->posx, car->posy) <= road->size / 2 + car->frame.h / 2)){
				
				if (road->tab_valid_checkPoints[i] == False){
					road->nb_valid_checkPoints++;
					road->tab_valid_checkPoints[i] = 1 + (road->nb_valid_checkPoints == 1); //1 == True; 2 == Start
				}else if (road->nb_valid_checkPoints >= road->long_tab_checkPoints && road->tab_valid_checkPoints[i] == Start){
					road->nb_valid_checkPoints = 0;
					reset_valid_tab(road);
				}
			}	
			//display:
			if (road->select && road->num_clos_check == i)
			{
				SDL_SetRenderDrawColor(renderer, RED);
				SDL_RenderFillRect(renderer, &road->tab_checkPoints[i]);
			}
			else
			{
				if (road->tab_valid_checkPoints[i] == True){
					SDL_SetRenderDrawColor(renderer, GREEN);
				}else if (road->tab_valid_checkPoints[i] == False){
					SDL_SetRenderDrawColor(renderer, ORANGE);
				}else{
					SDL_SetRenderDrawColor(renderer, YELLOW);
				}
				SDL_RenderDrawRect(renderer, &road->tab_checkPoints[i]);
			}
		}
		road->tab_checkPoints[i].h = h_prev;
		road->tab_checkPoints[i].w = w_prev;
		road->tab_checkPoints[i].x = x_prev;
		road->tab_checkPoints[i].y = y_prev;
	}	
}

void render_drift(SDL_Renderer *renderer, struct Entity* car, struct Camera* cam){
	int w, h;
	w = car->frame.w * cam->zoom;
	h = car->frame.h * cam->zoom;
	float dist;
	dist = (sqrt(pow(w/2.5, 2) + pow(h/3, 2)));
	
	int x[4] = {0};
	int y[4] = {0};

	SDL_SetRenderDrawColor(renderer, BLACK);//drift's black pixel
	
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
void calcul_spline(struct Entity* car, struct Camera* cam, struct Road* road, float* x, float* y, float* pt, short* draw){
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
	
	*x -= cam->x - road->square_width / 2;
	*x = (1 - cam->zoom) * centre_x + cam->zoom * *x;
	*y -= cam->y - road->square_width / 2;
	*y = (1 - cam->zoom) * centre_y + cam->zoom * *y;
}

void calcul_road(struct Camera* cam, struct Road* road, float* x, float* y, float* prevx, float* prevy, float* tabx, float* taby){
	float distx = *prevx - *x;
	float disty = *prevy - *y;
	float dist = 2 * distance(*prevx, *prevy, *x, *y);
	tabx[2] = *x + (disty * road->size * cam->zoom) / dist;
	taby[2] = *y - (distx * road->size * cam->zoom) / dist;			
	tabx[3] = *x - (disty * road->size * cam->zoom) / dist;
	taby[3] = *y + (distx * road->size * cam->zoom) / dist;
}	
void render_road(struct Entity* car, SDL_Renderer *renderer, struct Camera* cam, struct Road* road){
	// Draw Spline
	float t;
	float x, y;
	float prevx = 0, prevy = 0;
	float tabx[4] = {0., 0.};
	float taby[4] = {0., 0.};
	short draw = 0;
	SDL_SetRenderDrawColor(renderer, BLACK);
	for (t = 0; t < (float)road->long_tab_checkPoints - 3.0f * (road->long_tab_checkPoints <= 3); t += 1. / (NB_PTS * cam->zoom)){
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

	}
}

void display(SDL_Renderer *renderer, struct Entity* car, struct Road* road, struct Camera* cam, SDL_Event* event){
	//____spline display____
	render_road(car, renderer, cam, road);
	//____road display____
	render_checkPoints(renderer, road, cam, car, event);

	//_____drift display_____
	render_drift(renderer, car, cam);
	
	SDL_SetRenderDrawColor(renderer, WHITE);//drift's black pixel
	//_____car display____
	render_car(renderer, car, cam);
	SDL_RenderPresent(renderer); 
}

