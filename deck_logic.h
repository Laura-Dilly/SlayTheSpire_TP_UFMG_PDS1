#ifndef DECK_LOGIC_H
#define DECK_LOGIC_H

#include "constants.h"
#include <allegro5/allegro.h>

// Leitura / liberação de nomes
NomesCartas geraNomesCarta(void);
void embaralhaNomes(char **vetor, int tamanho);
void liberaNomesCartas(NomesCartas *nc);

// Geração / manipulação de baralho
Carta *geraCarta (CartaTipo type, char *name, int cost);
grupoCarta *geraBaralhoInicial (void);
grupoCarta* criaPilhaVazia();
void liberaBaralho(grupoCarta *baralho);

// Operações de jogo (prototypes existentes no .c)
void embaralhaCartas(grupoCarta *baralho);
void reciclaPilha(Jogador *j);
void compraCartas(Jogador *j, int qtd);
void descartaMao (Jogador *j);

#endif