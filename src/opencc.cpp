#include "opencc.h"

int main(void) {
    int screenWidth = 800;
    int screenHeight = 450;
    Color textColor = LIGHTGRAY;

    InitWindow(screenWidth, screenHeight, string("raylib [core] example - basic window").c_str());

    SetTargetFPS(60);

    while (WindowShouldClose())
    {
        BeginDrawing();
        {
            ClearBackground(RAYWHITE);
            DrawText(string("Congrats! You created your first window!").c_str(), 190, 200, 20, textColor);
        }
        EndDrawing();
    }

    return 0;
}