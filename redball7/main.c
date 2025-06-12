#include "raylib.h"
#include <stdio.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define LEVEL_COUNT 5

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Red Ball 7 - Level Select");
    SetTargetFPS(60);

    Texture2D background = LoadTexture("resources/background.png");
    Texture2D soundOn = LoadTexture("resources/sound_on.png");
    Texture2D soundOff = LoadTexture("resources/sound_off.png");
    Texture2D levelOpen = LoadTexture("resources/level_open.png");
    Texture2D levelLocked = LoadTexture("resources/level_locked.png");
    Texture2D lockIcon = LoadTexture("resources/lock.png");
    Texture2D starIcon = LoadTexture("resources/star.png");
    Texture2D quitBtn = LoadTexture("resources/quit.png");

    bool soundEnabled = true;
    int score = 3040;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawTexture(background, 0, 0, WHITE);

        DrawTexture(soundEnabled ? soundOn : soundOff, 20, 20, WHITE);

        DrawTexture(starIcon, SCREEN_WIDTH - 140, 30, WHITE);
        DrawText(TextFormat("%d", score), SCREEN_WIDTH - 100, 35, 32, DARKGRAY);

        DrawText("LEVEL SELECT", SCREEN_WIDTH/2 - MeasureText("LEVEL SELECT", 64)/2, 60, 64, RED);

        int startX = SCREEN_WIDTH/2 - (LEVEL_COUNT * 120)/2;
        int y = 180;
        for (int i = 0; i < LEVEL_COUNT; i++)
        {
            int x = startX + i * 120;
            if (i == 0) {
                DrawTexture(levelOpen, x, y, WHITE);
                DrawText(TextFormat("%d", i+1), x+35, y+30, 32, BLACK);
            } else {
                DrawTexture(levelLocked, x, y, WHITE);
                DrawTexture(lockIcon, x+30, y+30, WHITE);
                DrawText(TextFormat("%d", i+1), x+35, y+30, 32, GRAY);
            }
        }

        DrawTexture(quitBtn, SCREEN_WIDTH/2 - quitBtn.width/2, SCREEN_HEIGHT - 100, WHITE);

        EndDrawing();
    }

    UnloadTexture(background);
    UnloadTexture(soundOn);
    UnloadTexture(soundOff);
    UnloadTexture(levelOpen);
    UnloadTexture(levelLocked);
    UnloadTexture(lockIcon);
    UnloadTexture(starIcon);
    UnloadTexture(quitBtn);

    CloseWindow();
    return 0;
} 