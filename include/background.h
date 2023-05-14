//background.h

#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#include "jeu.h"

#define FONT "akaashNormal.ttf"
#define FONT_SIZE 18
#define COLOR_TOOLBAR ORANGE
#define COLOR_TOP_TOOLBAR 100, 25, 0, 255
#define SIZE_LINE_TOOLBAR 200
#define WIDTH_TOOLBAR 300

#define NB_SETTINGS 9
#define NB_PAGES 2

#define NB_COLORS_BG 6

typedef enum{
	Checkbox = 0,
	Line = 1,
    Button = 2
}Type_of_settings;

/** 1 setting of 1 variable, int or float.
 * As a line, checkbox or button */
typedef struct Setting{
	int* int_variable;
	float* float_variable;
    void* void_variable;
	SDL_Texture* texture; // The name
	SDL_Rect tex_size; //automatic
	Type_of_settings type;
	float min;
	float max;
}Setting;

/**  A Toolbar is a 2D array of pages of Settings that you have to select */
typedef struct Toolbar{
	SDL_Rect size; // size of Toolbar
    int top_h;
	//SDL_Color color;
	int pos_click_x;
    int* select_var_int;
    float* select_var_float;
    void* select_var_void;
    Bool is_selecting;
    int num_page; // of the toolbar
    int num_setting;
    Setting settings[NB_PAGES][NB_SETTINGS];
}Toolbar;


typedef struct Background{
	SDL_Texture* texture[NB_COLORS_BG];
    int nb_sq_fill;
    // TODO : mettre un enum
    bool show;
    SDL_Texture* number[10]; // 0 to 9
    SDL_Rect number_size;
}Background;

//init all the settings at the right of the screen
int init_toolbar(Toolbar* toolbar, SDL_Renderer *renderer, Entity* car, Road* road, Ia* ia, Camera* cam, Background* bg/*, void (*create_road)(Road*)*/);

void click_toolbar(Toolbar* toolbar);

void change_variable(Toolbar* toolbar);

void change_variable_keys(Toolbar* toolbar, short add);

void render_toolbar(SDL_Renderer *renderer, Toolbar* toolbar);

void render_keys(SDL_Renderer *renderer, Keys_pressed* key, Camera* cam);

// move cam if mouse in the edge of the screen
void move_screen(Camera* cam, Toolbar* toolbar);

void render_number(SDL_Renderer* renderer, Background* bg, int number, int pos_x, int pos_y);

void destroy_texture(Background* bg);
//create the tiles textures to put to the background
int init_background(SDL_Renderer* renderer, Background* bg);

//void fill_background(SDL_Renderer* renderer, Background* bg, Road* road, Camera* cam);

#endif
