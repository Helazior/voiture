#ifndef _IA_H
#define _IA_H

#define IA_MODE 1

void init_ia(Ia* ia);
void calcul_next_cp(struct Road* road, struct Ia* ia);
void angle_next_cp(struct Road* road, struct Ia* ia);

#endif
