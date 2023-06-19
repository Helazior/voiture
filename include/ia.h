//ia.h
#ifndef _IA_H
#define _IA_H

#include "jeu.h"
#define IA_ACTIVE False
#define IA_DRIFT True
#define SHOW_SIMU_TRAJ True

typedef enum{
	LEFT=0,
    RIGHT=1,
	NONE=2,
}Turn;

typedef struct Ia {
	Bool active; // active IA or not
	Bool drift; // IA drift active or not
	Bool show_simu_traj; // active IA or not
	Coord next_cp;
	int num_next_cp;
	float angle_cp; // final angle the car will take on the cp
	float angle_car_angle_cp; // angle between the angle car and the final angle in the cp
	float angle_car_cp; // angle to go to the next cp = traj_cp
	float car_angle_cp; // angle (car-cp) and angle cp -> (car_cp ^ angle_cp ) = angle_cp
	float angle_vect_car_cp; // angle (car-nextCP)
	Coord prev_cp;
	Coord next_next_cp;
	Bool go_ahead;
    int persist_direction;
	Bool active_traj;
	Coord vect_next_cp;
	Coord vect_car;
	Turn next_cp_turn;
    Bool car_turn_same_direction_that_road;
}Ia;

Ia* init_player_ia(Ia** ia, bool is_player_car);
void init_ia(Ia* ia, Road* road, Entity* car, PlayerCP* cp);
//to initialize the keys after the stop of the ia
void stop_ia(Player* player);
void release_the_keys(Keys_pressed* key);
void calcul_next_cp(Road* road, Ia* ia, PlayerCP* cp, Entity* car);
void ia_manage_keys(Ia* ia, Keys_pressed* key, Entity* car, SDL_Renderer* renderer, Camera* cam, Road* road);

#endif
