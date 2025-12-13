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
    
    Music luta = LoadMusicStream("music/luta.mp3");
    luta.looping = true;

    SetExitKey(KEY_NULL); 

    GameScreen telaAtual = TITULO;
    SetTargetFPS(60);

    // Hitboxes ajustadas
    Rectangle areaBotaoComecar = { (float)screenWidth/2 - 110, 270, 220, 50 }; 
    Rectangle areaBotaoCreditos = { (float)screenWidth/2 - 110, 340, 220, 50 };

    while (!WindowShouldClose())
    {
        UpdateMusicStream(luta);

        if (IsKeyPressed(KEY_ESCAPE))
        {
            if (telaAtual == TITULO) break;
            else 
            {
                telaAtual = TITULO; 
                StopMusicStream(luta);
            }
        }

        switch(telaAtual)
        {
            case TITULO:
            {
                Vector2 mousePoint = GetMousePosition();
                
                bool mouseEmCimaComecar = CheckCollisionPointRec(mousePoint, areaBotaoComecar);
                bool mouseEmCimaCreditos = CheckCollisionPointRec(mousePoint, areaBotaoCreditos);

                if (mouseEmCimaComecar || mouseEmCimaCreditos) SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    if (mouseEmCimaComecar)
                    {
                        InitFase1();
                        PlayMusicStream(luta);
                        telaAtual = JOGO;
                    }
                    else if (mouseEmCimaCreditos)
                    {
                        telaAtual = CREDITOS;
                    }
                }
            } break;

            case JOGO:
            {
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                int resultado = UpdateFase1(); 
                if (resultado == 1) { telaAtual = FASE2; InitFase2(); }
                else if (resultado == 2) InitFase1(); 
            } break;

            case FASE2:
            {
                int resultado2 = UpdateFase2();
                if (resultado2 == 1) telaAtual = CREDITOS; 
                else if (resultado2 == 2) InitFase2(); 
            } break;
            
            case CREDITOS:
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_ENTER)) telaAtual = TITULO;
            } break;
        }
        
        BeginDrawing();
            ClearBackground(RAYWHITE);

            switch(telaAtual)
            {
                case TITULO:
                {
                    DrawTexturePro(start, (Rectangle){0, 0, (float)start.width, (float)start.height},
                        (Rectangle){0, 0, (float)screenWidth, (float)screenHeight}, (Vector2){0, 0}, 0.0f, WHITE);
                } break;

                case JOGO: DrawFase1(); break;
                case FASE2: DrawFase2(); break;

                case CREDITOS:
                {
                    DrawTexturePro(cred, (Rectangle){0, 0, (float)cred.width, (float)cred.height},
                        (Rectangle){0, 0, (float)screenWidth, (float)screenHeight}, (Vector2){0, 0}, 0.0f, WHITE);
                } break;
            }
        EndDrawing();
    } 

    
    UnloadTexture(start);
    UnloadTexture(cred);
    UnloadMusicStream(luta);
    CloseAudioDevice();
    CloseWindow();

    return 0;
} 