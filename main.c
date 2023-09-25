#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "/opt/homebrew/Cellar/raylib/4.5.0/include/raylib.h"
#include "/opt/homebrew/Cellar/raylib/4.5.0/include/raymath.h"
#include "terrainGen.c"
#include "magicCircle.c"
#include "enemy.c"
#include "boss.c"

static int playerRadius = 20;
static int tileSize = 40;

typedef enum gameState
{
    menu,
    game,
    pause,
    gameOver
} gameState;

bool rectCollision(Rectangle rect, Vector2 point)
{
    return (rect.x < point.x && rect.x + rect.width > point.x && rect.y < point.y && rect.y + rect.height > point.y);
}

int main()
{
    time_t t;
    srand((unsigned)time(&t));
    // 1120,1280 är *5
    v2f windowSize = {1120, 1286};
    InitWindow(windowSize.x, windowSize.y, "SpellJam");

    // ------------
    // Get textures
    // ------------
    Texture2D worldSprites = LoadTexture("Assets/world.png");

    bool upDown = false;
    bool downDown = false;
    bool leftDown = false;
    bool rightDown = false;
    bool squarePressed = false;
    bool trianglePressed = false;
    bool circlePressed = false;
    bool executePressed = false;

    int moveSpeed = 150;

    const int maxSpellEntities = 16;
    SpellEntity spellEntities[maxSpellEntities];
    for (int i = 0; i < maxSpellEntities; i++)
    {
        spellEntities[i] = (SpellEntity){0};
    }

    v2f playerPosition = {windowSize.x / 2, windowSize.y / 2};
    v2f playerAim = {0, 0};
    v2f spellAim = {0, -1};

    Incantation magicCircle[16];
    int ringCount = 0;
    float angle = 0;

    Enemy enemies[32] = {0};
    enemies[0] = (Enemy){(Vector2){windowSize.x / 2, windowSize.y / 2}, (Vector2){1, 1}, 100, 100, 100, {0}, warrior, chase};
    enemies[1] = (Enemy){(Vector2){windowSize.x / 2, windowSize.y / 2}, (Vector2){1, 1}, 100, 100, 100, {0}, mage, chase};
    enemies[2] = (Enemy){(Vector2){windowSize.x / 2, windowSize.y / 2}, (Vector2){1, 1}, 80, 100, 100, {0}, mage, chase};
    enemies[3] = (Enemy){(Vector2){windowSize.x / 2, windowSize.y / 2}, (Vector2){1, 1}, 80, 100, 100, {0}, warrior, chase};

    Room room = DrunkardsWalk(false, false, false, false, 2500, (Point){14, 14});

    while (!WindowShouldClose())
    {
        // ------
        // INPUTS
        // ------
        upDown = IsKeyDown(KEY_W);
        downDown = IsKeyDown(KEY_S);
        leftDown = IsKeyDown(KEY_A);
        rightDown = IsKeyDown(KEY_D);
        squarePressed = IsKeyPressed(KEY_LEFT);
        trianglePressed = IsKeyPressed(KEY_UP);
        circlePressed = IsKeyPressed(KEY_RIGHT);
        executePressed = IsKeyPressed(KEY_DOWN);

        playerAim = (v2f){0, 0};
        if (upDown)
        {
            playerAim.y = -1;
        }
        if (downDown)
        {
            playerAim.y = 1;
        }
        if (leftDown)
        {
            playerAim.x = -1;
        }
        if (rightDown)
        {
            playerAim.x = 1;
        }
        v2f playerRadiusVector = {playerRadius * playerAim.x, playerRadius * playerAim.y};
        playerAim = Vector2Normalize(playerAim);

        // -----------
        // Move Player
        // -----------
        v2f redRect = {0, 0};
        bool redRectX = false;
        bool redRectY = false;
        v2f playerMovePosition = Vector2Add(playerPosition, Vector2Scale(playerAim, (moveSpeed * GetFrameTime())));
        if (room.data[(int)((playerMovePosition.x + playerRadiusVector.x) / tileSize)][(int)((playerMovePosition.y + playerRadiusVector.y) / tileSize)][0] != TILE_TYPE_WALL)
        {
            playerPosition.x = playerMovePosition.x;
            playerPosition.y = playerMovePosition.y;
        }
        else
        {
            redRect = (v2f){(int)((playerMovePosition.x + playerRadiusVector.x) / tileSize), (int)((playerMovePosition.y + playerRadiusVector.y) / tileSize)};
            if (room.data[(int)((playerMovePosition.x + playerRadiusVector.x) / tileSize)][(int)((playerPosition.y + playerRadiusVector.y) / tileSize)][0] != TILE_TYPE_WALL)
            {
                redRectX = true;
                playerPosition.x = playerMovePosition.x;
            }
            else if (room.data[(int)((playerPosition.x + playerRadiusVector.x) / tileSize)][(int)((playerMovePosition.y + playerRadiusVector.y) / tileSize)][0] != TILE_TYPE_WALL)
            {
                redRectY = true;
                playerPosition.y = playerMovePosition.y;
            }
        }

        // -------------
        // Spell Casting
        // -------------
        if (playerAim.x != 0 || playerAim.y != 0)
        {
            spellAim = playerAim;
        }

        if (executePressed)
        {
            magicCircle[ringCount] = execute;
            for (int i = 0; i < spellBookCount; i++)
            {
                for (int j = 0; j < ringCount + 1; j++)
                {
                    if (magicCircle[j] != spellBook[i].Incantation[j])
                    {
                        break;
                    }
                    else
                    {
                        if (magicCircle[j] == execute)
                        {
                            for (int k = 0; k < maxSpellEntities; k++)
                            {
                                if (spellEntities[k].lifetime <= 0)
                                {
                                    spellEntities[k] = (SpellEntity){spellBook[i].startingLifeTime, spellBook[i].name, playerPosition, Vector2Normalize(spellAim)};
                                    goto spellCast;
                                }
                            }
                        }
                    }
                }
            }
        spellCast:
            ringCount = 0;
        }
        if (ringCount < 16)
        {
            if (squarePressed)
            {
                magicCircle[ringCount] = square;
                ringCount++;
            }
            if (trianglePressed)
            {
                magicCircle[ringCount] = triangle;
                ringCount++;
            }
            if (circlePressed)
            {
                magicCircle[ringCount] = circle;
                ringCount++;
            }
        }

        // -------------
        // SPELL UPDATES
        // -------------
        for (int i = 0; i < maxSpellEntities; i++)
        {
            if (spellEntities[i].lifetime > 0)
            {
                switch (spellEntities[i].name)
                {
                case manaSpark:
                    spellEntities[i].position.x += spellEntities[i].aim.x * 500 * GetFrameTime();
                    spellEntities[i].position.y += spellEntities[i].aim.y * 500 * GetFrameTime();
                    if (!rectCollision((Rectangle){0, 0, windowSize.x, windowSize.y}, spellEntities[i].position))
                    {
                        spellEntities[i].lifetime = 0;
                    }
                    for (int j = 0; j < 32; j++)
                    {
                        if (enemies[j].health > 0)
                        {
                            if (CheckCollisionCircles(spellEntities[i].position, 10, enemies[j].position, enemyRadius))
                            {
                                enemies[j].health -= 10;
                                spellEntities[i].lifetime = 0;
                            }
                        }
                    }
                    break;

                case fireBall:
                    spellEntities[i].position.x += spellEntities[i].aim.x * 500 * GetFrameTime();
                    spellEntities[i].position.y += spellEntities[i].aim.y * 500 * GetFrameTime();
                    if (!rectCollision((Rectangle){0, 0, windowSize.x, windowSize.y}, spellEntities[i].position))
                    {
                        spellEntities[i].lifetime = 0;
                    }
                    for (int j = 0; j < 32; j++)
                    {
                        if (enemies[j].health > 0)
                        {
                            if (CheckCollisionCircles(spellEntities[i].position, 10, enemies[j].position, enemyRadius))
                            {
                                enemies[j].health -= 20;
                                spellEntities[i].lifetime = 0;
                                for (int buffIndex = 0; buffIndex < 8; buffIndex++)
                                {
                                    if (enemies[j].buffs[buffIndex].duration <= 0)
                                    {
                                        enemies[j].buffs[buffIndex] = (Buff){5, 5};
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    break;

                case block:
                    spellEntities[i].position = playerPosition;
                    spellEntities[i].lifetime -= GetFrameTime();
                    break;

                case moonBeam:
                    spellEntities[i].lifetime -= GetFrameTime();
                    for (int j = 0; j < 32; j++)
                    {
                        if (enemies[j].health > 0)
                        {
                            if (CheckCollisionCircles(spellEntities[i].position, 50, enemies[j].position, enemyRadius))
                            {
                                enemies[j].health -= 20 * GetFrameTime();
                            }
                        }
                    }
                    break;

                case chromaticOrb:
                    spellEntities[i].position.x += spellEntities[i].aim.x * 300 * GetFrameTime();
                    spellEntities[i].position.y += spellEntities[i].aim.y * 300 * GetFrameTime();
                    spellEntities[i].lifetime += GetFrameTime();
                    if (!rectCollision((Rectangle){0, 0, windowSize.x, windowSize.y}, spellEntities[i].position))
                    {
                        spellEntities[i].lifetime = 0;
                    }
                    for (int j = 0; j < 32; j++)
                    {
                        if (enemies[j].health > 0)
                        {
                            if (CheckCollisionCircles(spellEntities[i].position, 16, enemies[j].position, enemyRadius))
                            {
                                enemies[j].health -= 150 * GetFrameTime();
                            }
                        }
                    }
                    break;

                default:
                    break;
                }
            }
        }

        // ------------
        // ENEMY UPDATE
        // ------------
        for (int i = 0; i < 32; i++)
        {
            if (enemies[i].health > 0)
            {
                UpdateEnemy(&enemies[i], playerPosition);
            }
        }

        // ------
        // RENDER
        // ------
        BeginDrawing();
        ClearBackground(BLACK);
        // Draw Rooms
        for (int i = 0; i < roomSize; i++)
        {
            for (int j = 0; j < roomSize; j++)
            {
                switch (room.data[i][j][0])
                {
                case TILE_TYPE_WALL:
                    DrawTextureRec(worldSprites, (Rectangle){0, 40, 40, 80}, (Vector2){i * tileSize, j * tileSize}, (Color){255, 255, 255, 255});
                    break;
                case TILE_TYPE_DOOR_NORTH:
                    DrawRectangle(i * tileSize, j * tileSize, tileSize, tileSize, (Color){255, 255, 0, 255});
                    break;
                case TILE_TYPE_DOOR_EAST:
                    DrawRectangle(i * tileSize, j * tileSize, tileSize, tileSize, (Color){255, 0, 255, 255});
                    break;
                case TILE_TYPE_DOOR_SOUTH:
                    DrawRectangle(i * tileSize, j * tileSize, tileSize, tileSize, (Color){0, 255, 255, 255});
                    break;
                case TILE_TYPE_DOOR_WEST:
                    DrawRectangle(i * tileSize, j * tileSize, tileSize, tileSize, (Color){0, 255, 0, 255});
                    break;
                case TILE_TYPE_EMPTY:
                    DrawTextureRec(worldSprites, (Rectangle){0, 0, 40, 40}, (Vector2){i * tileSize, j * tileSize}, (Color){255, 255, 255, 255});
                    break;
                case SCHEDULED_FOR_DELETE:
                    DrawRectangle(i * tileSize, j * tileSize, tileSize, tileSize, (Color){80, 40, 45, 255});
                default:
                    break;
                }

                switch (room.data[i][j][1])
                {
                case TILE_TYPE_WALL_NORTH:
                    DrawTextureRec(worldSprites, (Rectangle){80, 40, 120, 80}, (Vector2){i * tileSize, j * tileSize}, (Color){255, 255, 255, 255});
                    break;
                case TILE_TYPE_WALL_EAST:
                    DrawTextureRec(worldSprites, (Rectangle){40, 40, 80, 80}, (Vector2){i * tileSize, j * tileSize}, (Color){255, 255, 255, 255});
                    break;
                case TILE_TYPE_WALL_SOUTH:
                    DrawTextureRec(worldSprites, (Rectangle){40, 0, 80, 40}, (Vector2){i * tileSize, j * tileSize}, (Color){255, 255, 255, 255});
                    break;
                case TILE_TYPE_WALL_WEST:
                    DrawTextureRec(worldSprites, (Rectangle){80, 0, 120, 40}, (Vector2){i * tileSize, j * tileSize}, (Color){255, 255, 255, 255});
                    break;
                case TILE_TYPE_CORNER_NORTH_EAST:
                    DrawTextureRec(worldSprites, (Rectangle){120, 40, 160, 80}, (Vector2){i * tileSize, j * tileSize}, (Color){255, 255, 255, 255});
                    break;
                case TILE_TYPE_CORNER_NORTH_WEST:
                    DrawTextureRec(worldSprites, (Rectangle){160, 40, 200, 80}, (Vector2){i * tileSize, j * tileSize}, (Color){255, 255, 255, 255});
                    break;
                case TILE_TYPE_CORNER_SOUTH_EAST:
                    DrawTextureRec(worldSprites, (Rectangle){120, 0, 160, 40}, (Vector2){i * tileSize, j * tileSize}, (Color){255, 255, 255, 255});
                    break;
                case TILE_TYPE_CORNER_SOUTH_WEST:
                    DrawTextureRec(worldSprites, (Rectangle){160, 0, 200, 40}, (Vector2){i * tileSize, j * tileSize}, (Color){255, 255, 255, 255});
                }
            }
        }

        DrawCircle(playerPosition.x + 1, playerPosition.y + 1, playerRadius, (Color){0, 0, 0, 255});
        DrawCircle(playerPosition.x, playerPosition.y, playerRadius, (Color){100, 255, 100, 255});
        DrawCircle(playerPosition.x - 12, playerPosition.y - 1, 2, (Color){0, 0, 0, 255});
        DrawCircle(playerPosition.x + 12, playerPosition.y - 1, 2, (Color){0, 0, 0, 255});
        DrawRectangle(playerPosition.x - 8, playerPosition.y + 2, 16, 2, (Color){0, 0, 0, 255});

        for (int i = 0; i < 32; i++)
        {
            if (enemies[i].health > 0)
            {
                DrawEnemy(enemies[i]);
            }
        }

        for (int i = 0; i < maxSpellEntities; i++)
        {
            if (spellEntities[i].lifetime > 0)
            {
                switch (spellEntities[i].name)
                {
                case manaSpark:
                    DrawSpellManaSpark(spellEntities[i].position, spellEntities[i].aim);
                    break;

                case block:
                    DrawSpellBlock(spellEntities[i].position);
                    break;

                case fireBall:
                    DrawSpellFireBall(spellEntities[i].position, spellEntities[i].aim);
                    break;

                case moonBeam:
                    DrawSpellMoonBeam(spellEntities[i].position, spellEntities[i].lifetime);
                    break;

                case chromaticOrb:
                    DrawSpellChromaticOrb(spellEntities[i].position, spellEntities[i].aim, spellEntities[i].lifetime);
                    break;

                default:
                    break;
                }
            }
        }

        if (redRect.x != 0 || redRect.y != 0)
        {
            DrawRectangleLines(redRect.x * tileSize, redRect.y * tileSize, tileSize, tileSize, (Color){255, 0, 0, 255});
            DrawRectangle(redRect.x * tileSize, redRect.y * tileSize + 15, tileSize, 10, redRectX ? GREEN : (Color){255, 0, 0, 255});
            DrawRectangle(redRect.x * tileSize + 15, redRect.y * tileSize, 10, tileSize, redRectY ? GREEN : (Color){255, 0, 0, 255});
        }
        
        DrawLine(playerPosition.x, playerPosition.y, playerPosition.x + spellAim.x * tileSize, playerPosition.y + spellAim.y * tileSize, (Color){255, 0, 255, 255});
        DrawLine(playerPosition.x, playerPosition.y, playerPosition.x + playerAim.x * tileSize, playerPosition.y + playerAim.y * tileSize, (Color){255, 255, 255, 255});

        DrawMagicCircle(playerPosition, magicCircle, ringCount, &angle);

        DrawText(TextFormat("%s", GetGamepadName(1)), 10, 10, 20, (Color){255, 255, 255, 255});
        DrawText(TextFormat("%f", GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X)), 10, 40, 20, (Color){255, 255, 255, 255});

        EndDrawing();
    }

    CloseWindow();

    return 0;
}