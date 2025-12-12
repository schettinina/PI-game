#include "fase1.h"
#include "raymath.h"
#include <stdio.h>
#include "raylib.h"

// --- Definições e Constantes ---
#define MAX_TIROS 5          // Limite de tiros simultâneos na tela
#define DANO_EXPLOSAO 60     // Dano da habilidade especial "Explosão"

// Estrutura para os tiros do jogador
typedef struct {
    Vector2 posicao;
    Vector2 velocidade; 
    bool ativo;              // Flag para saber se o tiro está em uso ou disponível
} Projetil;

// Estrutura genérica para Jogador e Chefe
typedef struct {
    Vector2 posicao;
    Rectangle retangulo;     // Hitbox para colisão
    int vida;
    int vidaMaxima;
    Color corPersonagem;

    int tempoRecargaTiro;     // Cooldown do tiro básico
    int tempoRecargaExplosao; // Cooldown da habilidade especial

    bool usandoExplosao;      // Flag visual para desenhar a explosão
} Personagem;

// --- Variáveis Globais (Estado do Jogo) ---
static Personagem jogador;
static Personagem chefe;
static Projetil projeteis[MAX_TIROS]; // Array de tiros (Object Pooling)

static int tempoDanoChefe = 0;   // Tempo que o chefe pisca branco ao levar dano
static int resultadoFase = 0;    // 0: Jogando, 1: Vitória/Próxima Fase, 2: Game Over

static float velocidadeMovimento = 4.0f;
static float velocidadeTiro = 7.0f;
static int danoTiroAtual = 10;
static int moedas = 0;           // Moedas ganhas ao derrotar o chefe
static bool lojaAberta = false;  // Flag para ativar a interface de loja
static bool jogoPausado = false; // Flag de pause

// --- Inicialização ---
void InitFase1(void)
{
    // Configura o Jogador
    jogador.posicao = (Vector2){ 100, 200 };
    jogador.retangulo = (Rectangle){ 100, 200, 40, 40 };
    jogador.vidaMaxima = 80;
    jogador.vida = 80;
    jogador.corPersonagem = BLUE;

    jogador.tempoRecargaTiro = 0;
    jogador.tempoRecargaExplosao = 0;
    jogador.usandoExplosao = false;

    // Configura o Chefe
    chefe.posicao = (Vector2){ 600, 200 };
    chefe.retangulo = (Rectangle){ 600, 200, 80, 80 };
    chefe.vidaMaxima = 1500;
    chefe.vida = 1500;
    chefe.corPersonagem = RED;

    // Reseta todos os projéteis para inativos
    for (int i = 0; i < MAX_TIROS; i++) projeteis[i].ativo = false;

    // Reseta status da partida
    velocidadeMovimento = 4.0f;
    velocidadeTiro = 7.0f;
    danoTiroAtual = 10;
    moedas = 0;
    lojaAberta = false;
    resultadoFase = 0;
    jogoPausado = false;
}

// --- Loop de Atualização (Lógica) ---
int UpdateFase1(void)
{
    // Se o jogo já acabou (vitória ou derrota), retorna o código imediatamente
    if (resultadoFase != 0) return resultadoFase;

    // Sistema de Pausa
    if (IsKeyPressed(KEY_P)) 
    {
        jogoPausado = !jogoPausado;
    }

    if (jogoPausado) return 0; // Se pausado, não executa o resto da lógica

    // --- Lógica da Loja (Só executa se a loja estiver aberta) ---
    if (lojaAberta)
    {
        Vector2 mouse = GetMousePosition();

        // Botão: Upgrade de Velocidade do Tiro
        Rectangle btnTiro = { 180, 150, 440, 40 };
        if (CheckCollisionPointRec(mouse, btnTiro) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (moedas >= 30) {
                moedas -= 30;
                velocidadeTiro *= 1.25f;
            }
        }

        // Botão: Upgrade de Velocidade de Movimento
        Rectangle btnSpeed = { 180, 200, 440, 40 };
        if (CheckCollisionPointRec(mouse, btnSpeed) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (moedas >= 25) {
                moedas -= 25;
                velocidadeMovimento *= 1.25f;
            }
        }

        // Botão: Upgrade de Dano
        Rectangle btnDano = { 180, 250, 440, 40 };
        if (CheckCollisionPointRec(mouse, btnDano) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (moedas >= 40) {
                moedas -= 40;
                danoTiroAtual += 10;
                jogador.corPersonagem = ORANGE; // Muda cor para indicar poder
            }
        }

        // Botão: Sair da fase (Avançar)
        Rectangle btnSair = { 300, 350, 200, 50 };
        if (CheckCollisionPointRec(mouse, btnSair) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            resultadoFase = 1; // Código 1 = Próxima fase
        }

        return 0; // Retorna cedo para não processar movimento/combate durante a loja
    }

    // --- Movimentação do Jogador ---
    Vector2 inputMovimento = {0.0f, 0.0f};

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) inputMovimento.x += 1.0f;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))  inputMovimento.x -= 1.0f;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))    inputMovimento.y -= 1.0f;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))  inputMovimento.y += 1.0f;

    // Normalização: Impede que andar na diagonal seja mais rápido que em linha reta
    if (inputMovimento.x != 0 || inputMovimento.y != 0)
    {
        inputMovimento = Vector2Normalize(inputMovimento);
        jogador.posicao.x += inputMovimento.x * velocidadeMovimento;
        jogador.posicao.y += inputMovimento.y * velocidadeMovimento;
    }

    // --- Limites de Tela (Wrap-around) ---
    // Se o jogador sair pela direita, aparece na esquerda (e vice-versa)
    int largura = GetScreenWidth();
    int altura  = GetScreenHeight();

    if (jogador.posicao.x > largura) jogador.posicao.x = -jogador.retangulo.width;
    else if (jogador.posicao.x < -jogador.retangulo.width) jogador.posicao.x = largura;

    if (jogador.posicao.y > altura) jogador.posicao.y = -jogador.retangulo.height;
    else if (jogador.posicao.y < -jogador.retangulo.height) jogador.posicao.y = altura;

    // Atualiza a hitbox do jogador para acompanhar a posição visual
    jogador.retangulo.x = jogador.posicao.x;
    jogador.retangulo.y = jogador.posicao.y;

    // --- Gerenciamento de Cooldowns ---
    if (jogador.tempoRecargaTiro > 0) jogador.tempoRecargaTiro--;
    if (jogador.tempoRecargaExplosao > 0) jogador.tempoRecargaExplosao--;

    jogador.usandoExplosao = false; // Reseta visual da explosão a cada frame

    // --- Mecânica de Tiro (Mouse ou Tecla Z) ---
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_Z)) 
    {
        if (jogador.tempoRecargaTiro == 0)
        {
            // Object Pooling: Procura um slot de tiro inativo no array
            for (int i = 0; i < MAX_TIROS; i++)
            {
                if (!projeteis[i].ativo)
                {
                    projeteis[i].ativo = true;
                    // O tiro sai do centro do jogador
                    Vector2 centroJogador = { jogador.posicao.x + 20, jogador.posicao.y + 20 };
                    projeteis[i].posicao = centroJogador;

                    // Calcula direção baseada no mouse
                    Vector2 mousePos = GetMousePosition();
                    Vector2 direcao = Vector2Subtract(mousePos, centroJogador);
                    direcao = Vector2Normalize(direcao);

                    projeteis[i].velocidade = Vector2Scale(direcao, velocidadeTiro);
                    jogador.tempoRecargaTiro = 30; // Define cooldown
                    break; // Sai do loop após disparar um tiro
                }
            }
        }
    }

    // --- Atualização dos Projéteis ---
    for (int i = 0; i < MAX_TIROS; i++)
    {
        if (projeteis[i].ativo)
        {
            projeteis[i].posicao = Vector2Add(projeteis[i].posicao, projeteis[i].velocidade);

            // Colisão Tiro vs Chefe
            if (CheckCollisionCircleRec(projeteis[i].posicao, 10, chefe.retangulo))
            {
                chefe.vida -= danoTiroAtual;
                tempoDanoChefe = 10; // Ativa efeito de "piscar" do chefe
                projeteis[i].ativo = false; // Destrói o tiro
                continue;
            }

            // Destrói o tiro se sair da tela
            if(projeteis[i].posicao.x < 0 || projeteis[i].posicao.x > largura ||
               projeteis[i].posicao.y < 0 || projeteis[i].posicao.y > altura)
            {
                projeteis[i].ativo = false;
            }
        }
    }

    // --- Habilidade Especial: Explosão (Tecla X) ---
    if (IsKeyPressed(KEY_X) && jogador.tempoRecargaExplosao == 0)
    {
        jogador.usandoExplosao = true;
        jogador.tempoRecargaExplosao = 120; // Cooldown longo
        Vector2 centro = { jogador.posicao.x + 20, jogador.posicao.y + 20 };

        // Verifica colisão radial (Explosão em área)
        if (CheckCollisionCircleRec(centro, 150, chefe.retangulo))
        {
            chefe.vida -= DANO_EXPLOSAO;
            tempoDanoChefe = 20;

            // Empurrão (Knockback) no chefe
            if (chefe.posicao.x > jogador.posicao.x)
                chefe.posicao.x = fminf(chefe.posicao.x + 80, largura - chefe.retangulo.width);
            else
                chefe.posicao.x = fmaxf(chefe.posicao.x - 80, 0);
        }
    }

    // --- Inteligência Artificial do Chefe (Perseguição) ---
    if (chefe.posicao.x < jogador.posicao.x - 1) chefe.posicao.x += 1.8f;
    else if (chefe.posicao.x > jogador.posicao.x + 1) chefe.posicao.x -= 1.8f;

    if (chefe.posicao.y < jogador.posicao.y - 1) chefe.posicao.y += 1.8f;
    else if (chefe.posicao.y > jogador.posicao.y + 1) chefe.posicao.y -= 1.8f;

    // Atualiza hitbox do chefe
    chefe.retangulo.x = chefe.posicao.x;
    chefe.retangulo.y = chefe.posicao.y;

    // --- Colisão Jogador vs Chefe ---
    if (CheckCollisionRecs(jogador.retangulo, chefe.retangulo))
        jogador.vida -= 2; // Dano contínuo por contato

    // --- Verificação de Morte do Chefe ---
    if (chefe.vida <= 0 && !lojaAberta) 
    { 
        chefe.vida = 0; 
        lojaAberta = true; // Abre a loja ao matar o chefe
        moedas = 40;       // Dá moedas ao jogador
    }

    // --- Verificação de Game Over ---
    if (jogador.vida <= 0) { 
        jogador.vida = 0; 
        resultadoFase = 2; // Código 2 = Derrota
    }

    return 0; // Continua jogando
}

// --- Renderização (Desenho) ---
void DrawFase1(void)
{
    // Desenha o jogador
    DrawRectangleRec(jogador.retangulo, jogador.corPersonagem);

    // Desenha efeito visual da explosão (apenas um frame)
    if (jogador.usandoExplosao)
        DrawCircleLines(jogador.posicao.x + 20, jogador.posicao.y + 20, 150, ORANGE);

    // Desenha todos os projéteis ativos
    for (int i = 0; i < MAX_TIROS; i++)
        if (projeteis[i].ativo)
            DrawCircleV(projeteis[i].posicao, 8, PURPLE);

    // Desenha o chefe (Piscando branco se levou dano)
    if (tempoDanoChefe > 0) {
        DrawRectangleRec(chefe.retangulo, WHITE);
        tempoDanoChefe--;
    } else {
        DrawRectangleRec(chefe.retangulo, chefe.corPersonagem);
    }

    // --- Interface (HUD) ---
    
    // Barra de vida do Chefe
    DrawText("BOSS (TANK)", 10, 10, 20, BLACK);
    DrawRectangle(10, 30, chefe.vida / 5, 20, RED);
    DrawRectangleLines(10, 30, chefe.vidaMaxima / 5, 20, BLACK);

    // Barra de vida do Jogador
    DrawText("JOGADOR", 10, 400, 20, BLACK);
    DrawRectangle(10, 420, jogador.vida * 2, 20, GREEN);
    DrawRectangleLines(10, 420, jogador.vidaMaxima * 2, 20, BLACK);

    // Indicadores de Cooldown (Texto muda de cor)
    Color corTiro = (jogador.tempoRecargaTiro == 0) ? BLACK : GRAY;
    Color corExplosao = (jogador.tempoRecargaExplosao == 0) ? BLACK : GRAY;
    DrawText("Mouse: Tiro 360", 350, 420, 10, corTiro);
    DrawText("X: Explosão", 460, 420, 10, corExplosao);
    
    // Mira do mouse
    DrawCircleV(GetMousePosition(), 5, RED);

    // --- Tela de Pause ---
    if (jogoPausado)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));
        DrawText("JOGO PAUSADO", 280, 200, 30, WHITE);
        DrawText("Pressione P para continuar", 260, 240, 20, LIGHTGRAY);
    }

    // --- Tela da Loja (Overlay) ---
    if (lojaAberta && !jogoPausado)
    {
        Vector2 mouse = GetMousePosition();
        
        // Fundo escuro semitransparente
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.85f));
        DrawText("LOJA DE UPGRADES", 280, 80, 30, GOLD);
        DrawText(TextFormat("MOEDAS: %d", moedas), 350, 120, 20, YELLOW);

        // Desenho do Botão Tiro
        Rectangle btnTiro = { 180, 150, 440, 40 };
        Color corBtn1 = LIGHTGRAY;
        Color corTexto1 = DARKGRAY;
        
        // Efeito Hover (passar o mouse por cima)
        if (CheckCollisionPointRec(mouse, btnTiro)) {
            corBtn1 = SKYBLUE;
            corTexto1 = BLACK;
        }
        
        DrawRectangleRec(btnTiro, corBtn1);
        DrawRectangleLinesEx(btnTiro, 2, WHITE);
        DrawText("+25% Velocidade do Tiro (30 moedas)", 200, 160, 18, corTexto1);

        // Desenho do Botão Velocidade
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

        // Desenho do Botão Dano
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

        // Desenho do Botão Sair/Avançar
        Rectangle btnSair = { 300, 350, 200, 50 };
        Color corSair = GREEN;
        
        if (CheckCollisionPointRec(mouse, btnSair)) {
            corSair = LIME;
        }

        DrawRectangleRec(btnSair, corSair);
        DrawText("AVANCAR >>", 340, 365, 20, BLACK);
    }
}