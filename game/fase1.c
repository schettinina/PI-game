#include "fase1.h"
#include "raymath.h"
#include <stdio.h>
#include "raylib.h"

#define MAX_TIROS 5
#define DANO_ESPADADA 15
#define DANO_EXPLOSAO 60

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

    int tempoRecargaEspada;
    int tempoRecargaTiro;
    int tempoRecargaExplosao;

    bool usandoEspada;
    bool usandoExplosao;
} Personagem;

static Personagem jogador;
static Personagem chefe;
static Projetil projeteis[MAX_TIROS];

static int tempoDanoChefe = 0;
static int resultadoFase = 0;

static float velocidadeMovimento = 4.0f;
static float velocidadeTiro = 7.0f;
static int danoTiroAtual = 10;
static int moedas = 0;
static bool lojaAberta = false;
static bool jogoPausado = false;

void InitFase1(void)
{
    jogador.posicao = (Vector2){ 100, 200 };
    jogador.retangulo = (Rectangle){ 100, 200, 40, 40 };
    jogador.vidaMaxima = 80;
    jogador.vida = 80;
    jogador.corPersonagem = BLUE;

    jogador.tempoRecargaEspada = 0;
    jogador.tempoRecargaTiro = 0;
    jogador.tempoRecargaExplosao = 0;
    jogador.usandoEspada = false;
    jogador.usandoExplosao = false;

    chefe.posicao = (Vector2){ 600, 200 };
    chefe.retangulo = (Rectangle){ 600, 200, 80, 80 };
    chefe.vidaMaxima = 1500;
    chefe.vida = 1500;
    chefe.corPersonagem = RED;

    for (int i = 0; i < MAX_TIROS; i++) projeteis[i].ativo = false;

    velocidadeMovimento = 4.0f;
    velocidadeTiro = 7.0f;
    danoTiroAtual = 10;
    moedas = 0;
    lojaAberta = false;
    resultadoFase = 0;
    jogoPausado = false;
}

int UpdateFase1(void)
{
    if (resultadoFase != 0) return resultadoFase;

    if (IsKeyPressed(KEY_P)) 
    {
        jogoPausado = !jogoPausado;
    }

    if (jogoPausado) return 0;

    if (lojaAberta)
    {
        Vector2 mouse = GetMousePosition();

        Rectangle btnTiro = { 180, 150, 440, 40 };
        if (CheckCollisionPointRec(mouse, btnTiro) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (moedas >= 30) {
                moedas -= 30;
                velocidadeTiro *= 1.25f;
            }
        }

        Rectangle btnSpeed = { 180, 200, 440, 40 };
        if (CheckCollisionPointRec(mouse, btnSpeed) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (moedas >= 25) {
                moedas -= 25;
                velocidadeMovimento *= 1.25f;
            }
        }

        Rectangle btnDano = { 180, 250, 440, 40 };
        if (CheckCollisionPointRec(mouse, btnDano) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (moedas >= 40) {
                moedas -= 40;
                danoTiroAtual += 10;
                jogador.corPersonagem = ORANGE;
            }
        }

        Rectangle btnSair = { 300, 350, 200, 50 };
        if (CheckCollisionPointRec(mouse, btnSair) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            resultadoFase = 1;
        }

        return 0;
    }

    Vector2 inputMovimento = {0.0f, 0.0f};

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) inputMovimento.x += 1.0f;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  inputMovimento.x -= 1.0f;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))    inputMovimento.y -= 1.0f;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))  inputMovimento.y += 1.0f;

    if (inputMovimento.x != 0 || inputMovimento.y != 0)
    {
        inputMovimento = Vector2Normalize(inputMovimento);
        jogador.posicao.x += inputMovimento.x * velocidadeMovimento;
        jogador.posicao.y += inputMovimento.y * velocidadeMovimento;
    }

    int largura = GetScreenWidth();
    int altura  = GetScreenHeight();

    if (jogador.posicao.x > largura) jogador.posicao.x = -jogador.retangulo.width;
    else if (jogador.posicao.x < -jogador.retangulo.width) jogador.posicao.x = largura;

    if (jogador.posicao.y > altura) jogador.posicao.y = -jogador.retangulo.height;
    else if (jogador.posicao.y < -jogador.retangulo.height) jogador.posicao.y = altura;

    jogador.retangulo.x = jogador.posicao.x;
    jogador.retangulo.y = jogador.posicao.y;

    if (jogador.tempoRecargaEspada > 0) jogador.tempoRecargaEspada--;
    if (jogador.tempoRecargaTiro > 0) jogador.tempoRecargaTiro--;
    if (jogador.tempoRecargaExplosao > 0) jogador.tempoRecargaExplosao--;

    jogador.usandoEspada = false;
    jogador.usandoExplosao = false;

    if (IsKeyPressed(KEY_SPACE) && jogador.tempoRecargaEspada == 0)
    {
        jogador.usandoEspada = true;
        jogador.tempoRecargaEspada = 20;
        Rectangle areaAtaque = { jogador.posicao.x + 40, jogador.posicao.y, 60, 40 };

        if (CheckCollisionRecs(areaAtaque, chefe.retangulo)) {
            chefe.vida -= DANO_ESPADADA;
            tempoDanoChefe = 10;
        }
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_Z)) 
    {
        if (jogador.tempoRecargaTiro == 0)
        {
            for (int i = 0; i < MAX_TIROS; i++)
            {
                if (!projeteis[i].ativo)
                {
                    projeteis[i].ativo = true;
                    Vector2 centroJogador = { jogador.posicao.x + 20, jogador.posicao.y + 20 };
                    projeteis[i].posicao = centroJogador;

                    Vector2 mousePos = GetMousePosition();
                    Vector2 direcao = Vector2Subtract(mousePos, centroJogador);
                    direcao = Vector2Normalize(direcao);

                    projeteis[i].velocidade = Vector2Scale(direcao, velocidadeTiro);
                    jogador.tempoRecargaTiro = 30;
                    break;
                }
            }
        }
    }

    for (int i = 0; i < MAX_TIROS; i++)
    {
        if (projeteis[i].ativo)
        {
            projeteis[i].posicao = Vector2Add(projeteis[i].posicao, projeteis[i].velocidade);

            if (CheckCollisionCircleRec(projeteis[i].posicao, 10, chefe.retangulo))
            {
                chefe.vida -= danoTiroAtual;
                tempoDanoChefe = 10;
                projeteis[i].ativo = false;
                continue;
            }

            if(projeteis[i].posicao.x < 0 || projeteis[i].posicao.x > largura ||
               projeteis[i].posicao.y < 0 || projeteis[i].posicao.y > altura)
            {
                projeteis[i].ativo = false;
            }
        }
    }

    if (IsKeyPressed(KEY_X) && jogador.tempoRecargaExplosao == 0)
    {
        jogador.usandoExplosao = true;
        jogador.tempoRecargaExplosao = 120;
        Vector2 centro = { jogador.posicao.x + 20, jogador.posicao.y + 20 };

        if (CheckCollisionCircleRec(centro, 150, chefe.retangulo))
        {
            chefe.vida -= DANO_EXPLOSAO;
            tempoDanoChefe = 20;

            if (chefe.posicao.x > jogador.posicao.x)
                chefe.posicao.x = fminf(chefe.posicao.x + 80, largura - chefe.retangulo.width);
            else
                chefe.posicao.x = fmaxf(chefe.posicao.x - 80, 0);
        }
    }

    if (chefe.posicao.x < jogador.posicao.x - 1) chefe.posicao.x += 1.8f;
    else if (chefe.posicao.x > jogador.posicao.x + 1) chefe.posicao.x -= 1.8f;

    if (chefe.posicao.y < jogador.posicao.y - 1) chefe.posicao.y += 1.8f;
    else if (chefe.posicao.y > jogador.posicao.y + 1) chefe.posicao.y -= 1.8f;

    chefe.retangulo.x = chefe.posicao.x;
    chefe.retangulo.y = chefe.posicao.y;

    if (CheckCollisionRecs(jogador.retangulo, chefe.retangulo))
        jogador.vida -= 2;

    if (chefe.vida <= 0 && !lojaAberta) 
    { 
        chefe.vida = 0; 
        lojaAberta = true;
        moedas = 40; 
    }

    if (jogador.vida <= 0) { jogador.vida = 0; resultadoFase = 2; }

    return 0;
}

void DrawFase1(void)
{
    DrawRectangleRec(jogador.retangulo, jogador.corPersonagem);

    if (jogador.usandoEspada)
        DrawRectangle(jogador.posicao.x + 40, jogador.posicao.y, 60, 40, YELLOW);

    if (jogador.usandoExplosao)
        DrawCircleLines(jogador.posicao.x + 20, jogador.posicao.y + 20, 150, ORANGE);

    for (int i = 0; i < MAX_TIROS; i++)
        if (projeteis[i].ativo)
            DrawCircleV(projeteis[i].posicao, 8, PURPLE);

    if (tempoDanoChefe > 0) {
        DrawRectangleRec(chefe.retangulo, WHITE);
        tempoDanoChefe--;
    } else {
        DrawRectangleRec(chefe.retangulo, chefe.corPersonagem);
    }

    DrawText("BOSS (TANK)", 10, 10, 20, BLACK);
    DrawRectangle(10, 30, chefe.vida / 5, 20, RED);
    DrawRectangleLines(10, 30, chefe.vidaMaxima / 5, 20, BLACK);

    DrawText("JOGADOR", 10, 400, 20, BLACK);
    DrawRectangle(10, 420, jogador.vida * 2, 20, GREEN);
    DrawRectangleLines(10, 420, jogador.vidaMaxima * 2, 20, BLACK);

    Color corTiro = (jogador.tempoRecargaTiro == 0) ? BLACK : GRAY;
    Color corExplosao = (jogador.tempoRecargaExplosao == 0) ? BLACK : GRAY;
    DrawText("Mouse: Tiro 360", 350, 420, 10, corTiro);
    DrawText("X: Explosão", 460, 420, 10, corExplosao);
    DrawCircleV(GetMousePosition(), 5, RED);

    if (jogoPausado)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));
        DrawText("JOGO PAUSADO", 280, 200, 30, WHITE);
        DrawText("Pressione P para continuar", 260, 240, 20, LIGHTGRAY);
    }

    if (lojaAberta && !jogoPausado)
    {
        Vector2 mouse = GetMousePosition();
        
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.85f));
        DrawText("LOJA DE UPGRADES", 280, 80, 30, GOLD);
        DrawText(TextFormat("MOEDAS: %d", moedas), 350, 120, 20, YELLOW);

        Rectangle btnTiro = { 180, 150, 440, 40 };
        Color corBtn1 = LIGHTGRAY;
        Color corTexto1 = DARKGRAY;
        
        if (CheckCollisionPointRec(mouse, btnTiro)) {
            corBtn1 = SKYBLUE;
            corTexto1 = BLACK;
        }
        
        DrawRectangleRec(btnTiro, corBtn1);
        DrawRectangleLinesEx(btnTiro, 2, WHITE);
        DrawText("+25% Velocidade do Tiro (30 moedas)", 200, 160, 18, corTexto1);

        Rectangle btnSpeed = { 180, 200, 440, 40 };
        Color corBtn2 = LIGHTGRAY;
        Color corTexto2 = DARKGRAY;

        if (CheckCollisionPointRec(mouse, btnSpeed)) {
            corBtn2 = SKYBLUE;
            corTexto2 = BLACK;
        }

        DrawRectangleRec(btnSpeed, corBtn2);
        DrawRectangleLinesEx(btnSpeed, 2, WHITE);
        DrawText("+25% Velocidade de Movimento (25 moedas)", 200, 210, 18, corTexto2);

        Rectangle btnDano = { 180, 250, 440, 40 };
        Color corBtn3 = LIGHTGRAY;
        Color corTexto3 = DARKGRAY;

        if (CheckCollisionPointRec(mouse, btnDano)) {
            corBtn3 = ORANGE;
            corTexto3 = BLACK;
        }

        DrawRectangleRec(btnDano, corBtn3);
        DrawRectangleLinesEx(btnDano, 2, WHITE);
        DrawText("Arma Dano+ (40 moedas)", 200, 260, 18, corTexto3);

        Rectangle btnSair = { 300, 350, 200, 50 };
        Color corSair = GREEN;
        
        if (CheckCollisionPointRec(mouse, btnSair)) {
            corSair = LIME;
        }

        DrawRectangleRec(btnSair, corSair);
        DrawText("AVANCAR >>", 340, 365, 20, BLACK);
    }
}