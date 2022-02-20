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
#include <SDL2/SDL_ttf.h>

#include "../include/jeu.h"
#include "../include/ia.h"
#include "../include/background.h"

#define ZOOM_INIT 0.24

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
		.len_tab_checkPoints = 0,
		.nb_valid_checkPoints = 0,
		.square_width = 40,
		.select = False,
		.size = 500,
        .num_clos_check = 0,
        .selectx = 0,
        .selecty = 0,
	};
	init_road(&road);


    // TODO : passer car, key et ia en pointeur et les allouer.
    // TODO : les passer dans player
    Player player[NB_OF_PLAYERS];
    printf("%d\n", NB_OF_PLAYERS);
    for (int i = 0; i < NB_OF_PLAYERS; ++i) {
        // init struct Entity
        init_car(&player[i].car, renderer);
        // init struct Keys_pressed;
        player[i].key.up = False;
        player[i].key.down = False;
        player[i].key.left = False;
        player[i].key.right = False;
        player[i].key.drift = none;

        //init struct Ia;
        if (!(player[i].ia = malloc(sizeof(Ia)))) {
            fprintf(stderr, "Error: malloc IA");
            goto Quit;
        }
        player[i].ia->active = (i == 0)?IA_ACTIVE:false;
        player[i].ia->drift = IA_DRIFT;
        player[i].ia->show_simu_traj = SHOW_SIMU_TRAJ;
        player[i].ia->next_cp.x = 0;
        player[i].ia->next_cp.y = 0;
        player[i].ia->num_next_cp = -1;
        player[i].ia->angle_cp = 0;
        player[i].ia->angle_car_angle_cp = 0;
        player[i].ia->angle_car_cp = 0;
        player[i].ia->car_angle_cp = 0;
        player[i].ia->angle_vect_car_cp = 0;
        player[i].ia->prev_cp.x = 0;
        player[i].ia->prev_cp.y = 0;
        player[i].ia->next_next_cp.x = 0;
        player[i].ia->next_next_cp.y = 0;
        player[i].ia->go_ahead = False;
        player[i].ia->active_traj = False;
        if (player[i].ia->active){
            init_ia(player[i].ia, &road, &player[i].car);
        }
    }

    //init struct Camera;
    Camera cam = {
            .zoom = ZOOM_INIT,
            .follow_car = CAM_FOLLOW_CAR,
            .cursor_x = 0,
            .cursor_y = 0
    };

    SDL_GetRendererOutputSize(renderer, &(cam.winSize_w), &(cam.winSize_h));
    init_cam(&cam, &player[0].car);


	//init struct Background;
	Background bg = {
            .texture = {NULL},
            .nb_sq_fill = 1,
            .show = true
    };
	if (init_background(renderer, &bg) == EXIT_FAILURE)
		goto Quit_texture;
	init_collision_grid(&road);

    //init struct Toolbar;
    Toolbar toolbar = {
            .select_var_int = NULL,
            .select_var_float = NULL
    };

    if (init_toolbar(&toolbar, renderer, &player[0].car, &road, player[0].ia, &cam, &bg) == EXIT_FAILURE)
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
            nb_fps = 1000 * (float) count_loops / (float) count_ms;
            count_loops = 0;
            count_ms = 0;
        }
        //limited fps
        remain_time = lroundf(1000. / FRAMES_PER_SECONDE + lastTime - SDL_GetTicks());
        remain_time *= (int) (remain_time > 0);
        SDL_Delay(remain_time); // wait*/
        lastTime = SDL_GetTicks();
        while (SDL_PollEvent(&event))//events
        {
            switch (event.type) {
                case SDL_QUIT:
                    gameRunning = False;
                    break;
                case SDL_KEYDOWN:
                    manage_key(&event, &player[0].key, True, &player[0].car, &cam, &road, &toolbar, player[0].ia);
                    break;
                case SDL_KEYUP:
                    manage_key(&event, &player[0].key, False, &player[0].car, &cam, &road, &toolbar, player[0].ia);
                    break;
                case SDL_MOUSEBUTTONDOWN://clique souris
                    switch (event.button.button) {
                        case SDL_BUTTON_LEFT:
                            if (event.button.x <= cam.winSize_w) {
                                add_checkPoint(&road, &event, &cam, &player[0].car, player[0].ia);
                            } else {
                                click_toolbar(&toolbar);
                            }
                            break;
                        case SDL_BUTTON_MIDDLE:
                            if (road.len_tab_checkPoints > 0) {
                                // TODO : mettre pour tous
                                del_checkPoint(&road, &event, &cam, &player[0].car);
                                if (road.len_tab_checkPoints == 3) {
                                    stop_ia(player);
                                }
                            }

                            break;
                        case SDL_BUTTON_RIGHT:
                            if (road.len_tab_checkPoints) {// if it exist at least 1 checkpoint
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
                        road.select = False;
                    } else if (event.button.button == SDL_BUTTON_LEFT) {
                        toolbar.is_selecting = False;
                        if (toolbar.settings[toolbar.num_setting].type == Checkbox &&
                            toolbar.select_var_int == toolbar.settings[toolbar.num_setting].int_variable) {
                            *toolbar.settings[toolbar.num_setting].int_variable =
                                    (*toolbar.settings[toolbar.num_setting].int_variable + 1) % 2;

                            // the box has just been checked
                            if (*toolbar.settings[toolbar.num_setting].int_variable == True) {
                                // the box is ia->active
                                if (toolbar.settings[toolbar.num_setting].int_variable == (int *) player[0].ia->active) {
                                    init_ia(player[0].ia, &road, &player[0].car);

                                }
                                // the box has just been unchecked
                            } else {
                                // the box is ia->active
                                if (toolbar.settings[toolbar.num_setting].int_variable == (int *) &player[0].ia->active) {
                                    // the ia change keys, so we need to fixe them to False
                                    stop_first_ia(&player[0].key);
                                }
                            }
                            toolbar.select_var_int = NULL;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        if (toolbar.is_selecting) {
            change_variable(&toolbar);
        }
        // if rezised
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
            move_car(&player[i].car, &player[i].key, &cam);

            // IA take control of the keys
            // TODO : mettre avant les contrôles humains
            if (player[i].ia->active && player[i].ia->num_next_cp != -1) {
                ia_manage_keys(player[i].ia, &player[i].key, &player[i].car, renderer, &cam, &road);
            }
        }
        clear(renderer);
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
