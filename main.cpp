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

//2d screen
struct Point {
    float x, y;
};

//3d point
struct Point3 {
    float x, y, z;
};

struct Triangle {
    Point a, b, c;
};

struct Triangle3 {
    Point3 a, b, c;
};

struct Circle {
    Point center;
    float radius;
};

//3d circle
struct Circle3 {
    Point3 center;
    float radius;
};

struct Camera {
    Point3 pos = {0, 0, 0};
    float speed = 3.0f;   // world units per second
    float f = 400.0f;     // focal length: distance eye to screen 
};

Triangle3 makeTriangle(){
    Triangle3 t;
    // 5 units in front of the camera
    t.a = {-1, -1, 5};
    t.b = { 1, -1, 5};
    t.c = { 0,  1, 5};
    return t;
}

Circle3 makeCircle(){
    Circle3 c;
    c.center = {0, 0, 5};
    c.radius = 1;
    return c;
}

bool project(const Point3& p, const Camera& cam, Point& out){
    //(world - camera)
    float x = p.x - cam.pos.x;
    float y = p.y - cam.pos.y;
    float z = p.z - cam.pos.z;

    //(division by 0 / mirrored image)
    if (z <= 0.1f) return false;

    //(similar triangles: screen / f = world / z)
    //    + half the screen size moves (0,0) to the middle of the window
    //    y is negated because in SDL y points down, but we want +y = up
    out.x =  (x / z) * cam.f + SCREEN_WIDTH  / 2.0f;
    out.y = -(y / z) * cam.f + SCREEN_HEIGHT / 2.0f;
    return true;
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
    int minx = std::max(0,                 (int)std::min({t.a.x, t.b.x, t.c.x}));
    int maxx = std::min(SCREEN_WIDTH - 1,  (int)std::max({t.a.x, t.b.x, t.c.x}));
    int miny = std::max(0,                 (int)std::min({t.a.y, t.b.y, t.c.y}));
    int maxy = std::min(SCREEN_HEIGHT - 1, (int)std::max({t.a.y, t.b.y, t.c.y}));

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            drawPointofTriangle(renderer, x, y, t);
        }
    }
}

// Projects it to 2d screen
void drawTriangle3(SDL_Renderer* renderer, const Triangle3& world, const Camera& cam){
    Triangle t;
    if (!project(world.a, cam, t.a)) return;
    if (!project(world.b, cam, t.b)) return;
    if (!project(world.c, cam, t.c)) return;
    drawTriangle(renderer, t);
}

void drawCircle(SDL_Renderer* renderer, const Circle& c) {
    int minx = std::max(0,                 (int)(c.center.x - c.radius));
    int maxx = std::min(SCREEN_WIDTH - 1,  (int)(c.center.x + c.radius));
    int miny = std::max(0,                 (int)(c.center.y - c.radius));
    int maxy = std::min(SCREEN_HEIGHT - 1, (int)(c.center.y + c.radius));

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            drawPointofCircle(renderer, x, y, c);
        }
    }
}

void drawCircle3(SDL_Renderer* renderer, const Circle3& world, const Camera& cam) {
    Circle c;
    if (!project(world.center, cam, c.center)) return;
    float z = world.center.z - cam.pos.z;
    c.radius = (world.radius / z) * cam.f;
    drawCircle(renderer, c);
}

enum class Shape { None, Triangle, Circle };

int main(){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Bruno's Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    Triangle3 tri = makeTriangle();
    Circle3 circle = makeCircle();
    Camera cam;
    Shape currentShape = Shape::None;
    SDL_Event e;
    bool quit = false;
    Uint64 lastTime = SDL_GetTicks64();
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        // time since last frame in seconds -> movement speed independent of FPS
        Uint64 now = SDL_GetTicks64();
        float dt = (now - lastTime) / 1000.0f;
        lastTime = now;

        ImGui_ImplSDL2_NewFrame();
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui::NewFrame();

        // camera movement
        if (!ImGui::GetIO().WantCaptureKeyboard) {
            const Uint8* keys = SDL_GetKeyboardState(nullptr);
            if (keys[SDL_SCANCODE_W])      cam.pos.z += cam.speed * dt; // forward
            if (keys[SDL_SCANCODE_S])      cam.pos.z -= cam.speed * dt; // back
            if (keys[SDL_SCANCODE_A])      cam.pos.x -= cam.speed * dt; // left
            if (keys[SDL_SCANCODE_D])      cam.pos.x += cam.speed * dt; // right
            if (keys[SDL_SCANCODE_SPACE])  cam.pos.y += cam.speed * dt; // up
            if (keys[SDL_SCANCODE_LSHIFT]) cam.pos.y -= cam.speed * dt; // down
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (currentShape == Shape::Triangle) {
            drawTriangle3(renderer, tri, cam);
        } else if (currentShape == Shape::Circle) {
            drawCircle3(renderer, circle, cam);
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
    ImGui::SliderFloat3("A", &tri.a.x, -10.0f, 10.0f);
    ImGui::SliderFloat3("B", &tri.b.x, -10.0f, 10.0f);
    ImGui::SliderFloat3("C", &tri.c.x, -10.0f, 10.0f);
} else if (currentShape == Shape::Circle) {
    ImGui::Separator();
    ImGui::SliderFloat3("Center", &circle.center.x, -10.0f, 10.0f);
    ImGui::SliderFloat("Radius",  &circle.radius,   0.1f, 5.0f);
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
