#include "raylib.h"
#include "fase1.h" 
#include "fase2.h"

typedef enum GameScreen {TITULO, JOGO, FASE2, CREDITOS} GameScreen;

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Jogo - Batalha contra o Boss");
    InitAudioDevice();
    
    Texture2D start = LoadTexture("images/start.jpg");
    Texture2D cred = LoadTexture("images/cred.jpg");
    Texture2D jogo = LoadTexture("images/gameplay.png"); 
    
    Music luta = LoadMusicStream("music/luta.mp3");
    luta.looping = true;

    SetExitKey(KEY_NULL);

    GameScreen telaAtual = TITULO;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateMusicStream(luta);

        if (IsKeyPressed(KEY_ESCAPE))
        {
            if (telaAtual == TITULO)
            {
                break;
            }
            else
            {
                telaAtual = TITULO;
            }
        }

        if (telaAtual == TITULO)
        {
            if (IsKeyPressed(KEY_ENTER)) 
            {
                InitFase1();
                PlayMusicStream(luta);
                telaAtual = JOGO;
            }
            if (IsKeyPressed(KEY_C)) telaAtual = CREDITOS;
        }
        else if (telaAtual == JOGO)
        {
            int resultado = UpdateFase1(); 

            if (resultado == 1) 
            {
                telaAtual = FASE2; 
                InitFase2();
            }
            else if (resultado == 2) 
            {
                InitFase1(); 
            }
        }
        else if (telaAtual == FASE2)
        {
            int resultado2 = UpdateFase2();

            if (resultado2 == 1) 
            {
                telaAtual = CREDITOS; 
            }
            else if (resultado2 == 2) 
            {
                InitFase2(); 
            }
        }
        
        BeginDrawing();

            ClearBackground(RAYWHITE);

            switch(telaAtual)
            {
                case TITULO:
                {
                    DrawTexturePro(
                        start,
                        (Rectangle){0, 0, (float)start.width, (float)start.height},
                        (Rectangle){0, 0, (float)screenWidth, (float)screenHeight},
                        (Vector2){0, 0}, 0.0f, WHITE);

                    // Botões interativos: Começar e Créditos
                    Rectangle startBtn = {(float)screenWidth/2 - 100.0f, 300.0f, 200.0f, 60.0f};
                    Rectangle creditsBtn = {(float)screenWidth/2 - 100.0f, 380.0f, 200.0f, 60.0f};
                    Vector2 mousePoint = GetMousePosition();

                    bool hoverStart = CheckCollisionPointRec(mousePoint, startBtn);
                    bool hoverCredits = CheckCollisionPointRec(mousePoint, creditsBtn);

                    Color startColor = hoverStart ? LIGHTGRAY : GRAY;
                    Color creditsColor = hoverCredits ? LIGHTGRAY : GRAY;

                    DrawRectangleRec(startBtn, startColor);
                    DrawRectangleRec(creditsBtn, creditsColor);

                    const char *txtStart = "COME\x87AR";
                    const char *txtCredits = "CR\xC9DITOS";
                    int fontSize = 20;
                    DrawText(txtStart, screenWidth/2 - MeasureText(txtStart, fontSize)/2, 315, fontSize, BLACK);
                    DrawText(txtCredits, screenWidth/2 - MeasureText(txtCredits, fontSize)/2, 395, fontSize, BLACK);

                    // Clique do mouse para ativar opções
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                    {
                        if (hoverStart)
                        {
                            InitFase1();
                            PlayMusicStream(luta);
                            telaAtual = JOGO;
                        }
                        else if (hoverCredits)
                        {
                            telaAtual = CREDITOS;
                        }
                    }

                } break;

                case JOGO:
                {
                    DrawFase1();

                } break;

                case FASE2:
                {
                    DrawFase2();

                } break;

                case CREDITOS:
                {
                    DrawTexturePro(
                        cred,
                        (Rectangle){0, 0, (float)cred.width, (float)cred.height},
                        (Rectangle){0, 0, (float)screenWidth, (float)screenHeight},
                        (Vector2){0, 0}, 0.0f, WHITE);

                } break;
                
                default: break;
            }

        EndDrawing();
    }

    UnloadTexture(start);
    UnloadTexture(cred);
    UnloadTexture(jogo);
    UnloadMusicStream(luta);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}