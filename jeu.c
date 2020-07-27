#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include "jeu.h"

#define pi 3.141592653589

#define noir 10, 10, 10, 255
#define orange 150, 50, 0, 255
#define vert 150, 255, 50, 255

#define acceleration 3.
#define frottement 3.
#define turn 10.
#define turn_drift 5.


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
void manage_skid_marks(struct Entity* car, struct Keys_pressed key)
{
	if (key.drift)
	{
		car->tab_skid_marks_x[car->pos_tab] = car->posx;
		car->tab_skid_marks_y[car->pos_tab] = car->posy;
		car->tab_skid_marks_angle[car->pos_tab] = car->angle + car->angle_drift;
		car->count_pos_tab += (int)(car->count_pos_tab < 400 - 1);
		car->pos_tab ++;
		car->pos_tab *= (int)(car->pos_tab < 400);
	}
}
void move_car(struct Entity *car, struct Keys_pressed* key, struct Camera* cam) 
{
	manage_skid_marks(car, *key);
	//keys_to_struct
	if (car->speed < 0.5 && key->drift)
	{
		key->drift = none;
		car->angle += car->angle_drift;
		car->angle_drift = 0.;
	}
	car->angle_drift += ((key->drift == left) - 2*(key->drift == right)) * turn_drift / 320.;
	car->speed += ((float)(key->up) - (float)(key->down)/2.) * acceleration / 20.;
	if (abs(car->speed) > 3.)
	{
		car->angle += (double)(turn *((double)(key->left) - (double)(key->right))) / (10 * (1. - 2. * (double)((car->speed) < 0.)) * 6 * sqrt(abs(car->speed)));
	}
	else if (abs(car->speed) <= 3.)
	{
		car->angle += (double)((double)(key->left) - (double)(key->right)) * (car->speed) * turn / 1280;
	}
	
	//struct_to_moveCar
	car->posx += car->speed * (float)cos(car->angle);
	car->posy -= car->speed * (float)sin(car->angle);
	car->frame.x = (int)(car->posx);
	car->frame.y = (int)(car->posy);
	car->speed += ((float)(((car->speed) < 0.) - ((car->speed) > 0.))) * (1. + (abs(car->speed))) * frottement / 640.;
	//manage cam
	float new_cam_x = car->posx - cam->winSize_w / 2 + 20*car->speed * cos(car->angle);
	float new_cam_y = car->posy - cam->winSize_h / 2 - 20*car->speed * sin(car->angle);
	cam->x = (int)((9.*(float)cam->x + new_cam_x) / 10.);
	cam->y = (int)((9.*(float)cam->y + new_cam_y) / 10.);
}

void manage_key(SDL_Event event, struct Keys_pressed* key, Bool stat, struct Entity* car)
{
	switch(event.key.keysym.sym)
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

		default:
			break;
	}
}

//add a checkpoint:
void add_checkPoint(struct Road* road, SDL_Event event, struct Camera cam)
{
	printf("add\n");
	if (road->long_tab_checkPoints < 1000)
	{
		road->tab_checkPoints[road->long_tab_checkPoints].x = event.button.x - road->square_width / 2 + cam.x;
	   	road->tab_checkPoints[road->long_tab_checkPoints].y = event.button.y - road->square_width / 2 + cam.y;
		road->tab_checkPoints[road->long_tab_checkPoints].w = road->square_width;
		road->tab_checkPoints[road->long_tab_checkPoints].h = road->square_width;	
	
	road->long_tab_checkPoints++;
	}
}
//del a checkpoint:
void del_checkPoint(struct Road road, SDL_Event event, struct Camera cam)
{
	printf("del\n"); 
}
//found the closest checkpoint to the clic:
int closest_checkpoint(struct Road road, SDL_Event event, struct Camera cam)
{
	printf("closest\n"); 
	return 0;
}
//manage a checkpoint:
void manage_checkpoint(struct Road road, SDL_Event event, struct Camera cam)
{
	printf("manage\n"); 
	int i;
	i = closest_checkpoint(road, event, cam);
}


void clear(SDL_Renderer *renderer)
{
	SDL_RenderClear(renderer);
}

void render_car(SDL_Renderer *renderer, struct Entity* car, struct Camera cam)
{
	SDL_Rect src;
	src.x = 0;
	src.y = 0;
	src.w = 641;
	src.h = 258;

	SDL_Point center;
	center.x = car->frame.w/2;
	center.y = car->frame.h/2;

	double angle = car->angle + car->angle_drift;
	car->frame.x -= cam.x;
	car->frame.y -= cam.y;
	SDL_RenderCopyEx(renderer, car->tex, &src, &car->frame, 360. * (1. - angle / (2. * pi)), &center, SDL_FLIP_NONE);
	car->frame.x += cam.x;
	car->frame.y += cam.y;
}

void render_road(SDL_Renderer *renderer, struct Road* road, struct Camera cam)
{
	SDL_SetRenderDrawColor(renderer, orange);//drift's black pixel
	int i;
	for (i = 0; i < road->long_tab_checkPoints; i++)
	{
		road->tab_checkPoints[i].x -= cam.x;
		road->tab_checkPoints[i].y -= cam.y;
		SDL_RenderDrawRect(renderer, &road->tab_checkPoints[i]);
		road->tab_checkPoints[i].x += cam.x;
		road->tab_checkPoints[i].y += cam.y;
	}	
}
void render_drift(SDL_Renderer *renderer, struct Entity car, struct Camera cam)
{
	int w, h;
	w = car.frame.w;
	h = car.frame.h;
	float dist;
	dist = (sqrt(pow(w/2.25, 2) + pow(h/3, 2)));
	int x1, x2, x3, x4, y1, y2, y3, y4;

	SDL_SetRenderDrawColor(renderer, noir);//drift's black pixel
	
	int centre_x, centre_y;
	double angle;
	unsigned int i;
	for (i = 0; i < car.count_pos_tab; i++)
	{
		centre_x = car.tab_skid_marks_x[i] + w / 2;
		centre_y = car.tab_skid_marks_y[i] + h / 2;
		angle = car.tab_skid_marks_angle[i];

		x1 = (int)(centre_x - dist * cos(angle - 0.2));
		y1 = (int)(centre_y + dist * sin(angle - 0.2));
		x2 = (int)(centre_x - dist * cos(angle - 0.23));
		y2 = (int)(centre_y + dist * sin(angle - 0.23));
		x3 = (int)(centre_x - dist * cos(angle + 0.4));
		y3 = (int)(centre_y + dist * sin(angle + 0.4));
		x4 = (int)(centre_x - dist * cos(angle + 0.43));
		y4 = (int)(centre_y + dist * sin(angle + 0.43));
		if (x1 > car.posx - cam.winSize_w / 2 && x1 < car.posx + cam.winSize_w / 2 && y1 > car.posy - cam.winSize_h / 2 && y1 < car.posy + cam.winSize_h / 2)
		{
			SDL_RenderDrawPoint(renderer, x1 - cam.x, y1 - cam.y);
			SDL_RenderDrawPoint(renderer, x2 - cam.x, y2 - cam.y);
			SDL_RenderDrawPoint(renderer, x3 - cam.x, y3 - cam.y);
			SDL_RenderDrawPoint(renderer, x4 - cam.x, y4 - cam.y);
		}
	}


}

void display(SDL_Renderer *renderer, struct Entity car, struct Road road, struct Camera cam)
{
	//____road display____
	render_road(renderer, &road, cam);

	//_____drift display_____
	render_drift(renderer, car, cam);
	
	SDL_SetRenderDrawColor(renderer, vert);//drift's black pixel
	//_____car display____
	render_car(renderer, &car, cam);
	SDL_RenderPresent(renderer); 
}

