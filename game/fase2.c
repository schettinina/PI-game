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
static Vector2 ultimo_movimento = {1.0f, 0.0f}; 
static Projetil projeteis[MAX_TIROS];
static Inimigo inimigos[MAX_INIMIGOS_CAP]; 

static int resultadoFase = 0; 

// Controle de Hordas
static int hordaAtual = 1;
static int inimigosPorHorda = 5;      // Começa com 5 inimigos
static int inimigosMortosHorda = 0;   // Contador para passar de fase
static int timerSpawn = 0;            // Controla a frequência que eles nascem


void SpawnInimigo()
{
    
    
    for (int i = 0; i < inimigosPorHorda; i++) 
    {
        if (!inimigos[i].ativo)
        {
            inimigos[i].ativo = true;
            inimigos[i].vida = 30; // Morre com 3 tiros
            inimigos[i].velocidade = (float)GetRandomValue(20, 40) / 10.0f; 
            inimigos[i].cor = RED;

            // Nasce em uma borda aleatória (Cima, Baixo, Esq, Dir)
            int lado = GetRandomValue(0, 3);
            if (lado == 0) inimigos[i].posicao = (Vector2){ GetRandomValue(0, 800), -50 }; // Cima
            if (lado == 1) inimigos[i].posicao = (Vector2){ GetRandomValue(0, 800), 500 }; // Baixo
            if (lado == 2) inimigos[i].posicao = (Vector2){ -50, GetRandomValue(0, 450) }; // Esq
            if (lado == 3) inimigos[i].posicao = (Vector2){ 850, GetRandomValue(0, 450) }; // Dir
            
            inimigos[i].retangulo = (Rectangle){ inimigos[i].posicao.x, inimigos[i].posicao.y, 40, 40 };
            break; // Só spawna um por vez
        }
    }
}


void InitFase2(void)
{
    
    jogador.posicao = (Vector2){ 400, 225 }; // Centro da tela
    jogador.retangulo = (Rectangle){ 400, 225, 40, 40 };
    jogador.vidaMaxima = 100; 
    jogador.vida = 100;
    jogador.corPersonagem = BLUE;
    jogador.tempoRecargaTiro = 0;
    jogador.tempoRecargaExplosao = 0;
    jogador.usandoExplosao = false;

    
    for (int i = 0; i < MAX_TIROS; i++) projeteis[i].ativo = false;

   
    for (int i = 0; i < MAX_INIMIGOS_CAP; i++) inimigos[i].ativo = false;

    
    hordaAtual = 1;
    inimigosPorHorda = 5;
    inimigosMortosHorda = 0;
    timerSpawn = 0;
    resultadoFase = 0;
}


int UpdateFase2(void)
{
    if (resultadoFase != 0) return resultadoFase;

    
    Vector2 inputMovimento = {0.0f, 0.0f};
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) inputMovimento.x += 1.0f;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  inputMovimento.x -= 1.0f;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))    inputMovimento.y -= 1.0f;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))  inputMovimento.y += 1.0f;

    if (inputMovimento.x != 0 || inputMovimento.y != 0) {
        inputMovimento = Vector2Normalize(inputMovimento);
        ultimo_movimento = inputMovimento; 
        jogador.posicao.x += inputMovimento.x * 5.0f;
        jogador.posicao.y += inputMovimento.y * 5.0f;
    }

   
    if (jogador.posicao.x > 800) jogador.posicao.x = 0;
    else if (jogador.posicao.x < 0) jogador.posicao.x = 800;
    if (jogador.posicao.y > 450) jogador.posicao.y = 0;
    else if (jogador.posicao.y < 0) jogador.posicao.y = 450;
    
    jogador.retangulo.x = jogador.posicao.x;
    jogador.retangulo.y = jogador.posicao.y;

    
    if (jogador.tempoRecargaTiro > 0) jogador.tempoRecargaTiro--;
    if (jogador.tempoRecargaExplosao > 0) jogador.tempoRecargaExplosao--;
    jogador.usandoExplosao = false;

    
    timerSpawn++;
    // A cada 60 frames (1 seg) tenta spawnar um inimigo se houver vaga na horda
    if (timerSpawn > 60) {
        SpawnInimigo();
        timerSpawn = 0;
    }

   
    

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
                    jogador.tempoRecargaTiro = 15; // Tiro rápido
                    break;
                }
            }
        }
    }
    if (IsKeyPressed(KEY_X) && jogador.tempoRecargaExplosao == 0)
    {
        jogador.usandoExplosao = true;
        jogador.tempoRecargaExplosao = 180; 
        Vector2 centro = { jogador.posicao.x + 20, jogador.posicao.y + 20 };

        // Verifica colisão da explosão com TODOS os inimigos
        for (int i = 0; i < MAX_INIMIGOS_CAP; i++) {
            if (inimigos[i].ativo) {
                if (CheckCollisionCircleRec(centro, 150, inimigos[i].retangulo)) {
                    inimigos[i].vida -= DANO_EXPLOSAO;
                }
            }
        }
    }

    for (int i = 0; i < MAX_TIROS; i++) {
        if (projeteis[i].ativo) {
            projeteis[i].posicao = Vector2Add(projeteis[i].posicao, projeteis[i].velocidade);

            // Colisão Tiro x Inimigos
            for (int j = 0; j < MAX_INIMIGOS_CAP; j++) {
                if (inimigos[j].ativo) {
                    if (CheckCollisionCircleRec(projeteis[i].posicao, 8, inimigos[j].retangulo)) {
                        inimigos[j].vida -= DANO_TIRO;
                        projeteis[i].ativo = false; // Destroi tiro
                        break; // Para loop de inimigos
                    }
                }
            }
            
            // Saiu da tela
            if (projeteis[i].posicao.x < 0 || projeteis[i].posicao.x > 800 ||
                projeteis[i].posicao.y < 0 || projeteis[i].posicao.y > 450) {
                projeteis[i].ativo = false;
            }
        }
    }


    int contagemInimigosVivos = 0;

    for (int i = 0; i < MAX_INIMIGOS_CAP; i++) 
    {
        if (inimigos[i].ativo) 
        {
            contagemInimigosVivos++;

            // Persegue o jogador
            Vector2 dir = Vector2Subtract(jogador.posicao, inimigos[i].posicao);
            dir = Vector2Normalize(dir);
            inimigos[i].posicao.x += dir.x * inimigos[i].velocidade;
            inimigos[i].posicao.y += dir.y * inimigos[i].velocidade;
            
            // Atualiza retangulo
            inimigos[i].retangulo.x = inimigos[i].posicao.x;
            inimigos[i].retangulo.y = inimigos[i].posicao.y;

            //  Causa Dano no Jogador
            if (CheckCollisionRecs(jogador.retangulo, inimigos[i].retangulo)) {
                jogador.vida -= 1; // Dano contínuo ao encostar
            }

            // Verifica Morte do Inimigo
            if (inimigos[i].vida <= 0) {
                inimigos[i].ativo = false;
                inimigosMortosHorda++;
            }
        }
    }

   
    
    if (inimigosMortosHorda >= inimigosPorHorda) 
    {
        hordaAtual++;            // Sobe nível da horda
        inimigosPorHorda += 5;   // Aumenta +5 inimigos
        inimigosMortosHorda = 0; // Reseta contador de mortes
        
        // Cura um pouco o jogador ao passar de horda
        jogador.vida += 20;
        if (jogador.vida > 100) jogador.vida = 100;

        // Se passar da Horda com 30 inimigos (Horda 6), GANHOU!
        if (inimigosPorHorda > 30) {
            resultadoFase = 1; 
        }
    }

    if (jogador.vida <= 0) resultadoFase = 2; 

    return 0;
}


void DrawFase2(void)
{
    // Fundo muda de cor conforme a horda fica mais difícil
    if (hordaAtual < 3) DrawRectangle(0,0,800,450, DARKGRAY);
    else if (hordaAtual < 5) DrawRectangle(0,0,800,450, MAROON);
    else DrawRectangle(0,0,800,450, BLACK); // Horda Final é preta

    // Desenha Jogador
    DrawRectangleRec(jogador.retangulo, jogador.corPersonagem);
    if (jogador.usandoExplosao) DrawCircleLines(jogador.posicao.x+20, jogador.posicao.y+20, 150, ORANGE);

    // Desenha Inimigos
    for (int i = 0; i < MAX_INIMIGOS_CAP; i++) {
        if (inimigos[i].ativo) {
            DrawRectangleRec(inimigos[i].retangulo, inimigos[i].cor);
            // Barra de vida pequena em cima do inimigo
            DrawRectangle(inimigos[i].posicao.x, inimigos[i].posicao.y - 5, inimigos[i].vida, 3, GREEN);
        }
    }

    // Desenha Projéteis
    for (int i = 0; i < MAX_TIROS; i++) {
        if (projeteis[i].ativo) DrawCircleV(projeteis[i].posicao, 5, YELLOW);
    }

    // HUD (Interface)
    DrawText(TextFormat("HORDA: %d", hordaAtual), 350, 10, 30, WHITE);
    DrawText(TextFormat("Inimigos: %d / %d", inimigosMortosHorda, inimigosPorHorda), 320, 45, 20, LIGHTGRAY);

    // Barra de Vida Jogador
    DrawText("VIDA", 10, 400, 20, WHITE);
    DrawRectangle(10, 420, jogador.vida * 2, 20, SKYBLUE);
    DrawRectangleLines(10, 420, 200, 20, BLACK);
    
    // Mira
    DrawCircleV(GetMousePosition(), 5, RED);
}