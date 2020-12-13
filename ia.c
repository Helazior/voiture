#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include "jeu.h"
#include "ia.h"

#define PI 3.141592653589

/*void angle_next_cp(struct Road* road, struct Ia* ia){*/
	/*//moyenne entre vect ortho cp1 et vect cp1cp2*/
	/*//pour l'instant on test vect ortho cp1*/
	

/*}*/
void calcul_next_cp(struct Road* road, struct Ia* ia){
	/*____1: next_cp____*/
	if (ia->num_next_cp == -1 && road->nb_valid_checkPoints > 1){
		ia->num_next_cp = 0;
		while (road->tab_valid_checkPoints[ia->num_next_cp++] != Start);
		if (ia->num_next_cp > road->long_tab_checkPoints)
			printf("error");
	}
	if (!road->tab_valid_checkPoints[ia->num_next_cp] && ia->num_next_cp >= 0){
		return;
	}
	while (road->tab_valid_checkPoints[ia->num_next_cp = (ia->num_next_cp + 1) % road->long_tab_checkPoints]){ 
			if (road->nb_valid_checkPoints == road->long_tab_checkPoints){
				ia->num_next_cp = 0;
				break;
			}
	}
	printf("%d\n", ia->num_next_cp);
		
	ia->next_cp.x = road->tab_checkPoints[ia->num_next_cp].x;
	ia->next_cp.y = road->tab_checkPoints[ia->num_next_cp].y;
	
	/*____2: angle______*/

	/*____3: move_cp____*/
}
