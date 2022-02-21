#include "math.h"
#include "../include/collision_car.h"

#if 0
/*
 * À partir d'un vieux code python que j'avais fait
 */

#define SIZE 3


static double dot_product(const double* u, const double* v) {
    double sum = 0.f;
    // TODO: revoir SIZE
    for (int i = 0; i < SIZE; ++i) {
        sum += u[i] * v[i];
    }
    return sum;
}

void crossProduct(float u[], float v[], float cross_P[]) {
    cross_P[0] = u[1] * v[2] - u[2] * v[1];
    cross_P[1] = u[2] * v[0] - u[0] * v[2];
    cross_P[2] = u[0] * v[1] - u[1] * v[0];
}

static double norm(double* u) {
    return sqrt(dot_product(u, u));
}

static void mean(Coord* pts, Coord* mean_pts) {
    // mean_coord = [0, 0]
    for (int i = 0; i < SIZE; ++i) {
        mean_pts->x += pts[i].x / SIZE,
        mean_pts->y += pts[i].y / SIZE;
    }
}

static void vect(Coord a, Coord b, Coord* vec) {
   vec->x = a.x - b.x;
   vec->y = a.y - b.y;
}

static uint8_t signe(int x) {
    return 1 - 2 * (x < 0);
}

//TODO: ATTENTION : rotation = [0, 0, w]
// TODO: vitesse = [vx, vy, 0]
// déterminer a1 et a2 pour que ça rende bien
// E = energie = a1 * prod_scal(v, v) + a2 * dot_product(w, w)

// if cars are moving away, do not collide
// TODO ? utile ?

// car collision :
static void draw_collision(SDL_Renderer* renderer, Entity* car, SDL_Rect* pts, Camera* cam) {
    int i;
    int x;
    int y;
    int x_prev;
    int y_prev;
    int w_prev;
    int h_prev;
    int mouse_x, mouse_y;

    SDL_GetMouseState(&mouse_x, &mouse_y);

    float centre_x = car->posx - cam->x;
    float centre_y = car->posy - cam->y;

    x_prev = pts->x;
    y_prev = pts->y;
// TODO : à mettre en dehors du for
    w_prev = pts->w;
    h_prev = pts->h;
    pts->x -= cam->x;
    pts->x = (1 - cam->zoom) * centre_x + cam->zoom * pts->x;
    pts->y -= cam->y;
    pts->y = (1 - cam->zoom) * centre_y + cam->zoom * pts->y;
    pts->w *= cam->zoom;
    pts->h *= cam->zoom;

//display:
    SDL_SetRenderDrawColor(renderer, BLACK); // start checkpoint
    SDL_RenderFillRect(renderer, pts);
    pts->h = h_prev;
    pts->w = w_prev;
    pts->x = x_prev;
    pts->y = y_prev;
}
#endif


static bool get_away(Entity car1, Entity car2, Coord* v1, Coord* v2) {
    float dist_before = dist2(car1.posx, car1.posy, car2.posx, car2.posy);
    car1.posx += v1->x;
    car1.posy -= v1->y;
    car2.posx += v2->x;
    car2.posy -= v2->y;
    float dist_after = dist2(car1.posx, car1.posy, car2.posx, car2.posy);
    return dist_after >= dist_before;
}

void collision(Entity* car1, Entity* car2, Camera* cam, bool first_player) {
    // easy version
    if (!SDL_HasIntersection(&car1->frame, &car2->frame))
        return;
//    printf("collision\n");
    Coord v1 = {car1->speed * cos(car1->angle), car1->speed * sin(car1->angle)};
    Coord v2 = {car2->speed * cos(car2->angle), car2->speed * sin(car2->angle)};
    if (get_away(*car1, *car2, &v1, &v2)) {
        return;
    }
    Coord v1_tmp = {v1.x, v1.y};
    v1.x = v1.x / 3. + v2.x / 2.;
    v1.y = v1.y / 3. + v2.y / 2.;
    v2.x = v2.x / 3. + v1_tmp.x / 2.;
    v2.y = v2.y / 3. + v1_tmp.y / 2.;

    double angle_tmp_1 = atan2f(v1.y, v1.x);
    double angle_tmp_2 = atan2f(v2.y, v2.x);

    // TODO: revoir
    if ((int)(180.f * (angle_tmp_1 - car1->angle) / PI) % 360 > 180) {
        angle_tmp_1 += PI;
    }
    if ((int)(180.f * (angle_tmp_2 - car2->angle) / PI) % 360 > 180) {
        angle_tmp_2 += PI;
    }
    car1->angle = angle_tmp_1;
    car2->angle = angle_tmp_2;

    car1->speed = sqrtf(v1.x * v1.x + v1.y * v1.y);
    car2->speed = sqrtf(v2.x * v2.x + v2.y * v2.y);

    // we move the 2 cars away
    float dist_away_x = 5.f * (car2->posx - car1->posx) / distance(car1->posx, car1->posy, car2->posx, car2->posy);
    float dist_away_y = 5.f * (car2->posy - car1->posy) / distance(car1->posx, car1->posy, car2->posx, car2->posy);
    car1->posx -= dist_away_x;
    car1->posy -= dist_away_y;

    car2->posx += dist_away_x;
    car2->posy += dist_away_y;

    if (first_player) {
        cam->x += (1 - cam->zoom) * (-dist_away_x);
        cam->y += (1 - cam->zoom) * (-dist_away_y);
    }

//    printf("speed1, 2 = %f, %f\n", car1->speed, car2->speed);
    /*
     // hard version with real calculation
     SDL_Rect pts_collision;
    // calcul du point de collision
    bool are_collisions = SDL_IntersectRect(
            (const SDL_Rect*) &car1->frame,
            (const SDL_Rect*) &car2->frame,
            &pts_collision
    );
     if (!are_collisions)
        return;
    draw_collision(renderer, car1, &pts_collision, cam);
     // formule avec m = pt moyen
    Coord M = {
            pts_collision.x + pts_collision.w / 2,
            pts_collision.x + pts_collision.h / 2
    };
    // 0.1 vecteur G-M
    //TODO: check ça
    float GM[3] = {
            car1->frame.x - M.x,
            car1->frame.y - M.y,
            0.f
    };
    // 0.2 calcul du vecteur m et mo (m orthogonal). Avec m vecteur de la collision
    float v[3] = {car1->speed * cos(car1->angle), car1->speed * sin(car1->angle), 0.f};
    float w[3] = {0.f, 0.f, car1->angle};
    // vitesse du point m avant la collision
    float Vm[3];
    crossProduct(GM, w, Vm);
    float m[3] = {
            pts_collision.x -
    };
    */
}
