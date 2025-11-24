#include "fase1.h"
#include "raymath.h"
#include <stdio.h>
#include "raylib.h"

//Ajustes
#define MAX_TIROS 5
#define VEL_TIRO 7.0f
#define DANO_ESPADADA 15
#define DANO_TIRO 10
#define DANO_EXPLOSAO 60

//Estruturas
typedef struct {
    Vector2 posicao;
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

//Globais
static Personagem jogador;
static Personagem chefe;
static Projetil projeteis[MAX_TIROS];

static int tempoDanoChefe = 0;
static int resultadoFase = 0;

//Funções
void InitFase1(void)
{
    //Jogador
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

    //Chefe
    chefe.posicao = (Vector2){ 600, 200 };
    chefe.retangulo = (Rectangle){ 600, 200, 80, 80 };
    chefe.vidaMaxima = 1500;
    chefe.vida = 1500;
    chefe.corPersonagem = RED;

    //Projeteis
    for (int i = 0; i < MAX_TIROS; i++) projeteis[i].ativo = false;

    resultadoFase = 0;
}

int UpdateFase1(void)
{
    if (resultadoFase != 0) return resultadoFase;

    //Movimento do jogador
    if (IsKeyDown(KEY_RIGHT)) jogador.posicao.x += 5.0f;
    if (IsKeyDown(KEY_LEFT))  jogador.posicao.x -= 5.0f;
    if (IsKeyDown(KEY_UP))    jogador.posicao.y -= 5.0f;
    if (IsKeyDown(KEY_DOWN))  jogador.posicao.y += 5.0f;

    int largura = GetScreenWidth();
    int altura  = GetScreenHeight();

    //Limites de tela com teletransporte
    if (jogador.posicao.x > largura)
        jogador.posicao.x = -jogador.retangulo.width;
    else if (jogador.posicao.x < -jogador.retangulo.width)
        jogador.posicao.x = largura;

    if (jogador.posicao.y > altura)
        jogador.posicao.y = -jogador.retangulo.height;
    else if (jogador.posicao.y < -jogador.retangulo.height)
        jogador.posicao.y = altura;

    //Hitbox
    jogador.retangulo.x = jogador.posicao.x;
    jogador.retangulo.y = jogador.posicao.y;

    //Cooldowns
    if (jogador.tempoRecargaEspada > 0) jogador.tempoRecargaEspada--;
    if (jogador.tempoRecargaTiro > 0) jogador.tempoRecargaTiro--;
    if (jogador.tempoRecargaExplosao > 0) jogador.tempoRecargaExplosao--;

    jogador.usandoEspada = false;
    jogador.usandoExplosao = false;

    //Ataque Espada
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

    //Ataque Tiro
    if (IsKeyPressed(KEY_Z) && jogador.tempoRecargaTiro == 0)
    {
        for (int i = 0; i < MAX_TIROS; i++)
        {
            if (!projeteis[i].ativo)
            {
                projeteis[i].ativo = true;
                projeteis[i].posicao = (Vector2){ jogador.posicao.x + 20, jogador.posicao.y + 20 };
                jogador.tempoRecargaTiro = 30;
                break;
            }
        }
    }

    //Atualização dos projéteis
    for (int i = 0; i < MAX_TIROS; i++)
    {
        if (projeteis[i].ativo)
        {
            Vector2 alvo = (Vector2){ chefe.posicao.x + 40, chefe.posicao.y + 40 };
            projeteis[i].posicao = Vector2MoveTowards(projeteis[i].posicao, alvo, VEL_TIRO);

            if (CheckCollisionCircleRec(projeteis[i].posicao, 10, chefe.retangulo))
            {
                chefe.vida -= DANO_TIRO;
                tempoDanoChefe = 10;
                projeteis[i].ativo = false;
                continue;
            }

            if (Vector2Distance(projeteis[i].posicao, alvo) < 5.0f)
            {
                projeteis[i].ativo = false;
            }
        }
    }

    //Explosão
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

    //IA do Chefe
    if (chefe.posicao.x < jogador.posicao.x - 1) chefe.posicao.x += 1.8f;
    else if (chefe.posicao.x > jogador.posicao.x + 1) chefe.posicao.x -= 1.8f;

    if (chefe.posicao.y < jogador.posicao.y - 1) chefe.posicao.y += 1.8f;
    else if (chefe.posicao.y > jogador.posicao.y + 1) chefe.posicao.y -= 1.8f;

    chefe.retangulo.x = chefe.posicao.x;
    chefe.retangulo.y = chefe.posicao.y;

    //Contato com o jogador
    if (CheckCollisionRecs(jogador.retangulo, chefe.retangulo))
        jogador.vida -= 2;

    if (chefe.vida <= 0) { chefe.vida = 0; resultadoFase = 1; }
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
    DrawText("Z: Magia Teleguiada", 350, 420, 10, corTiro);
    DrawText("X: Explosão", 460, 420, 10, corExplosao);
}
