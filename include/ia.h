//ia.h
#ifndef _IA_H
#define _IA_H

#include "jeu.h"
#define IA_ACTIVE False
#define SHOW_SIMU_TRAJ True


typedef struct Ia{
	Bool active; // active IA or not
	Bool show_simu_traj; // active IA or not
	Coord next_cp;
	int num_next_cp;
	float angle_cp; // final angle in the cp
	float angle_car_angle_cp; // angle between the angle car and the final angle in the cp
	float angle_car_cp; // angle to go to the next cp
	float car_angle_cp; // angle (car-cp) and angle cp
	float angle_vect_car_cp; // angle (car-nextCP)
	Coord next_cp_roadside[2];
	
}Ia;

void init_ia(Ia* ia, Road* road, Entity* car);
void calcul_next_cp(Road* road, Ia* ia, Entity* car);
void ia_manage_keys(Ia* ia, Keys_pressed* key, Entity* car, SDL_Renderer* renderer, Camera* cam, Road* road);

#endif
