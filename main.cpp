#include <SDL2/SDL.h>


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

struct Point {
    float x, y;
};

struct Triangle {
    Point a, b, c;
};

Triangle makeTriangle(){
    Triangle t;
    t.a = {100, 100};
    t.b = {200, 100};
    t.c = {150, 200};
    return t;
}

bool checkTriangle(const Triangle& t, float px, float py){
    // Check if the pixel is inside the triangle using barycentric coordinates
    const Point& a = t.a;
    const Point& b = t.b;
    const Point& c = t.c;

    float denom = (b.y - c.y)*(a.x - c.x) + (c.x - b.x)*(a.y - c.y);
    if (denom == 0) return false; // degenerate triangle (all points on a line)

    float alpha = ((b.y - c.y)*(px - c.x) + (c.x - b.x)*(py - c.y)) / denom;
    float beta  = ((c.y - a.y)*(px - c.x) + (a.x - c.x)*(py - c.y)) / denom;
    float gamma = 1.0f - alpha - beta;

    return alpha >= 0 && beta >= 0 && gamma >= 0;
}

void drawPointofTriangle(SDL_Renderer* renderer, int px, int py, const Triangle& t) {
    if (checkTriangle(t, px, py)) {
        SDL_RenderDrawPoint(renderer, px, py);
    }
}

void drawTriangle(SDL_Renderer* renderer, const Triangle& t){
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            drawPointofTriangle(renderer, x, y, t);
        }
    }
}


int main(){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Graphics Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    Triangle tri = makeTriangle();
    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        drawTriangle(renderer, tri);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
