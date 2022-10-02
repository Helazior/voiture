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
#define START_X 1760
#define START_Y 534

#define TIGHT_TURNS 2.


void create_fixe_road(Road* road){
	road->len_tab_checkPoints = 6;
	road->tab_checkPoints[0].x = START_X;
	road->tab_checkPoints[0].y = START_Y;
	road->tab_checkPoints[1].x = 1752;
	road->tab_checkPoints[1].y = -828;
	road->tab_checkPoints[2].x = 1061;
	road->tab_checkPoints[2].y = -826;
	road->tab_checkPoints[3].x = 672;
	road->tab_checkPoints[3].y = -80;
	road->tab_checkPoints[4].x = -418;
	road->tab_checkPoints[4].y = -408;
	road->tab_checkPoints[5].x = -260;
	road->tab_checkPoints[5].y = 856;
	for (int i = 0; i < road->len_tab_checkPoints; i++) {
		road->tab_checkPoints[i].w = road->square_width;
		road->tab_checkPoints[i].h = road->square_width;
	}
}

void create_alea_road(Road* road) {
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
		new_pos_x += cos(direction) * DIST_CP;
		new_pos_y += sin(direction) * DIST_CP;
		road->tab_checkPoints[i].x = new_pos_x;
		road->tab_checkPoints[i].y = new_pos_y;

		road->tab_checkPoints[i].w = road->square_width;
		road->tab_checkPoints[i].h = road->square_width;
	}
}
