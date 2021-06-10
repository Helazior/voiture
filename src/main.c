/*main.c*/

/*Jeu de course ayant 3 buts:
 * Jouer à un jeu de course
 * Voir comment fonctionne l'IA de la voiture avec des annimations
 * Créer un univers (3D ?) de façon procédurale, et pouvoir le visiter avec plusieurs caméras
 * Le tout parametrable à volonté, à tout moment
 *
 * Voir détails dans le fichier texte*/


#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <math.h>
#include "../include/jeu.h"
#include "../include/ia.h"
#include "../include/background.h"

#define ZOOM_INIT 0.25

extern unsigned int startLapTime;

int main(void){
    int status = EXIT_FAILURE;

	//init time
	unsigned int lastTime;
	lastTime = SDL_GetTicks();
	//init SDL
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	if (init(&window, &renderer, 1851, 1050)){
			goto Quit;
		}
	

	Entity car;
	init_car(&car, renderer);

	//init struct Camera;
	Camera cam;
	SDL_GetRendererOutputSize(renderer, &(cam.winSize_w), &(cam.winSize_h));

	cam.x = (int)car.pos_initx - cam.winSize_w / 2;
	cam.y = (int)car.pos_inity - cam.winSize_h / 2;
	cam.zoom = ZOOM_INIT;

	//init struct Road;
	Road road = {
		.len_tab_checkPoints = 0,
		.nb_valid_checkPoints = 0,
		.square_width = 40,
		.select = False,
		.size = 500
	};
	//init struct Keys_pressed;
	Keys_pressed key = {
		.up = False,
		.down = False,
		.left = False,
		.right = False,
		.drift = none
	};

	//init struct Ia;
	Ia ia = {
		.active = IA_ACTIVE,
		.next_cp.x = 0.,
		.next_cp.y = 0.,
		.num_next_cp = -1
	};

	//init struct Toolbar;
	Toolbar toolbar;
	if (init_toolbar(&toolbar, renderer, &car, &road, &ia) == EXIT_FAILURE)
		goto Quit;
	

	//__________________Start________________
	int remain_time;
	Bool gameRunning = True;

	SDL_Event event;
	while (gameRunning){
		//limited fps
		/*printf("%d\n", SDL_GetTicks() - lastTime);*/
		remain_time = (int)(1000./FRAMES_PER_SECONDE + 0.5) + lastTime - SDL_GetTicks();
		remain_time *= (int)(remain_time > 0);
		SDL_Delay(remain_time); // wait*/
		lastTime = SDL_GetTicks();
		while (SDL_PollEvent(&event))//events
		{
			switch(event.type){
				case SDL_QUIT:
					gameRunning = False;
					break;
				case SDL_KEYDOWN:
					manage_key(&event, &key, True, &car, &cam, &road, &toolbar);
					break;
				case SDL_KEYUP:
					manage_key(&event, &key, False, &car, &cam, &road, &toolbar);
					break;
				case SDL_MOUSEBUTTONDOWN://clique souris
					switch(event.button.button){
						case SDL_BUTTON_LEFT:
							if (event.button.x <= cam.winSize_w){
								add_checkPoint(&road, &event, &cam, &car, &ia);
							} else {
								click_toolbar(&toolbar, &event);
							}
							break;
						case SDL_BUTTON_MIDDLE:
							if (road.len_tab_checkPoints > 0)
								del_checkPoint(&road, &event, &cam, &car, &ia);
							break;
						case SDL_BUTTON_RIGHT:
							if (road.len_tab_checkPoints){// if it exist at least 1 checkpoint
								manage_checkpoint(&road, &event, &cam, &car);
							}
							break;

						default:
							break;
					}
					break;
				case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_RIGHT){
						road.select = False;
					} else if (event.button.button == SDL_BUTTON_LEFT){
						toolbar.is_selecting = False;
						if (toolbar.settings[toolbar.num_setting].type == Checkbox && toolbar.select_var_int == toolbar.settings[toolbar.num_setting].int_variable){
							*toolbar.settings[toolbar.num_setting].int_variable = (*toolbar.settings[toolbar.num_setting].int_variable + 1) % 2;
							toolbar.select_var_int = NULL;
						}
					}
					break;
				default:
					break;
			}
		}
		if (toolbar.is_selecting){
			change_variable(&toolbar, &event);
		}
		// if rezised
		// à mettre dans une fonction
		SDL_GetRendererOutputSize(renderer, &(cam.winSize_w), &(cam.winSize_h));
		cam.winSize_w -= toolbar.size.w;
		toolbar.size.h = cam.winSize_h;
		toolbar.size.x = cam.winSize_w;
		
		// IA take control of the keys
		if (ia.active && ia.num_next_cp != -1){
			ia_manage_keys(&ia, &key, &car);
		}
		move_car(&car, &key, &cam);
		clear(renderer);
		display(renderer, &car, &road, &cam, &event, &ia, &toolbar);
		//printf("%d\n", (int)car.speed);
		//printf("%ld			\r", SDL_GetPerformanceCounter());	//performances
	}
    status = EXIT_SUCCESS;

Quit:
    if(NULL != renderer)
        SDL_DestroyRenderer(renderer);
    if(NULL != window)
		SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
    SDL_Quit();
    return status;
}