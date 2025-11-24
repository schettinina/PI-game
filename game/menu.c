#include "raylib.h"
#include "fase1.h" 
#include "fase1.c"


typedef enum GameScreen {TITULO, JOGO, FASE2, CREDITOS} GameScreen;

int main(void)
{
    //Determinando as dimensões do jogo
    const int screenWidth = 800;
    const int screenHeight = 450;

    //Inicializando a janela
    InitWindow(screenWidth, screenHeight, "Jogo - Batalha contra o Boss");
    //Inicializando dispositivo de música
    InitAudioDevice();
    
    //Carregando imagens para cada tela
    Texture2D start = LoadTexture("images/start.jpg");
    Texture2D cred = LoadTexture("images/cred.jpg");
    Texture2D jogo = LoadTexture("images/gameplay.png"); // Fundo da fase
    
    //Carregando as músicas
    Music luta = LoadMusicStream("music/luta.mp3");
    luta.looping = true;

    SetExitKey(KEY_NULL);

    //Definindo a tela de TITULO como principal
    GameScreen telaAtual = TITULO;

    //Determinando o FPS
    SetTargetFPS(60);

    //Loop principal
    while (!WindowShouldClose())
    {
        UpdateMusicStream(luta);
        // --- LOGICA GERAL (ESC) ---
        if (IsKeyPressed(KEY_ESCAPE))
        {
            // Se estiver no Título, fecha o jogo
            if (telaAtual == TITULO)
            {
                break;
            }
            else
            {
                // Qualquer outra tela volta para o Título
                telaAtual = TITULO;
            }
        }

        // --- SISTEMA DE TROCA DE JANELAS E LÓGICA ---
        
        // Lógica do TÍTULO
        if (telaAtual == TITULO)
        {
            if (IsKeyPressed(KEY_ENTER)) 
            {
                InitFase1();// <--- Zera a vida e posição antes de começar
                PlayMusicStream(luta);
                telaAtual = JOGO;
            }
            if (IsKeyPressed(KEY_C)) telaAtual = CREDITOS;
        }
        
        // Lógica da FASE 1 (JOGO)
        else if (telaAtual == JOGO)
        {
            // Roda a lógica da batalha e pega o resultado
            int resultado = UpdateFase1(); 

            if (resultado == 1) 
            {
                // VITORIA: Vai para a tela de parabéns
                telaAtual = FASE2; 
            }
            else if (resultado == 2) 
            {
                // DERROTA: Reinicia a batalha imediatamente
                InitFase1(); 
            }
        }

        // Lógica da TELA DE VITÓRIA (FASE 2)
        else if (telaAtual == FASE2)
        {
            if (IsKeyPressed(KEY_ENTER)) 
            {
                telaAtual = TITULO; // Volta pro menu 
            }
        }
        
        //Tela
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

                } break;

                case JOGO:
                {
                    //Desenha os personagens
                    DrawFase1();

                } break;

                case FASE2: //Tela de Vitória
                {
                    
                    ClearBackground(RAYWHITE);
                    DrawText("PARABENS!", 280, 150, 40, GOLD);
                    DrawText("VOCE DERROTOU O BOSS", 240, 200, 20, DARKGRAY);
                    DrawText("Pressione ENTER para voltar", 250, 300, 20, LIGHTGRAY);

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

    //Limpeza de memória
    UnloadTexture(start);
    UnloadTexture(cred);
    UnloadTexture(jogo);
    UnloadMusicStream(luta);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
