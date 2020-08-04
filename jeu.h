#ifndef _JEU_H
#define _JEU_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define NB_PIX_DRIFT 800
#define NB_SQUARE 400
typedef enum
{
	False = 0,
	True = 1
}Bool;


int init(SDL_Window** window, SDL_Renderer** renderer, int w, int h); //initialisation SDL

int setWindowColor(SDL_Renderer *renderer, SDL_Color color); //to have a new color

SDL_Texture* loadTexture(SDL_Renderer *renderer, const char* p_filePath);

struct Entity
{
	float posx;
	float posy;
	float pos_initx;
	float pos_inity;

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
};

struct Road
{
	SDL_Rect tab_checkPoints[NB_SQUARE];//bien vérifier qu'on ne dépasse pas 100.
	int long_tab_checkPoints;
	int square_width;
	int num_clos_check;
	Bool select;
	int selectx;
	int selecty;
	int size;
};

struct Keys_pressed
{
	Bool up;
	Bool down;
	Bool left;
	Bool right;
	enum
	{
		none = 0,
		left = 1,
		right = 2
	}drift;
};

struct Camera
{
	int x;
	int y;
	int winSize_w;
	int winSize_h;
	float zoom;
};

float distance(float x1, float y1, float x2, float y2);
//drift
void manage_skid_marks(struct Entity* car, struct Keys_pressed* key);
//car
void move_car(struct Entity *car, struct Keys_pressed* key, struct Camera* cam);
//key
void manage_key(SDL_Event* event, struct Keys_pressed* key, Bool stat, struct Entity* car, struct Camera* cam);
//add a checkpoint:
void add_checkPoint(struct Road* road, SDL_Event* event, struct Camera* cam, struct Entity* car);
//del a checkpoint:
void del_checkPoint(struct Road* road, SDL_Event* event, struct Camera* cam, struct Entity* car);
//found the closest checkpoint to the clic:
void closest_checkpoint(struct Road* road, SDL_Event* event, struct Camera* cam, struct Entity* car);
//manage a checkpoint:
void manage_checkpoint(struct Road* road, SDL_Event* event, struct Camera* cam, struct Entity* car);
void render_car(SDL_Renderer *renderer, struct Entity* car, struct Camera* cam);//_car display_
void render_checkPoints(SDL_Renderer *renderer, struct Road* road, struct Camera* cam, struct Entity* car, SDL_Event* event);//_road display_
void render_drift(SDL_Renderer *renderer, struct Entity* car, struct Camera* cam);//_drift display_
void calcul_spline(struct Entity* car, struct Camera* cam, struct Road* road, float* x, float* y, float t);
void calcul_road(struct Camera* cam, struct Road* road, float* x, float* y, float* prevx, float* prevy, float* tabx, float* taby);
void render_road(struct Entity* car, SDL_Renderer *renderer, struct Camera* cam, struct Road* road);
void display(SDL_Renderer *renderer, struct Entity* car, struct Road* road, struct Camera* cam, SDL_Event* event);// display all
void clear(SDL_Renderer *renderer);

#endif
