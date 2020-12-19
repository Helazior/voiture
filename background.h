//background.h

#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#define FONT "akaashNormal.ttf"
#define FONT_SIZE 18
#define COLOR_TOOLBAR ORANGE

void init_toolbar(Toolbar* toolbar, SDL_Renderer *renderer, Camera* cam, Road* road);

void click_toolbar(Toolbar* toolbar, SDL_Event* event);

void render_toolbar(SDL_Renderer *renderer, Toolbar* toolbar);

void free_font(Toolbar* toolbar);
#endif
