#define _CRT_SECURE_NO_WARNINGS
#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h> 

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define BG_WIDTH 1280
#define BG_HEIGHT 720
#define GRAVITY 0.5f
#define PLAYER_SPEED 6.0f
#define JUMP_FORCE 14.0f
#define MAX_LIVES 3
#define PLAYER_WIDTH 48.0f
#define PLAYER_HEIGHT 48.0f
#define LONG_PLATFORM 500.0f
#define SHORT_PLATFORM 120.0f

#define MAX_PLATFORMS 32
#define MAX_SPIKES 16
#define MAX_STARS 32
#define MAX_ENEMIES 8
#define MAX_GIFTS 8
#define SNOW_COUNT 100

#define BUTTON_BROWN (Color){ 139, 69, 19, 255 }
#define BUTTON_BORDER (Color){ 101, 67, 33, 255 }

#define SNOW_COLOR (Color){ 255, 255, 255, 180 }
#define MIN_SNOW_SIZE 0.8f
#define MAX_SNOW_SIZE 2.0f

Texture2D cloudTex;
#define MAX_CLOUDS 8

typedef struct {
    float x, y;
    float speed;
    float scale;
} Cloud;

Cloud clouds[MAX_CLOUDS];

typedef struct {
    float x, y, width, height;
    float vx, vy;
    bool onGround;
    int lives;
    int score;
    bool hasKey;
    bool spikeImmune;
    int spikeImmuneTimer;
    float rotation;
} Player;

typedef struct {
    float x, y, width, height;
} Platform;

typedef struct {
    float x, y;
    float speedY;
    float size;
} Snowflake;
Snowflake snowflakes[SNOW_COUNT];

typedef struct {
    float x, y, width, height;
    bool active;
} Spike, Key, Door, Flag, Gift, Star;

typedef struct {
    float x, y, width, height, speed;
    int direction;
    float leftBound, rightBound;
    bool alive;
} Enemy;

typedef enum { LEVEL_1, LEVEL_2, LEVEL_3 } GameLevel;
GameLevel currentLevel = LEVEL_1;
bool level2Unlocked = false;
bool level3Unlocked = false;

Platform platforms[MAX_PLATFORMS]; int platformCount = 0;
Spike spikes[MAX_SPIKES]; int spikeCount = 0;
Star stars[MAX_STARS]; int starCount = 0;
Enemy enemies[MAX_ENEMIES]; int enemyCount = 0;
Gift gifts[MAX_GIFTS]; int giftCount = 0;
Key key; Door door; Flag flag;

Texture2D background, playerTex, platformTex, spikeTex, enemyTex, keyTex, doorTex, flagTex, heartTex, starTex, giftTex, waterTex;
Texture2D levelOpenTex, levelLockedTex, quitTex, lockTex;
Texture2D soundOnTex, soundOffTex;

Sound jumpSnd, hitSnd, collectSnd, keySnd, doorSnd, flagSnd, buttonSnd, bgm;
Sound gameOverSnd;

Rectangle water = { 0.0f, 0.0f, BG_WIDTH, 80.0f };

bool levelComplete = false;
bool paused = false;
bool showMenu = true;
bool soundOn = true;
bool needsReset = false;
bool showHowToPlay = false;

typedef enum { RED_BALL, BALL_2 } BallType;
BallType selectedBall = RED_BALL;
bool showCharacterSelect = false;
bool ball2Unlocked = false;
bool ball2SelectionStage = false;
bool ball2Unlocked = false;
bool ball2SelectionStage = false;

Camera2D gameCamera = { 0 };

void ResetLevel(Player* player);
void UpdateGame(Player* player);
void DrawGame(Player* player);
void LoadLevel(const char* filename);

void InitClouds() {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        clouds[i].x = (float)(rand() % SCREEN_WIDTH * 2);
        clouds[i].y = (float)(rand() % (SCREEN_HEIGHT/3));
        clouds[i].speed = 0.2f + ((float)rand() / RAND_MAX) * 0.3f;
        clouds[i].scale = 0.8f + ((float)rand() / RAND_MAX) * 0.4f;
    }
}

void UpdateClouds() {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        clouds[i].x -= clouds[i].speed;
        
        if (clouds[i].x + (cloudTex.width * clouds[i].scale) < gameCamera.target.x - SCREEN_WIDTH/2) {
            clouds[i].x = gameCamera.target.x + SCREEN_WIDTH/2 + (float)(rand() % 200);
            clouds[i].y = (float)(rand() % (SCREEN_HEIGHT/3));
            clouds[i].scale = 0.8f + ((float)rand() / RAND_MAX) * 0.4f;
        }
    }
}

void HandleSpikeCollision(Player* player, Spike* spike) {
    if (soundOn) PlaySound(hitSnd);
    player->lives--;
    player->spikeImmune = true;
    player->spikeImmuneTimer = 45;
    player->vy = -JUMP_FORCE * 0.8f;
}

void UpdateWaterPosition() {
    float lowestPlatformY = platforms[0].y + platforms[0].height;
    for (int i = 1; i < platformCount; i++) {
        float y = platforms[i].y + platforms[i].height;
        if (y > lowestPlatformY) lowestPlatformY = y;
    }
    
    water.y = lowestPlatformY + 100.0f;
    water.height = (SCREEN_HEIGHT - water.y) / 4;
    water.x = 0.0f;
    water.width = 1300 * 100;
}

int main(void) {
    srand((unsigned int)time(NULL));
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Red Ball - Level 1");
    InitAudioDevice();

    gameCamera.zoom = 1.0f;
    gameCamera.rotation = 0.0f;

    for (int i = 0; i < SNOW_COUNT; i++) {
        snowflakes[i].x = (float)(rand() % SCREEN_WIDTH);
        snowflakes[i].y = (float)(rand() % SCREEN_HEIGHT);
        snowflakes[i].speedY = 1.0f + (float)(rand() % 30) / 10.0f;
        snowflakes[i].size = 1.0f + (float)(rand() % 20) / 10.0f;
    }

    background = LoadTexture("resources/yeni resorce/background.png");
    playerTex = LoadTexture("resources/yeni resorce/redball.png");
    platformTex = LoadTexture("resources/yeni resorce/longplatform.png");
    spikeTex = LoadTexture("resources/yeni resorce/spike.png");
    enemyTex = LoadTexture("resources/yeni resorce/enemy.png");
    keyTex = LoadTexture("resources/yeni resorce/key.png");
    doorTex = LoadTexture("resources/yeni resorce/short.png");
    flagTex = LoadTexture("resources/yeni resorce/flag.png");
    heartTex = LoadTexture("resources/heart.png");
    starTex = LoadTexture("resources/yeni resorce/star.png");
    giftTex = LoadTexture("resources/yeni resorce/gift.png");
    waterTex = LoadTexture("resources/yeni resorce/water.png");
    levelOpenTex = LoadTexture("resources/level_open.png");
    levelLockedTex = LoadTexture("resources/level_locked.png");
    quitTex = LoadTexture("resources/quit.png");
    lockTex = LoadTexture("resources/lock.png");
    soundOnTex = LoadTexture("resources/sound_on.png");
    soundOffTex = LoadTexture("resources/sound_off.png");

    door.width = 48.0f;
    door.height = 96.0f;

    jumpSnd = LoadSound("resources/music/jump.wav");
    hitSnd = LoadSound("resources/music/hit.wav");
    collectSnd = LoadSound("resources/music/collect.wav");
    keySnd = LoadSound("resources/music/key.wav");
    doorSnd = LoadSound("resources/music/door.wav");
    flagSnd = LoadSound("resources/music/flag.wav");
    buttonSnd = LoadSound("resources/music/button.wav");
    bgm = LoadSound("resources/music/gamemusic.wav");
    gameOverSnd = LoadSound("resources/music/gameover.wav");
    
    SetSoundVolume(bgm, 0.3f);
    SetSoundVolume(doorSnd, 0.7f);
    SetSoundVolume(gameOverSnd, 0.8f);
    PlaySound(bgm);

    SetTargetFPS(60);

    Player player;
    LoadLevel("levels/level1.lvl");
    ResetLevel(&player);
    needsReset = false;

    player.score = 0;
    ball2Unlocked = false;

    Rectangle characterSelectBtn = { 20, SCREEN_HEIGHT - 60, 140, 40 };
    Rectangle ball1Rect = { SCREEN_WIDTH / 2 - 270, 200, PLAYER_WIDTH, PLAYER_HEIGHT };
    Rectangle ball2Rect = { SCREEN_WIDTH / 2 + 50, 200, PLAYER_WIDTH, PLAYER_HEIGHT };
    Rectangle backToMenuRect = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 80, 200, 50 };
    Rectangle howToPlayRect = { SCREEN_WIDTH - 180, SCREEN_HEIGHT - 40, 160, 30 };

    InitClouds();

    while (!WindowShouldClose()) {
        if (showMenu) {
            BeginDrawing();
            DrawTexture(background, 0, 0, WHITE);
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));
            DrawText("RED BALL", SCREEN_WIDTH / 2 - MeasureText("RED BALL", 80) / 2, 100, 80, RED);

            int soundBtnX = SCREEN_WIDTH - 80;
            int soundBtnY = 20;
            Rectangle soundBtn = { soundBtnX, soundBtnY, 48, 48 };
            DrawTexture(soundOn ? soundOnTex : soundOffTex, soundBtnX, soundBtnY, WHITE);

            int btnY = 250;
            int btnSpacing = 160;
            int btnX1 = SCREEN_WIDTH / 2 - btnSpacing - 100;
            int btnX2 = SCREEN_WIDTH / 2 - levelOpenTex.width/2;
            int btnX3 = SCREEN_WIDTH / 2 + btnSpacing;
            int quitY = btnY + 200;
            int quitX = SCREEN_WIDTH / 2 - quitTex.width / 2;

            const float bottomButtonWidth = 200;
            const float bottomButtonHeight = 45;
            const int bottomButtonFontSize = 20;

            Rectangle characterSelectBtn = { 20, SCREEN_HEIGHT - bottomButtonHeight - 15, bottomButtonWidth, bottomButtonHeight };
            DrawRectangleRec(characterSelectBtn, BUTTON_BROWN);
            DrawRectangleLinesEx(characterSelectBtn, 2, BUTTON_BORDER);
            
            const char* charSelectText = "Character Select";
            int charSelectWidth = MeasureText(charSelectText, bottomButtonFontSize);
            int charSelectX = characterSelectBtn.x + (characterSelectBtn.width - charSelectWidth) / 2;
            int charSelectY = characterSelectBtn.y + (characterSelectBtn.height - bottomButtonFontSize) / 2;
            DrawText(charSelectText, charSelectX, charSelectY, bottomButtonFontSize, WHITE);

            Rectangle howToPlayRect = { SCREEN_WIDTH - bottomButtonWidth - 20, SCREEN_HEIGHT - bottomButtonHeight - 15, bottomButtonWidth, bottomButtonHeight };
            DrawRectangleRec(howToPlayRect, BUTTON_BROWN);
            DrawRectangleLinesEx(howToPlayRect, 2, BUTTON_BORDER);
            
            const char* howToPlayText = "How to Play";
            int howToPlayWidth = MeasureText(howToPlayText, bottomButtonFontSize);
            int howToPlayX = howToPlayRect.x + (howToPlayRect.width - howToPlayWidth) / 2;
            int howToPlayY = howToPlayRect.y + (howToPlayRect.height - bottomButtonFontSize) / 2;
            DrawText(howToPlayText, howToPlayX, howToPlayY, bottomButtonFontSize, WHITE);

            DrawTexture(levelOpenTex, btnX1, btnY, WHITE);
            DrawText("1", btnX1 + levelOpenTex.width / 2 - 10, btnY + levelOpenTex.height / 2 - 20, 40, BLACK);

            if (level2Unlocked) {
                DrawTexture(levelOpenTex, btnX2, btnY, WHITE);
                DrawText("2", btnX2 + levelOpenTex.width / 2 - 10, btnY + levelOpenTex.height / 2 - 20, 40, BLACK);
            }
            else {
                DrawTexture(levelLockedTex, btnX2, btnY, WHITE);
                DrawTexture(lockTex, btnX2 + levelLockedTex.width / 2 - lockTex.width / 2, btnY + levelLockedTex.height / 2 - lockTex.height / 2, WHITE);
                DrawText("2", btnX2 + levelLockedTex.width / 2 - 10, btnY + levelLockedTex.height / 2 - 20, 40, GRAY);
            }

            if (level3Unlocked) {
                DrawTexture(levelOpenTex, btnX3, btnY, WHITE);
                DrawText("3", btnX3 + levelOpenTex.width / 2 - 10, btnY + levelOpenTex.height / 2 - 20, 40, BLACK);
            }
            else {
                DrawTexture(levelLockedTex, btnX3, btnY, WHITE);
                DrawTexture(lockTex, btnX3 + levelLockedTex.width / 2 - lockTex.width / 2, btnY + levelLockedTex.height / 2 - lockTex.height / 2, WHITE);
                DrawText("3", btnX3 + levelLockedTex.width / 2 - 10, btnY + levelLockedTex.height / 2 - 20, 40, GRAY);
            }

            DrawTexture(quitTex, quitX, quitY, WHITE);

            Vector2 mouse = GetMousePosition();

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mouse, soundBtn)) {
                    soundOn = !soundOn;
                    if (soundOn) PlaySound(bgm);
                    else StopSound(bgm);
                }
                else if (CheckCollisionPointRec(mouse, characterSelectBtn)) {
                    showCharacterSelect = true;
                    showMenu = false;
                    PlaySound(buttonSnd);
                }
                else if (CheckCollisionPointRec(mouse, howToPlayRect)) {
                    showHowToPlay = true;
                    showMenu = false;
                    PlaySound(buttonSnd);
                }
                else if (CheckCollisionPointRec(mouse, (Rectangle) { btnX1, btnY, levelOpenTex.width, levelOpenTex.height })) {
                    showMenu = false;
                    currentLevel = LEVEL_1;
                    PlaySound(buttonSnd);
                    LoadLevel("levels/level1.lvl");
                    int currentScore = player.score;
                    ResetLevel(&player);
                    player.score = currentScore;
                    needsReset = false;
                }
                else if (level2Unlocked && CheckCollisionPointRec(mouse, (Rectangle) { btnX2, btnY, levelOpenTex.width, levelOpenTex.height })) {
                    showMenu = false;
                    currentLevel = LEVEL_2;
                    PlaySound(buttonSnd);
                    LoadLevel("levels/level2.lvl");
                    int currentScore = player.score;
                    ResetLevel(&player);
                    player.score = currentScore;
                    needsReset = false;
                }
                else if (level3Unlocked && CheckCollisionPointRec(mouse, (Rectangle) { btnX3, btnY, levelOpenTex.width, levelOpenTex.height })) {
                    showMenu = false;
                    currentLevel = LEVEL_3;
                    PlaySound(buttonSnd);
                    LoadLevel("levels/level3.lvl");
                    int currentScore = player.score;
                    ResetLevel(&player);
                    player.score = currentScore;
                    needsReset = false;
                }
                else if (CheckCollisionPointRec(mouse, (Rectangle) { quitX, quitY, quitTex.width, quitTex.height })) {
                    PlaySound(buttonSnd);
                    CloseWindow();
                    return 0;
                }
            }

            EndDrawing();
            continue;
        }

        if (showHowToPlay) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawTexture(background, 0, 0, WHITE);
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.85f));

            int leftX = 100;
            int rightX = SCREEN_WIDTH / 2 + 100;
            int startY = 100;
            int lineGap = 30;

            DrawText("How to Play", SCREEN_WIDTH / 2 - MeasureText("How to Play", 40) / 2, 50, 40, WHITE);

            DrawText("Controls:", leftX, startY, 30, WHITE);
            DrawText("Move Left  : Left Arrow / A", leftX, startY + lineGap, 20, WHITE);
            DrawText("Move Right : Right Arrow / D", leftX, startY + lineGap * 2, 20, WHITE);
            DrawText("Jump       : Space / Up Arrow", leftX, startY + lineGap * 3, 20, WHITE);
            DrawText("Pause      : P", leftX, startY + lineGap * 4, 20, WHITE);
            DrawText("Reset      : R", leftX, startY + lineGap * 5, 20, WHITE);
            DrawText("Menu       : M", leftX, startY + lineGap * 6, 20, WHITE);
            DrawText("Sound Toggle: S", leftX, startY + lineGap * 7, 20, WHITE);

            DrawText("Game Info:", rightX, startY, 30, WHITE);
            DrawText("-> Collect stars for points", rightX, startY + lineGap, 20, WHITE);
            DrawText("-> Avoid spikes and enemies", rightX, startY + lineGap * 2, 20, WHITE);
            DrawText("-> Jump on enemies to defeat", rightX, startY + lineGap * 3, 20, WHITE);
            DrawText("-> Collect keys to unlock doors", rightX, startY + lineGap * 4, 20, WHITE);
            DrawText("-> Reach the flag to finish", rightX, startY + lineGap * 5, 20, WHITE);
            DrawText("-> You have multiple lives", rightX, startY + lineGap * 6, 20, WHITE);
            DrawText("-> Collect gifts for bonus points", rightX, startY + lineGap * 7, 20, WHITE);

            Rectangle backToMenuRect = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 80, 200, 50 };
            DrawRectangleRec(backToMenuRect, DARKGRAY);
            DrawText("Back to Menu", backToMenuRect.x + 35, backToMenuRect.y + 15, 20, WHITE);

            Vector2 mouse = GetMousePosition();
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mouse, backToMenuRect)) {
                    showHowToPlay = false;
                    showMenu = true;
                    PlaySound(buttonSnd);
                }
            }

            EndDrawing();
            continue;
        }

        if (showCharacterSelect) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawTexture(background, 0, 0, WHITE);
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.8f));

            DrawText("Character Select", SCREEN_WIDTH / 2 - MeasureText("Character Select", 40) / 2, 50, 40, WHITE);
            DrawText(TextFormat("Score: %d", player.score), SCREEN_WIDTH / 2 - MeasureText(TextFormat("Score: %d", player.score), 30) / 2, 110, 30, WHITE);

            Rectangle characterSelectArea = { SCREEN_WIDTH/2 - 350, 140, 700, 300 };
            DrawRectangleRec(characterSelectArea, Fade(BUTTON_BROWN, 0.3f));
            DrawRectangleLinesEx(characterSelectArea, 3, BUTTON_BORDER);

            Vector2 mouse = GetMousePosition();

            DrawText("Ball 1", SCREEN_WIDTH / 2 - 250, 160, 30, WHITE);
            Rectangle ballRect = { SCREEN_WIDTH / 2 - 270, 200, PLAYER_WIDTH, PLAYER_HEIGHT };
            
            static Texture2D redBallTex = { 0 };
            if (redBallTex.id == 0) {
                redBallTex = LoadTexture("resources/yeni resorce/redball.png");
            }
            DrawTexture(redBallTex, ballRect.x, ballRect.y, WHITE);
            
            if (selectedBall == RED_BALL) {
                DrawText("Selected", SCREEN_WIDTH / 2 - 250, 270, 20, GREEN);
            }
            else if (CheckCollisionPointRec(mouse, ballRect)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    selectedBall = RED_BALL;
                    UnloadTexture(playerTex);
                    playerTex = LoadTexture("resources/yeni resorce/redball.png");
                    PlaySound(buttonSnd);
                }
            }

            DrawText("Ball 2", SCREEN_WIDTH / 2 + 50, 160, 30, WHITE);
            Rectangle ball2Rect = { SCREEN_WIDTH / 2 + 50, 200, PLAYER_WIDTH, PLAYER_HEIGHT };
            
            if (ball2Unlocked) {
                static Texture2D ball2Tex = { 0 };
                if (ball2Tex.id == 0) {
                    ball2Tex = LoadTexture("resources/yeni resorce/ball2.png");
                }
                DrawTexture(ball2Tex, ball2Rect.x, ball2Rect.y, WHITE);

                if (selectedBall == BALL_2) {
                    DrawText("Selected", SCREEN_WIDTH / 2 + 50, 270, 20, GREEN);
                }
                else if (CheckCollisionPointRec(mouse, ball2Rect)) {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        selectedBall = BALL_2;
                        UnloadTexture(playerTex);
                        playerTex = LoadTexture("resources/yeni resorce/ball2.png");
                        PlaySound(buttonSnd);
                    }
                }
            }
            else {
                DrawTexture(lockTex, ball2Rect.x, ball2Rect.y, WHITE);
                DrawText("Reach 350 score to unlock", SCREEN_WIDTH / 2 + 50, 270, 20, RED);
                
                if (player.score >= 350 && CheckCollisionPointRec(mouse, ball2Rect)) {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        ball2Unlocked = true;
                        player.score -= 350;
                        PlaySound(buttonSnd);
                    }
                }
            }
            if (ball2Unlocked) {
                static Texture2D ball2Tex = { 0 };
                if (ball2Tex.id == 0) {
                    ball2Tex = LoadTexture("resources/yeni resorce/ball2.png");
                }
                DrawTexture(ball2Tex, ball2Rect.x, ball2Rect.y, WHITE);

                if (selectedBall == BALL_2) {
                    DrawText("Selected", SCREEN_WIDTH / 2 + 50, 270, 20, GREEN);
                }
                else if (CheckCollisionPointRec(mouse, ball2Rect)) {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        selectedBall = BALL_2;
                        UnloadTexture(playerTex);
                        playerTex = LoadTexture("resources/yeni resorce/ball2.png");
                        PlaySound(buttonSnd);
                    }
                }
            }
            else {
                DrawTexture(lockTex, ball2Rect.x, ball2Rect.y, WHITE);
                DrawText("Reach 350 score to unlock", SCREEN_WIDTH / 2 + 50, 270, 20, RED);

                if (player.score >= 350 && CheckCollisionPointRec(mouse, ball2Rect)) {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        ball2Unlocked = true;
                        player.score -= 350;
                        PlaySound(buttonSnd);
                    }
                }
            }
            Rectangle backToMenuRect = { SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 80, 200, 50 };
            DrawRectangleRec(backToMenuRect, DARKGRAY);
            DrawText("Back to Menu", backToMenuRect.x + 35, backToMenuRect.y + 15, 20, WHITE);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mouse, backToMenuRect)) {
                    showCharacterSelect = false;
                    showMenu = true;
                    PlaySound(buttonSnd);
                }
            }

            EndDrawing();
            continue;
        }

        if (needsReset) {
            ResetLevel(&player);
            needsReset = false;
        }
        if (!paused && !levelComplete) UpdateGame(&player);

        if (IsKeyPressed(KEY_P)) {
            paused = !paused;
            PlaySound(buttonSnd);
        }
        if (IsKeyPressed(KEY_R)) {
            PlaySound(buttonSnd);
            if (currentLevel == LEVEL_1) {
                LoadLevel("levels/level1.lvl");
            }
            else {
                LoadLevel("levels/level2.lvl");
            }
            ResetLevel(&player);
            needsReset = false;
        }
        if (IsKeyPressed(KEY_M)) {
            showMenu = true;
            PlaySound(buttonSnd);
        }
        if (IsKeyPressed(KEY_S)) {
            soundOn = !soundOn;
            if (soundOn) PlaySound(bgm);
            else StopSound(bgm);
        }

        DrawGame(&player);
    }

    FILE* scoreWriteFile = fopen("score.txt", "w");
    if (scoreWriteFile) {
        fprintf(scoreWriteFile, "%d\n", player.score);
        fclose(scoreWriteFile);
    }

    UnloadTexture(background); UnloadTexture(playerTex); UnloadTexture(platformTex);
    UnloadTexture(spikeTex); UnloadTexture(enemyTex); UnloadTexture(keyTex);
    UnloadTexture(doorTex); UnloadTexture(flagTex); UnloadTexture(heartTex);
    UnloadTexture(starTex); UnloadTexture(giftTex); UnloadTexture(waterTex);
    UnloadTexture(levelOpenTex); UnloadTexture(levelLockedTex); UnloadTexture(quitTex);
    UnloadTexture(lockTex);
    UnloadTexture(soundOnTex); UnloadTexture(soundOffTex);
    UnloadTexture(cloudTex);
    UnloadSound(jumpSnd); UnloadSound(hitSnd); UnloadSound(collectSnd);
    UnloadSound(keySnd); UnloadSound(doorSnd); UnloadSound(flagSnd);
    UnloadSound(buttonSnd); UnloadSound(bgm);
    UnloadSound(gameOverSnd);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}

void ResetLevel(Player* player) {
    int currentScore = player->score;
    
    if (platformCount > 0) {
        player->x = platforms[0].x + 20.0f;
        player->y = platforms[0].y - PLAYER_HEIGHT;
    }
    else {
        player->x = 100.0f;
        player->y = 600.0f;
    }

    player->width = PLAYER_WIDTH;
    player->height = PLAYER_HEIGHT;
    player->vx = 0.0f;
    player->vy = 0.0f;
    player->onGround = false;
    player->lives = MAX_LIVES;
    player->hasKey = false;
    player->spikeImmune = false;
    player->spikeImmuneTimer = 0;
    player->rotation = 0.0f;
    levelComplete = false;
    
    if (player->lives <= 0 && soundOn) {
        PlaySound(gameOverSnd);
    }
    
    player->score = currentScore;

    for (int i = 0; i < spikeCount; i++) spikes[i].active = true;
    for (int i = 0; i < starCount; i++) stars[i].active = true;
    for (int i = 0; i < giftCount; i++) gifts[i].active = true;
    key.active = true;
    door.active = true;
    flag.active = true;

    for (int i = 0; i < enemyCount; i++) {
        enemies[i].x = enemies[i].leftBound;
        enemies[i].direction = 1;
        enemies[i].alive = true;
    }

    if (selectedBall == RED_BALL) {
        UnloadTexture(playerTex);
        playerTex = LoadTexture("resources/yeni resorce/redball.png");
    }
    else if (selectedBall == BALL_2) {
        UnloadTexture(playerTex);
        playerTex = LoadTexture("resources/yeni resorce/ball2.png");
    }
}

void UpdateGame(Player* player) {
    gameCamera.target = (Vector2){ player->x + player->width / 2, player->y + player->height / 2 };
    gameCamera.offset = (Vector2){ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
    gameCamera.zoom = 1.0f;

    for (int i = 0; i < SNOW_COUNT; i++) {
        snowflakes[i].y += snowflakes[i].speedY;
        snowflakes[i].x += (float)(rand() % 3 - 1) * 0.3f;
        
        if (snowflakes[i].y > gameCamera.target.y + SCREEN_HEIGHT/2) {
            snowflakes[i].y = gameCamera.target.y - SCREEN_HEIGHT/2;
            snowflakes[i].x = gameCamera.target.x + (float)(rand() % SCREEN_WIDTH - SCREEN_WIDTH/2);
            snowflakes[i].speedY = 0.8f + (float)(rand() % 20) / 10.0f;
            snowflakes[i].size = MIN_SNOW_SIZE + 
                               (float)(rand() % (int)((MAX_SNOW_SIZE - MIN_SNOW_SIZE) * 10)) / 10.0f;
        }
        
        if (snowflakes[i].x < gameCamera.target.x - SCREEN_WIDTH/2) {
            snowflakes[i].x = gameCamera.target.x + SCREEN_WIDTH/2;
        }
        if (snowflakes[i].x > gameCamera.target.x + SCREEN_WIDTH/2) {
            snowflakes[i].x = gameCamera.target.x - SCREEN_WIDTH/2;
        }
    }

    if (player->spikeImmune) {
        player->spikeImmuneTimer--;
        if (player->spikeImmuneTimer <= 0)
            player->spikeImmune = false;
    }

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        player->vx = -PLAYER_SPEED;
        player->rotation -= 5.0f;
    }
    else if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        player->vx = PLAYER_SPEED;
        player->rotation += 5.0f;
    }
    else {
        player->vx *= 0.8f;
    }

    if ((IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_UP)) && player->onGround) {
        player->vy = -JUMP_FORCE;
        player->onGround = false;
        if (soundOn)
            PlaySound(jumpSnd);
    }

    player->x += player->vx;
    for (int i = 0; i < platformCount; i++) {
        Rectangle plat = { platforms[i].x, platforms[i].y, platforms[i].width, platforms[i].height };
        Rectangle ply = { player->x, player->y, player->width, player->height };
        if (CheckCollisionRecs(ply, plat) &&
            !(player->y + player->height <= plat.y + 5 && player->y + player->height > plat.y - 10)) {
            if (player->vx > 0)
                player->x = plat.x - player->width;
            else if (player->vx < 0)
                player->x = plat.x + plat.width;
        }
    }

    player->vy += GRAVITY;
    player->y += player->vy;
    player->onGround = false;
    for (int i = 0; i < platformCount; i++) {
        Rectangle plat = { platforms[i].x, platforms[i].y, platforms[i].width, platforms[i].height };
        Rectangle ply = { player->x, player->y, player->width, player->height };
        if (CheckCollisionRecs(ply, plat)) {
            float playerBottomPrev = player->y - player->vy + player->height;
            float platformTop = plat.y;
            if (player->vy > 0 && playerBottomPrev <= platformTop + 1) {
                player->y = plat.y - player->height;
                player->vy = 0.0f;
                player->onGround = true;
            }
            else if (player->vy < 0) {
                player->y = plat.y + plat.height;
                player->vy = 0.0f;
            }
        }
    }

    if (!player->spikeImmune) {
        for (int i = 0; i < spikeCount; i++) {
            if (!spikes[i].active) continue;
            Rectangle spk = { spikes[i].x, spikes[i].y, spikes[i].width, spikes[i].height };
            Rectangle ply = { player->x, player->y, player->width, player->height };
            if (CheckCollisionRecs(ply, spk)) {
                HandleSpikeCollision(player, &spikes[i]);
                if (player->lives <= 0) {
                    needsReset = true;
                    return;
                }
                break;
            }
        }
    }

    if (player->y > water.y) {
        if (soundOn) {
            PlaySound(gameOverSnd);
        }
        needsReset = true;
        return;
    }

    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].alive) continue;
        enemies[i].x += enemies[i].speed * enemies[i].direction;
        if (enemies[i].x <= enemies[i].leftBound) {
            enemies[i].x = enemies[i].leftBound;
            enemies[i].direction = 1;
        }
        if (enemies[i].x + enemies[i].width >= enemies[i].rightBound) {
            enemies[i].x = enemies[i].rightBound - enemies[i].width;
            enemies[i].direction = -1;
        }
        Rectangle enm = { enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height };
        Rectangle ply = { player->x, player->y, player->width, player->height };
        if (CheckCollisionRecs(ply, enm)) {
            if (player->vy > 0 && player->y + player->height - player->vy <= enemies[i].y + 10.0f) {
                if (soundOn)
                    PlaySound(hitSnd);
                enemies[i].alive = false;
                player->vy = -JUMP_FORCE * 0.7f;
                player->score += 50;
            }
            else {
                if (soundOn)
                    PlaySound(hitSnd);
                player->lives--;
                player->spikeImmune = true;
                player->spikeImmuneTimer = 45;
                player->vy = -JUMP_FORCE * 0.8f;
                if (player->vx >= 0.0f)
                    player->x -= 30.0f;
                else
                    player->x += 30.0f;
                if (player->lives <= 0) {
                    needsReset = true;
                    return;
                }
            }
            break;
        }
    }

    if (key.active) {
        Rectangle k = { key.x, key.y, key.width, key.height };
        Rectangle ply = { player->x, player->y, player->width, player->height };
        if (CheckCollisionRecs(ply, k)) {
            if (soundOn)
                PlaySound(keySnd);
            player->hasKey = true;
            key.active = false;
        }
    }

    if (door.active) {
        Rectangle d = { door.x, door.y, door.width, door.height };
        Rectangle ply = { player->x, player->y, player->width, player->height };
        if (CheckCollisionRecs(ply, d)) {
            if (player->hasKey) {
                if (soundOn) {
                    PlaySound(doorSnd);
                }
                float maxPlayerX = BG_WIDTH * 100 - player->width;
                player->x = door.x + door.width + 1;
                if (player->x > maxPlayerX)
                    player->x = maxPlayerX;
            }
            else {
                if (player->vx > 0)
                    player->x = door.x - player->width;
                else if (player->vx < 0)
                    player->x = door.x + door.width;
            }
        }
    }

    if (flag.active) {
        Rectangle f = { flag.x, flag.y, flag.width, flag.height };
        Rectangle ply = { player->x, player->y, player->width, player->height };
        if (CheckCollisionRecs(ply, f)) {
            if (soundOn)
                PlaySound(flagSnd);
            flag.active = false;
            levelComplete = true;
            FILE* scoreFile = fopen("score.txt", "w");
            if (scoreFile) {
                if (player->score > 0) {
                    fprintf(scoreFile, "%d\n", player->score);
                }
                fclose(scoreFile);
            }
            if (currentLevel == LEVEL_1) {
                level2Unlocked = true;
            }
            else if (currentLevel == LEVEL_2) {
                level3Unlocked = true;
            }
        }
    }

    for (int i = 0; i < starCount; i++) {
        if (!stars[i].active) continue;
        Rectangle s = { stars[i].x, stars[i].y, stars[i].width, stars[i].height };
        Rectangle ply = { player->x, player->y, player->width, player->height };
        if (CheckCollisionRecs(ply, s)) {
            if (soundOn)
                PlaySound(collectSnd);
            stars[i].active = false;
            player->score += 10;
        }
    }

    for (int i = 0; i < giftCount; i++) {
        if (!gifts[i].active) continue;
        Rectangle g = { gifts[i].x, gifts[i].y, gifts[i].width, gifts[i].height };
        Rectangle ply = { player->x, player->y, player->width, player->height };
        if (CheckCollisionRecs(ply, g)) {
            if (soundOn)
                PlaySound(collectSnd);
            gifts[i].active = false;
            player->score += 25;
        }
    }

    if (currentLevel == LEVEL_1 && levelComplete) level2Unlocked = true;

    UpdateClouds();
}

void DrawGame(Player* player) {
    float levelWidth = BG_WIDTH * 100;
    float camX = gameCamera.target.x - SCREEN_WIDTH / 2;
    int firstTile = (int)(camX / BG_WIDTH) - 1;
    if (firstTile < 0) firstTile = 0;
    int lastTile = (int)((camX + SCREEN_WIDTH) / BG_WIDTH) + 2;
    int maxTile = (int)(levelWidth / BG_WIDTH) + 2;
    if (lastTile > maxTile) lastTile = maxTile;
    
    if (gameCamera.target.x < SCREEN_WIDTH / 2) gameCamera.target.x = SCREEN_WIDTH / 2;
    if (gameCamera.target.x > levelWidth - SCREEN_WIDTH / 2) gameCamera.target.x = levelWidth - SCREEN_WIDTH / 2;

    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode2D(gameCamera);
    
    float camWaterStart = camX - BG_WIDTH * 4;
    float camWaterEnd = camX + SCREEN_WIDTH + BG_WIDTH * 4;
    for (float wx = camWaterStart; wx < camWaterEnd; wx += waterTex.width) {
        DrawTexture(waterTex, (int)wx, (int)water.y, Fade(WHITE, 0.9f));
    }

    for (int i = -15; i < lastTile + 15; i++) {
        DrawTexture(background, i * BG_WIDTH, water.y - BG_HEIGHT * 1.5f, WHITE);
        
        DrawTexture(background, i * BG_WIDTH, water.y - BG_HEIGHT, WHITE);
    }

    for (int i = 0; i < MAX_CLOUDS; i++) {
        Rectangle source = { 0, 0, cloudTex.width, cloudTex.height };
        Rectangle dest = { 
            clouds[i].x, 
            clouds[i].y, 
            cloudTex.width * clouds[i].scale, 
            cloudTex.height * clouds[i].scale 
        };
        Vector2 origin = { 0, 0 };
        DrawTexturePro(cloudTex, source, dest, origin, 0, Fade(WHITE, 0.9f));
    }

    for (int i = 0; i < SNOW_COUNT; i++) {
        DrawCircleV((Vector2){snowflakes[i].x, snowflakes[i].y}, 
                   snowflakes[i].size, 
                   Fade(SNOW_COLOR, 0.6f));
    }

    for (int i = 0; i < platformCount; i++) {
        DrawTextureRec(platformTex,
            (Rectangle) { 0, 0, platforms[i].width, platforms[i].height },
            (Vector2) { (int)platforms[i].x, (int)platforms[i].y },
            WHITE);
    }

    for (int i = 0; i < spikeCount; i++) {
        if (spikes[i].active) {
            DrawTexture(spikeTex, (int)spikes[i].x, (int)spikes[i].y, WHITE);
        }
    }

    for (int i = 0; i < starCount; i++) {
        if (stars[i].active) {
            DrawTexture(starTex, (int)stars[i].x, (int)stars[i].y, WHITE);
        }
    }

    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].alive) {
            DrawTexture(enemyTex, (int)enemies[i].x, (int)enemies[i].y, WHITE);
        }
    }

    if (key.active) {
        DrawTexture(keyTex, (int)key.x, (int)key.y, WHITE);
    }

    if (door.active) {
        DrawTextureRec(doorTex, 
            (Rectangle) { 0, 0, door.width, door.height },
            (Vector2) { (int)door.x, (int)door.y },
            WHITE);
    }

    if (flag.active) {
        DrawTexture(flagTex, (int)flag.x, (int)flag.y, WHITE);
    }

    for (int i = 0; i < giftCount; i++) {
        if (gifts[i].active) {
            DrawTexture(giftTex, (int)gifts[i].x, (int)gifts[i].y, WHITE);
        }
    }

    DrawTexturePro(playerTex,
        (Rectangle) { 0, 0, player->width, player->height },
        (Rectangle) { player->x + player->width / 2, 
                     player->y + player->height / 2, 
                     player->width, player->height },
        (Vector2) { player->width / 2, player->height / 2 },
        player->rotation,
        WHITE);

    EndMode2D();

    for (int i = 0; i < player->lives; i++)
        DrawTexture(heartTex, 20 + i * 50, 20, WHITE);

    DrawText(TextFormat("Skor: %d", player->score), SCREEN_WIDTH - 200, 20, 30, DARKGRAY);

    DrawText("P: Pause", 20, 80, 30, GRAY);
    DrawText("R: Reset", 20, 120, 30, GRAY);
    DrawText("M: Menü", SCREEN_WIDTH - 200, 60, 30, GRAY);
    DrawText("S: Ses", SCREEN_WIDTH - 200, 100, 30, GRAY);
    if (paused)
        DrawText("PAUSE", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, 60, RED);
    if (levelComplete) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));
        DrawText("Bolum Tamamlandi!", SCREEN_WIDTH / 2 - MeasureText("Bolum Tamamlandi!", 40) / 2, SCREEN_HEIGHT / 2 - 20, 40, YELLOW);
        DrawText("Menuye donmek icin M'ye bas", SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 + 40, 30, WHITE);
    }
    
    EndDrawing();
}

void LoadLevel(const char* filename) {
    platformCount = spikeCount = starCount = enemyCount = giftCount = 0;
    
    char fullPath[256];
    snprintf(fullPath, sizeof(fullPath), "../%s", filename);
    FILE* file = fopen(fullPath, "r");
    if (!file) {
        return;
    }

    char line[256];
    char type[32] = { 0 };
    float x, y, w, h;
    float speed, left, right;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        if (sscanf(line, "%31s", type) != 1) continue;

        if (strcmp(type, "PLATFORM") == 0) {
            if (platformCount < MAX_PLATFORMS &&
                sscanf(line, "%*s %f %f %f %f", &x, &y, &w, &h) == 4) {
                platforms[platformCount++] = (Platform){ x, y, w, h };
            }
        }
        else if (strcmp(type, "SPIKE") == 0 || strcmp(type, "SPIKE_RESET") == 0) {
            if (spikeCount < MAX_SPIKES &&
                sscanf(line, "%*s %f %f %f %f", &x, &y, &w, &h) == 4) {
                for (int i = 0; i < platformCount; i++) {
                    if (x >= platforms[i].x && x + w <= platforms[i].x + platforms[i].width) {
                        y = platforms[i].y - h;
                        break;
                    }
                }
                spikes[spikeCount++] = (Spike){ x, y, w, h, true };
            }
        }
        else if (strcmp(type, "STAR") == 0) {
            if (starCount < MAX_STARS &&
                sscanf(line, "%*s %f %f %f %f", &x, &y, &w, &h) == 4) {
                for (int i = 0; i < platformCount; i++) {
                    if (x >= platforms[i].x && x + w <= platforms[i].x + platforms[i].width) {
                        y = platforms[i].y - h;
                        break;
                    }
                }
                stars[starCount++] = (Star){ x, y, w, h, true };
            }
        }
        else if (strcmp(type, "ENEMY") == 0) {
            if (enemyCount < MAX_ENEMIES &&
                sscanf(line, "%*s %f %f %f %f %f %f %f", &x, &y, &w, &h, &speed, &left, &right) == 7) {
                enemies[enemyCount++] = (Enemy){ x, y, w, h, speed, 1, left, right, true };
            }
        }
        else if (strcmp(type, "GIFT") == 0) {
            if (giftCount < MAX_GIFTS &&
                sscanf(line, "%*s %f %f %f %f", &x, &y, &w, &h) == 4) {
                gifts[giftCount++] = (Gift){ x, y, w, h, true };
            }
        }
        else if (strcmp(type, "KEY") == 0) {
            if (sscanf(line, "%*s %f %f %f %f", &x, &y, &w, &h) == 4) {
                key = (Key){ x, y, w, h, true };
            }
        }
        else if (strcmp(type, "DOOR") == 0) {
            if (sscanf(line, "%*s %f %f %f %f", &x, &y, &w, &h) == 4) {
                door.x = x;
                door.y = y;
                door.width = w;
                door.height = h;
                door.active = true;
            }
        }
        else if (strcmp(type, "FLAG") == 0) {
            if (sscanf(line, "%*s %f %f %f %f", &x, &y, &w, &h) == 4) {
                flag = (Flag){ x, y, w, h, true };
            }
        }
    }
    
    fclose(file);
    UpdateWaterPosition();
}

void DrawMenuButton(Rectangle bounds, const char* text, bool isLocked) {
    DrawRectangleRec(bounds, BUTTON_BROWN);
    
    DrawRectangleLinesEx(bounds, 3, BUTTON_BORDER);
    
    int fontSize = 40;
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, fontSize, 1);
    
    float textX = bounds.x + (bounds.width - textSize.x) / 2;
    float textY = bounds.y + (bounds.height - textSize.y) / 2;
    
    if (isLocked) {
        DrawText(text, textX, textY, fontSize, GRAY);
        DrawTexture(lockTex, bounds.x + bounds.width/2 - lockTex.width/2, 
                   bounds.y + bounds.height/2 - lockTex.height/2, WHITE);
    } else {
        DrawText(text, textX, textY, fontSize, WHITE);
    }
}

