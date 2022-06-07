#include "gui.h"

void StartGui() {
    Color textColor = LIGHTGRAY;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, string("raylib [core] example - basic window").c_str());

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
}