#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <allegro5/allegro.h>

#define GAME_KEY_SEEN 1
#define GAME_KEY_DOWN 2

#define DISPLAY_BUFFER_WIDTH 1920.0
#define DISPLAY_BUFFER_HEIGHT 1080.0

#define DISPLAY_SCALE 1.0
#define DISPLAY_WIDTH DISPLAY_SCALE* DISPLAY_BUFFER_WIDTH
#define DISPLAY_HEIGHT DISPLAY_SCALE* DISPLAY_BUFFER_HEIGHT

#define DRAW_DECK_X 20
#define DRAW_DECK_Y 700

#define DECK_WIDTH 200
#define DECK_HEIGHT 300

#define PLAYER_BEGIN_X 400
#define PLAYER_BEGIN_Y 350
#define PLAYER_RADIUS 130

#define HEALTH_BAR_HEIGHT 20
#define HEALTH_BAR_BACKGROUND_EXTRA 1

#define HEALTH_BAR_RX 0
#define HEALTH_BAR_RY 0

#define HAND_BEGIN_X 300
#define HAND_BEGIN_Y 700

#define CARD_WIDTH 200
#define CARD_HEIGHT 300

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
  ALLEGRO_BITMAP *img;
  int ptsVida;
  int vidaMax;
  int ptsEscudo;
} Criatura;

typedef struct {
  Criatura player;
  int energia;
  int energiaMax;
  grupoCarta *pilha;         // pilha de compras do jogador
  grupoCarta *mao;           // baralho atual do jogador
  grupoCarta *descarte;      // pilha de descarte do jogador
  int vitorias;             // nº de combates vencidos
} Jogador;

typedef enum { 
  FRACO, 
  FORTE 
} inimigoTipo;

typedef struct {
    char **nome;
    char **imag;
    int totalNomes;
} NomesInimigos;


typedef struct {
  CartaTipo tipoAcao;
  int custo;
  int efeito;
} acao;

typedef struct {
  Criatura *enemy;
  inimigoTipo tipo;
  acao *acoes;
  int qtdAcoes;
} Inimigo;

typedef struct {
  int qtd;    //Quantidade de inimigos
  Inimigo *inimigos;
} grupoInimigos;

// Definindo o Controle do jogo [Situação]
typedef struct {
  char *estado;
  int turnoCount;
  Jogador jog_atv;
  grupoInimigos inim_atv;
  int nivel;    // Contador do núm de combates ou nível
  int indiceInimigoAtual;
  NomesInimigos *listainimigos;
} controleCombate;


#endif