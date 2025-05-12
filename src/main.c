/**
 * main.h
 * Created on Aug, 23th 2023
 * Author: Tiago Barros + Mateus Xavier 
 * Based on "From C to C++ course - 2002"
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "screen.h"
#include "keyboard.h"
#include "timer.h"

/**********************
 * CONFIGURAÇÕES DO JOGO
 **********************/
#define PADDLE_SYMBOL "[=======]"  // Símbolo da raquete
#define BALL_SYMBOL "●"            // Símbolo da bola (Unicode)
#define BRICK_SYMBOL "▄▄▄▄"        // Símbolo dos tijolos (Unicode)
#define PADDLE_WIDTH (sizeof(PADDLE_SYMBOL) - 1)
#define BALL_WIDTH 1                // Largura da bola (1 caractere)
#define BRICK_WIDTH (sizeof(BRICK_SYMBOL) - 1)
#define BRICK_HEIGHT 1
#define BRICK_ROWS 5                // Linhas de tijolos
#define BRICK_COLS 12               // Colunas de tijolos
#define INITIAL_LIVES 3             // Vidas iniciais
#define MAX_LEVELS 5                // Níveis máximos
#define SCORE_MULTIPLIER 10         // Multiplicador base de pontos

/********************
 * CONTROLES DO JOGO
 ********************/
#define KEY_LEFT 'a'                // Tecla esquerda (alternativa)
#define KEY_RIGHT 'd'               // Tecla direita (alternativa)
#define KEY_LEFT_ALT 75             // Seta esquerda
#define KEY_RIGHT_ALT 77            // Seta direita
#define KEY_PAUSE 'p'               // Pausar jogo
#define KEY_QUIT 27                 // Tecla ESC para sair

/*********************
 * ESTRUTURA DO JOGO
 *********************/
typedef struct {
    int x, y;            // Posição da bola
    int dx, dy;          // Direção da bola
    int paddle_x;        // Posição X da raquete
    int score;           // Pontuação atual
    int lives;           // Vidas restantes
    int level;           // Nível atual
    int multiplier;      // Multiplicador de pontos para combos
    bool game_over;      // Jogo terminado?
    bool game_won;       // Jogo vencido?
    bool paused;         // Jogo pausado?
    time_t last_hit;     // Último hit para calcular multiplicador
    bool bricks[BRICK_ROWS][BRICK_COLS]; // Matriz de tijolos
} GameState;

GameState game; // Variável global do estado do jogo

/*************************
 * FUNÇÕES DO JOGO
 *************************/

/**
 * Inicializa o estado do jogo
 * @param new_level Se true, apenas avança para o próximo nível
 */
void initGame(bool new_level) {
    if (!new_level) {
        // Reinício completo do jogo
        game.score = 0;
        game.lives = INITIAL_LIVES;
        game.level = 1;
        game.multiplier = 1;
    } else {
        // Apenas avança para o próximo nível
        game.level++;
        if (game.level > MAX_LEVELS) {
            game.game_won = true;
            return;
        }
    }

    // Posição inicial da bola
    game.x = MAXX / 2;
    game.y = MAXY / 2;
    
    // Direção inicial aleatória
    game.dx = (rand() % 2) ? 1 : -1;
    game.dy = -1;
    
    // Posição inicial da raquete
    game.paddle_x = MAXX / 2 - PADDLE_WIDTH / 2;
    
    // Estado do jogo
    game.game_over = false;
    game.game_won = false;
    game.paused = false;
    game.last_hit = time(NULL);

    // Inicializa todos os tijolos como ativos
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            game.bricks[i][j] = true;
        }
    }
}

/**
 * Desenha a raquete na tela
 */
void drawPaddle() {
    screenSetColor(CYAN, DARKGRAY);
    screenGotoxy(game.paddle_x, MAXY - 3);
    printf(PADDLE_SYMBOL);
    screenSetNormal();
}

/**
 * Desenha a bola na tela
 */
void drawBall() {
    screenSetColor(YELLOW, DARKGRAY);
    screenGotoxy(game.x, game.y);
    printf(BALL_SYMBOL);
    screenSetNormal();
}

/**
 * Desenha os tijolos na tela com cores diferentes
 */
void drawBricks() {
    for (int i = 0; i < BRICK_ROWS; i++) {
        screenColor color;
        // Define cores diferentes para cada linha
        switch (i % 5) {
            case 0: color = LIGHTRED; break;
            case 1: color = YELLOW; break;
            case 2: color = LIGHTGREEN; break;
            case 3: color = LIGHTBLUE; break;
            case 4: color = LIGHTMAGENTA; break;
        }
        
        screenSetColor(color, DARKGRAY);
        for (int j = 0; j < BRICK_COLS; j++) {
            if (game.bricks[i][j]) {
                int bx = MINX + 4 + j * (BRICK_WIDTH + 1);
                int by = MINY + 2 + i * (BRICK_HEIGHT + 1);
                screenGotoxy(bx, by);
                printf(BRICK_SYMBOL);
            }
        }
    }
    screenSetNormal();
}

/**
 * Desenha a interface do usuário (pontuação, vidas, nível)
 */
void drawUI() {
    // Mostra pontuação
    screenSetColor(YELLOW, DARKGRAY);
    screenGotoxy(2, 1);
    printf("Pontos: %d", game.score);
    
    // Mostra vidas restantes
    screenGotoxy(20, 1);
    printf("Vidas: %d", game.lives);
    
    // Mostra nível atual
    screenGotoxy(40, 1);
    printf("Nível: %d/%d", game.level, MAX_LEVELS);
    
    // Mostra multiplicador se maior que 1
    if (game.multiplier > 1) {
        screenSetColor(LIGHTRED, DARKGRAY);
        screenGotoxy(60, 1);
        printf("x%d!", game.multiplier);
    }
    
    screenSetNormal();
}

/**
 * Desenha mensagens de jogo (game over, vitória, pausa)
 */
void drawMessage() {
    if (game.game_over) {
        screenSetColor(LIGHTRED, DARKGRAY);
        screenGotoxy(MAXX/2 - 5, MAXY/2);
        printf("FIM DE JOGO!");
        screenGotoxy(MAXX/2 - 10, MAXY/2 + 1);
        printf("Pressione ESPAÇO para recomeçar");
    } 
    else if (game.game_won) {
        screenSetColor(LIGHTGREEN, DARKGRAY);
        screenGotoxy(MAXX/2 - 5, MAXY/2);
        printf("VOCÊ VENCEU!");
        screenGotoxy(MAXX/2 - 15, MAXY/2 + 1);
        printf("Pontuação Final: %d (Pressione ESPAÇO)", game.score);
    }
    else if (game.paused) {
        screenSetColor(YELLOW, DARKGRAY);
        screenGotoxy(MAXX/2 - 3, MAXY/2);
        printf("PAUSADO");
    }
    screenSetNormal();
}

/**
 * Verifica colisão da bola com os tijolos
 */
void checkBrickCollision() {
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            if (game.bricks[i][j]) {
                int bx = MINX + 4 + j * (BRICK_WIDTH + 1);
                int by = MINY + 2 + i * (BRICK_HEIGHT + 1);
                
                // Verifica se a bola está dentro do tijolo
                if (game.x >= bx && game.x < bx + BRICK_WIDTH &&
                    game.y >= by && game.y < by + BRICK_HEIGHT) {
                    
                    game.bricks[i][j] = false;
                    
                    // Atualiza multiplicador de pontos
                    time_t now = time(NULL);
                    if (now - game.last_hit <= 1) {
                        game.multiplier++;
                    } else {
                        game.multiplier = 1;
                    }
                    game.last_hit = now;
                    
                    // Calcula pontos (linhas superiores valem mais)
                    game.score += (BRICK_ROWS - i) * SCORE_MULTIPLIER * game.multiplier;
                    
                    // Inverte direção Y
                    game.dy = -game.dy;
                    
                    // Ajusta ângulo baseado na posição do hit
                    float hit_pos = (game.x - bx) / (float)BRICK_WIDTH;
                    game.dx = (hit_pos - 0.5f) * 2;
                    
                    return;
                }
            }
        }
    }
}

/**
 * Verifica se o nível foi completado
 */
bool isLevelComplete() {
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            if (game.bricks[i][j]) return false;
        }
    }
    return true;
}

/**
 * Processa entrada do teclado
 */
void handleInput() {
    if (keyhit()) {
        int ch = readch();
        
        // Trata teclas de seta (sequência especial)
        if (ch == 0xE0) {
            ch = readch();
            switch (ch) {
                case KEY_LEFT_ALT:
                    if (game.paddle_x > MINX + 2)
                        game.paddle_x -= 3;  // Move mais rápido
                    break;
                case KEY_RIGHT_ALT:
                    if (game.paddle_x + PADDLE_WIDTH < MAXX - 2)
                        game.paddle_x += 3;
                    break;
            }
        } 
        else {
            switch (ch) {
                case KEY_LEFT:
                    if (game.paddle_x > MINX + 2)
                        game.paddle_x -= 3;
                    break;
                case KEY_RIGHT:
                    if (game.paddle_x + PADDLE_WIDTH < MAXX - 2)
                        game.paddle_x += 3;
                    break;
                case KEY_PAUSE:
                    game.paused = !game.paused;  // Pausa/despausa
                    break;
                case ' ':  // Espaço para reiniciar
                    if (game.game_over || game.game_won) {
                        initGame(false);
                    }
                    break;
                case KEY_QUIT:
                    exit(0);  // Sai do jogo
                    break;
            }
        }
    }
}

/**
 * Atualiza o estado do jogo
 */
void updateGame() {
    if (game.game_over || game.game_won || game.paused) return;

    // Move a bola
    game.x += game.dx;
    game.y += game.dy;

    // Colisão com as paredes
    if (game.x <= MINX + 1 || game.x >= MAXX - 2)
        game.dx = -game.dx;
    if (game.y <= MINY + 1)
        game.dy = -game.dy;

    // Colisão com a raquete
    if (game.y >= MAXY - 4 && 
        game.x >= game.paddle_x && 
        game.x <= game.paddle_x + PADDLE_WIDTH) {
        
        // Calcula posição do hit na raquete (-1 a 1)
        float hit_pos = ((game.x - game.paddle_x) / (float)PADDLE_WIDTH) * 2 - 1;
        
        // Ajusta ângulo baseado na posição
        game.dx = hit_pos * 1.5f;
        game.dy = -abs(game.dy);  // Sempre quica para cima
        
        // Reseta multiplicador ao bater na raquete
        game.multiplier = 1;
    }

    // Colisão com o fundo (perde vida)
    if (game.y >= MAXY - 1) {
        if (--game.lives <= 0) {
            game.game_over = true;
        } else {
            // Reposiciona a bola
            game.x = MAXX / 2;
            game.y = MAXY / 2;
            game.dx = (rand() % 2) ? 1 : -1;
            game.dy = -1;
        }
    }

    checkBrickCollision();

    // Verifica se completou o nível
    if (isLevelComplete()) {
        if (game.level < MAX_LEVELS) {
            initGame(true);  // Próximo nível
        } else {
            game.game_won = true;
        }
    }
}

/**
 * Função principal
 */
int main() {
    srand(time(NULL));  // Inicializa gerador de números aleatórios
    
    // Inicializa sistemas do jogo
    screenInit(1);     // Tela
    keyboardInit();     // Teclado
    timerInit(50);      // Temporizador (20 FPS)
    
    // Inicializa estado do jogo
    initGame(false);

    // Loop principal do jogo
    while (1) {
        if (timerTimeOver()) {
            handleInput();
            updateGame();
            
            // Limpa a tela
            screenClear();
            
            // Desenha elementos do jogo
            drawBricks();
            drawBall();
            drawPaddle();
            drawUI();
            drawMessage();
            
            // Atualiza a tela
            screenUpdate();
        }
    }

    // Limpeza (não alcançável no fluxo atual)
    keyboardDestroy();
    screenDestroy();
    timerDestroy();
    
    return 0;
}