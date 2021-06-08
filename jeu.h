//jeu.h

#ifndef _JEU_H
#define _JEU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define FRAMES_PER_SECONDE 60

#define PI 3.141592653589

#define BLACK 0, 0, 0, 255
#define ORANGE 150, 50, 0, 255
#define RED 100, 0, 0, 255
#define GREEN 50, 100, 0, 255
#define WHITE 255, 200, 175, 255
#define YELLOW 200, 150, 0, 255

#define DRIFT_COLOR BLACK
#define LINE_ROAD_COLOR BLACK
#define BACKGROUND_COLOR WHITE
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

#define NB_PTS 250.

typedef enum{
	False = 0,
	True = 1,
	Start = 2
}Bool;

typedef enum{
	Checkbox = 0,
	Line = 1
}Type_of_settings;

int init(SDL_Window** window, SDL_Renderer** renderer, int w, int h); //initialisation SDL

int setWindowColor(SDL_Renderer *renderer, SDL_Color color); //to have a new color

SDL_Texture* loadTexture(SDL_Renderer *renderer, const char* p_filePath);

typedef struct Setting Setting;

typedef struct Toolbar Toolbar;

typedef struct Coord{
	float x;
	float y;
}Coord;

typedef struct Ia Ia;

typedef struct Entity{
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

typedef struct Road{
	SDL_Rect tab_checkPoints[NB_SQUARE];//bien vérifier qu'on ne dépasse pas 100.
	Bool tab_valid_checkPoints[NB_SQUARE];
	int long_tab_checkPoints;
	int nb_valid_checkPoints;
	int square_width;
	int num_clos_check;
	Bool select;
	int selectx;
	int selecty;
	int size;
}Road;

typedef struct Keys_pressed{
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

typedef struct Camera{
	int x;
	int y;
	int winSize_w;
	int winSize_h;
	float zoom;
}Camera;

void init_car(Entity* car, SDL_Renderer *renderer);
float distance(float x1, float y1, float x2, float y2);
void reset_valid_tab(Road* road);
//drift
void manage_skid_marks(Entity* car, Keys_pressed* key);
//car
void move_car(Entity *car, Keys_pressed* key, Camera* cam);
//key
void manage_key(SDL_Event* event, Keys_pressed* key, Bool stat, Entity* car, Camera* cam, Road* road, Toolbar* toolbar);
//put the 3 last checkpoints into the 3 first:
void close_circuit(Road road);
//add a checkpoint:
void add_checkPoint(Road* road, SDL_Event* event, Camera* cam, Entity* car);
//del a checkpoint:
void del_checkPoint(Road* road, SDL_Event* event, Camera* cam, Entity* car);
//found the closest checkpoint to the clic:
void closest_checkpoint(Road* road, SDL_Event* event, Camera* cam, Entity* car);
//manage a checkpoint:
void manage_checkpoint(Road* road, SDL_Event* event, Camera* cam, Entity* car);
void render_car(SDL_Renderer *renderer, Entity* car, Camera* cam);//_car display_
void render_checkPoints(SDL_Renderer *renderer, Road* road, Camera* cam, Entity* car, SDL_Event* event, Ia* ia);//_road display_
void render_drift(SDL_Renderer *renderer, Entity* car, Camera* cam);//_drift display_
void calcul_spline(Entity* car, Camera* cam, Road* road, float* x, float* y, float* pt, short* draw);
void calcul_road(Camera* cam, Road* road, float* x, float* y, float* prevx, float* prevy, float* tabx, float* taby);
void render_road(Entity* car, SDL_Renderer *renderer, Camera* cam, Road* road);
void display(SDL_Renderer *renderer, Entity* car, Road* road, Camera* cam, SDL_Event* event, Ia* ia, Toolbar* toolbar);// display all
void clear(SDL_Renderer *renderer);

#endif
