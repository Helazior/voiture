//background.h

#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#define FONT "akaashNormal.ttf"
#define FONT_SIZE 18
#define COLOR_TOOLBAR ORANGE
#define SIZE_LINE_TOOLBAR 200

void init_toolbar(Toolbar* toolbar, SDL_Renderer *renderer, Camera* cam, Road* road);

void click_toolbar(Toolbar* toolbar, SDL_Event* event);

/*check if the user click in a box of setting*/
Bool is_in(int x, int y, SDL_Rect* size);

void change_variable(Toolbar* toolbar, SDL_Event* event);

void change_variable_keys(Toolbar* toolbar, short add);

void render_toolbar(SDL_Renderer *renderer, Toolbar* toolbar);

void free_font(Toolbar* toolbar);
#endif
