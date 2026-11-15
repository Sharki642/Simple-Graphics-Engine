// compile: g++ main.cpp imgui/imgui*.cpp imgui/backends/imgui_impl_sdl2.cpp imgui/backends/imgui_impl_sdlrenderer2.cpp -Iimgui -Iimgui/backends $(sdl2-config --cflags --libs) -o main
// run: ./main

#include <SDL2/SDL.h>
#include <algorithm>
#include <iostream>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

struct Point {
    float x, y;
};

struct Triangle {
    Point a, b, c;
};

struct Circle {
    Point center;
    float radius;
};

Triangle makeTriangle(){
    Triangle t;
    t.a = {100, 100};
    t.b = {200, 200};
    t.c = {400, 400};
    return t;
}

Circle makeCircle(){
    Circle c;
    c.center = {400, 300};
    c.radius = 100;
    return c;
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

bool checkCircle(const Circle& c, float px, float py){
    float dx = px - c.center.x;
    float dy = py - c.center.y;
    return (dx * dx + dy * dy) <= (c.radius * c.radius);
}

void drawPointofTriangle(SDL_Renderer* renderer, int px, int py, const Triangle& t) {
    if (checkTriangle(t, px, py)) {
        SDL_RenderDrawPoint(renderer, px, py);
    }
}

void drawPointofCircle(SDL_Renderer* renderer, int px, int py, const Circle& c) {
    if (checkCircle(c, px, py)) {
        SDL_RenderDrawPoint(renderer, px, py);
    }
}

void drawTriangle(SDL_Renderer* renderer, const Triangle& t){

    int minx = std::min({t.a.x, t.b.x, t.c.x});
    int maxx = std::max({t.a.x, t.b.x, t.c.x});

    int miny = std::min({t.a.y, t.b.y, t.c.y});
    int maxy = std::max({t.a.y, t.b.y, t.c.y});

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            drawPointofTriangle(renderer, x, y, t);
        }
    }
}

void drawCircle(SDL_Renderer* renderer, const Circle& c) {
    int minx = c.center.x - c.radius;
    int maxx = c.center.x + c.radius;
    int miny = c.center.y - c.radius;
    int maxy = c.center.y + c.radius;

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            drawPointofCircle(renderer, x, y, c);
        }
    }
}

enum class Shape { None, Triangle, Circle };

int main(){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Bruno's Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    Triangle tri = makeTriangle();
    Circle circle = makeCircle();
    Shape currentShape = Shape::None;
    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
        ImGui_ImplSDL2_NewFrame();
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui::NewFrame();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (currentShape == Shape::Triangle) {
            drawTriangle(renderer, tri);
        } else if (currentShape == Shape::Circle) {
            drawCircle(renderer, circle);
        }


        ImGui::Begin("Shapes");

if (ImGui::Button("Triangle")) {
    currentShape = Shape::Triangle;
}
ImGui::SameLine();               
if (ImGui::Button("Circle")) {
    currentShape = Shape::Circle;
}


if (currentShape == Shape::Triangle) {
    ImGui::Separator();
    ImGui::SliderFloat("A.x", &tri.a.x, 0.0f, SCREEN_WIDTH);
    ImGui::SliderFloat("A.y", &tri.a.y, 0.0f, SCREEN_HEIGHT);
    ImGui::SliderFloat("B.x", &tri.b.x, 0.0f, SCREEN_WIDTH);
    ImGui::SliderFloat("B.y", &tri.b.y, 0.0f, SCREEN_HEIGHT);
    ImGui::SliderFloat("C.x", &tri.c.x, 0.0f, SCREEN_WIDTH);
    ImGui::SliderFloat("C.y", &tri.c.y, 0.0f, SCREEN_HEIGHT);
} else if (currentShape == Shape::Circle) {
    ImGui::Separator();
    ImGui::SliderFloat("Center X", &circle.center.x, 0.0f, SCREEN_WIDTH);
    ImGui::SliderFloat("Center Y", &circle.center.y, 0.0f, SCREEN_HEIGHT);
    ImGui::SliderFloat("Radius",   &circle.radius,   1.0f, 300.0f);
}


ImGui::End();

        ImGui::Render();


        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
