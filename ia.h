#ifndef _IA_H
#define _IA_H

#include "jeu.h"
#define IA_ACTIVE True


typedef struct Ia{
	Bool active;
	Coord next_cp;
	int num_next_cp;
	float angle_cp;
	Coord next_cp_bord[2];
}Ia;



void init_ia(Ia* ia);
void calcul_next_cp(struct Road* road, struct Ia* ia);
void angle_next_cp(struct Road* road, struct Ia* ia);

#endif
