#ifndef VOITURE_CREATE_MAP_H
#define VOITURE_CREATE_MAP_H

#include "jeu.h"

#define FIXE_ROAD 0
#define SAVED_ROAD 1
#define NAIF_ROAD 2
#define TRAVELLING_SALESMAN_ROAD 3
//#define CREATE_MAP_AUTO FIXE_ROAD
#define CREATE_MAP_AUTO TRAVELLING_SALESMAN_ROAD

#define NB_CP 50
#define DIST_CP 1200

/**
 * Call function create_fixe/naif/travelling_road
 * @param road
 */
void create_road(Road* road);

/**
 * Create the basic map
 * @param road
 */
void create_fixe_road(Road* road);

/**
 * Create a road with random turning angle
 * @param road
 */
void create_naif_road(Road* road);

void uncross_and_remove(Road* road, Player* player);

/**
 * Implement a travelling salesman algo
 * @param road
 */
void create_travelling_road(Road* road);

// TODO: load_map_from_file()
// create_saved_map(Road* road);

void travelling_set_up_cp(Road* road);//à suppr TODO mettre en privé
void greedy(Road* road);
#endif //VOITURE_CREATE_MAP_H
