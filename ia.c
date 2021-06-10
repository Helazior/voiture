/*ia.c*/

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <math.h>
#include "jeu.h"
#include "ia.h"


void init_ia(Ia* ia){
	ia->active = IA_ACTIVE;
	ia->next_cp.x = 0.;
	ia->next_cp.y = 0.;
	ia->num_next_cp = -1;
}

static void angle_next_cp(Road* road, Ia* ia, Entity* car){
	//moyenne entre vect ortho cp1 et vect cp1cp2
	//pour l'instant on teste angle cp actuel - next next cp
	
	// cp actuel
	int prev_index = (ia->num_next_cp + road->len_tab_checkPoints - 1) % road->len_tab_checkPoints;
	float prev_cp_x = road->tab_checkPoints[prev_index].x;
	float prev_cp_y = road->tab_checkPoints[prev_index].y;

	// 2 cp after
	int next_index = (ia->num_next_cp + 1) % road->len_tab_checkPoints;
	float next_cp_x = road->tab_checkPoints[next_index].x;
	float next_cp_y = road->tab_checkPoints[(next_index) % road->len_tab_checkPoints].y;

	// y are inversed
	ia->angle_cp = atan2f(-(next_cp_y - prev_cp_y), next_cp_x - prev_cp_x);

}
	
static void angle_car_angle_cp(Ia* ia, Entity* car){
	ia->angle_car_angle_cp = (float)fmod(car->angle - ia->angle_cp + 3*PI, 2 * PI) - PI;
}

static void angle_car_cp(Ia* ia, Entity* car){
	float angle_vect_car_cp = atan2f(-(ia->next_cp.y - car->posy), ia->next_cp.x - car->posx); // angle for the vect (car.x, car.y) - (cp.x, cp.y) 
	ia->angle_car_cp = (float)fmod(car->angle - angle_vect_car_cp + 3*PI, 2 * PI) - PI;
	/*printf("angle  vect car-cp = %f\n", angle_vect_car_cp);*/
}


void calcul_next_cp(Road* road, Ia* ia, Entity* car){
	/*____1: next_cp____*/
	if (ia->num_next_cp == -1 && road->nb_valid_checkPoints > 1){
		ia->num_next_cp = 0;
		while (road->tab_valid_checkPoints[ia->num_next_cp++] != Start);
		if (ia->num_next_cp > road->len_tab_checkPoints)
			printf("error\n");
	}
	if (!road->tab_valid_checkPoints[ia->num_next_cp] && ia->num_next_cp >= 0){
		return;
	}
	while (road->tab_valid_checkPoints[ia->num_next_cp = (ia->num_next_cp + 1) % road->len_tab_checkPoints]){ 
			if (road->nb_valid_checkPoints == road->len_tab_checkPoints){
				ia->num_next_cp = 0;
				break;
			}
	}
	printf("next_cp = %d\n", ia->num_next_cp);
		
	ia->next_cp.x = road->tab_checkPoints[ia->num_next_cp].x;
	ia->next_cp.y = road->tab_checkPoints[ia->num_next_cp].y;
	
	/*____2: angle______*/
	angle_next_cp(road, ia, car);

	printf("angle cp = %f\n", ia->angle_cp);
	
	angle_car_angle_cp(ia, car);
	printf("angle car - cp = %f\n", ia->angle_car_cp);

}

void ia_manage_keys(Ia* ia, Keys_pressed* key, Entity* car){
	/*____3: move_to_cp____*/
	// calculate the next key combinations by simulating the trajectory in advance
	// (4 cases)
	// TODO : mettre à jour la position du cp !!!
	// TODO : bug sur le départ, il ne passe pas par le départ ! (bug vu qu'une fois)
	angle_car_cp(ia, car);
	if (ia->angle_car_cp < 0){
		key->left = True;
		key->right = False;
	} else if (ia->angle_car_cp > 0){
		key->left = False;
		key->right = True;
	}	
	key->up = True;

}
