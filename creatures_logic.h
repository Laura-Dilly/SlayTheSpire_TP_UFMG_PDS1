#ifndef CREATURES_LOGIC_H
#define CREATURES_LOGIC_H

#include "constants.h"
#include <allegro5/allegro.h>

//Jogador
Criatura* criaOpcoesJogadores();
Jogador* geraJogador(Criatura *opcoes, int escolha);
void liberaJogador(Jogador *j);

//Inimigos
NomesInimigos *geraNomesInimigos(void);
void liberaNomesInimigos(NomesInimigos *ni);
void embaralhaInimigos(char **inimigos, char **imagens, int tamanho);
grupoInimigos *geraGrupoInimigos(NomesInimigos *ni, int *idxGlobal);
void liberaGrupoInimigos(grupoInimigos *gp);

#endif