/*ia.c*/

#include <stdio.h>
#include <SDL2/SDL.h>

#include <math.h>
#include "../include/jeu.h"
#include "../include/ia.h"


typedef enum{
	SPEED_UP=0,
	BREAK=1,
	GO_LEFT=2,
	GO_RIGHT=3,
	DRIFT_LEFT=4,
	DRIFT_RIGHT=5,
	GO_AHEAD=6,
}Forecast;


Ia* init_player_ia(Ia** ia, bool is_player_car) {
	if (!(*ia = malloc(sizeof(Ia)))) {
		fprintf(stderr, "Error: malloc IA");
		return NULL;
	}
	(*ia)->active = is_player_car?IA_ACTIVE:true; // Only the player (i == 0) can be manual
	(*ia)->drift = IA_DRIFT;
	(*ia)->show_simu_traj = SHOW_SIMU_TRAJ;
	(*ia)->next_cp.x = 0;
	(*ia)->next_cp.y = 0;
	(*ia)->num_next_cp = 1;
	(*ia)->angle_cp = 0;
	(*ia)->angle_car_angle_cp = 0;
	(*ia)->angle_car_cp = 0;
	(*ia)->car_angle_cp = 0;
	(*ia)->angle_vect_car_cp = 0;
	(*ia)->prev_cp.x = 0;
	(*ia)->prev_cp.y = 0;
	(*ia)->next_next_cp.x = 0;
	(*ia)->next_next_cp.y = 0;
	(*ia)->go_ahead = False;
	(*ia)->active_traj = False;
    (*ia)->car_turn_same_direction_that_road = False;
	// TEST
	(*ia)->next_cp_turn = NONE;
	return (*ia);
}

void init_ia(Ia* ia, Road* road, Entity* car, PlayerCP* cp){
	ia->active = True;
	ia->num_next_cp = -1; // TODO: à changer
	calcul_next_cp(road, ia, cp, car);
}

void stop_ia(Player* player){
    for (int i = 0; i < NB_OF_PLAYERS; ++i) {
        release_the_keys(&player[i].key);
        player[i].key.drift = none;
    }
}

void release_the_keys(Keys_pressed* key){
    key->up = False;
    key->down = False;
    key->left = False;
    key->right = False;
    key->drift = none;
}

static void calcul_angle_next_cp(Road* road, Ia* ia){
	//moyenne entre vect ortho cp1 et vect cp1cp2
	//pour l'instant on teste angle cp actuel - next next cp
	// TODO :  faire la moyenne avec l'angle de la voiture au moment de passer cp0 et l'angle cp1, cp2, car s'il n'est pas au cp au début de l'ia, il va prendre une très mauvaise traj
	
	// cp actuel
	int prev_index = (ia->num_next_cp + road->len_tab_cp - 1) % road->len_tab_cp;
	float prev_cp_x = (float)road->tab_cp[prev_index].x;
	float prev_cp_y = (float)road->tab_cp[prev_index].y;

	// 2 cp after
	int next_index = (ia->num_next_cp + 1) % road->len_tab_cp;
	float next_cp_x = (float)road->tab_cp[next_index].x;
	float next_cp_y = (float)road->tab_cp[(next_index) % road->len_tab_cp].y;
	ia->prev_cp.x = prev_cp_x;
	ia->prev_cp.y = prev_cp_y;
	ia->next_next_cp.x = next_cp_x;
	ia->next_next_cp.y = next_cp_y;

	// to have the same distance between the 3 CP
	// TODO ça rend l'IA pire : pourquoi ?
	if (ia->show_simu_traj){
		float coeff = distance(ia->next_cp.x, ia->next_cp.y, prev_cp_x, prev_cp_y) / distance(ia->next_cp.x, ia->next_cp.y, ia->next_next_cp.x, ia->next_next_cp.y);
		next_cp_x = ia->next_cp.x + (ia->next_next_cp.x - ia->next_cp.x) * coeff;
		next_cp_y = ia->next_cp.y + (ia->next_next_cp.y - ia->next_cp.y) * coeff;
	}


	// y is unversed
	ia->angle_cp = atan2f(-(next_cp_y - prev_cp_y), next_cp_x - prev_cp_x);
}
	
/*
static float dot_product(float x0, float y0, float x1, float y1) {
	return x0 * y1 - x1 * y0;
}

static Turn turn_direction(Ia* ia, Entity* car) {
	dot_product(
			cos(car->angle) +,
			float y0,
			float x1,
			float y1
			);
}
*/

static void calcul_angle_car_angle_cp(Ia* ia, Entity* car){
	ia->angle_car_angle_cp = (float)(fmod(car->angle - ia->angle_cp + 3*PI, 2 * PI) - PI);
}

static void calcul_angle_car_cp(Ia* ia, Entity* car){
	ia->angle_vect_car_cp = atan2f(-(ia->next_cp.y - car->posy), ia->next_cp.x - car->posx); // angle for the vect (car.x, car.y) - (cp.x, cp.y) 
	ia->angle_car_cp = (float)(fmod(car->angle - ia->angle_vect_car_cp + 3*PI, 2 * PI) - PI); // [-pi, pi]
	//TODO!!!!!!!!!!! Faire un produit scalaire ! x0*y1 - x1*y0
	/*printf("angle  vect car-cp = %f\n", angle_vect_car_cp);*/
}

static void calcul_car_angle_cp(Ia* ia, Entity* car){
	calcul_angle_car_cp(ia, car);
	ia->car_angle_cp = (float)(fmod(ia->angle_vect_car_cp - ia->angle_cp + 3*PI, 2 * PI) - PI);
}

static void calcul_coord_cp(Road* road, Ia* ia, Entity* car){
	ia->next_cp.x = (float)road->tab_cp[ia->num_next_cp].x;
	ia->next_cp.y = (float)road->tab_cp[ia->num_next_cp].y;

	/*____2: angle______*/
	calcul_angle_next_cp(road, ia);

	/*printf("angle cp = %f\n", ia->angle_cp);*/

	calcul_angle_car_angle_cp(ia, car);
	/*printf("angle car - cp = %f\n", ia->angle_car_cp);*/


	/*____3: edge cp____*/
#if 0
	float distx = ia->prev_cp.x - ia->next_next_cp.x;
	float disty = ia->prev_cp.y - ia->next_next_cp.y;
	float dist = 2 * distance(ia->prev_cp.x ,ia->prev_cp.y , ia->next_next_cp.x, ia->next_next_cp.y);
	Coord next_cp_roadside[2];
	next_cp_roadside[0].x = ia->next_cp.x + disty * (float)road->size / dist;
	next_cp_roadside[0].y = ia->next_cp.y - distx * (float)road->size / dist;
	next_cp_roadside[1].x = ia->next_cp.x - disty * (float)road->size / dist;
	next_cp_roadside[1].y = ia->next_cp.y + distx * (float)road->size / dist;

	// take the biggest angle to have the inside of the turn
	// TODO c'est la version super basique, il faut aussi le faire avec le next, et si l'intersection arrive au milieu de la route, calculer l'intersection pour bien placer le CP.
	// TODO mieux, selon l'angle et la vitesse de la voiture, mettre le CP à un autre endroit
	float dist_prev0 = dist2(next_cp_roadside[0].x, next_cp_roadside[0].y,  ia->prev_cp.x, ia->prev_cp.y);
	float dist_prev1 = dist2(next_cp_roadside[1].x, next_cp_roadside[1].y,  ia->prev_cp.x, ia->prev_cp.y);
	float dist_next0 = dist2(next_cp_roadside[0].x, next_cp_roadside[0].y,  ia->next_next_cp.x, ia->next_next_cp.y);
	float dist_next1 = dist2(next_cp_roadside[1].x, next_cp_roadside[1].y,  ia->next_next_cp.x, ia->next_next_cp.y);
	ia->active_traj = True;
	if (dist_prev0 < dist_prev1 && dist_next0 < dist_next1){
		// mean (a+b)/2 = a - (a-b)/2
		ia->next_cp.x = (float)(next_cp_roadside[0].x - (next_cp_roadside[0].x - ia->next_cp.x) / 6.);
		ia->next_cp.y = (float)(next_cp_roadside[0].y - (next_cp_roadside[0].y - ia->next_cp.y) / 6.);
	} else if (dist_prev0 > dist_prev1 && dist_next0 > dist_next1){
		ia->next_cp.x = (float)(next_cp_roadside[1].x - (next_cp_roadside[1].x - ia->next_cp.x) / 6.);
		ia->next_cp.y = (float)(next_cp_roadside[1].y - (next_cp_roadside[1].y - ia->next_cp.y) / 6.);
	} else {
		ia->active_traj = False;
	}
#endif
}


void calcul_next_cp(Road* road, Ia* ia, PlayerCP* cp, Entity* car){
	/*____1: next_cp____*/
	//printf("\n"); 
	ia->go_ahead = False;
	if (ia->active_traj == False){
		ia->go_ahead = True;
	}
    if (road->len_tab_cp == 0) {
        return;
    }
	if (ia->num_next_cp == -1 && cp->nb_valid_checkPoints > 1){
		ia->num_next_cp = 0; // TODO à changer, faut prendre le prochain CP, pas le premier car il peut avoir été pris
		while (cp->tab_valid_checkPoints[ia->num_next_cp++] != Start);
		if (ia->num_next_cp > road->len_tab_cp)
			printf("error\n");
	}
	if (!cp->tab_valid_checkPoints[ia->num_next_cp] && ia->num_next_cp >= 0){
		return;
	}
	while (cp->tab_valid_checkPoints[ia->num_next_cp = (ia->num_next_cp + 1) % road->len_tab_cp]){
		if (cp->nb_valid_checkPoints == road->len_tab_cp){
			ia->num_next_cp = 0;
			break;
		}
	}
	calcul_coord_cp(road, ia, car);
}

// lighter function of move_car
static void simu_move_car(Entity* car, Keys_pressed* key){
	// TODO: faire un truc en commun avec move_car
	//keys_to_struct
	if (car->speed < 1 && key->drift){
		key->drift = none;
		car->angle += car->angle_drift;
		car->angle_drift = 0.;
	}
	car->angle_drift += (float)((key->drift == drift_left) - 2*(key->drift == drift_right)) * car->turn_drift * (60. / FRAMES_PER_SECONDE) / 320.;
	car->speed += (float)((key->up - (float)(key->down)/2.) * car->acceleration * (60. / FRAMES_PER_SECONDE) / 20.);
	if (fabsf(car->speed) > 3.){
		car->angle += (double)(car->turn * (60. / FRAMES_PER_SECONDE) * ((double)(key->left) - (double)(key->right))) / (10 * (1. - 2. * (double)((car->speed) < 0.)) * 6 * sqrtf(fabsf(car->speed)));
	}
	else if (fabsf(car->speed) <= 3.){
		car->angle += (double)((double)(key->left) - (double)(key->right)) * (car->speed) * car->turn * (60. / FRAMES_PER_SECONDE) / 1280;
	}
	
	//struct_to_moveCar
	car->posx += car->speed * (float)cos(car->angle);
	car->posy -= car->speed * (float)sin(car->angle);
	car->speed += (float)((((car->speed) < 0.) - ((car->speed) > 0.)) * (1. + (fabsf(car->speed))) * car->friction / 640.);
}

/** Take the distance between the car and the finale simu traj and see if it's further than the distance car - CP. If it
 * is, see if the the position of the simu traj is far from the CP (at least more than tolerance)*/
static Bool too_far(Entity* car, Ia* ia, int pos_initx, int pos_inity, double tolerance){
	// if dist (traj) > dist(cp) : return True
	// TODO : vérifier que road->size c'est pas trop 
	/*return ((car->posx  - pos_initx) * (car->posx  - pos_initx) + (car->posy  - pos_inity) * (car->posy  - pos_inity) > (ia->next_cp.x  - pos_initx) * (ia->next_cp.x  - pos_initx) + (ia->next_cp.y  - pos_inity) * (ia->next_cp.y  - pos_inity) + road->size + 100);*/
	return (
            (distance(car->posx, car->posy, (float)pos_initx, (float)pos_inity)
            > distance(ia->next_cp.x, ia->next_cp.y, (float)pos_initx, (float)pos_inity))
            && distance(ia->next_cp.x, ia->next_cp.y, car->posx, car->posy) > tolerance);
}

static Bool too_close(Entity* car, Ia* ia, int pos_initx, int pos_inity, double tolerance){
	// if dist (traj) > dist(cp) : return True
	// TODO : vérifier que road->size c'est pas trop 
	/*return ((car->posx  - pos_initx) * (car->posx  - pos_initx) + (car->posy  - pos_inity) * (car->posy  - pos_inity) > (ia->next_cp.x  - pos_initx) * (ia->next_cp.x  - pos_initx) + (ia->next_cp.y  - pos_inity) * (ia->next_cp.y  - pos_inity) + road->size + 100);*/
	return (distance(car->posx, car->posy, (float)pos_initx, (float)pos_inity) < distance(ia->next_cp.x, ia->next_cp.y, (float)pos_initx, (float)pos_inity) - tolerance) && distance(car->posx, car->posy, ia->next_cp.x, ia->next_cp.y) > distance(ia->next_cp.x, ia->next_cp.y, (float)pos_initx, (float)pos_inity);
}

static Bool is_angle_positive(Ia* ia, float tolerance, Entity* car){ // the car angle is too right  -> must to go left
	calcul_angle_car_angle_cp(ia, car);
	return (ia->angle_car_angle_cp > tolerance);
}

static Bool is_angle_negative(Ia* ia, float tolerance, Entity* car){ // car angle too left -> have to go right
	calcul_angle_car_angle_cp(ia, car);
	return (ia->angle_car_angle_cp <= - tolerance);
}

/**
 * @return if three points are listed in a counterclockwise order
 */
static int ccw(Coord* a, Coord* b, Coord* c) {
    return (c->y - a->y) * (b->x - a->x) > (b->y - a->y) * (c->x - a->x);
}

static Bool is_car_turn_same_direction_as_road(Turn road_turn, Turn car_turn) {
//    printf("road_turn = %d, car_turn = %d\n", road_turn, car_turn);
    return road_turn == car_turn;
}

typedef enum {
    ANGLE_INSIDE_BEND,
    ANGLE_OUTSIDE_BEND,
    GOOD_ANGLE,
} AngleAtCP;

/**
 * @return if the angle of the predictable trajectory at the CP go towards the inside ou outside of the bend
 */
static AngleAtCP is_angle_inside_or_outside_bend(Turn road_turn, float car_angle_at_cp, float tolerance_inside, float tolerance_outside) {
    // TODO: je crois que dans le schéma c'est par rapport au virage de la voiture alors qu'ici par rapport à la route : changer
    if ((road_turn == RIGHT && car_angle_at_cp > tolerance_inside) || (road_turn == LEFT && car_angle_at_cp < tolerance_inside))
        return ANGLE_INSIDE_BEND;
    else if ((road_turn == RIGHT && car_angle_at_cp < tolerance_outside || road_turn == LEFT && car_angle_at_cp > tolerance_outside))
        return ANGLE_OUTSIDE_BEND;
    return GOOD_ANGLE;
}


typedef enum {
    TRAJ_INSIDE_BEND,
    TRAJ_OUTSIDE_BEND,
    GOOD_TRAJ,
} TurnTrajectory;

/**
 * @return If the simulated trajectory arrive in the good zone of the road (GOOD_TRAJ), inside the bend (TRAJ_INSIDE_BEND) or outside (TRAJ_OUTSIDE_BEND)
 */
static TurnTrajectory is_traj_in_road(Turn road_turn, Ia* ia, Entity* car, float tolerance) {
    // regarde si le point d'arriver est dans la zone de tolerance par rapport au CP (ou à la traj optimale).
    // Si oui -> GOOD_TRAJ
    // Si non -> regarde ccw pour voir si à gauche ou à droite -> compare avec le virage -> retourne si c'est à l'intérieur ou extérieur.
    // Donc ccw des 3 cp pour road_turn, ccw de l'ancien CP, le prochain CP et la simulation pour savoir si on arrive à droite ou gauche.
    Coord pos_car = {car->posx, car->posy};
    int car_pos_at_cp = ccw(&ia->prev_cp, &ia->next_cp, &pos_car);
//    printf("\nroad_turn = %d, \n");
    if (dist2Coord(&ia->next_cp, &pos_car) < tolerance * tolerance)
        return GOOD_ANGLE;
    else if (road_turn == car_pos_at_cp)
        return ANGLE_INSIDE_BEND;
    else
        return ANGLE_OUTSIDE_BEND;
}


static Forecast drift_same_direction(Turn road_turn) {
    return road_turn == RIGHT? DRIFT_RIGHT : DRIFT_LEFT;
}

static Forecast drift_opposite_direction(Turn road_turn) {
    return road_turn == RIGHT? DRIFT_LEFT : DRIFT_RIGHT;
}

static Forecast turn_same_direction(Turn road_turn) {
    return road_turn == RIGHT? GO_RIGHT : GO_LEFT;
}

static Forecast turn_opposite_direction(Turn road_turn) {
    return road_turn == RIGHT? GO_LEFT : GO_RIGHT;
}

static void show_simu_traj(Entity* car, Camera* cam, SDL_Renderer* renderer, float centre_x, float centre_y){
    SDL_Rect rect = {
            (int)(car->posx - cam->x),
            (int)(car->posy - cam->y),
            4,
            4
    };

    rect.x = (int)((1 - cam->zoom) * centre_x + cam->zoom * rect.x);
    rect.y = (int)((1 - cam->zoom) * centre_y + cam->zoom * rect.y);
    if (rect.x > 0 && rect.x < cam->winSize_w && rect.y > 0 && rect.y < cam->winSize_h){
        // TODO : ne faire que quand c'est voiture 0, ou alors bien placer les points
        SDL_RenderFillRect(renderer, &rect);
    }
}

static bool calcul_went_to_cp(Entity* car, Ia* ia){
	//TODO: à revoir
	calcul_car_angle_cp(ia, car);
	/*printf("______angle = %f | %f - %f____\n\n", ia->car_angle_cp, ia->angle_vect_car_cp, ia->angle_cp);*/
	return (fabsf(ia->car_angle_cp) > PI / 2.0); // NOT < PI/2 => too far from the CP => no need to slow down !
}

static void simu_traj_no_line(Forecast* forecast, int nb_iter, Entity* car, Camera* cam, Ia* ia, Keys_pressed* key, SDL_Renderer* renderer, float centre_x, float centre_y, int nb_iter_line, Road* road, Turn first_turn){
	// TODO : pour optimiser on pourrait repprendre au moment de la ligne droite !
	// TODO : ATTENTION, il ne faut pas qu'il remonte trop ! Donc limiter la remonter
	// TODO : laisser un peu plus decsendre ! Il freine souvent pour rien
	// TODO : Pour aller plus loin il faudrait vérifier s'il peut prendre le prochain CP ou non.
	float car_initx = car->posx;
	float car_inity = car->posy;

	if (ia->show_simu_traj){
		SDL_SetRenderDrawColor(renderer, RED);
	}
	bool have_time_to_turn = (nb_iter - 3 * nb_iter_line < 0);
	/*printf("nb_iter = %d | 3*nb_iter_line = %d\n\n", nb_iter, 3 * nb_iter_line); */
	bool zero_turn = (nb_iter_line <= 1); // TODO!!! 1 or 0 ?
	int nb_iter_second_turn = 2 * nb_iter_line;

	while (nb_iter > 2 * nb_iter_line){ // while he hasn't began the straight line
		if (ia->angle_car_cp < 0.0){
			key->left = True;
			key->right = False;
		} else if (ia->angle_car_cp >= 0.0){
			key->left = False;
			key->right = True;
		}

		simu_move_car(car, key);
		// TODO : NE SERT à rien ?
		calcul_car_angle_cp(ia, car);
		if (ia->show_simu_traj){
			show_simu_traj(car, cam, renderer, centre_x, centre_y);
		}
		nb_iter --;
	}

	// we turn to get a better angle (first turn !)
	/*if (first_turn == LEFT){*/

	if (ia->show_simu_traj){
		SDL_SetRenderDrawColor(renderer, CP_START_COLOR);
	}
	while (nb_iter_line > nb_iter_second_turn / 4){
		simu_move_car(car, key);
		// TODO : NE SERT à rien ?
		calcul_car_angle_cp(ia, car);
		nb_iter_line --;

		if (ia->show_simu_traj){
			show_simu_traj(car, cam, renderer, centre_x, centre_y);
		}
	}

	if (ia->show_simu_traj){
		SDL_SetRenderDrawColor(renderer, NEXT_CP_COLOR);
	}

	// we come back from the turn in the opposite direction (2nd turn)
	/*if (first_turn == RIGHT){*/
	if (*forecast == GO_RIGHT){
		key->left = True;
		key->right = False;
	} else {
		key->left = False;
		key->right = True;
	}

	while (ia->car_angle_cp > - PI / 2  - 0.2 && ia->car_angle_cp < PI / 2 - 0.2 && nb_iter_second_turn){ // while he hasn't passed the cp !
		simu_move_car(car, key);
		calcul_car_angle_cp(ia, car);
		nb_iter_second_turn --;

		if (ia->show_simu_traj){
			show_simu_traj(car, cam, renderer, centre_x, centre_y);
		}
	}

	float car_posx = car->posx;
	float car_posy = car->posy;
	key->left = False;
	key->right = False;
	for (int i = 0; i < 5; i++){
		simu_move_car(car, key);
	} 

	
	// check the angle
	calcul_angle_car_angle_cp(ia, car);
	// if it turn to much it's ok, if no enough -> drift or slow down
	// nothing change -> slow down or drift to turn more

	bool went_to_cp = calcul_went_to_cp(car, ia);
	car->posx= car_posx;
	car->posy= car_posy;
	/*car->posx = car_posx;*/
	/*printf("fabs(angle_vect_car_cp_vect_init) : %d\n", (fabs(angle_vect_car_cp_vect_init) > PI/2. - 0.2)); */
	// 1 : does not go to the cp
	// 2 : 
	if (too_close(car, ia, (int)car_initx, (int)car_inity, (float)(road->size + 1./6. + (float)car->frame.h / 2.))){
		printf("pas le temps de tourner %d \n", have_time_to_turn);
	}
	if (too_far(car, ia, (int)car_initx, (int)car_inity, road->size * 2/6)){
		printf("traj trop loin\n");
	}
	if (went_to_cp == false){
		printf("n'arrive pas au CP \n");
	}

	if (went_to_cp && zero_turn && ((*forecast == GO_LEFT && first_turn == RIGHT) || (*forecast == GO_RIGHT && first_turn == LEFT))) { // everything like the other trajectory in the right direction
		printf("même traj !, %d\n", *forecast);
			*forecast = GO_AHEAD;
	} else if (*forecast == GO_LEFT && ((is_angle_positive(ia, 0.1f, car) && went_to_cp == false) || too_far(car, ia, (int)car_initx, (int)car_inity, (float)road->size * 2/6) || (too_close(car, ia, (int)car_initx, (int)car_inity, (float)(road->size + 1./6. + (float)car->frame.h / 2.)) && have_time_to_turn == False) ||(have_time_to_turn == False && first_turn == LEFT))){
		printf("forecast = 4\n"); 
		*forecast = DRIFT_LEFT;
	} else if (*forecast == GO_RIGHT && ((is_angle_negative(ia, 0.1f, car) && went_to_cp == false) || too_far(car, ia, (int)car_initx, (int)car_inity, (float)road->size * 2/6) || (too_close(car, ia, (int)car_initx, (int)car_inity, (float)(road->size * 1./6. + (float)car->frame.h / 2.)) && have_time_to_turn == False) || (have_time_to_turn == False && first_turn == RIGHT))){
		printf("forecast = 5, %d, %d, %d\n", went_to_cp == 0, too_far(car, ia, car_initx, car_inity, (float)road->size * 2/6), (too_close(car, ia, car_initx, car_inity, (float)road->size * 1./6. + (float)car->frame.h / 2.))); 
		*forecast = DRIFT_RIGHT;
	// faut plus souvent y aller
	} else if (*forecast == GO_LEFT && is_angle_negative(ia, 0.1f, car) && went_to_cp == false && have_time_to_turn == False){
		printf("arrête de tourner à gauche et fonce !\n"); 
		*forecast = GO_AHEAD;
	} else if (*forecast == GO_RIGHT && is_angle_positive(ia, 0.1f, car) && went_to_cp == false && have_time_to_turn == False){
		printf("arrête de tourner à droite et fonce !\n"); 
		*forecast = GO_AHEAD;
	}
}


/**
 *
 * @return What to do in the next frame (see enum "forecast")
 */
static Forecast simu_traj(Ia ia, Entity car, Entity car_init, Keys_pressed key, SDL_Renderer* renderer, Camera* cam, Road* road){
    // It is a copy of the structure to not to modify it!
    // TODO : voir si la copie est vraiment longue
    // tant qu'il n'a pas dépassé le checkpoint (par rapport à la perpendicular)
    // TODO ia.car_angle_cp est mal modulo, on doit rattraper ça... mais bon ça marche quand même

    float w = (float)car.frame.w * cam->zoom;
    float h = (float)car.frame.h * cam->zoom;
    float centre_x = car.posx + w / 2 - cam->x;
    float centre_y = car.posy + h / 2 - cam->y;

    Turn road_turn = ccw(&ia.prev_cp, &ia.next_cp, &ia.next_next_cp); // TODO : encore quelques mauvaises réponses.


    if (ia.show_simu_traj){
        SDL_SetRenderDrawColor(renderer, BLACK);
    }

    car.angle += car.angle_drift;
    car_init.angle += car.angle_drift;
    int nb_iter = 0;
    int nb_iter_line = 0;
    calcul_car_angle_cp(&ia, &car);
    /*printf("__________\n%f ;%f\n", ia.angle_car_cp, ia.angle_cp); */
    /*printf("angle = %f\n", ia.car_angle_cp); */
    calcul_angle_car_cp(&ia, &car);
    /*printf("ia.angle_car_angle_cp = %f\n", ia.angle_car_angle_cp); */
    Turn first_turn;
    // TODO :  mettre 0.01 ou plutôt faire avec le produit scalaire pour estimer s'il peut tomber juste
    if (ia.angle_car_cp < 0){
        first_turn = LEFT;
    } else {
        first_turn = RIGHT;
    }
    key.down = False;
    // TODO faire avec clock cycle comme pour la création de route pour savoir si on a passé le CP !
    int behind_cp = (ia.car_angle_cp < - PI / 2 || ia.car_angle_cp > PI / 2);
    while (ia.car_angle_cp > - PI / 2 && ia.car_angle_cp < PI / 2 && (float)nb_iter <= 15*FRAMES_PER_SECONDE / car.turn){ // while he hasn't passed the cp !
        /*key.left = False;*/
        /*key.right = False;*/
        /*nb_iter_line ++;*/
        if (ia.angle_car_cp < 0.0){
            key.left = True;
            key.right = False;
            if (first_turn == RIGHT){
                nb_iter_line ++;
            }
        } else if (ia.angle_car_cp >= 0.0){
            key.left = False;
            key.right = True;
            if (first_turn == LEFT){
                nb_iter_line ++;
            }
        }
        simu_move_car(&car, &key);
        calcul_car_angle_cp(&ia, &car);
        if (ia.show_simu_traj){
            show_simu_traj(&car, cam, renderer, centre_x, centre_y);
        }
        nb_iter++;
    }
    Forecast forecast = SPEED_UP; // continue to speed up
    float angle_of_car_at_CP_pos = ia.car_angle_cp;
    calcul_angle_car_cp(&ia, &car_init); // re calcul the initial value
    ia.car_turn_same_direction_that_road =  is_car_turn_same_direction_as_road(road_turn, first_turn);
    TurnTrajectory trajectory = is_traj_in_road(road_turn, &ia, &car, road->size / 2 - car.frame.w / 2);
    // 1. if the car turn the same direction that the road, or in the opposite direction or straight forward
    // TODO revoir too_close et too_far et faire interior_band && exterior_band
    AngleAtCP angle_of_car_to_bend;
    if (behind_cp) {
        printf("behind_cp\n");
        // TODO à revoir
        forecast = GO_AHEAD;
    } else if (fabsf(ia.angle_car_cp) < 0.1) {
        printf("tout droit");
        // TODO : n'arrivera jamais, faire par rapport à l'angle plutôt
        if (TRAJ_OUTSIDE_BEND == trajectory) {
            printf(" -> Extérieur virage -> dérape même sens TODO tourne\n");
            // TODO cas turn
            forecast = drift_same_direction(road_turn);
        } else if (TRAJ_INSIDE_BEND == trajectory) {
            printf(" -> Intérieur virage -> tourne/dérape sens inverse TODO tourne\n");
            // TODO cas turn
            forecast = drift_opposite_direction(road_turn);
        } else {
            printf(" -> Sur trajectoire CP -> tout droit\n");
            forecast = SPEED_UP;
        }
    } else if (ia.car_turn_same_direction_that_road) {
        printf("tourne même sens");
        if (TRAJ_OUTSIDE_BEND == trajectory){
            printf(" -> Extérieur virage -> drift même sens : TODO faire les 3 cas\n");
            // TODO: angle intérieur

            // TODO: faire les 3 cas
            forecast = drift_same_direction(road_turn);

        } else if (TRAJ_INSIDE_BEND == trajectory){
            angle_of_car_to_bend = is_angle_inside_or_outside_bend(road_turn, angle_of_car_at_CP_pos, 0.3f, 0.1f);
            printf(" -> Intérieur virage");
                // TODO: voir selon direction voiture
                if (angle_of_car_to_bend == ANGLE_INSIDE_BEND) {
                    printf(" -> angle trop serré -> TODO : voir selon direction voiture\n");
                    forecast = turn_same_direction(road_turn);
                } else {
                    printf(" -> angle trop lâche / même angle -> aller/déraper sens inverse\n");
                    forecast = drift_opposite_direction(road_turn);
                }
        } else {
            angle_of_car_to_bend = is_angle_inside_or_outside_bend(road_turn, angle_of_car_at_CP_pos, 0.5f, 0.2f);
            printf(" -> Sur route CP");
            if (angle_of_car_to_bend == ANGLE_INSIDE_BEND) {
                printf(" -> angle trop serré -> dérapage même sens\n");
                forecast = drift_same_direction(road_turn);
            } else if (angle_of_car_to_bend == ANGLE_OUTSIDE_BEND) {
                printf(" -> angle trop lâche -> TODO\n");
                // TODO faire cas complémentaires
                // TODO: ne se déclenche pas, revoir angle_of_car_at_CP_pos
/*                if (fabsf(angle_of_car_at_CP_pos) < 0.4) {
                    printf("dérapage bon sens\n");
                    forecast = turn_same_direction(road_turn); // TODO drift dans certains cas
                } else if (fabsf(angle_of_car_at_CP_pos) < 1.5){
                    printf("dérapage opp sens\n");
                    forecast = drift_opposite_direction(road_turn);
                }*/
            } else {
                printf(" -> même angle -> virage même sens\n");
                forecast = turn_same_direction(road_turn);
            }
        }
    } else {
        printf("tourne sens inverse");
        // outside turn
        if (TRAJ_OUTSIDE_BEND == trajectory) {
            printf(" -> Extérieur virage");
            angle_of_car_to_bend = is_angle_inside_or_outside_bend(road_turn, angle_of_car_at_CP_pos, 0.5f, 0.1f);
            if (angle_of_car_to_bend == ANGLE_INSIDE_BEND) {
                printf(" -> angle trop serré -> dérapage sens inverse TODO cas plus précis\n");
                // TODO cas plus précis
                forecast = drift_opposite_direction(road_turn);
            } else if (angle_of_car_to_bend == ANGLE_OUTSIDE_BEND) {
                printf(" -> angle trop lâche -> dérapage bon sens\n");
                forecast = drift_same_direction(road_turn);
            } else {
                printf(" -> même angle -> tourner bon sens, TODO ça dépend à quel point trop loin\n");
                forecast = drift_same_direction(road_turn);
            }
        } else if (TRAJ_INSIDE_BEND == trajectory) {
            printf(" -> Intérieur virage");
            angle_of_car_to_bend = is_angle_inside_or_outside_bend(road_turn, angle_of_car_at_CP_pos, 0.4f, 0.1f);
            if (angle_of_car_to_bend == ANGLE_INSIDE_BEND) {
                printf(" -> angle trop serré -> dérapage sens inverse au virage\n");
                forecast = drift_opposite_direction(road_turn);
            } else if (angle_of_car_to_bend == ANGLE_OUTSIDE_BEND) {
                printf(" -> angle trop lâche -> dérapage sens virage\n");
                forecast = drift_same_direction(road_turn);
            } else {
                printf(" -> même angle -> continue à tourner sens inverse\n");
                forecast = turn_opposite_direction(road_turn);
            }
        } else {
            angle_of_car_to_bend = is_angle_inside_or_outside_bend(road_turn, angle_of_car_at_CP_pos, 0.3f, 0.1f);
            printf(" -> Sur route CP");
            if (angle_of_car_to_bend == ANGLE_INSIDE_BEND) {
                printf(" -> angle trop serré -> dérapage même sens\n");
                forecast = drift_opposite_direction(road_turn);
            } else if (angle_of_car_to_bend == ANGLE_OUTSIDE_BEND) {
                printf(" -> angle trop lâche -> virage \n");
                forecast = drift_opposite_direction(road_turn);
            } else {
                printf(" -> même angle -> virage sens inverse, mais TODO\n");
                // TODO revoir cas plus précis
                forecast = turn_same_direction(road_turn);
            }
        }
    }





#if 0
// 1 - the traj arrive too far from the CP
// TODO : [BUG][j'ai commencé un premier jet] attention, des fois quand il est près il est trop loin au lieu de trop près. checker dist car_init-car < dist car-CP
// to avoid to go to the bad if statement by error (to fix the bug)
    /*printf("dist = %f\n", distance(car.posx, car.posy, car_init.posx, car_init.posy) - distance(car.posx, car.posy, ia.next_cp.x, ia.next_cp.y)); */
    if (too_far(&car, &ia, (int)car_init.posx, (int)car_init.posy, (float)road->size * 2/6)){
        //printf("too far\n");


        // 1/ not enough angle and left
        if (ia.angle_car_cp <= 0. && is_angle_positive(&ia, 0.f, &car)){
            /*forecast = BREAK;*/
            // TODO : essayer en tournant à droite en freinant pour voir
            forecast = DRIFT_RIGHT;
        }
            // 1/ not enough angle and right
        else if (ia.angle_car_cp >= 0. && is_angle_negative(&ia, 0.f, &car)){
            /*forecast = BREAK;*/
            // TODO : essayer en tournant à gauche en freinant pour voir
            forecast = DRIFT_LEFT;
            // 2/ too much angle

        } else if (ia.angle_car_cp >= 0. && is_angle_positive(&ia, 0.f, &car)){
            forecast = BREAK;
            // TODO drif if possible

        } else if (ia.angle_car_cp <= 0. && is_angle_negative(&ia, 0.f, &car)){
            forecast = BREAK;
            // TODO drif if possible

        } else {
            printf("Error angle IA\n");
        }


        // 2 - the traj arrive too close from the CP
        // TODO: régler les trajectoires décalé par rapport au CP
    } else if (too_close(&car, &ia, (int)car_init.posx, (int)car_init.posy, (float)(road->size + 1./6. + (float)car.frame.h / 2.)) && nb_iter > 0){
        //printf("Too close \n");

        // 1/ not enough angle and left
        if (ia.angle_car_cp <= 0. && is_angle_positive(&ia, 0.1f, &car)) {
            /*forecast = BREAK;*/
            forecast = DRIFT_LEFT;
        }
            // 1/ not enough angle and right
        else if (ia.angle_car_cp >= 0. && is_angle_negative(&ia, 0.1f, &car)) {
            /*forecast = BREAK;*/
            forecast = DRIFT_RIGHT;
        }

            // 2/ too much angle

        else if (ia.angle_car_cp >= 0. && is_angle_positive(&ia, 0.1f, &car)) {
            printf("SPEED_UP");
            forecast = SPEED_UP;
        } else if (ia.angle_car_cp <= 0. && is_angle_negative(&ia, 0.1f, &car)) {
            printf("SPEED_UP2");
            forecast = SPEED_UP;

        } else {
            printf("On fonce !\n");
            forecast = SPEED_UP;
        }


    } else if (ia.go_ahead == False){
        // TODO Mal géré mais y a de l'idée
        // 1/ not enought angle, cp on the left and the car have to go to the right (but first left)
        if (is_angle_positive(&ia, 0.1f, &car)){
            /*printf("1 : angle right -> (go to left) : %f \n",ia.angle_car_angle_cp ); */
            forecast = GO_LEFT; // left
            simu_traj_no_line(&forecast, nb_iter, &car_init, cam, &ia, &key, renderer, centre_x, centre_y, nb_iter_line, road, first_turn);
            // TODO : drift au lieu de freiner peut être bien
        }
            // 1/ not enought angle, cp on the right and the car have to go to the left (but first right)
        else if (is_angle_negative(&ia, 0.1f, &car)){
            /*printf("2 :  angle left -> (go to right) : %f \n",ia.angle_car_angle_cp ); */
            forecast = GO_RIGHT; // right
            simu_traj_no_line(&forecast, nb_iter, &car_init, cam, &ia, &key, renderer, centre_x, centre_y, nb_iter_line, road, first_turn);
            // TODO : drift au lieu de freiner peut être bien
        } else {
            /*printf("3 : ok ! \n");	*/
            forecast = SPEED_UP;
        }

    } else {
        /*printf("go_ahead validé ! %d\n", ia.go_ahead); */
    }
#endif
/*printf("forecast = %d\n", forecast); */
/*printf("nb_iter = %d\n", nb_iter); */
    return forecast;
}
static void ia_manage_drift(Keys_pressed* key, Entity* car){
	if (key->up){
		key->drift = none;
		car->angle += car->angle_drift;
		car->angle_drift = 0.;
	} else if (key->down && (key->left ^ key->right) && car->speed > 3.){
		key->drift = drift_left * (key->left) + drift_right * (key->right);
	}
	if (key->drift == drift_right && key->right == False){
		key->drift = none;
		car->angle += car->angle_drift;
		car->angle_drift = 0.;
	}
	if (key->drift == drift_left && key->left == False){
		key->drift = none;
		car->angle += car->angle_drift;
		car->angle_drift = 0.;
	}
	/*if (key->drift && key->down){*/
		/*key->down = False;*/
	/*}*/
}

void ia_manage_keys(Ia* ia, Keys_pressed* key, Entity* car, SDL_Renderer* renderer, Camera* cam, Road* road){
	/*____3: move_to_cp____*/
	// calculate the next key combinations by simulating the trajectory in advance
	// (4 cases)
	// TODO : mettre à jour la position du cp !!!
	// TODO : bug sur le départ, il ne passe pas par le départ !
	/*printf("_____________\n"); */

	calcul_coord_cp(road, ia, car);
	calcul_angle_car_cp(ia, car);
	key->down = False;
	key->up = True;
	int forecast = simu_traj(*ia, *car, *car, *key, renderer, cam, road);
	if ((forecast == BREAK && car->speed > 10) || ((forecast == DRIFT_LEFT || forecast == DRIFT_RIGHT) && car->speed > 7)){
		key->down = True;
		key->up = False;
	}
	if (ia->angle_car_cp < -0.1){
		key->left = True;
		key->right = False;
	} else if (ia->angle_car_cp >= 0.1){
		key->left = False;
		key->right = True;
	} else {
		key->left = False;
		key->right = False;
	}
	if (forecast == GO_LEFT || forecast == DRIFT_LEFT){
		/*printf("ok -> gauche %f %f %f\n", ia->angle_car_angle_cp, car->angle, ia->angle_cp); */
		key->left = True;
		key->right = False;
	} else if (forecast == GO_RIGHT || forecast == DRIFT_RIGHT){
		/*printf("ok -> droite %f %f %f\n", ia->angle_car_angle_cp,car->angle, ia->angle_cp ); */
		key->left = False;
		key->right = True;
	}
	if (forecast == GO_AHEAD){
		//printf("go_ahead actif\n");
		ia->go_ahead = True;
	}
	if (ia->drift){	
		ia_manage_drift(key, car);
	}	
}
