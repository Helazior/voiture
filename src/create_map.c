// create random map with Travelling salesman algo to link the points or my algo or perlin

#include <SDL2/SDL_stdinc.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "../include/create_map.h"

// pourra être changé dans une variable plus tard
#define DIST_CP 700
#define NB_CP 27
// TODO: faire par rapport à la position de départ et la direction de la voiture
#define START_X 1060
#define START_Y (-234)

#define TIGHT_TURNS 2.


void create_road(Road* road) {

    switch (CREATE_MAP_AUTO) {
        case FIXE_ROAD:
            create_fixe_road(road);
            break;
        case SAVED_ROAD:
            // TODO
            perror("Not implemented yet!");
            break;
        case NAIF_ROAD:
            create_naif_road(road);
            break;
        case TRAVELLING_SALESMAN_ROAD:
            create_travelling_road(road);
            break;
        default:
            perror("CREATE_MAP_AUTO has bad value");
            break;
    }
}

void create_fixe_road(Road* road){
	road->len_tab_checkPoints = 6;
	road->tab_checkPoints[0].x = START_X;
	road->tab_checkPoints[0].y = START_Y;
	road->tab_checkPoints[1].x = 1052;
	road->tab_checkPoints[1].y = -1528;
	road->tab_checkPoints[2].x = 361;
	road->tab_checkPoints[2].y = -1526;
	road->tab_checkPoints[3].x = -172;
	road->tab_checkPoints[3].y = -780;
	road->tab_checkPoints[4].x = -1118;
	road->tab_checkPoints[4].y = -1108;
	road->tab_checkPoints[5].x = -960;
	road->tab_checkPoints[5].y = 156;
	for (int i = 0; i < road->len_tab_checkPoints; i++) {
		road->tab_checkPoints[i].w = road->square_width;
		road->tab_checkPoints[i].h = road->square_width;
	}
}

void create_naif_road(Road* road) {
    // TODO rendre plus modulable
	road->len_tab_checkPoints = NB_CP;
	road->tab_checkPoints[0].x = START_X;
	road->tab_checkPoints[0].y = START_Y;
	double direction = 0.;
	int new_pos_x = START_X;
	int new_pos_y = START_Y;
	road->tab_checkPoints[0].w = road->square_width;
	road->tab_checkPoints[0].h = road->square_width;

	time_t t;
	srand ((unsigned)time(&t));
	for (uint16_t i = 1; i < NB_CP; i++) {
		// turn around when to fan away
		float turn_to_loop = PI * (1. - ((float)NB_CP - 2.) / (float)NB_CP);
		direction += (((double)(rand() % 6)) / 5. - 0.5) * TIGHT_TURNS + turn_to_loop; //[-0.5; 0.5]
		// angle in radius
		new_pos_x += (int)(cos(direction) * DIST_CP);
		new_pos_y += (int)(sin(direction) * DIST_CP);
		road->tab_checkPoints[i].x = new_pos_x;
		road->tab_checkPoints[i].y = new_pos_y;

		road->tab_checkPoints[i].w = road->square_width;
		road->tab_checkPoints[i].h = road->square_width;
	}
}

static void travelling_init_road(Road* road) {
    road->len_tab_checkPoints = NB_CP;
//    road->tab_checkPoints[0].x = START_X;
//    road->tab_checkPoints[0].y = START_Y;
    int new_pos_x = START_X;
    int new_pos_y = START_Y;
    road->tab_checkPoints[0].w = road->square_width;
    road->tab_checkPoints[0].h = road->square_width;
}

/**
 * Only restrict parameter
 * @param road
 */
static void random_set_up_algo(Road* road) {

    time_t t;
    srand ((unsigned)time(&t));

    int radius_max = (int)(700. * sqrtf(NB_CP));

    for (uint16_t i = 1; i < NB_CP; i++) {
        // TODO : pour l'instant c'est en carré mais faut voir avec un cercle
        road->tab_checkPoints[i].x = rand() % (2 * radius_max) - radius_max;
        road->tab_checkPoints[i].y = rand() % (2 * radius_max) - radius_max;
        // TODO: Dans une autre version on pourrait demander un espace min entre chaque CP

        road->tab_checkPoints[i].w = road->square_width;
        road->tab_checkPoints[i].h = road->square_width;
    }
}

/**
 * Set up each CP into a case with random location
 * @param road
 */
static void grid_set_up_algo(Road* road) {
    // TODO
}

static void travelling_set_up_cp(Road* road) {
    random_set_up_algo(road);

}

void create_travelling_road(Road* road) {
    // TODO : créer un fichier spécial pour chaque type d'algo dans des dossiers séparés
    // TODO : Et enlever "travelling devant les fonctions statiques
    travelling_init_road(road);
    travelling_set_up_cp(road);
}
