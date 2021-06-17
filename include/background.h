//background.h

#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#include "jeu.h"

#define FONT "akaashNormal.ttf"
#define FONT_SIZE 18
#define COLOR_TOOLBAR ORANGE
#define SIZE_LINE_TOOLBAR 200
#define WIDTH_TOOLBAR 300

#define NB_SETTINGS 7

typedef enum{
	Checkbox = 0,
	Line = 1
}Type_of_settings;


typedef struct Setting{
	int* int_variable;
	float* float_variable;
	SDL_Texture* texture;
	SDL_Rect tex_size; //automatic
	Type_of_settings type;
	float min;
	float max;
}Setting;


typedef struct Toolbar{
	SDL_Rect size; // size of Toolbar
	// le texte sera une structure, on fera donc un tableau de texte
	//SDL_Color color;
	Setting settings[NB_SETTINGS];

	int num_page; // of the toolbar
	int num_setting;
	int* select_var_int;
	float* select_var_float;
	Bool is_selecting;
	//int pages[5];
	//int tab_settings[30]; //tab
	int pos_click_x;
}Toolbar;


typedef struct Background{
	SDL_Texture* texture;
}Background;

//init all the toolbar at the right of the screen
int init_toolbar(Toolbar* toolbar, SDL_Renderer *renderer, Entity* car, Road* road, Ia* ia, Camera* cam);

void click_toolbar(Toolbar* toolbar, SDL_Event* event);

void change_variable(Toolbar* toolbar, SDL_Event* event);

void change_variable_keys(Toolbar* toolbar, short add);

void render_toolbar(SDL_Renderer *renderer, Toolbar* toolbar);

void render_keys(SDL_Renderer *renderer, Keys_pressed* key, Camera* cam);
//create the tiles textures to put to the background
int init_background(SDL_Renderer* renderer, Background* bg);

void fill_background(SDL_Renderer* renderer, Background* bg, Road* road);

#endif
