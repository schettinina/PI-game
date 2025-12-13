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

// --- SPRITES DOS INIMIGOS ---
#define BOSS_FRAMES 7
#define MINION_FRAMES 7 

static Texture2D bossSprites[BOSS_FRAMES];   // R-Series (Robô Menor)
static Texture2D minionSprites[MINION_FRAMES]; // G-Series (Robô Maior)

static int bossFrameAtual = 0;
static int bossFrameCounter = 0; 

static int minionFrameAtual = 0; 
static int minionFrameCounter = 0;
// ----------------------------

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

// Enum para definir o tipo de desenho do inimigo
typedef enum {
    TIPO_QUADRADO,        
    TIPO_MINION_SPRITE,   // Minions (G-Series, maior)
    TIPO_BOSS_SPRITE      // Boss (R-Series, menor)
} TipoDesenho;

typedef struct {
    bool ativo;              
    Vector2 posicao;
    Rectangle retangulo;
    int vida;
    float velocidade;
    Color cor;
    TipoDesenho tipoDesenho; 
} Inimigo;

// Globais
static Jogador jogador;
static Projetil projeteis[MAX_TIROS];
static Inimigo inimigos[MAX_INIMIGOS_CAP]; 

// --- VARIÁVEIS DE IMAGEM ---
static Texture2D fundoFase2; 
static Texture2D imgGameOverFase2; 
static Texture2D imgVictory; 
static Texture2D imgIntro2; // <--- NOVA IMAGEM DE INTRO FASE 2
static bool texturaCarregada = false; 
static int tempoDanoInimigo[MAX_INIMIGOS_CAP] = { 0 }; 

// Variáveis de Controle da Intro
static bool mostrandoIntro = true;      // <--- Começa mostrando a intro
static float tempoIntroContador = 0.0f; // <--- Contador de tempo da intro

// Estado do Jogo
static int resultadoFase = 0; 
static bool gameOverAtivo = false; 
static bool vitoriaAtiva = false; 

// Controle de Hordas
static int hordaAtual = 1;
static int inimigosPorHorda = 5;      
static int inimigosMortosHorda = 0;   
static int timerSpawn = 0;            

// --- Função para Reiniciar Variáveis ---
void ResetarVariaveisFase2(void) 
{
    // ... (Configurações do Jogador) ...
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
    for (int i = 0; i < MAX_INIMIGOS_CAP; i++) {
        inimigos[i].ativo = false;
        tempoDanoInimigo[i] = 0;
        inimigos[i].tipoDesenho = TIPO_QUADRADO; 
    }

    // Reseta Frames de Animação
    bossFrameAtual = 0;
    bossFrameCounter = 0;
    minionFrameAtual = 0;
    minionFrameCounter = 0;

    // Reseta Hordas
    hordaAtual = 1;
    inimigosPorHorda = 5;
    inimigosMortosHorda = 0;
    timerSpawn = 0;
    
    // Reseta Estados
    resultadoFase = 0;
    gameOverAtivo = false;
    vitoriaAtiva = false; 
}

void SpawnInimigo()
{
    // Lógica para spawnar o Boss Principal (EXEMPLO: Apenas uma vez na Horda 5)
    bool deveSpawnarBossPrincipal = (hordaAtual == 5 && inimigosMortosHorda == 0); 

    for (int i = 0; i < MAX_INIMIGOS_CAP; i++) 
    {
        if (!inimigos[i].ativo)
        {
            // --- Configuração Padrão ---
            inimigos[i].velocidade = (float)GetRandomValue(20, 40) / 10.0f; 
            inimigos[i].ativo = true;
            inimigos[i].cor = WHITE; 

            if (deveSpawnarBossPrincipal) {
                // --- BOSS PRINCIPAL (VISUALMENTE O R-Series/Robô Menor) ---
                inimigos[i].vida = 300; 
                inimigos[i].velocidade = 2.5f;
                inimigos[i].tipoDesenho = TIPO_BOSS_SPRITE;
                
                // Hitbox para o Boss (Menor)
                inimigos[i].posicao = (Vector2){ 400, -50 };
                inimigos[i].retangulo = (Rectangle){ inimigos[i].posicao.x, inimigos[i].posicao.y, 40, 40 };
                
                deveSpawnarBossPrincipal = false; 
            } else {
                // --- INIMIGO COMUM (VISUALMENTE O G-Series/Robô Maior) ---
                inimigos[i].vida = 40; 
                inimigos[i].tipoDesenho = TIPO_MINION_SPRITE;
                
                // Hitbox Padrão (Maior)
                int lado = GetRandomValue(0, 3);
                if (lado == 0) inimigos[i].posicao = (Vector2){ (float)GetRandomValue(0, 800), -50 }; 
                else if (lado == 1) inimigos[i].posicao = (Vector2){ (float)GetRandomValue(0, 800), 500 }; 
                else if (lado == 2) inimigos[i].posicao = (Vector2){ -50, (float)GetRandomValue(0, 450) }; 
                else if (lado == 3) inimigos[i].posicao = (Vector2){ 850, (float)GetRandomValue(0, 450) }; 
                
                inimigos[i].retangulo = (Rectangle){ inimigos[i].posicao.x, inimigos[i].posicao.y, 80, 80 };
            }
            
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
        imgIntro2 = LoadTexture("images/intro2.jpg"); // <--- CARREGA A INTRO 2
        
        // BOSS (R-SERIES)
        bossSprites[0] = LoadTexture("images/r01.png");
        bossSprites[1] = LoadTexture("images/r02.png");
        bossSprites[2] = LoadTexture("images/r03.png");
        bossSprites[3] = LoadTexture("images/r04.png");
        bossSprites[4] = LoadTexture("images/r05.png");
        bossSprites[5] = LoadTexture("images/r06.png");
        bossSprites[6] = LoadTexture("images/r07.png");

        // MINION (G-SERIES)
        minionSprites[0] = LoadTexture("images/g01.png");
        minionSprites[1] = LoadTexture("images/g02.png");
        minionSprites[2] = LoadTexture("images/g03.png");
        minionSprites[3] = LoadTexture("images/g04.png");
        minionSprites[4] = LoadTexture("images/g05.png");
        minionSprites[5] = LoadTexture("images/g06.png");
        minionSprites[6] = LoadTexture("images/g07.png");

        texturaCarregada = true;
    }

    // Configuração inicial da Intro
    mostrandoIntro = true;
    tempoIntroContador = 0.0f;

    ResetarVariaveisFase2();
}

void UnloadFase2(void) {
    if (texturaCarregada) {
        UnloadTexture(fundoFase2);
        UnloadTexture(imgGameOverFase2);
        UnloadTexture(imgVictory);
        UnloadTexture(imgIntro2); // <--- DESCARREGA INTRO 2
        
        // Descarregando sprites
        for (int i = 0; i < BOSS_FRAMES; i++) UnloadTexture(bossSprites[i]);
        for (int i = 0; i < MINION_FRAMES; i++) UnloadTexture(minionSprites[i]);
        
        texturaCarregada = false;
    }
}

int UpdateFase2(void)
{
    // --- LÓGICA DA INTRODUÇÃO ---
    if (mostrandoIntro)
    {
        tempoIntroContador += GetFrameTime(); 
        
        if (tempoIntroContador >= 3.0f) // 3 segundos de duração
        {
            mostrandoIntro = false; 
        }
        
        return 0; // Pausa o update do jogo enquanto intro roda
    }

    if (resultadoFase != 0) return resultadoFase;

    if (gameOverAtivo) 
    {
        if (IsKeyPressed(KEY_R)) {
            ResetarVariaveisFase2();
        }
        return 0; 
    }

    if (vitoriaAtiva)
    {
        if (IsKeyPressed(KEY_ENTER)) {
            resultadoFase = 1; 
        }
        if (IsKeyPressed(KEY_R)) {
            ResetarVariaveisFase2();
        }
        return 0; 
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
    if (timerSpawn > 120) { 
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
                    tempoDanoInimigo[i] = 10;
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
                        tempoDanoInimigo[j] = 10; 
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
    for (int i = 0; i < MAX_INIMIGOS_CAP; i++) {
        if (inimigos[i].ativo) {
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
                tempoDanoInimigo[i] = 0;
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
        
        if (inimigosPorHorda > 30) {
            vitoriaAtiva = true; 
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
    // --- DESENHO DA INTRO (Se estiver ativa) ---
    if (mostrandoIntro)
    {
        Rectangle sourceIntro = { 0.0f, 0.0f, (float)imgIntro2.width, (float)imgIntro2.height };
        Rectangle destIntro = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
        Vector2 origin = { 0.0f, 0.0f };
        DrawTexturePro(imgIntro2, sourceIntro, destIntro, origin, 0.0f, WHITE);
        
        return; // Retorna para não desenhar o jogo
    }

    // --- DESENHO DO FUNDO E JOGADOR ---
    Rectangle source = { 0.0f, 0.0f, (float)fundoFase2.width, (float)fundoFase2.height };
    Rectangle dest = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
    Vector2 origin = { 0.0f, 0.0f };
    DrawTexturePro(fundoFase2, source, dest, origin, 0.0f, WHITE);

    DrawRectangleRec(jogador.retangulo, jogador.corPersonagem);
    if (jogador.usandoExplosao) DrawCircleLines(jogador.posicao.x+20, jogador.posicao.y+20, 150, ORANGE);

    // Desenha Projéteis
    for (int i = 0; i < MAX_TIROS; i++) {
        if (projeteis[i].ativo) DrawCircleV(projeteis[i].posicao, 5, YELLOW);
    }

    // --- DESENHO DOS INIMIGOS (COM SPRITES R e G) ---
    
    // Atualização da Animação do Boss (R-Series)
    bossFrameCounter++;
    if (bossFrameCounter >= 8) 
    {
        bossFrameCounter = 0;
        bossFrameAtual++;
        if (bossFrameAtual >= BOSS_FRAMES)
            bossFrameAtual = 0;
    }

    // Atualização da Animação do Minion (G-Series)
    minionFrameCounter++;
    if (minionFrameCounter >= 10) 
    {
        minionFrameCounter = 0;
        minionFrameAtual++;
        if (minionFrameAtual >= MINION_FRAMES)
            minionFrameAtual = 0;
    }
    
    for (int i = 0; i < MAX_INIMIGOS_CAP; i++) {
        if (inimigos[i].ativo) {
            
            // Define a cor de piscar (Dano)
            Color tint = WHITE;
            if (tempoDanoInimigo[i] > 0) {
                tint = LIGHTGRAY; 
                tempoDanoInimigo[i]--;
            }

            if (inimigos[i].tipoDesenho == TIPO_BOSS_SPRITE) {
                
                // Desenha o BOSS (R-Series)
                DrawTextureEx(
                    bossSprites[bossFrameAtual],
                    inimigos[i].posicao,
                    0.0f,
                    0.5f, // Escala menor (0.5)
                    tint
                );
                
                // Barra de Vida do BOSS
                int larguraBarra = 40; 
                DrawRectangle(inimigos[i].posicao.x, inimigos[i].posicao.y - 10, larguraBarra, 5, RED);
                DrawRectangle(inimigos[i].posicao.x, inimigos[i].posicao.y - 10, 
                    (int)(((float)inimigos[i].vida / 300.0f) * larguraBarra), 5, GREEN); 

            } else if (inimigos[i].tipoDesenho == TIPO_MINION_SPRITE) {
                
                // Desenha o MINION (G-Series)
                 DrawTextureEx(
                    minionSprites[minionFrameAtual],
                    inimigos[i].posicao,
                    0.0f,
                    1.0f, // Escala maior (1.0)
                    tint
                );
                
                // Barra de Vida do MINION
                int larguraBarra = 80; 
                DrawRectangle(inimigos[i].posicao.x, inimigos[i].posicao.y - 5, larguraBarra, 3, RED);
                DrawRectangle(inimigos[i].posicao.x, inimigos[i].posicao.y - 5, 
                    (int)(((float)inimigos[i].vida / 40.0f) * larguraBarra), 3, GREEN); 
                
            } else {
                 DrawRectangleRec(inimigos[i].retangulo, RED);
                 DrawRectangle(inimigos[i].posicao.x, inimigos[i].posicao.y - 5, inimigos[i].vida, 3, GREEN);
            }
        }
    }

    // --- HUD ---
    DrawText(TextFormat("HORDA: %d", hordaAtual), 350, 10, 30, WHITE);
    DrawText(TextFormat("Inimigos: %d / %d", inimigosMortosHorda, inimigosPorHorda), 320, 45, 20, LIGHTGRAY);
    
    DrawText("VIDA", 10, 400, 20, WHITE);
    DrawRectangle(10, 420, jogador.vida * 2, 20, SKYBLUE);
    DrawRectangleLines(10, 420, 200, 20, BLACK);
    
    DrawCircleV(GetMousePosition(), 5, RED);

    // --- Telas Finais ---
    if (gameOverAtivo)
    {
        Rectangle sourceGO = { 0.0f, 0.0f, (float)imgGameOverFase2.width, (float)imgGameOverFase2.height };
        Rectangle destGO = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
        Vector2 originGO = { 0.0f, 0.0f };
        DrawTexturePro(imgGameOverFase2, sourceGO, destGO, originGO, 0.0f, WHITE);
        if ((int)(GetTime() * 2) % 2 == 0) {
            const char* texto = "TECLE [R] PARA REINICIAR";
            int fontSize = 30;
            int larguraTexto = MeasureText(texto, fontSize);
            DrawText(texto, (GetScreenWidth() - larguraTexto) / 2, GetScreenHeight() - 80, fontSize, WHITE);
        }
    }

    if (vitoriaAtiva)
    {
        Rectangle sourceWin = { 0.0f, 0.0f, (float)imgVictory.width, (float)imgVictory.height };
        Rectangle destWin = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };
        Vector2 originWin = { 0.0f, 0.0f };
        DrawTexturePro(imgVictory, sourceWin, destWin, originWin, 0.0f, WHITE);
    }
}