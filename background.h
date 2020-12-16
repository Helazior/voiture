//background.h

#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#define FONT "akaashNormal.ttf"
#define FONT_SIZE 18
#define ANTIALIASING 0

void init_toolbar(Toolbar* toolbar, SDL_Renderer *renderer);

void render_toolbar(SDL_Renderer *renderer, Toolbar* toolbar);

void free_font(Toolbar* toolbar);
#endif
