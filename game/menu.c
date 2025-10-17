#include "raylib.h"

typedef enum GameScreen {TITULO, JOGO, CREDITOS} GameScreen;

int main(void)
{
    //Determinando as dimensões do jogo
    const int screenWidth = 800;
    const int screenHeight = 450;

    //Inicializando a janela
    InitWindow(screenWidth, screenHeight, "Jogo");
    
    //Carregando imagens para cada tela
    Texture2D start = LoadTexture("images/start.jpg");
    Texture2D cred = LoadTexture("images/cred.jpg");
    Texture2D jogo = LoadTexture("images/gameplay.png");

    SetExitKey(KEY_NULL);

    //Definindo a tela de TITULO como principal
    GameScreen telaAtual = TITULO;

    //Determinando o FPS
    SetTargetFPS(60);

    //Loop principal
    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_ESCAPE))
        {
            //Fechar o jogo somente na tela inicial (ao clicar ESCAPE)
            if (telaAtual == TITULO)
            {
                break;
            }
            else
            {
                //Volta à tela inicial
                telaAtual = TITULO;
            }
        }

        //Sistema de troca de janelas
        if (telaAtual == TITULO)
        {
            if (IsKeyPressed(KEY_ENTER)) telaAtual = JOGO;
            if (IsKeyPressed(KEY_C)) telaAtual = CREDITOS;
        }
        
        //Desenhando na tela
        BeginDrawing();

            ClearBackground(RAYWHITE);

            switch(telaAtual)
            {
                case TITULO:
                {
                    //Ajustando imagem para as dimensões da janela
                    DrawTexturePro(
                        start,
                        (Rectangle){0, 0, (float)start.width, (float)start.height},
                        (Rectangle){0, 0, (float)screenWidth, (float)screenHeight},
                        (Vector2){0, 0},
                        0.0f,
                        WHITE);

                } break;
                case JOGO:
                {
                    //Ajustando imagem para as dimensões da janela
                    DrawTexturePro(
                        jogo,
                        (Rectangle){0, 0, (float)jogo.width, (float)jogo.height},
                        (Rectangle){0, 0, (float)screenWidth, (float)screenHeight},
                        (Vector2){0, 0},
                        0.0f,
                        WHITE);

                } break;
                case CREDITOS:
                {
                    //Ajustando imagem para as dimensões da janela
                    DrawTexturePro(
                        cred,
                        (Rectangle){0, 0, (float)cred.width, (float)cred.height},
                        (Rectangle){0, 0, (float)screenWidth, (float)screenHeight},
                        (Vector2){0, 0},
                        0.0f,
                        WHITE);

                } break;
                default: break;
            }

        EndDrawing();

    }
    UnloadTexture(start);
    UnloadTexture(cred);
    UnloadTexture(jogo);
    CloseWindow();


    return 0;
}
