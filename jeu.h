#ifndef _JEU_H
#define _JEU_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
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
	int tab_skid_marks_x[400];
	int tab_skid_marks_y[400];
	double tab_skid_marks_angle[400];
	unsigned int pos_tab;//return to 0 when arrived to 100
	unsigned int count_pos_tab;//stay to 100
};

struct Road
{
	SDL_Rect tab_checkPoints[1000];//bien vérifier qu'on ne dépasse pas 100.
	int long_tab_checkPoints;
	int square_width;
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
};

//drift
void manage_skid_marks(struct Entity* car, struct Keys_pressed key);
//car
void move_car(struct Entity *car, struct Keys_pressed* key, struct Camera* cam);
//key
void manage_key(SDL_Event event, struct Keys_pressed* key,Bool stat, struct Entity* car);
//add a checkpoint:
void add_checkPoint(struct Road* road, SDL_Event event, struct Camera cam);
//del a checkpoint:
void del_checkPoint(struct Road road, SDL_Event event, struct Camera cam);
//found the closest checkpoint to the clic:
int closest_checkpoint(struct Road road, SDL_Event event, struct Camera cam);
//manage a checkpoint:
void manage_checkpoint(struct Road road, SDL_Event event, struct Camera cam);

void render_car(SDL_Renderer *renderer, struct Entity* car, struct Camera cam);//_car display_
void render_road(SDL_Renderer *renderer, struct Road* road, struct Camera cam);//_road display_
void render_drift(SDL_Renderer *renderer, struct Entity car, struct Camera cam);//_drift display_
void display(SDL_Renderer *renderer, struct Entity car, struct Road road, struct Camera cam);// display all
void clear(SDL_Renderer *renderer);


#endif
