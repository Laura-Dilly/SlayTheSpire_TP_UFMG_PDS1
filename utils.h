#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdlib.h>
#include "constants.h"

void must_init(_Bool test, const char* description);

int NumToDigits(int n);

void ClearKeyboardKeys(unsigned char* keyboard_keys);

void ShuffleArray(int* array, int size);

// Funções adicionadas

void iniciaTurnoJogador (Jogador *jog);

int aplicaDano (Criatura *target, int dano);

void jogaCarta (controleCombate *control, int index_carta);

void turnoInimigos (controleCombate *control);

void exibirMensagem(controleCombate *c, const char *texto, ALLEGRO_COLOR cor);

void reiniciarJogo(controleCombate *c);



#endif