/*ia.c*/

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <math.h>
#include "../include/jeu.h"
#include "../include/ia.h"


void init_ia(Ia* ia, Road* road, Entity* car){
	ia->active = True;
	ia->num_next_cp = -1;
	calcul_next_cp(road, ia, car);
}

void stop_ia(Keys_pressed* key){
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
	int prev_index = (ia->num_next_cp + road->len_tab_checkPoints - 1) % road->len_tab_checkPoints;
	float prev_cp_x = road->tab_checkPoints[prev_index].x;
	float prev_cp_y = road->tab_checkPoints[prev_index].y;

	// 2 cp after
	int next_index = (ia->num_next_cp + 1) % road->len_tab_checkPoints;
	float next_cp_x = road->tab_checkPoints[next_index].x;
	float next_cp_y = road->tab_checkPoints[(next_index) % road->len_tab_checkPoints].y;
	ia->prev_cp.x = prev_cp_x;
	ia->prev_cp.y = prev_cp_y;
	ia->next_next_cp.x = next_cp_x;
	ia->next_next_cp.y = next_cp_y;

	// y are inversed
	ia->angle_cp = atan2f(-(next_cp_y - prev_cp_y), next_cp_x - prev_cp_x);

}
	
static void calcul_angle_car_angle_cp(Ia* ia, Entity* car){
	ia->angle_car_angle_cp = (float)fmod(car->angle - ia->angle_cp + 3*PI, 2 * PI) - PI;
}

static void calcul_angle_car_cp(Ia* ia, Entity* car){
	ia->angle_vect_car_cp = atan2f(-(ia->next_cp.y - car->posy), ia->next_cp.x - car->posx); // angle for the vect (car.x, car.y) - (cp.x, cp.y) 
	ia->angle_car_cp = (float)fmod(car->angle - ia->angle_vect_car_cp + 3*PI, 2 * PI) - PI;
	/*printf("angle  vect car-cp = %f\n", angle_vect_car_cp);*/
}

static void calcul_car_angle_cp(Ia* ia, Entity* car){
	calcul_angle_car_cp(ia, car);
	ia->car_angle_cp = (float)fmod(ia->angle_vect_car_cp - ia->angle_cp + 3*PI, 2 * PI) - PI;
}

static void calcul_coord_cp(Road* road, Ia* ia, Entity* car){
	ia->next_cp.x = road->tab_checkPoints[ia->num_next_cp].x;
	ia->next_cp.y = road->tab_checkPoints[ia->num_next_cp].y;

	/*____2: angle______*/
	calcul_angle_next_cp(road, ia);

	/*printf("angle cp = %f\n", ia->angle_cp);*/

	calcul_angle_car_angle_cp(ia, car);
	/*printf("angle car - cp = %f\n", ia->angle_car_cp);*/


	/*____3: edge cp____*/
	float distx = ia->prev_cp.x - ia->next_next_cp.x;
	float disty = ia->prev_cp.y - ia->next_next_cp.y;
	float dist = 2 * distance(ia->prev_cp.x ,ia->prev_cp.y , ia->next_next_cp.x, ia->next_next_cp.y);
	Coord next_cp_roadside[2];
	next_cp_roadside[0].x = ia->next_cp.x + disty * road->size / dist;
	next_cp_roadside[0].y = ia->next_cp.y - distx * road->size / dist;
	next_cp_roadside[1].x = ia->next_cp.x - disty * road->size / dist;
	next_cp_roadside[1].y = ia->next_cp.y + distx * road->size / dist;

	// take the biggest angle to have the inside of the turn
	// TODO c'est la version super basique, il faut aussi le faire avec le next, et si l'intersection arrive au milieu de la route, calculer l'intersection pour bien placer le CP.
	// TODO mieux, selon l'angle et la vitesse de la voiture, mettre le CP à un autre endroit
	float dist_prev0 = dist2(next_cp_roadside[0].x, next_cp_roadside[0].y,  ia->prev_cp.x, ia->prev_cp.y);
	float dist_prev1 = dist2(next_cp_roadside[1].x, next_cp_roadside[1].y,  ia->prev_cp.x, ia->prev_cp.y);
	float dist_next0 = dist2(next_cp_roadside[0].x, next_cp_roadside[0].y,  ia->next_next_cp.x, ia->next_next_cp.y);
	float dist_next1 = dist2(next_cp_roadside[1].x, next_cp_roadside[1].y,  ia->next_next_cp.x, ia->next_next_cp.y);
	ia->active_traj = True;
	if (dist_prev0 < dist_prev1 && dist_next0 < dist_next1){
		// mean (a+b)/2
		ia->next_cp.x = next_cp_roadside[0].x - (next_cp_roadside[0].x - ia->next_cp.x) / 6.;
		ia->next_cp.y = next_cp_roadside[0].y - (next_cp_roadside[0].y - ia->next_cp.y) / 6.;
	} else if (dist_prev0 > dist_prev1 && dist_next0 > dist_next1){
		ia->next_cp.x = next_cp_roadside[1].x - (next_cp_roadside[1].x - ia->next_cp.x) / 6.;
		ia->next_cp.y = next_cp_roadside[1].y - (next_cp_roadside[1].y - ia->next_cp.y) / 6.;
	} else {
		ia->active_traj = False;
	}
}


void calcul_next_cp(Road* road, Ia* ia, Entity* car){
	/*____1: next_cp____*/
	//printf("\n"); 
	ia->go_ahead = False;
	if (ia->active_traj == False){
		ia->go_ahead = True;
	}
	if (ia->num_next_cp == -1 && road->nb_valid_checkPoints > 1){
		ia->num_next_cp = 0; // TODO à changer, faut prendre le prochain CP, pas le premier car il peut avoir été pris
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
	calcul_coord_cp(road, ia, car);
}

// lighter function of move_car
static void simu_move_car(Entity* car, Keys_pressed* key){
	//keys_to_struct
	if (car->speed < 1 && key->drift){
		key->drift = none;
		car->angle += car->angle_drift;
		car->angle_drift = 0.;
	}
	car->angle_drift += ((key->drift == drift_left) - 2*(key->drift == drift_right)) * car->turn_drift * (60. / FRAMES_PER_SECONDE) / 320.;
	car->speed += ((float)(key->up) - (float)(key->down)/2.) * car->acceleration * (60. / FRAMES_PER_SECONDE) / 20.;
	if (fabs(car->speed) > 3.){
		car->angle += (double)(car->turn * (60. / FRAMES_PER_SECONDE) * ((double)(key->left) - (double)(key->right))) / (10 * (1. - 2. * (double)((car->speed) < 0.)) * 6 * sqrt(fabs(car->speed)));
	}
	else if (fabs(car->speed) <= 3.){
		car->angle += (double)((double)(key->left) - (double)(key->right)) * (car->speed) * car->turn * (60. / FRAMES_PER_SECONDE) / 1280;
	}
	
	//struct_to_moveCar
	car->posx += car->speed * (float)cos(car->angle);
	car->posy -= car->speed * (float)sin(car->angle);
	car->speed += ((float)(((car->speed) < 0.) - ((car->speed) > 0.))) * (1. + (fabs(car->speed))) * car->frottement / 640.;
}

static Bool too_far(Entity* car, Ia* ia, int pos_initx, int pos_inity, float tolerance){
	// if dist (traj) > dist(cp) : return True
	// TODO : vérifier que road->size c'est pas trop 
	/*return ((car->posx  - pos_initx) * (car->posx  - pos_initx) + (car->posy  - pos_inity) * (car->posy  - pos_inity) > (ia->next_cp.x  - pos_initx) * (ia->next_cp.x  - pos_initx) + (ia->next_cp.y  - pos_inity) * (ia->next_cp.y  - pos_inity) + road->size + 100);*/
	return ((distance(car->posx, car->posy, pos_initx, pos_inity) > distance(ia->next_cp.x, ia->next_cp.y, pos_initx, pos_inity)) && distance(ia->next_cp.x, ia->next_cp.y, car->posx, car->posy) > tolerance);
}

static Bool too_close(Entity* car, Ia* ia, int pos_initx, int pos_inity, float tolerance){
	// if dist (traj) > dist(cp) : return True
	// TODO : vérifier que road->size c'est pas trop 
	/*return ((car->posx  - pos_initx) * (car->posx  - pos_initx) + (car->posy  - pos_inity) * (car->posy  - pos_inity) > (ia->next_cp.x  - pos_initx) * (ia->next_cp.x  - pos_initx) + (ia->next_cp.y  - pos_inity) * (ia->next_cp.y  - pos_inity) + road->size + 100);*/
	return (distance(car->posx, car->posy, pos_initx, pos_inity) < distance(ia->next_cp.x, ia->next_cp.y, pos_initx, pos_inity) - tolerance) && distance(car->posx, car->posy, ia->next_cp.x, ia->next_cp.y) > distance(ia->next_cp.x, ia->next_cp.y, pos_initx, pos_inity);
}

static Bool is_angle_positive(Ia* ia, float tolerance, Entity* car){ // the car angle is too right  -> must to go left
	calcul_angle_car_angle_cp(ia, car);
	return (ia->angle_car_angle_cp > tolerance);
}

static Bool is_angle_negative(Ia* ia, float tolerance, Entity* car){ // car angle too left -> have to go right
	calcul_angle_car_angle_cp(ia, car);
	return (ia->angle_car_angle_cp <= - tolerance);
}

static void show_simu_traj(Entity* car, Camera* cam, SDL_Renderer* renderer, float centre_x, float centre_y){
	SDL_Rect rect = {
		car->posx - cam->x, 
		car->posy - cam->y,
		4, 
		4
	};
	
	rect.x = (1 - cam->zoom) * centre_x + cam->zoom * rect.x;
	rect.y = (1 - cam->zoom) * centre_y + cam->zoom * rect.y;
	if (rect.x > 0 && rect.x < cam->winSize_w && rect.y > 0 && rect.y < cam->winSize_h){
		SDL_RenderFillRect(renderer, &rect);
	}
}

typedef enum{
	LEFT,
	RIGHT
}Turn;


static Bool calcul_went_to_cp(Entity* car, Ia* ia){
	calcul_car_angle_cp(ia, car);
	/*printf("______angle = %f | %f - %f____\n\n", ia->car_angle_cp, ia->angle_vect_car_cp, ia->angle_cp);*/
	return (fabs(ia->car_angle_cp) > PI / 2.0); // NOT < PI/2 => too far from the CP => no need to slow down !
}


static void simu_traj_no_line(int* forecast, int nb_iter, Entity* car, Camera* cam, Ia* ia, Keys_pressed* key, SDL_Renderer* renderer, float centre_x, float centre_y, int nb_iter_line, Road* road, Turn first_turn){
	// TODO : pour optimiser on pourrait repprendre au moment de la ligne droite !
	// TODO : ATTENTION, il ne faut pas qu'il remonte trop ! Donc limiter la remonter
	// TODO : laisser un peu plus decsendre ! Il freine souvent pour rien
	// TODO : Pour aller plus loin il faudrait vérifier s'il peut prendre le prochain CP ou non.
	float car_initx = car->posx;
	float car_inity = car->posy;

	if (ia->show_simu_traj){
		SDL_SetRenderDrawColor(renderer, CP_SELECTED_COLOR);
	}
	Bool have_time_to_turn = (nb_iter - 3 * nb_iter_line < 0);
	/*printf("nb_iter = %d | 3*nb_iter_line = %d\n\n", nb_iter, 3 * nb_iter_line); */
	Bool zero_turn = (nb_iter_line <= 1);
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
	if (*forecast == 3){
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

	Bool went_to_cp = calcul_went_to_cp(car, ia);
	car->posx= car_posx;
	car->posy= car_posy;
	/*car->posx = car_posx;*/
	/*printf("fabs(angle_vect_car_cp_vect_init) : %d\n", (fabs(angle_vect_car_cp_vect_init) > PI/2. - 0.2)); */
	// 1 : does not go to the cp
	// 2 : 
	if (too_close(car, ia, car_initx, car_inity, (float)road->size + 1./6. + (float)car->frame.h / 2.)){
		//printf("pas le temps de tourner %d \n", have_time_to_turn)
		;}
	if (too_far(car, ia, car_initx, car_inity, (float)road->size * 2/6)){
		//printf("traj trop loin\n")
		;}
	if (went_to_cp == 0){
		//printf("n'arrive pas au CP \n")
		;}
			
	if (went_to_cp && zero_turn && ((*forecast == 2 && first_turn == RIGHT) || (*forecast == 3 && first_turn == LEFT))){ // everything like the other trajectory in the right direction
		// TODO vérifier qu'il tourne dans le bon sens, sinon freiner !
		//printf("même traj !\n");
		*forecast = 6;
	} else if (*forecast == 2 && ((is_angle_positive(ia, 0., car) && went_to_cp == 0) || too_far(car, ia, car_initx, car_inity, (float)road->size * 2/6) || (too_close(car, ia, car_initx, car_inity, (float)road->size + 1./6. + (float)car->frame.h / 2.) && have_time_to_turn == False) ||(have_time_to_turn == False && first_turn == LEFT))){
		//printf("forecast = 4\n"); 
		*forecast = 4;
	} else if (*forecast == 3 && ((is_angle_negative(ia, 0., car) && went_to_cp == 0) || too_far(car, ia, car_initx, car_inity, (float)road->size * 2/6) || (too_close(car, ia, car_initx, car_inity, (float)road->size * 1./6. + (float)car->frame.h / 2.) && have_time_to_turn == False) || (have_time_to_turn == False && first_turn == RIGHT))){
		//printf("forecast = 5, %d, %d, %d\n", went_to_cp == 0, too_far(car, ia, car_initx, car_inity, (float)road->size * 2/6), (too_close(car, ia, car_initx, car_inity, (float)road->size * 1./6. + (float)car->frame.h / 2.))); 
		*forecast = 5;
	// faut plus souvent y aller
	} else if (*forecast == 2 && is_angle_negative(ia, 0., car) && went_to_cp == 0 && have_time_to_turn == False){
		//printf("arrête de tourner à gauche et fonce !\n"); 
		*forecast = 6;
	} else if (*forecast == 3 && is_angle_positive(ia, 0., car) && went_to_cp == 0 && have_time_to_turn == False){
		//printf("arrête de tourner à droite et fonce !\n"); 
		*forecast = 6;
	}
}

static int simu_traj(Ia ia, Entity car, Entity car_init, Keys_pressed key, SDL_Renderer* renderer, Camera* cam, Road* road){
	// It is a copy of the structure not to modify it!
	// TODO : voir si la copie est vraiment longue
	// tant qu'il n'a pas dépassé le checkpoint (par rapport à la parpendiculaire)
	
	// TEST : on fait un nombre d'itération assez bas, pour qu'il fasse la stratédie que s'il n'est pas trop loin
	float w = (float)car.frame.w * cam->zoom;
	float h = (float)car.frame.h * cam->zoom;
	float centre_x = car.posx + w / 2 - cam->x;
	float centre_y = car.posy + h / 2 - cam->y;

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
	// TODO :  mettre 0.01
	if (ia.angle_car_cp < 0){
		first_turn = LEFT;
	} else {
		first_turn = RIGHT;
	}
	key.down = False;
	while (ia.car_angle_cp > - PI / 2 && ia.car_angle_cp < PI / 2 && nb_iter <= 15*FRAMES_PER_SECONDE / car.turn){ // while he hasn't passed the cp !
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
	// TODO : faire un type enum
	int forecast = 0; // continue to speed up
	calcul_angle_car_cp(&ia, &car_init);
	// 1 - the traj arrive too far from the CP
	// TODO : [BUG][j'ai commencé un premier jet] attention, des fois quand il est prêt il est trop loin au lieu de trop prêt. checker dist car_init-car < dist car-CP
	// to avoid to go to the bad if statement by error (to fix the bug)
	/*printf("dist = %f\n", distance(car.posx, car.posy, car_init.posx, car_init.posy) - distance(car.posx, car.posy, ia.next_cp.x, ia.next_cp.y)); */
	if (too_far(&car, &ia, car_init.posx, car_init.posy, (float)road->size * 2/6)){
		//printf("too far\n"); 


		// 1/ not enought angle and left
		if (ia.angle_car_cp <= 0. && is_angle_positive(&ia, 0., &car)){
			forecast = 1;
			// TODO : essayer en tournant à droite en freinant pour voir
		}
		// 1/ not enought angle and right
		else if (ia.angle_car_cp >= 0. && is_angle_negative(&ia, 0., &car)){
			forecast = 1;
			// TODO : essayer en tournant à gauche en freinant pour voir

			// 2/ too much angle

		} else if (ia.angle_car_cp >= 0. && is_angle_positive(&ia, 0., &car)){
			forecast = 1;
			// TODO drif if possible

		} else if (ia.angle_car_cp <= 0. && is_angle_negative(&ia, 0., &car)){
			forecast = 1;
			// TODO drif if possible

		} else {
			printf("ERRORRRRRRRRRRR l.329 ia.c\n"); 
		}


		// 2 - the traj arrive too close from the CP
	} else if (too_close(&car, &ia, car_init.posx, car_init.posy, (float)road->size + 1./6. + (float)car.frame.h / 2.) && nb_iter > 0){	
		//printf("Too close \n");	

		// 1/ not enought angle and left
		if (ia.angle_car_cp <= 0. && is_angle_positive(&ia, 0.1, &car)){
			forecast = 1;
			// TODO : drifter
		}
		// 1/ not enought angle and right
		else if (ia.angle_car_cp >= 0. && is_angle_negative(&ia, 0.1, &car)){
			forecast = 1;
			// TODO : drifter
		}

		// 2/ too much angle

		else if (ia.angle_car_cp >= 0. && is_angle_positive(&ia, 0.1, &car)){
			forecast = 0;

		} else if (ia.angle_car_cp <= 0. && is_angle_negative(&ia, 0.1, &car)){
			forecast = 0;

		} else {
			//printf("On fonce !\n"); 
			forecast = 0;
			/*printf("ERRORRRRRRRRRRR l.361 ia.c\n"); */
		}


	} else if (ia.go_ahead == False){
		// TODO Mal géré mais y a de l'idée
		// 1/ not enought angle, cp on the left and the car have to go to the right (but first left)
		if (is_angle_positive(&ia, 0.1, &car)){
			/*printf("1 : angle right -> (go to left) : %f \n",ia.angle_car_angle_cp ); */
			forecast = 2; // left
			simu_traj_no_line(&forecast, nb_iter, &car_init, cam, &ia, &key, renderer, centre_x, centre_y, nb_iter_line, road, first_turn);
			// TODO : drift au lieu de freiner peut être bien
		}
		// 1/ not enought angle, cp on the right and the car have to go to the left (but first right)
		else if (is_angle_negative(&ia, 0.1, &car)){
			/*printf("2 :  angle left -> (go to right) : %f \n",ia.angle_car_angle_cp ); */
			forecast = 3; // right
			simu_traj_no_line(&forecast, nb_iter, &car_init, cam, &ia, &key, renderer, centre_x, centre_y, nb_iter_line, road, first_turn);
			// TODO : drift au lieu de freiner peut être bien
		} else {
			/*printf("3 : ok ! \n");	*/
			forecast = 0;
		}

	} else {
		/*printf("go_ahead validé ! %d\n", ia.go_ahead); */
	}
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
	// TODO : bug sur le départ, il ne passe pas par le départ ! (bug vu qu'une fois)
	/*printf("_____________\n"); */

	calcul_coord_cp(road, ia, car);
	calcul_angle_car_cp(ia, car);
	key->down = False;
	key->up = True;
	int forecast = simu_traj(*ia, *car, *car, *key, renderer, cam, road);
	if ((forecast == 1 && car->speed > 10) || ((forecast == 4 || forecast == 5) && car->speed > 7)){
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
	if (forecast == 2 || forecast == 4){
		/*printf("ok -> gauche %f %f %f\n", ia->angle_car_angle_cp, car->angle, ia->angle_cp); */
		key->left = True;
		key->right = False;
	} else if (forecast == 3 || forecast == 5){
		/*printf("ok -> droite %f %f %f\n", ia->angle_car_angle_cp,car->angle, ia->angle_cp ); */
		key->left = False;
		key->right = True;
	}
	if (forecast == 6){
		//printf("go_ahead actif\n");
		ia->go_ahead = True;
	}
	if (ia->drift){	
		ia_manage_drift(key, car);
	}	
}
