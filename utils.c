#include "utils.h"

#include <allegro5/allegro5.h>
#include <stdio.h>
#include "constants.h"
#include "deck_logic.h"
#include "creatures_logic.h"

void must_init(_Bool test, const char* description) {
  if (test) return;

  fprintf(stderr, "couldn't initialize %s\n", description);
  exit(1);
}

int NumToDigits(int n) {
  if (n < 0) return 1 + NumToDigits(-n);
  if (n < 10) return 1;
  if (n < 100) return 2;
  if (n < 1000) return 3;
  if (n < 10000) return 4;
  if (n < 100000) return 5;
  if (n < 1000000) return 6;
  if (n < 10000000) return 7;
  if (n < 100000000) return 8;
  if (n < 1000000000) return 9;
  /*      2147483647 is 2^31-1 - add more ifs as needed
     and adjust this final return as well. */
  return 10;
}

void ClearKeyboardKeys(unsigned char* keyboard_keys) {
  memset(keyboard_keys, 0, ALLEGRO_KEY_MAX * sizeof(unsigned char));
}

void ShuffleArray(int* array, int size) {
  if (size > 1) {
    int i;
    for (i = 0; i < size - 1; i++) {
      int j = i + rand() / (RAND_MAX / (size - i) + 1);
      if (j == i) {
        continue;
      }
      int int_j = array[j];

      array[j] = array[i];

      array[i] = int_j;
    }
  }
}


// Funções auxiliares para a lógica de combate

void iniciaTurnoJogador (Jogador *jog){
  
  if (jog == NULL) return;

  // define energia e escudo do jogador no início do turno
  jog->energia = 3;
  jog->player.ptsEscudo = 0;

  // compra 5 cartas da pilha - mão do jogador
  compraCartas(jog, 5);
}

// Aplicação do dano em caso de ataques
// Aplica o efeito e retorna se a criatura morreu (1) ou não (0)
int aplicaDano (Criatura *target, int dano){
  if (target == NULL) return 0;
  if (dano == 0) return 0;

  if(target->ptsEscudo > 0){
    if(dano <= target->ptsEscudo){
      target->ptsEscudo -= dano;
    } else {
      dano -= target->ptsEscudo;
      target->ptsEscudo = 0;
      if (target->ptsVida <= dano){
        target->ptsVida = 0;
        return 1;
      } else {
        target->ptsVida -= dano;
      }
    }
  } else {
    if (target->ptsVida <= dano){
        target->ptsVida = 0;
        return 1;
      } else {
        target->ptsVida -= dano;
      }
  }

  return 0;
}



void jogaCarta (controleCombate *control, int index_carta){
  
  if(!control) return;

  // Para facilitar cria variáveis com as referências ao jogador, ao inimigo e à carta escolhida
  Jogador *j = &control->jog_atv;
  Inimigo *i = &control->inim_atv.inimigos[control->indiceInimigoAtual];

  if (index_carta < 0 || index_carta >= j->mao->tam) return;
  Carta c = j->mao->cartas[index_carta];

  // Só permitir jogar a carta se custo desta for menor que energia
  if (c.custo > j->energia){
    printf("Você não possui energia suficiente para jogar essa carta.");
    return;
  } else {
    // já diminuir custo da energia
    j->energia -= c.custo;
  }
    
  // adicionar a carta utilizada ao descarte
  int novoTam = j->descarte->tam + 1;
  j->descarte->cartas = (Carta *) realloc(j->descarte->cartas, novoTam * sizeof(Carta));
  if (!j->descarte->cartas){
    printf("Erro ao descartar a carta a ser utilizada pelo jogador.");
    exit(EXIT_FAILURE);
  }
  j->descarte->cartas[novoTam - 1] = c;
  j->descarte->tam = novoTam;

  // remover carta da mão
  // deslocar todas as cartas após a removida uma posição para trás
  for (int w = index_carta; w < j->mao->tam - 1; w++){
    j->mao->cartas[w] = j->mao->cartas[w + 1];
  }
  j->mao->tam--;
  if (j->mao->tam > 0) {
      j->mao->cartas = (Carta *) realloc(j->mao->cartas, j->mao->tam * sizeof(Carta));
  } else {
      free(j->mao->cartas);
      j->mao->cartas = NULL;
  }

  // CARTA DE ATAQUE: aplicar os efeitos da carta sobre o inimigo
  if (c.tipo == ATAQUE){
    // Se o inimigo morrer, troca o index para o próximo inimigo
    // Se não tiver próx inimigo (2 por nível), muda de nível e retorna index p primeiro inimigo do nível
    if(aplicaDano(i->enemy, c.efeito) == 1){
      control->indiceInimigoAtual++;
      if(control->indiceInimigoAtual >= 2){
        control->nivel++;
        control->indiceInimigoAtual = 0;
      }
    }
  }

  // CARTA DE DEFESA: aplicar os efeitos da carta sobre o escudo do jogador
  if (c.tipo == DEFESA){
    j->player.ptsEscudo += c.efeito;
  }

  // CARTA ESPECIAL: 
  if (c.tipo == ESPECIAL){
    // aplica o dano
    if(aplicaDano(i->enemy, c.efeito) == 1){
      control->indiceInimigoAtual++;
      if(control->indiceInimigoAtual >= 2){
        control->nivel++;
        control->indiceInimigoAtual = 0;
      }
    }
    // descarta a mão do jogador e compra 5 cartas da pilha
    descartaMao(j);
    compraCartas(j, 5);
  }

  return;

}


void turnoInimigos (controleCombate *control){
    if (!control) return;

    // Separa os inimigos e o jogador
    grupoInimigos *gp = &control->inim_atv;
    Jogador *jog = &control->jog_atv;

    // Loop por todos os inimigos do grupo (2)
    for (int i = 0; i < gp->qtd; i++) {
        Inimigo *atual = &gp->inimigos[i];

        // Se a vida do primeiro inimigo for <= 0, pula para o próximo.
        if (atual->enemy->ptsVida <= 0) {
            continue; 
        }

        // reseta escudo
        atual->enemy->ptsEscudo = 0;

        // Define a ação que será executada com base no turno atual e na qtd de ações do inimigo
        int indiceAcao = control->turnoCount % atual->qtdAcoes;
        acao acaoDoTurno = atual->acoes[indiceAcao];

        // Executa a ação
        if (acaoDoTurno.tipoAcao == ATAQUE) {
            // Aplica efeito no jogador e verifica se morreu
            if (aplicaDano(&jog->player, acaoDoTurno.efeito) == 1) {
                control->estado = "GAMEOVER";
                return;
            }
        } else if (acaoDoTurno.tipoAcao == DEFESA) {
            // aumenta os pontos de escudo
            atual->enemy->ptsEscudo += acaoDoTurno.efeito;
        }
    }

    // Incrementa o contador de turnos do inimigo
    control->turnoCount++;

    // Passa o estado da vez pro jogador
    control->estado = "VEZ_JOGADOR";
    iniciaTurnoJogador(jog);
}