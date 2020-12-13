/*main.c*/

/*Jeu de course ayant 3 buts:
 * Jouer à un jeu de course
 * Voir comment fonctionne l'IA de la voiture avec des annimations
 * Créer un univers (3D ?) de façon procédurale, et pouvoir le visiter avec plusieurs caméras
 * Le tout parametrable à volonté, à tout moment
 *
 * Voir détails dans le fichier texte*/


#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include "jeu.h"
#include "ia.h"

#define FRAMES_PER_SECONDE 60


extern unsigned int startLapTime;

int main(void){
    int status = EXIT_FAILURE;

	//init du temps
	unsigned int lastTime, currentTime;
	lastTime = 0;
	currentTime = 0;
	//init SDL
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	if (init(&window, &renderer, 1851, 1050)){
			goto Quit;
		}


	struct Entity car;
	car.speed = 0.;//pixels per frame
	car.angle = 0.;
	car.angle_drift = 0.;
	car.pos_initx = 700.;
	car.pos_inity = 700.;
	car.posx = car.pos_initx;
	car.posy = car.pos_inity;
	car.frame.x = (int)(car.posx);
	car.frame.y = (int)(car.posy);
	car.frame.w = 128;
	car.frame.h = 52;
	car.tex = loadTexture(renderer, "image/car.png");//car image
	car.pos_tab = 0;
	car.count_pos_tab = 0;

	//init struct Camera;
	struct Camera cam;
	SDL_GetWindowSize(window, &(cam.winSize_w), &(cam.winSize_h));
	cam.winSize_w = 1851;
	cam.winSize_h = 1050;

	cam.x = car.pos_initx - cam.winSize_w / 2;
	cam.y = car.pos_inity - cam.winSize_h / 2;
	cam.zoom = 0.25;

	//init struct Road;
	struct Road road;
	road.long_tab_checkPoints = 0;
	road.nb_valid_checkPoints = 0;
	road.square_width = 40;
	road.select = False;
	road.size = 500;
	//init struct Keys_pressed;
	struct Keys_pressed* key = (struct Keys_pressed* )malloc(sizeof(struct Keys_pressed));
	if (!key){
		printf("Error dynamic allocation of key.");
		goto Quit;
	}
	key->up = False;
	key->down = False;
	key->left = False;
	key->right = False;
	key->drift = none;
	//init struct Ia;
	struct Ia ia;
	ia.next_cp.x = 0.;
	ia.next_cp.y = 0.;
	ia.num_next_cp = -1;

	//__________________Start________________
	unsigned int remaind_time;
	remaind_time = lastTime + 1000/FRAMES_PER_SECONDE - currentTime;
	remaind_time *= (remaind_time > 0);
	Bool gameRunning = True;

	SDL_Event event;
	while (gameRunning){
		//limited fps
		currentTime = SDL_GetTicks();
		SDL_Delay(remaind_time);
		while (SDL_PollEvent(&event))//events
		{
		   switch(event.type)
			{
				case SDL_QUIT:
					gameRunning = False;
					break;
				case SDL_KEYDOWN:
					manage_key(&event, key, True, &car, &cam, &road);
					break;
				case SDL_KEYUP:
					manage_key(&event, key, False, &car, &cam, &road);
					break;
				case SDL_MOUSEBUTTONDOWN://clique souris
					switch(event.button.button)
					{
						case SDL_BUTTON_LEFT:
							add_checkPoint(&road, &event, &cam, &car);
							break;
						case SDL_BUTTON_MIDDLE:
							if (road.long_tab_checkPoints > 0)
								del_checkPoint(&road, &event, &cam, &car);
							break;
						case SDL_BUTTON_RIGHT:
							if (road.long_tab_checkPoints)// if it exist at least 1 checkpoint
							{
								manage_checkpoint(&road, &event, &cam, &car);
							}
							break;

						default:
							break;
					}
					break;
				case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_RIGHT)
					{
							road.select = False;
					}
					break;
				default:
					break;
			}
		}

		move_car(&car, key, &cam);
		clear(renderer);
		display(renderer, &car, &road, &cam, &event, &ia);
		//printf("%d\n", (int)car.speed);
		lastTime = currentTime;
		//printf("%ld			\r", SDL_GetPerformanceCounter());	//performances
	}
    status = EXIT_SUCCESS;
Quit:
	if(NULL != key)
		free(key);
    if(NULL != renderer)
        SDL_DestroyRenderer(renderer);
    if(NULL != window)
        SDL_DestroyWindow(window);
    SDL_Quit();
    return status;
}
