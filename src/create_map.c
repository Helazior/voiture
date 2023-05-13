/**
 * create random map with Travelling salesman algo to link the points (or my algo or perlin noise)
*/
// TODO: refactoriser dans plusieurs fichiers
#include <SDL2/SDL_stdinc.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "../include/create_map.h"

// pourra être changé dans une variable plus tard
#define DIST_CP 700
#define NB_CP 6
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
    // TODO : à mettre dans un fichier
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

    srand(time(NULL));
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

    srand(time(NULL));
    int radius_max = (int)(700. * sqrtf(NB_CP));
    // TODO : mettre le premier à côté de la voiture
    for (uint16_t i = 0; i < NB_CP; i++) {
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
    srand(time(NULL));
    int width = (int)round(sqrt(NB_CP) + 0.5);
    int i;
    for (int row = 0; row < width; ++row) {
        for (int column = 0; column < width; ++column) {
            if (row * width + column >= NB_CP) {
                return;
            }
            i = row * width + column;
            road->tab_checkPoints[i].x =
                    (column - (width >> 1)) * DIST_CP + (rand() % 100) - 50;
            road->tab_checkPoints[i].y =
                    (row - (width >> 1)) * DIST_CP + (rand() % 100) - 50;
            road->tab_checkPoints[i].w = road->square_width;
            road->tab_checkPoints[i].h = road->square_width;
        }
    }
}

/**
 * Manage witch algo to use to set up the CPs
 * @param road
 */
void travelling_set_up_cp(Road* road) {
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
 * NB_CP must be <= 2
 * @param road
 * @param cp_index Must be 0 < cp_index < NB_CP
 * @return The index of the nearest CP
 */
static int index_of_nearest_CP(SDL_Rect* tab_checkpoints, int cp_index) {
    // TODO : j'ai pas déjà fait le même algo pour autre chose ? Voir si on peut factoriser
    if (NB_CP <= 2) {
        perror("Not enough CP");
        return 0;
    }
    if (cp_index < 0 || cp_index > NB_CP) {
        perror("Index_CP out of bound");
    }

    int nearest_cp_index = cp_index ? 0 : 1;
    int nearest_sqrt_dist = sqrt_dist_CP(
            &tab_checkpoints[cp_index],
            &tab_checkpoints[nearest_cp_index]
            );

    int tmp_dist;
    for (int current_index = cp_index + 1; current_index < NB_CP; ++current_index) {
        tmp_dist = sqrt_dist_CP(
                &tab_checkpoints[cp_index],
                &tab_checkpoints[current_index]
        );
        if (tmp_dist < nearest_sqrt_dist) {
            nearest_sqrt_dist = tmp_dist;
            nearest_cp_index = current_index;
        }
    }
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
void greedy(SDL_Rect tab_checkpoints[]) {
    for (int i = 0; i < NB_CP-1; ++i) {
        int nearest_cp_index = index_of_nearest_CP(tab_checkpoints, i);
        swap(&tab_checkpoints[nearest_cp_index], &tab_checkpoints[(i + 1) % NB_CP]);
    }
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


void uncross_segments(SDL_Rect tab_checkpoints[]) {
    // index_segm1 && index_segm2 are always distincts to have an intersection :
    // so index_segm2 is at least 2 above index_segm1
    for (int index_segm1 = 0; index_segm1 < NB_CP - 2; ++index_segm1) {
        for (int index_segm2 = index_segm1 + 2; index_segm2 < NB_CP; ++index_segm2) {
            if (is_intersect(&tab_checkpoints[index_segm1],
                             &tab_checkpoints[(index_segm1 + 1) % NB_CP],
                             &tab_checkpoints[index_segm2 % NB_CP],
                             &tab_checkpoints[(index_segm2 + 1) % NB_CP]
                             )) {
                printf("%d %d\n", index_segm1, index_segm2);
                // swap the two nearest indexes to invert the smallest loop
                if (index_segm2 - (index_segm1 + 1) < ((index_segm2 + 1) - index_segm1 + NB_CP) % NB_CP) {
                    swap(&tab_checkpoints[index_segm1 + 1], &tab_checkpoints[index_segm2 % NB_CP]);
                } else {
                    swap(&tab_checkpoints[index_segm1], &tab_checkpoints[(index_segm2 + 1) % NB_CP]);
                }
            }
        }
    }
}

/**
 * Manage witch algo to use to implement the travelling salesman problem
 * @param road
 */
static void travelling_salesman_on_cp(SDL_Rect tab_checkpoints[]) {
    greedy(tab_checkpoints);
}

void create_travelling_road(Road* road) {
    // TODO : créer un fichier spécial pour chaque type d'algo dans des dossiers séparés
    // TODO : Et enlever "travelling" devant les fonctions statiques
    // TODO : comprendre pourquoi il y a toujours un CP sous la voiture
    travelling_init_road(road);
    travelling_set_up_cp(road);
    printf("\n");
    long int sum = 0;
    for (int i = 0; i < NB_CP; ++i) {
        printf("%d ", sqrt_dist_CP(&road->tab_checkPoints[i], &road->tab_checkPoints[(i + 1) % NB_CP]));
        sum += sqrt_dist_CP(&road->tab_checkPoints[i], &road->tab_checkPoints[(i + 1) % NB_CP]);
    }
    printf("\n sum = %ld", sum);
    //travelling_salesman_on_cp(road->tab_checkPoints);
    printf("\n");
    sum = 0;
    for (int i = 0; i < NB_CP; ++i) {
        printf("%d ", sqrt_dist_CP(&road->tab_checkPoints[i], &road->tab_checkPoints[(i + 1) % NB_CP]));
        sum += sqrt_dist_CP(&road->tab_checkPoints[i], &road->tab_checkPoints[(i + 1) % NB_CP]);
    }
    printf("\n sum = %ld\n", sum);
}
