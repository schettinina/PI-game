#include "raylib.h"   
#include "raymath.h"  
#include "fase2.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_TIROS 20            
#define MAX_INIMIGOS_CAP 30     
#define VEL_TIRO 9.0f
#define DANO_TIRO 10
#define DANO_EXPLOSAO 100       

typedef struct {
    Vector2 posicao;
    Vector2 velocidade; 
    bool ativo;
} Projetil;

typedef struct {
    Vector2 posicao;
    Rectangle retangulo;
    int vida;
    int vidaMaxima;
    Color corPersonagem;
    int tempoRecargaTiro;
    int tempoRecargaExplosao;
    bool usandoExplosao;
} Jogador;

typedef struct {
    bool ativo;             
    Vector2 posicao;
    Rectangle retangulo;
    int vida;
    float velocidade;
    Color cor;
} Inimigo;

// Globais
static Jogador jogador;
static Projetil projeteis[MAX_TIROS];
static Inimigo inimigos[MAX_INIMIGOS_CAP]; 

// --- VARIÁVEIS DE IMAGEM ---
static Texture2D fundoFase2; 
static Texture2D imgGameOverFase2; 
static Texture2D imgVictory; // <--- IMAGEM DE VITÓRIA (wintela.png)
static bool texturaCarregada = false; 

// Estado do Jogo
static int resultadoFase = 0; 
static bool gameOverAtivo = false; 
static bool vitoriaAtiva = false; // <--- Flag de Vitória

// Controle de Hordas
static int hordaAtual = 1;
static int inimigosPorHorda = 5;      
static int inimigosMortosHorda = 0;   
static int timerSpawn = 0;            

// --- Função para Reiniciar Variáveis ---
void ResetarVariaveisFase2(void) 
{
    jogador.posicao = (Vector2){ 400, 225 }; 
    jogador.retangulo = (Rectangle){ 400, 225, 40, 40 };
    jogador.vidaMaxima = 100; 
    jogador.vida = 100;
    jogador.corPersonagem = BLUE;
    jogador.tempoRecargaTiro = 0;
    jogador.tempoRecargaExplosao = 0;
    jogador.usandoExplosao = false;

    // Reseta arrays
    for (int i = 0; i < MAX_TIROS; i++) projeteis[i].ativo = false;
    for (int i = 0; i < MAX_INIMIGOS_CAP; i++) inimigos[i].ativo = false;

    // Reseta Hordas
    hordaAtual = 1;
    inimigosPorHorda = 5;
    inimigosMortosHorda = 0;
    timerSpawn = 0;
    
    // Reseta Estados
    resultadoFase = 0;
    gameOverAtivo = false;
    vitoriaAtiva = false; // Reseta vitória também
}

void SpawnInimigo()
{
    for (int i = 0; i < inimigosPorHorda; i++) 
    {
        if (!inimigos[i].ativo)
        {
            inimigos[i].ativo = true;
            inimigos[i].vida = 30; 
            inimigos[i].velocidade = (float)GetRandomValue(20, 40) / 10.0f; 
            inimigos[i].cor = RED;
            int lado = GetRandomValue(0, 3);
            
            // Spawn nas bordas
            if (lado == 0) inimigos[i].posicao = (Vector2){ (float)GetRandomValue(0, 800), -50 }; 
            if (lado == 1) inimigos[i].posicao = (Vector2){ (float)GetRandomValue(0, 800), 500 }; 
            if (lado == 2) inimigos[i].posicao = (Vector2){ -50, (float)GetRandomValue(0, 450) }; 
            if (lado == 3) inimigos[i].posicao = (Vector2){ 850, (float)GetRandomValue(0, 450) }; 
            
            inimigos[i].retangulo = (Rectangle){ inimigos[i].posicao.x, inimigos[i].posicao.y, 40, 40 };
            break; 
        }
    }
}

void InitFase2(void)
{
    // Carregamento de Texturas
    if (!texturaCarregada) {
        fundoFase2 = LoadTexture("images/fase2.png");
        imgGameOverFase2 = LoadTexture("images/gameover2.png"); 
        imgVictory = LoadTexture("images/wintela.png"); 
        texturaCarregada = true;
    }

    ResetarVariaveisFase2();
}

void UnloadFase2(void) {
    if (texturaCarregada) {
        UnloadTexture(fundoFase2);
        UnloadTexture(imgGameOverFase2);
        UnloadTexture(imgVictory);
        texturaCarregada = false;
    }
}

int UpdateFase2(void)
{
    if (resultadoFase != 0) return resultadoFase;

    // --- LÓGICA DE GAME OVER (Derrota) ---
    if (gameOverAtivo) 
    {
        if (IsKeyPressed(KEY_R)) {
            ResetarVariaveisFase2();
        }
        return 0; 
    }

    // --- LÓGICA DE VITÓRIA (Vitória) ---
    if (vitoriaAtiva)
    {
        // Se ganhar, pode apertar ENTER para sair ou R para reiniciar
        if (IsKeyPressed(KEY_ENTER)) {
            resultadoFase = 1; // Retorna 1 (Sucesso/Próxima tela - Créditos)
        }
        if (IsKeyPressed(KEY_R)) {
            ResetarVariaveisFase2();
        }
        return 0; // Para o jogo enquanto mostra a tela
    }

    // Movimentação Jogador
    Vector2 inputMovimento = {0.0f, 0.0f};
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) inputMovimento.x += 1.0f;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  inputMovimento.x -= 1.0f;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))    inputMovimento.y -= 1.0f;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))  inputMovimento.y += 1.0f;

    if (inputMovimento.x != 0 || inputMovimento.y != 0) {
        inputMovimento = Vector2Normalize(inputMovimento);
        jogador.posicao.x += inputMovimento.x * 5.0f;
        jogador.posicao.y += inputMovimento.y * 5.0f;
    }

    // Wrap-around
    if (jogador.posicao.x > 800) jogador.posicao.x = 0;
    else if (jogador.posicao.x < 0) jogador.posicao.x = 800;
    if (jogador.posicao.y > 450) jogador.posicao.y = 0;
    else if (jogador.posicao.y < 0) jogador.posicao.y = 450;
    
    jogador.retangulo.x = jogador.posicao.x;
    jogador.retangulo.y = jogador.posicao.y;

    // Cooldowns
    if (jogador.tempoRecargaTiro > 0) jogador.tempoRecargaTiro--;
    if (jogador.tempoRecargaExplosao > 0) jogador.tempoRecargaExplosao--;
    jogador.usandoExplosao = false;

    // Spawner
    timerSpawn++;
    if (timerSpawn > 60) {
        SpawnInimigo();
        timerSpawn = 0;
    }

    // Tiro
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_Z)) 
    {
        if (jogador.tempoRecargaTiro == 0) {
            for (int i = 0; i < MAX_TIROS; i++) {
                if (!projeteis[i].ativo) {
                    projeteis[i].ativo = true;
                    Vector2 centro = { jogador.posicao.x + 20, jogador.posicao.y + 20 };
                    projeteis[i].posicao = centro;
                    Vector2 mousePos = GetMousePosition();
                    Vector2 dir = Vector2Normalize(Vector2Subtract(mousePos, centro));
                    projeteis[i].velocidade = Vector2Scale(dir, VEL_TIRO);
                    jogador.tempoRecargaTiro = 15; 
                    break;
                }
            }
        }
    }

    // Habilidade Explosão
    if (IsKeyPressed(KEY_X) && jogador.tempoRecargaExplosao == 0)
    {
        jogador.usandoExplosao = true;
        jogador.tempoRecargaExplosao = 180; 
        Vector2 centro = { jogador.posicao.x + 20, jogador.posicao.y + 20 };
        for (int i = 0; i < MAX_INIMIGOS_CAP; i++) {
            if (inimigos[i].ativo) {
                if (CheckCollisionCircleRec(centro, 150, inimigos[i].retangulo)) {
                    inimigos[i].vida -= DANO_EXPLOSAO;
                }
            }
        }
    }

    // Atualiza Projéteis
    for (int i = 0; i < MAX_TIROS; i++) {
        if (projeteis[i].ativo) {
            projeteis[i].posicao = Vector2Add(projeteis[i].posicao, projeteis[i].velocidade);
            for (int j = 0; j < MAX_INIMIGOS_CAP; j++) {
                if (inimigos[j].ativo) {
                    if (CheckCollisionCircleRec(projeteis[i].posicao, 8, inimigos[j].retangulo)) {
                        inimigos[j].vida -= DANO_TIRO;
                        projeteis[i].ativo = false; 
                        break; 
                    }
                }
            }
            if (projeteis[i].posicao.x < 0 || projeteis[i].posicao.x > 800 ||
                projeteis[i].posicao.y < 0 || projeteis[i].posicao.y > 450) {
                projeteis[i].ativo = false;
            }
        }
    }

    // Atualiza Inimigos
    int contagemInimigosVivos = 0;
    for (int i = 0; i < MAX_INIMIGOS_CAP; i++) {
        if (inimigos[i].ativo) {
            contagemInimigosVivos++;
            Vector2 dir = Vector2Subtract(jogador.posicao, inimigos[i].posicao);
            dir = Vector2Normalize(dir);
            inimigos[i].posicao.x += dir.x * inimigos[i].velocidade;
            inimigos[i].posicao.y += dir.y * inimigos[i].velocidade;
            inimigos[i].retangulo.x = inimigos[i].posicao.x;
            inimigos[i].retangulo.y = inimigos[i].posicao.y;

            if (CheckCollisionRecs(jogador.retangulo, inimigos[i].retangulo)) {
                jogador.vida -= 1; 
            }
            if (inimigos[i].vida <= 0) {
                inimigos[i].ativo = false;
                inimigosMortosHorda++;
            }
        }
    }

    // Checa progresso da Horda
    if (inimigosMortosHorda >= inimigosPorHorda) {
        hordaAtual++;            
        inimigosPorHorda += 5;   
        inimigosMortosHorda = 0; 
        jogador.vida += 20;
        if (jogador.vida > 100) jogador.vida = 100;
        
        // --- CONDIÇÃO DE VITÓRIA ---
        // Se a horda ficar muito grande (ex: > 30), vence o jogo
        if (inimigosPorHorda > 30) {
            vitoriaAtiva = true; // Ativa a tela de vitória
        }
    }

    // Checagem de Morte
    if (jogador.vida <= 0) {
        jogador.vida = 0;
        gameOverAtivo = true; 
    }
    
    return 0;
}


void DrawFase2(void)
{
    // --- DESENHO DO FUNDO AJUSTADO ---
    if (fundoFase2.id <= 0) {
        DrawRectangle(0, 0, 800, 450, MAGENTA);
        DrawText("ERRO: IMAGEM FASE2 NAO ENCONTRADA", 200, 200, 20, WHITE);
    } else {
        Rectangle source = { 0.0f, 0.0f, (float)fundoFase2.width, (float)fundoFase2.height };
        Rectangle dest = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
        Vector2 origin = { 0.0f, 0.0f };
        DrawTexturePro(fundoFase2, source, dest, origin, 0.0f, WHITE);
    }

    // Desenho Jogo Normal
    DrawRectangleRec(jogador.retangulo, jogador.corPersonagem);
    if (jogador.usandoExplosao) DrawCircleLines(jogador.posicao.x+20, jogador.posicao.y+20, 150, ORANGE);

    for (int i = 0; i < MAX_INIMIGOS_CAP; i++) {
        if (inimigos[i].ativo) {
            DrawRectangleRec(inimigos[i].retangulo, inimigos[i].cor);
            DrawRectangle(inimigos[i].posicao.x, inimigos[i].posicao.y - 5, inimigos[i].vida, 3, GREEN);
        }
    }

    for (int i = 0; i < MAX_TIROS; i++) {
        if (projeteis[i].ativo) DrawCircleV(projeteis[i].posicao, 5, YELLOW);
    }

    DrawText(TextFormat("HORDA: %d", hordaAtual), 350, 10, 30, WHITE);
    DrawText(TextFormat("Inimigos: %d / %d", inimigosMortosHorda, inimigosPorHorda), 320, 45, 20, LIGHTGRAY);
    
    DrawText("VIDA", 10, 400, 20, WHITE);
    DrawRectangle(10, 420, jogador.vida * 2, 20, SKYBLUE);
    DrawRectangleLines(10, 420, 200, 20, BLACK);
    
    DrawCircleV(GetMousePosition(), 5, RED);

    // --- DESENHO DO GAME OVER (DERROTA) ---
    if (gameOverAtivo)
    {
        if (imgGameOverFase2.id > 0) {
            Rectangle sourceGO = { 0.0f, 0.0f, (float)imgGameOverFase2.width, (float)imgGameOverFase2.height };
            Rectangle destGO = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
            Vector2 originGO = { 0.0f, 0.0f };
            DrawTexturePro(imgGameOverFase2, sourceGO, destGO, originGO, 0.0f, WHITE);
        } else {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
        }

        // Texto piscando "TECLE R" na derrota
        if ((int)(GetTime() * 2) % 2 == 0) {
            const char* texto = "TECLE [R] PARA REINICIAR";
            int fontSize = 30;
            int larguraTexto = MeasureText(texto, fontSize);
            DrawText(texto, (GetScreenWidth() - larguraTexto) / 2, GetScreenHeight() - 80, fontSize, WHITE);
        }
    }

    // --- DESENHO DA VITÓRIA (WINTELA) ---
    if (vitoriaAtiva)
    {
        // Apenas desenha a imagem, pois o texto já está nela
        if (imgVictory.id > 0) {
            Rectangle sourceWin = { 0.0f, 0.0f, (float)imgVictory.width, (float)imgVictory.height };
            Rectangle destWin = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
            Vector2 originWin = { 0.0f, 0.0f };
            DrawTexturePro(imgVictory, sourceWin, destWin, originWin, 0.0f, WHITE);
        } else {
            // Fallback caso a imagem não carregue
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), GOLD);
            DrawText("VOCE VENCEU!", 250, 200, 40, BLACK);
        }
    }
}