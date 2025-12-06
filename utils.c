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


void passaNivel (controleCombate *control){
  Jogador *j = &control->jog_atv;
  // Incrementa o nível
  control->nivel++;
  // Verifica se ultrapassou de 10 níveis (obs: 1 a 10)
  if (control->nivel > 10) {
          control->estado = "VITORIA"; // Muda o estado para desenhar a tela final
          exibirMensagem(control, "PARABENS! O PREMIO É SEU!", al_map_rgb(255, 215, 0));
          return; // Encerra a função aqui, NÃO gera novos inimigos
      }

    exibirMensagem(control, "Você venceu! Novos inimigos a caminho...", al_map_rgb(50, 255, 50));
    descartaMao(j);             // descarta as cartas para nova rodada
    control->turnoCount = 0;    // reinicia o contador de turnos
    iniciaTurnoJogador(j);      // reseta energia escudo etc
    control->indiceInimigoAtual = 0; // Reseta para o primeiro

    // Limpeza de memória dos inimigos mortos
    for(int k=0; k < control->inim_atv.qtd; k++){
          if(control->inim_atv.inimigos[k].enemy->img)
            al_destroy_bitmap(control->inim_atv.inimigos[k].enemy->img);  //limpa imagem
          if(control->inim_atv.inimigos[k].enemy->nome)
            free(control->inim_atv.inimigos[k].enemy->nome);  //limpa nome
          if(control->inim_atv.inimigos[k].enemy)
            free(control->inim_atv.inimigos[k].enemy);  //limpa struct Criatura
          if(control->inim_atv.inimigos[k].acoes)
            free(control->inim_atv.inimigos[k].acoes);  //limpa a struct de ações
    }
    if(control->inim_atv.inimigos) free(control->inim_atv.inimigos);  //limpa os inimigos
    
    // Chama a função que cria um novo grupo baseado na lista global e no contador
    grupoInimigos *novoGp = geraGrupoInimigos(control->listainimigos, &control->contadorInimigos);
    
    // Copia o novo grupo para a struct de controle
    control->inim_atv = *novoGp; 
    
    // Libera o ponteiro temporário
    free(novoGp); 
}




void jogaCarta (controleCombate *control, int index_carta){
  
  if(!control) return;

  // Para facilitar cria variáveis com as referências ao jogador, ao inimigo e à carta escolhida
  Jogador *j = &control->jog_atv;
  Inimigo *i = &control->inim_atv.inimigos[control->indiceInimigoAtual];

  if (index_carta < 0 || index_carta >= j->mao->tam) return;
  Carta c = j->mao->cartas[index_carta];

  // Só permitir jogar a carta se custo desta for menor ou igual a energia
  if (c.custo > j->energia){
    exibirMensagem(control, "Voce nao possui energia suficiente para usar esta carta! Tente outra carta ou passe a vez (ESC).", al_map_rgb(255, 50, 50));
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

  int causouDano;
  if (c.tipo == ATAQUE){
    aplicaDano(i->enemy, c.efeito);
    causouDano = 1;
  }

  // CARTA DE ATAQUE: aplicar os efeitos da carta sobre o inimigo
  if (causouDano){
    // Se o inimigo morrer, troca o index para o próximo inimigo
    // Se não tiver próx inimigo (2 por nível), muda de nível e retorna index p primeiro inimigo do nível
         
    // Verifica se há inimigos vivos
    int vivos = 0;
    for(int k = 0; k < control->inim_atv.qtd; k++){
        if(control->inim_atv.inimigos[k].enemy->ptsVida > 0) {
            vivos++;
        }
    }

    // Se não tiver inimigos vivos -> Passa de Nível
    if (vivos == 0) {
      passaNivel(control);
    } 
    // Se ainda tem inimigos vivos -> Troca alvo
    else {
        if (control->inim_atv.inimigos[control->indiceInimigoAtual].enemy->ptsVida <= 0) {
              // Troca para o próximo (inverte 0 e 1)
              control->indiceInimigoAtual = (control->indiceInimigoAtual + 1) % 2;
        }
    }
  }
  

  // CARTA DE DEFESA: aplicar os efeitos da carta sobre o escudo do jogador
  if (c.tipo == DEFESA){
    j->player.ptsEscudo += c.efeito;
  }

  // CARTA ESPECIAL: reembaralha
  if (c.tipo == ESPECIAL){
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

    // Texto que será exibido ao final
    char relatorioTurno[256] = "";

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
            // Salva status ANTES do dano - para a mensagem final
            int escudoAntes = jog->player.ptsEscudo;
            int vidaAntes = jog->player.ptsVida;

            // Aplica efeito no jogador e verifica se morreu
            if (aplicaDano(&jog->player, acaoDoTurno.efeito) == 1) {
                control->estado = "GAMEOVER";
                return;
            }
            
            // Calcula o quanto perdeu
            int escudoPerdido = escudoAntes - jog->player.ptsEscudo;
            int vidaPerdida = vidaAntes - jog->player.ptsVida;

            // Adiciona ao texto da mensagem a ser exibida
            // Ex: "Manu atacou: -10 Escudo, -5 Vida. "
            char acaoMsg[100];
            if (escudoPerdido == 0){
            sprintf(acaoMsg, "%s atacou: -%d Vida. \n", 
                    atual->enemy->nome, vidaPerdida);
            strcat(relatorioTurno, acaoMsg);
            } else {
            sprintf(acaoMsg, "%s atacou: -%d Escudo, -%d Vida. \n", 
                    atual->enemy->nome, escudoPerdido, vidaPerdida);
            strcat(relatorioTurno, acaoMsg);             
            }          
  
        } else if (acaoDoTurno.tipoAcao == DEFESA) {
            // aumenta os pontos de escudo
            atual->enemy->ptsEscudo += acaoDoTurno.efeito;
            // monta mensagem do inimigo e adiciona à lista
            char acaoMsg[100];
            sprintf(acaoMsg, "%s defendeu (+%d escudo).", atual->enemy->nome, acaoDoTurno.efeito);
            strcat(relatorioTurno, acaoMsg);
        }
    }

    // Exibe as mensagens de todos os inimigos
    exibirMensagem(control, relatorioTurno, al_map_rgb(255, 255, 100));

    // Incrementa o contador de turnos do inimigo
    control->turnoCount++;

    // Passa o estado da vez pro jogador
    control->estado = "VEZ_JOGADOR";
    iniciaTurnoJogador(jog);
}

// Função auxiliar para configurar a mensagem de exibição na tela
void exibirMensagem(controleCombate *c, const char *texto, ALLEGRO_COLOR cor) {
    if (!c) return;
    
    // Copia os parametros pra struct de controle e define um tempo padrão
    snprintf(c->msg.texto, sizeof(c->msg.texto), "%s", texto);
    c->msg.cor = cor;
    c->msg.timer = 240; // 4 segundos (60 frames * 5)
}


/*
void reiniciarJogo(controleCombate *c) {
    // Reseta Status do Jogador
    c->jog_atv.player.ptsVida = c->jog_atv.player.vidaMax; // Cura total
    c->jog_atv.player.ptsEscudo = 0;
    c->jog_atv.energia = 3;
    
    // Reseta Nível e Turnos
    c->nivel = 1;
    c->turnoCount = 0;
    c->estado = "VEZ_JOGADOR";
    c->selecionandoAlvo = 0;
    c->selected_card_idx = 0;
    
    // Reseta Baralho (Recolhe mão e descarte de volta pro deck)
    descartaMao(&c->jog_atv);   // Joga mao no descarte
    reciclaPilha(&c->jog_atv);  // Joga descarte no deck e embaralha
    
    // Compra mão inicial (5 cartas)
    iniciaTurnoJogador(&c->jog_atv);

    // Limpa Inimigos Antigos (se houver) e Gera Novos
    if (c->inim_atv.inimigos != NULL) {
        // Limpeza manual rápida
        for(int k=0; k < c->inim_atv.qtd; k++){
            if(c->inim_atv.inimigos[k].enemy->img) al_destroy_bitmap(c->inim_atv.inimigos[k].enemy->img);
            if(c->inim_atv.inimigos[k].enemy->nome) free(c->inim_atv.inimigos[k].enemy->nome);
            free(c->inim_atv.inimigos[k].enemy);
            free(c->inim_atv.inimigos[k].acoes);
        }
        free(c->inim_atv.inimigos);
    }

    c->contadorInimigos = 0; // Reinicia a seed de escolha
    grupoInimigos *gp = geraGrupoInimigos(c->listainimigos, &c->contadorInimigos);
    c->inim_atv = *gp;
    free(gp);
    
    // Mensagem inicial
    exibirMensagem(c, "Você começou uma nova competição!", al_map_rgb(255, 255, 255));
} 
*/