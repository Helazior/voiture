//background.h

#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#include "jeu.h"

#define FONT "akaashNormal.ttf"
#define FONT_SIZE 18
#define COLOR_TOOLBAR ORANGE
#define SIZE_LINE_TOOLBAR 200

#define NB_SETTINGS 3


typedef struct Setting{
	int* variable; // sera sans doute un ** pour ne plus avoir de type
	SDL_Texture* texture;
	SDL_Rect tex_size; //automatic
	Type_of_settings type;
	int min; // pour l'instant int, mais sera changé pour du multi type
	int max;

}Setting;


typedef struct Toolbar{
	SDL_Rect size; // size of Toolbar
	// le texte sera une structure, on fera donc un tableau de texte
	//SDL_Color color;
	Setting settings[NB_SETTINGS];

	int num_page; // of the toolbar
	int num_setting;
	int* select_var; //multi type ** bientôt
	Bool is_selecting;
	//int pages[5];
	//int tab_settings[30]; //tab
	int pos_click_x;
}Toolbar;

//init all the toolbar at the right of the screen
void init_toolbar(Toolbar* toolbar, SDL_Renderer *renderer, Entity* car, Road* road);

void click_toolbar(Toolbar* toolbar, SDL_Event* event);

//check if the user click in a box of setting
Bool is_in(int x, int y, SDL_Rect* size);

void change_variable(Toolbar* toolbar, SDL_Event* event);

void change_variable_keys(Toolbar* toolbar, short add);

void render_toolbar(SDL_Renderer *renderer, Toolbar* toolbar);

#endif
