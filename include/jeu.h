//jeu.h

#ifndef _JEU_H
#define _JEU_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define NB_OF_PLAYERS 1

#define FRAMES_PER_SECONDE 60

#define PI 3.141592653589

#define BLACK 0, 0, 0, 255
#define ORANGE 150, 50, 0, 255
#define RED 100, 0, 0, 255
#define GREEN 50, 100, 0, 255
#define WHITE 255, 200, 175, 255
#define YELLOW 200, 150, 0, 255

#define DRIFT_COLOR BLACK
#define LINE_ROAD_COLOR WHITE
#define BACKGROUND_COLOR GREEN
#define CP_TAKEN_COLOR GREEN
#define CP_COLOR ORANGE
#define CP_START_COLOR YELLOW
#define CP_SELECTED_COLOR RED
#define NEXT_CP_COLOR GREEN

#define NB_PIX_DRIFT 800
#define NB_SQUARE 400

#define ACCELERATION 10.
#define FROTTEMENT 8.
#define TURN 8.
#define TURN_DRIFT 7.

#define REAR_CAMERA 20.

//#define NB_PTS_COLL 3
#define NB_PTS 300.

#define NB_GRID_ROW 15
#define NB_GRID_COLUMN 27

#define CAM_FOLLOW_CAR True


typedef enum {
	False = 0,
	True = 1,
	Start = 2
}Bool;


int init(SDL_Window** window, SDL_Renderer** renderer, int w, int h); //initialisation SDL

__attribute__((unused)) int setWindowColor(SDL_Renderer *renderer, SDL_Color color); //to have a new color

SDL_Texture* loadTexture(SDL_Renderer *renderer, const char* p_filePath);

typedef struct Setting Setting;
typedef struct Toolbar Toolbar;
typedef struct Ia Ia;
typedef struct Background Background;

typedef struct Coord{
	float x;
	float y;
}Coord;


typedef struct Entity {
	float posx;
	float posy;
	float pos_initx;
	float pos_inity;

	float acceleration;
	float frottement;
	float turn;
	float turn_drift;

	float speed;
	double angle;
	double angle_drift;
	SDL_Rect frame;
	SDL_Texture* tex;
	int tab_skid_marks_x[NB_PIX_DRIFT];
	int tab_skid_marks_y[NB_PIX_DRIFT];
	double tab_skid_marks_angle[NB_PIX_DRIFT];
	unsigned int pos_tab;//return to 0 when arrived to max
	unsigned int count_pos_tab;//stay max
}Entity;

typedef struct Road {
	SDL_Rect tab_cp[NB_SQUARE];
	int len_tab_cp;
    int nb_cp_max; // To create random road
    int num_closest_cp;
	int square_width;
	Bool select;
	int selectx;
	int selecty;
	int size;
    // TODO : à changer par un set ou alors faire un dico c'est mieux
    /*
    Coord collision_grid[NB_GRID_ROW][NB_GRID_COLUMN][(unsigned int)NB_PTS_COLL]; // [row][column]
	unsigned int nb_pts_collision[NB_GRID_ROW][NB_GRID_COLUMN];
    bool edge_right[NB_GRID_ROW][NB_GRID_COLUMN];
    bool pt_in_road[NB_GRID_ROW][NB_GRID_COLUMN];
     */

} Road;

typedef struct PlayerCP {
    Bool tab_valid_checkPoints[NB_SQUARE];
    int nb_valid_checkPoints;
} PlayerCP;

typedef struct Keys_pressed {
	Bool up;
	Bool down;
	Bool left;
	Bool right;
	enum{
		none = 0,
		drift_left = 1,
		drift_right = 2
	}drift;
}Keys_pressed;

typedef struct Camera {
	float x;
	float y;
	int winSize_w;
	int winSize_h;
	float zoom;
	Bool follow_car;
	int cursor_x;
	int cursor_y;
}Camera;

typedef struct player {
    Entity car;
    Ia *ia;
    Keys_pressed key;
    PlayerCP cp;
}Player;

void pause();

void init_car(Entity* car, SDL_Renderer *renderer, uint8_t num);

void init_player_cp(PlayerCP* cp, int len_tab_checkPoints);

void init_cam(Camera* cam, Entity* car);

void free_players(Player* player);

float distance(float x1, float y1, float x2, float y2);

float dist2(float x1, float y1, float x2, float y2);

void reset_valid_tab(Road* road, PlayerCP* cp, bool first_player);
//drift
void manage_skid_marks(Entity* car, Keys_pressed* key);
//car
void move_car(Entity *car, Keys_pressed* key, Camera* cam, Bool first_car);
//key
void manage_key(SDL_Event* event, Keys_pressed* key, Bool state, Camera* cam, Road* road, Toolbar* toolbar, Player* player, uint8_t num_player);
//put the 3 last checkpoints into the 3 first:
__attribute__((unused)) void close_circuit(Road road);
//add a checkpoint:
void add_checkPoint(Road* road, SDL_Event* event, Camera* cam, Entity* car, Player* player);
// del the checkpoint in road->num_closest_cp
void del_checkPoint(Road* road, Player* player);
//del the closest checkpoint of the click:
void del_closest_checkPoint(Road* road, SDL_Event* event, Camera* cam, Player* player);
//found the closest checkpoint to the clic:
void closest_checkpoint(Road* road, SDL_Event* event, Camera* cam, Entity* car);
//manage a checkpoint:
void manage_checkpoint(Road* road, SDL_Event* event, Camera* cam, Entity* car);

void display(SDL_Renderer *renderer, Player* player, Road* road, Camera* cam, Toolbar* toolbar, Background* bg, int nb_fps);// display all

void clear(SDL_Renderer *renderer);

#endif
