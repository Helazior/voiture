#ifndef VOITURE_CREATE_MAP_H
#define VOITURE_CREATE_MAP_H

#include "jeu.h"

#define FIXE_ROAD 0
#define SAVED_ROAD 1
#define NAIF_ROAD 2
#define TRAVELLING_SALESMAN_ROAD 3
#define CREATE_MAP_AUTO TRAVELLING_SALESMAN_ROAD

#define NB_CP 200
#define DIST_CP 900

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


/**
 * The optimal solution does not have intersection in the road, so the function uncross all the segments
 * @param tab_checkpoints
 * @return If a switch has been done
 *
 */
bool uncross_all_segments(Road* road);

/**
 * Implement a travelling salesman algo
 * @param road
 */
void create_travelling_road(Road* road);

// TODO: load_map_from_file()
// create_saved_map(Road* road);

/**
 * After the travelling salesman, it remain some hairpin_turns.
 * This solution consist to remove the corresponding CPs
 * For the moment the function reinitialize players' CPs, so the function can be call in game
 * @param road
 * @param player
 * @return If a cp has been deleted
 */
void remove_hairpin_turns(Road* road , Player* player);

void travelling_set_up_cp(Road* road);//à suppr TODO mettre en privé
void greedy(Road* road);
#endif //VOITURE_CREATE_MAP_H
