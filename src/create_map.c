/**
 * create random map with Travelling salesman algo to link the points (or my algo or perlin noise)
*/
// TODO : uncross inutile seul, appeler remove angle de uncross + Bug qui fait que la boucle ne s'arrête dès qu'elle n'a plus de modif.
// TODO: refactoriser dans plusieurs fichiers
#include <SDL2/SDL_stdinc.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "../include/create_map.h"

// pourra être changé dans une variable plus tard
// TODO: faire par rapport à la position de départ et la direction de la voiture
#define START_X 1060
#define START_Y (-234)

#define TIGHT_TURNS 2.

#define DEMO_MODE 1


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

static void save_road_debug(Road* road, int num_road_saved) {
    road->generation.len_tab_cp[num_road_saved] = road->len_tab_cp;
    for (int i = 0; i < road->len_tab_cp; ++i) {
        road->generation.tab_cp_at_step[num_road_saved][i] = road->tab_cp[i];
    }
}

void create_fixe_road(Road* road){
    // TODO : à mettre dans un fichier
	road->len_tab_cp = 6;
	road->tab_cp[0].x = START_X;
	road->tab_cp[0].y = START_Y;
	road->tab_cp[1].x = 1052;
	road->tab_cp[1].y = -1528;
	road->tab_cp[2].x = 361;
	road->tab_cp[2].y = -1526;
	road->tab_cp[3].x = -172;
	road->tab_cp[3].y = -780;
	road->tab_cp[4].x = -1118;
	road->tab_cp[4].y = -1108;
	road->tab_cp[5].x = -960;
	road->tab_cp[5].y = 156;
	for (int i = 0; i < road->len_tab_cp; i++) {
		road->tab_cp[i].w = road->square_width;
		road->tab_cp[i].h = road->square_width;
	}
}

void create_naif_road(Road* road) {
    // TODO rendre plus modulable
	road->len_tab_cp = road->generation.nb_cp_max;
	road->tab_cp[0].x = START_X;
	road->tab_cp[0].y = START_Y;
	double direction = 0.;
	int new_pos_x = START_X;
	int new_pos_y = START_Y;
	road->tab_cp[0].w = road->square_width;
	road->tab_cp[0].h = road->square_width;

    srand(time(NULL));
	for (int i = 1; i < road->len_tab_cp; i++) {
		// turn around when to fan away
		float turn_to_loop = PI * (1. - ((float)road->len_tab_cp - 2.) / (float)road->len_tab_cp);
		direction += (((double)(rand() % 6)) / 5. - 0.5) * TIGHT_TURNS + turn_to_loop; //[-0.5; 0.5]
		// angle in radius
		new_pos_x += (int)(cos(direction) * road->generation.dist_cp);
		new_pos_y += (int)(sin(direction) * road->generation.dist_cp);
		road->tab_cp[i].x = new_pos_x;
		road->tab_cp[i].y = new_pos_y;

		road->tab_cp[i].w = road->square_width;
		road->tab_cp[i].h = road->square_width;
	}
}

/**
 * Set up each CP completely randomly in a radius
 * @param road
 */
static void random_set_up_algo(Road* road) {
    srand(time(NULL));
    int radius_max = (int)(700. * sqrtf(road->len_tab_cp));
    // TODO : mettre le premier à côté de la voiture
    for (uint16_t i = 0; i < road->len_tab_cp; i++) {
        // TODO : pour l'instant c'est en carré mais faut voir avec un cercle
        road->tab_cp[i].x = rand() % (2 * radius_max) - radius_max;
        road->tab_cp[i].y = rand() % (2 * radius_max) - radius_max;
        // TODO: Dans une autre version on pourrait demander un espace min entre chaque CP

        road->tab_cp[i].w = road->square_width;
        road->tab_cp[i].h = road->square_width;
    }
}

/**
 * Set up each CP into a case with random location
 * @param road
 */
static void grid_set_up_algo(Road* road) {
    srand(time(NULL));
    int dist_cp = road->generation.dist_cp;
    int width = (int)round(sqrt(road->len_tab_cp) + 0.5);
    int i;
    for (int row = 0; row < width; ++row) {
        for (int column = 0; column < width; ++column) {
            if (row * width + column >= road->len_tab_cp) {
#if DEMO_MODE
                save_road_debug(road, 0);
#endif
                return;
            }
            i = row * width + column;
            road->tab_cp[i].x =
                    (column - (width >> 1)) * dist_cp + (rand() % dist_cp / 5) - dist_cp / 10 ;
            road->tab_cp[i].y =
                    (row - (width >> 1)) * dist_cp + (rand() % dist_cp / 5) - dist_cp / 10;
            road->tab_cp[i].w = road->square_width;
            road->tab_cp[i].h = road->square_width;
        }
    }
#if DEMO_MODE
    save_road_debug(road, 0);
#endif
}

/**
 * Manage witch algo to use to set up the CPs
 * @param road
 */
void travelling_set_up_cp(Road* road) {
    road->len_tab_cp = road->generation.nb_cp_max;

#if 0
    random_set_up_algo(road);
#else
    grid_set_up_algo(road);
#endif
}

static int sqrt_dist_CP(SDL_Rect* CP0, SDL_Rect* CP1) {
    return (CP1->x - CP0->x) * (CP1->x - CP0->x) + (CP1->y - CP0->y) * (CP1->y - CP0->y);
}

/**
 * @param road
 * @param cp_index Must be 0 < cp_index < nb_cp
 * @param nb_cp Must be > 2
 * @return The index of the nearest CP
 */
static int index_of_nearest_CP(SDL_Rect* tab_checkpoints, int cp_index, int nb_cp) {
    // TODO : j'ai pas déjà fait le même algo pour autre chose ? Voir si on peut factoriser
    if (nb_cp <= 2) {
        perror("Not enough CP");
        return 0;
    }
    if (cp_index < 0 || cp_index > nb_cp) {
        perror("Index_CP out of bound");
    }

    int nearest_cp_index = cp_index ? 0 : 1;
    int nearest_sqrt_dist = sqrt_dist_CP(
            &tab_checkpoints[cp_index],
            &tab_checkpoints[nearest_cp_index]
            );

    int tmp_dist;
    for (int current_index = cp_index + 1; current_index < nb_cp; ++current_index) {
        tmp_dist = sqrt_dist_CP(
                &tab_checkpoints[cp_index],
                &tab_checkpoints[current_index]
        );
        if (tmp_dist < nearest_sqrt_dist) {
            nearest_sqrt_dist = tmp_dist;
//            if (!nearest_sqrt_dist)
//                perror("=0");
            nearest_cp_index = current_index;
        }
    }
//    if(!nearest_sqrt_dist)
//        exit(1);
    return nearest_cp_index;
}

static void swap(SDL_Rect* a, SDL_Rect* b) {
    SDL_Rect tmp = *a;
    *a = *b;
    *b = tmp;
}
/**
 * Take the nearest CP and then iterate over all CPs one by one.
 * This the first step of other algos that manage the crossed road
 * @param road
 */
void greedy(Road* road) {
    for (int i = 0; i < road->len_tab_cp-1; ++i) {
        int nearest_cp_index = index_of_nearest_CP(road->tab_cp, i, road->len_tab_cp);
        swap(&road->tab_cp[nearest_cp_index], &road->tab_cp[(i + 1) % road->len_tab_cp]);
    }
#if DEMO_MODE
    save_road_debug(road, 1);
#endif
}

/**
 * @return if three points are listed in a counterclockwise order
 */
static int ccw(SDL_Rect* a, SDL_Rect* b, SDL_Rect* c) {
    return (c->y - a->y) * (b->x - a->x) > (b->y - a->y) * (c->x - a->x);
}

/**
 * @return if "ab" and "bc" have an intersection, does not deal with co-linearity
 */
static bool is_intersect(SDL_Rect* a, SDL_Rect* b, SDL_Rect* c, SDL_Rect* d) {
    return ccw(a, c, d) != ccw(b, c, d)
           && ccw(a, b, c) != ccw(a, b, d);
}

/**
 * Extend each segment to avoid collision with a large road
 * The segments need to be not null
 * @return is_intersect
 */
static bool is_intersect_extend(SDL_Rect a, SDL_Rect b, SDL_Rect c, SDL_Rect d, int extend_size) {
    // make segments of size road_size
    SDL_Rect ab;
    ab.x = b.x - a.x;
    ab.y = b.y - a.y;
    int size_ab = (int)sqrt(ab.x * ab.x + ab.y * ab.y);
//    printf("ab = %d\n", size_ab);
//    if (size_ab == 0) {
//        exit(2);
//    }
    ab.x = ab.x * extend_size / size_ab;
    ab.y = ab.y * extend_size / size_ab;
    a.x -= ab.x;
    b.x += ab.x;
    a.y -= ab.y;
    b.y += ab.y;

    SDL_Rect cd;
    cd.x = d.x - c.x;
    cd.y = d.y - c.y;
    int size_cd = (int)sqrt(cd.x * cd.x + cd.y * cd.y);
//    printf("cd = %d\n", size_cd);
//    if (size_cd == 0)
//        exit(3);
    cd.x = cd.x * extend_size / size_cd;
    cd.y = cd.y * extend_size / size_cd;
    c.x -= cd.x;
    d.x += cd.x;
    c.y -= cd.y;
    d.y += cd.y;
//    printf("%d, %d, %d, %d | %d, %d, %d\n__________\n", a.x, a.y, b.x, b.y, size_ab, ab.x, ab.y);

    return is_intersect(&a, &b, &c, &d);
}

/**
 * @param nb_cp At least 4
 * @param extend_size the size to extend the segment to avoid that to road touch another road without crossing it
 * @return has_been_changed
 */
static bool uncross_segments(SDL_Rect tab_checkpoints[], int nb_cp, int extend_size) {
    // index_segm1 && index_segm2 are always distincts to have an intersection :
    // so index_segm2 is at least 2 above index_segm1
    if (nb_cp < 4)
        return false;
    bool has_been_changed = false;
    for (int index_segm1 = 0; index_segm1 < nb_cp - 3; ++index_segm1) {
        for (int index_segm2 = index_segm1 + 2; index_segm2 < nb_cp; ++index_segm2) {
            // TODO : si ça prend du temps on pourrait opti en ne le faisant que sur les CP qui ont changés
            if (is_intersect_extend(tab_checkpoints[index_segm1 % nb_cp],
                                    tab_checkpoints[(index_segm1 + 1) % nb_cp],
                                    tab_checkpoints[index_segm2 % nb_cp],
                                    tab_checkpoints[(index_segm2 + 1) % nb_cp],
                                    extend_size
            )) {
                has_been_changed = true;
                // swap the two nearest indexes to invert the smallest loop
                if (((index_segm2 % nb_cp) - (index_segm1 + 1)) % nb_cp < ((index_segm2 + 1) - index_segm1 + nb_cp) % nb_cp) {
                    swap(&tab_checkpoints[(index_segm1 + 1) % nb_cp], &tab_checkpoints[index_segm2 % nb_cp]);
                } else {
                    swap(&tab_checkpoints[index_segm1 % nb_cp], &tab_checkpoints[(index_segm2 + 1) % nb_cp]);
                }
            }
        }
    }
    return has_been_changed;
}


static int remove_hairpin_turns(Road* road, Player* player) {
//    int nb_loops_uncross = 0;
    int nb_loops_remove_angle = 0;
    bool has_removed;
    bool has_removed_once = false;
//    do {
    do {
        has_removed = false;
        for (int i = 0; i < road->len_tab_cp; ++i) {
            double angle =
                    atan2(road->tab_cp[(i - 1 + road->len_tab_cp) % road->len_tab_cp].y - road->tab_cp[i].y,
                          road->tab_cp[(i - 1 + road->len_tab_cp) % road->len_tab_cp].x - road->tab_cp[i].x)
                    -
                    atan2(road->tab_cp[(i + 1) % road->len_tab_cp].y - road->tab_cp[i].y,
                          road->tab_cp[(i + 1) % road->len_tab_cp].x - road->tab_cp[i].x);

            angle += (angle < M_PI) ? 2 * M_PI : 0;
            angle -= (angle > M_PI) ? 2 * M_PI : 0;
            if (fabs(angle) < 0.1 * road->generation.cp_size_angle_to_remove) {
                road->num_closest_cp = i;
                del_checkPoint(road, player);
                has_removed = true;
                has_removed_once = true;
            }
        }
    } while (has_removed && nb_loops_remove_angle++ < 10);
//    } while(uncross_and_remove(road) && nb_loops_uncross++ < road->generation.nb_loops);

//    for (int i = 0; i < NB_OF_PLAYERS; ++i) {
//        player[i].car.pos_initx = (float)road->tab_cp[0].x - 200;
//        player[i].car.pos_inity = (float)road->tab_cp[0].y + 100.f * (float)i;
//        player[i].car.posx = player[i].car.pos_initx;
//        player[i].car.posy = player[i].car.pos_inity;
//        player[i].car.frame.x = (int)(player[i].car.posx);
//        player[i].car.frame.y = (int)(player[i].car.posy);
//    }
    return has_removed_once;
}

/**
 * Firstly, to optimize to road, it's know that the optimal solution does not have intersection in the road,
 * so the function uncross all the segments.
 * After the travelling salesman, it remain some hairpin_turns.
 * This solution consist to remove the corresponding CPs.
 * Note that this is no longer the travelling salesman algo but this is good for a car track.
 * For the moment the function reinitialize players' CPs, so the function can be call in game
 * @param road
 * @param player
 * @return
 */
void uncross_and_remove(Road* road, Player* player) {
    if (!road->len_tab_cp)
        return;
    int nb_change;
    int has_uncross = true;
    int has_remove = true;
    int nb_loop = 0;
    do {
        nb_change = 0;
        while (uncross_segments(road->tab_cp, road->len_tab_cp, 4 * road->tab_cp[0].w)
                             && nb_change++ < 20);
        has_uncross = (nb_change > 0);
        has_remove = remove_hairpin_turns(road, player);
    } while ((has_uncross || has_remove) && nb_loop++ < road->generation.nb_loops);

    for (int i = 0; i < NB_OF_PLAYERS; ++i) {
        player[i].car.pos_initx = (float)road->tab_cp[0].x - 200;
        player[i].car.pos_inity = (float)road->tab_cp[0].y + 100.f * (float)i;
        player[i].car.posx = player[i].car.pos_initx;
        player[i].car.posy = player[i].car.pos_inity;
        player[i].car.frame.x = (int)(player[i].car.posx);
        player[i].car.frame.y = (int)(player[i].car.posy);
    }
}

/**
 * Manage witch algo to use to implement the travelling salesman problem
 * @param road
 */
static void travelling_salesman_on_cp(Road* road) {
    greedy(road);
//    uncross_and_remove(road, player);
}

void create_travelling_road(Road* road) {
    // TODO : créer un fichier spécial pour chaque type d'algo dans des dossiers séparés
    // TODO : Et enlever "travelling" devant les fonctions statiques
    // TODO : comprendre pourquoi il y a toujours un CP sous la voiture
    travelling_set_up_cp(road);
    travelling_salesman_on_cp(road); // TODO seulement greedy poyr l'instant
}
