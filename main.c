/*autre*/
// pouvoir modifier les FPS sans changer la vitesse du jeu (fait ?)
// commenter chaque fonction/struct
// affiche vitesse
// caméra limité au bout du circuit
//roule mieux sur spline que sur herbe
//terre/dur (plusieurs surfaces dans les splines)
//procedural generation pour le décore

/*À faire*/
// barre et touche pour agrandir la largeur de route
// barre pour gerer la vitesse/acceleration/frottement/virage/derapage
// case à cocher pour qu'il ne suive plus la voiture, dans ce cas, on peut se déplacer avec les flêches ou avec la souris au bord de l'écran
// case à cocher pour mettre/ enlever l'IA

//trajectoire optimal sans dérapage
//trajectoire optimal sans sortir de la route
//trajectoire optimal avec dérapage
// pouvoir mettre des obstacles sur la route (mur etc.)
//IA opti
//jeu pause
//fantome de la course
//retenir plusieurs circuits

// plusieurs bots
// plusieurs joueurs en local
// plusieurs joueurs en ligne

// faire un truc très beau et très très visuel
//3D
//TrackMania

/*
 *
Voilà comment sera la version final:
Un fichier config avec tous les réglages enregistrés (modifiables en jeu)
On démarre avec une fenêtre qui s'adapte à notre écran
on a à gauche (modifiable, resizable et enlevable) un panneau de controle qui permet de tout modifier (un racourci clavier pourra remplacer chaque bouton)

Options raccourcis clavier:
	Le panneau peut s'agrandir (ou alors plusieurs onglets) pour donner encore plus de possibilités:
	On pourra ainsi modifier les racourcis clavier, mais plus important, avoir des raccourcis pour remplacer une souris sur un ordi portable.
	Le clique peut donc placer des CP, les enlever/modifier, mais aussi dans un mode plus édition BOUGER LA VOITURE de place

Options jeu:
	je veux pouvour régler:
	GRAPHIQUES: 
		-affichage simu (voir plus bas)
		-affichage info (vitesse, touche appuyé actuellement etc.)
		-texture route
		-nb de points pour la route etc.
		-couleur fond etc.

	-vitesse de jeu ([*0;*8]avec un curseur qui se bloque sur des emplacements et un -+ à côté)
	-FPS ([1; 120] qui n'influent pas sur la vitesse de jeu)
	-grosseur voiture ?
	-largeur route (curseur)
	-caméra (4 cam: 
		0 fixe sur tout le circuit, 
		1 fixe resizable && movable, 
		2 classique: suit la voiture mais ne tourne pas)
		3 tourne avec la voiture (sauf dérapage)
		4 3D, pour l'instant juste la perspective avec tout en 2D

	-collision ? OUI; NON
	-toutes les carac de jeu (speed, acceleration, turn, frottement etc.)
	-mais aussi 

Options mode de jeu:
	-jouable manuel (l'IA peut reprendre à tout moment)
	-IA (qu'on peut bouger à tout moment (donc doit recalculer sa trajectoire si on a touché à une commande))
	-joueur à plusieurs voitures... (avec ou sans collision)
	-Mode replay avec une barre du temps pour se déplacer. On doit pouvoir avoir toutes les options dispo dans ce mode, mais on ne peut pas jouer. Par contre on peut repprendre la partie à un moment donné.

Options simu:
	En manuel ou auto:
	-montre la traj optimal (donc en manuel va devoir recalculer à chaque instant, sauf dans le cas 1 traj au début)
		-ligne avec couleur (oui, non)
		-voiture en transparant (oui, non)
		-iniquement au CP ou tout le temps (régler le temps en 'ms').
		-efface la traj quand passé dessus (oui, non)
		-efface l'ancienne trace si la traj à changer (oui, non)
	-laisse une trace derrière elle
		- ligne de couleur (rouge quand freine et vert quand acclère; bleu à doite, jaune à gauche (dessus ou à côté (doite/gauche)?))
		-voiture en transparant
_____________________________________________________


*/



/*Dans IA:
	remplir ia->next_cp_bord[0].x = ...;
	lorsque dans calcul_road
	x = ia->next_cp.x;
...
	ne pas oublier de rajouter struct Ia* ia en parametre
*/
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include "jeu.h"
#include "ia.h"

#define FRAMES_PER_SECONDE 60


extern unsigned int startLapTime;

int main(void){
    int status = EXIT_FAILURE;

	//init du temps
	unsigned int lastTime, currentTime;
	lastTime = 0;
	currentTime = 0;
	//init SDL
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	if (init(&window, &renderer, 1851, 1050)){
			goto Quit;
		}


	struct Entity car;
	car.speed = 0.;//pixels per frame
	car.angle = 0.;
	car.angle_drift = 0.;
	car.pos_initx = 700.;
	car.pos_inity = 700.;
	car.posx = car.pos_initx;
	car.posy = car.pos_inity;
	car.frame.x = (int)(car.posx);
	car.frame.y = (int)(car.posy);
	car.frame.w = 128;
	car.frame.h = 52;
	car.tex = loadTexture(renderer, "image/car.png");//car image
	car.pos_tab = 0;
	car.count_pos_tab = 0;

	//init struct Camera;
	struct Camera cam;
	SDL_GetWindowSize(window, &(cam.winSize_w), &(cam.winSize_h));
	cam.winSize_w = 1851;
	cam.winSize_h = 1050;

	cam.x = car.pos_initx - cam.winSize_w / 2;
	cam.y = car.pos_inity - cam.winSize_h / 2;
	cam.zoom = 0.25;

	//init struct Road;
	struct Road road;
	road.long_tab_checkPoints = 0;
	road.nb_valid_checkPoints = 0;
	road.square_width = 40;
	road.select = False;
	road.size = 500;
	//init struct Keys_pressed;
	struct Keys_pressed* key = (struct Keys_pressed* )malloc(sizeof(struct Keys_pressed));
	if (!key){
		printf("Error dynamic allocation of key.");
		goto Quit;
	}
	key->up = False;
	key->down = False;
	key->left = False;
	key->right = False;
	key->drift = none;
	//init struct Ia;
	struct Ia ia;
	ia.next_cp.x = 0.;
	ia.next_cp.y = 0.;
	ia.num_next_cp = -1;

	//__________________Start________________
	unsigned int remaind_time;
	remaind_time = lastTime + 1000/FRAMES_PER_SECONDE - currentTime;
	remaind_time *= (remaind_time > 0);
	Bool gameRunning = True;

	SDL_Event event;
	while (gameRunning){
		//limited fps
		currentTime = SDL_GetTicks();
		SDL_Delay(remaind_time);
		while (SDL_PollEvent(&event))//events
		{
		   switch(event.type)
			{
				case SDL_QUIT:
					gameRunning = False;
					break;
				case SDL_KEYDOWN:
					manage_key(&event, key, True, &car, &cam, &road);
					break;
				case SDL_KEYUP:
					manage_key(&event, key, False, &car, &cam, &road);
					break;
				case SDL_MOUSEBUTTONDOWN://clique souris
					switch(event.button.button)
					{
						case SDL_BUTTON_LEFT:
							add_checkPoint(&road, &event, &cam, &car);
							break;
						case SDL_BUTTON_MIDDLE:
							if (road.long_tab_checkPoints > 0)
								del_checkPoint(&road, &event, &cam, &car);
							break;
						case SDL_BUTTON_RIGHT:
							if (road.long_tab_checkPoints)// if it exist at least 1 checkpoint
							{
								manage_checkpoint(&road, &event, &cam, &car);
							}
							break;

						default:
							break;
					}
					break;
				case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_RIGHT)
					{
							road.select = False;
					}
					break;
				default:
					break;
			}
		}

		move_car(&car, key, &cam);
		clear(renderer);
		display(renderer, &car, &road, &cam, &event, &ia);
		//printf("%d\n", (int)car.speed);
		lastTime = currentTime;
		//printf("%ld			\r", SDL_GetPerformanceCounter());	//performances
	}
    status = EXIT_SUCCESS;
Quit:
	if(NULL != key)
		free(key);
    if(NULL != renderer)
        SDL_DestroyRenderer(renderer);
    if(NULL != window)
        SDL_DestroyWindow(window);
    SDL_Quit();
    return status;
}
