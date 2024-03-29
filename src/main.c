/*main.c*/

/*Jeu de course ayant 3 buts :
 * Jouer à un jeu de course
 * Voir comment fonctionne l'IA de la voiture avec des animations
 * Créer un univers (3D ?) de façon procédurale, et pouvoir le visiter avec plusieurs caméras
 * Le tout paramétrable à volonté, à tout moment
 *
 * Voir détails dans le fichier texte*/


#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "../include/jeu.h"
#include "../include/ia.h"
#include "../include/background.h"
#include "../include/collision_car.h"
#include "../include/create_map.h"

#define ZOOM_INIT 0.35
//#define ZOOM_INIT 1

extern unsigned int startLapTime;

int main(void) {
	int status = EXIT_FAILURE;

	//init time
	unsigned int lastTime;
	lastTime = SDL_GetTicks();
	// init for calculate the fps
	uint32_t count_loops = 0;
	uint32_t count_ms = 0;
	int nb_fps = 0;
	//init SDL
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	if (init(&window, &renderer, 1851, 1050)){
		goto Quit;
	}

	//init struct Road;
	Road road = {
		.len_tab_cp = 0,
		.generation = {
			.nb_cp_max = NB_CP,
			.dist_cp = DIST_CP,
			.cp_size_angle_to_remove = 10.f,
			.nb_loops = 20,
            .set_up_cp = True,
			.greedy = True,
			.uncross_and_remove = True,
			.generate_continuously = False,
			{
				.nb_cp_max = False,
				.dist_cp = False,
				.cp_size_angle_to_remove = False,
				.nb_loops_uncross_segments = False,
			}
		},
		.square_width = 40,
		.num_closest_cp = 0,
		.select = false,
		.size = 600,
		.selectx = 0,
		.selecty = 0,
	};

	create_road(&road);


	// TODO : passer car, key et ia en pointeur et les allouer.
	// TODO : les passer dans player
	Player player[NB_OF_PLAYERS];
	for (int i = 0; i < NB_OF_PLAYERS; ++i) {
		// init struct Entity
		player[i].car.turn = TURN;
		init_car(&player[i].car, renderer, i);
		// init struct Keys_pressed;
		release_the_keys(&player[i].key);
		//init cp
		player[i].cp.nb_valid_checkPoints = 0;
		init_player_cp(&player[i].cp, road.len_tab_cp);
		//init struct Ia;
		if (!init_player_ia(&player[i].ia, (i == 0)))
			goto Quit;
		if (player[i].ia->active){
			init_ia(player[i].ia, &road, &player[i].car, &player[i].cp);
		}
	}

    uncross_and_remove(&road, player);

	//init struct Camera;
	Camera cam = {
		.zoom = ZOOM_INIT,
		.follow_car = CAM_FOLLOW_CAR,
		.cursor_x = 200,
		.cursor_y = 200,
	};

	SDL_GetRendererOutputSize(renderer, &(cam.winSize_w), &(cam.winSize_h));
	init_cam(&cam, &player[0].car);


	//init struct Background;
	Background bg = {
		.texture = {NULL},
		.nb_sq_fill = 1,
		.show = false
	};
	if (init_background(renderer, &bg) == EXIT_FAILURE)
		goto Quit_texture;

	//init CallBack
	Callback callback = {
		.create_road = False
	};
	//init struct Toolbar;
	Toolbar toolbar;
	if (init_toolbar(&toolbar, renderer, &player[0].car, &road, player[0].ia, &cam, &bg, &callback) == EXIT_FAILURE)
		goto Quit;


	//__________________Start________________
	int remain_time;
	Bool gameRunning = True;

	SDL_Event event;
	while (gameRunning) {
		// count the fps
		count_loops++;
		count_ms += SDL_GetTicks() - lastTime;
		if (count_loops >= FRAMES_PER_SECONDE) {
			//printf("%u\n", (uint32_t) (1000 * (float)count_loops / (float)count_ms));
			nb_fps = (int)(1000 * (float) count_loops / (float) count_ms);
			count_loops = 0;
			count_ms = 0;
		}
		//limited fps
		remain_time = (int)lroundf(1000.f / FRAMES_PER_SECONDE + (float)lastTime - (float)SDL_GetTicks());
		remain_time *= (int)(remain_time > 0);
		SDL_Delay(remain_time); // wait*/

		lastTime = SDL_GetTicks();
		while (SDL_PollEvent(&event))//events
		{
			switch (event.type) {
				case SDL_QUIT:
					gameRunning = False;
					break;
				case SDL_KEYDOWN:
					manage_key(&event, &player[0].key, True, &cam, &road, &toolbar, player, 0);
					break;
				case SDL_KEYUP:
					manage_key(&event, &player[0].key, False, &cam, &road, &toolbar, player, 0);
					break;
				case SDL_MOUSEBUTTONDOWN://clique souris
					switch (event.button.button) {
						case SDL_BUTTON_LEFT:
							if (event.button.x <= cam.winSize_w) {
								add_checkPoint(&road, &event, &cam, &player[0].car, player);
							} else {
								click_toolbar(&toolbar);
							}
							break;
						case SDL_BUTTON_MIDDLE:
							if (road.len_tab_cp > 0) {
								// TODO : mettre pour tous
								del_closest_checkPoint(&road, &event, &cam, player);
								if (road.len_tab_cp <= 3) {
									stop_ia(player);
								}
							}
							break;
						case SDL_BUTTON_RIGHT:
							if (road.len_tab_cp) {// if it exist at least 1 checkpoint
												  // TODO : mettre pour tous
								manage_checkpoint(&road, &event, &cam, &player[0].car);
							}
							break;

						default:
							break;
					}
					break;
				case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_RIGHT) {
						road.select = false;
					} else if (event.button.button == SDL_BUTTON_LEFT) {
						if (toolbar.is_selecting) {
							// TODO: à revoir !!!
							manage_selected_toolbar(&toolbar, &road, &cam, &callback, player);
						}
					}
					break;
				default:
					break;
			}
		}
		if (toolbar.is_selecting) {
			change_variable(&toolbar, &road);
		}
        if (has_road_var_changed(&road))
            regenerate_map(&road, &cam, player, &callback);
		// if resized
		// TODO : à mettre dans une fonction
		SDL_GetRendererOutputSize(renderer, &(cam.winSize_w), &(cam.winSize_h));
		cam.winSize_w -= toolbar.size.w;
		toolbar.size.h = cam.winSize_h;
		toolbar.size.x = cam.winSize_w;

		if (cam.follow_car == False) {
			// move cam if mouse in the edge of the screen
			move_screen(&cam, &toolbar);
		}
		for (int i = 0; i < NB_OF_PLAYERS; ++i) {
			for (int j = i + 1; j < NB_OF_PLAYERS; ++j) {
				collision(&player[i].car, &player[j].car, &cam, i == 0);
			}
			move_car(&player[i].car, &player[i].key, &cam, (i == 0));
		}

		clear(renderer);
		for (int i = 0; i < NB_OF_PLAYERS; ++i) {
			// IA take control of the keys
			// TODO : mettre avant les contrôles humains
			if (player[i].ia->active && player[i].ia->num_next_cp != -1 && road.len_tab_cp >= 4) {
				ia_manage_keys(player[i].ia, &player[i].key, &player[i].car, renderer, &cam, &road);
			}
		}
		display(renderer, player, &road, &cam, &toolbar, &bg, nb_fps);
		//printf("%d\n", (int)car.speed);
		//printf("%ld			\r", SDL_GetPerformanceCounter());	//performances
	}
	status = EXIT_SUCCESS;

Quit_texture:
	destroy_texture(&bg);
Quit:
	free_players(player);
	if(NULL != renderer)
		SDL_DestroyRenderer(renderer);
	if(NULL != window)
		SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();
	return status;
}
