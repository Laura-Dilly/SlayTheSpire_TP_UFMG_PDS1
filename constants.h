#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#define GAME_KEY_SEEN 1
#define GAME_KEY_DOWN 2

#define DISPLAY_BUFFER_WIDTH 1920.0
#define DISPLAY_BUFFER_HEIGHT 1080.0

#define DISPLAY_SCALE 1.0
#define DISPLAY_WIDTH DISPLAY_SCALE* DISPLAY_BUFFER_WIDTH
#define DISPLAY_HEIGHT DISPLAY_SCALE* DISPLAY_BUFFER_HEIGHT

#define DRAW_DECK_X 15
#define DRAW_DECK_Y 910

#define DECK_WIDTH 80
#define DECK_HEIGHT 80

#define PLAYER_BEGIN_X 500
#define PLAYER_BEGIN_Y 375
#define PLAYER_RADIUS 100

#define HEALTH_BAR_HEIGHT 20
#define HEALTH_BAR_BACKGROUND_EXTRA 1

#define HEALTH_BAR_RX 0
#define HEALTH_BAR_RY 0

#define HAND_BEGIN_X 325
#define HAND_BEGIN_Y 780

#define CARD_WIDTH 280
#define CARD_HEIGHT 420

// You might want to use this.
typedef enum {
  MOVE_LEFT = 0,
  MOVE_RIGHT = 1,
} CursorMovementDirection;

// Definindo as cartas e seus tipos, além do grupoCarta (baralho)

typedef struct {
    char **names;
    int names_count;
    char **special_names;
    int special_count;
} NomesCartas;

typedef enum { 
  ATAQUE, 
  DEFESA, 
  ESPECIAL 
} CartaTipo;

typedef struct {
  char *nome;
  CartaTipo tipo;
  int custo;
  int efeito;
} Carta;

typedef struct {
  Carta *cartas;
  int tam;       //Tamanho da pilha
} grupoCarta;


// Definindo a base dos jogadores, bem como jogador e inimigo
typedef struct {
  char *nome;
  int ptsVida;
  int maxVida;
  int ptsEscudo;
  int coord_x;
  int coord_y;
} Criatura;

typedef struct {
  Criatura player;
  grupoCarta mao;       // baralho atual do jogador
  grupoCarta descarte;  // baralho descartado pelo jogador
} Jogador;

typedef enum { 
  FRACO, 
  FORTE 
} inimigoTipo;

typedef enum {
  TESTE,
  TESTE2
} acoes;

typedef struct {
  Criatura enemy;
  inimigoTipo tipo;
  acoes Acao;
} Inimigo;

typedef struct {
  int qtd;    //Quantidade de inimigos
  Inimigo inimigos[];
} grupoInimigos;

// Definindo o Controle do jogo [Situação]
typedef struct {
  char *estado;
  Jogador jog_atv;
  grupoInimigos inim_atv;
  int nivel;    // Contador do núm de combates ou nível
} controleCombate;


#endif