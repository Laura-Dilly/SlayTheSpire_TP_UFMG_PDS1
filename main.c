#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "constants.h"
#include "renderer.h"
#include "utils.h"
#include "deck_logic.h"
#include "creatures_logic.h"

int main() {
  srand(time(NULL));
  // Inicialização do Allegro e addons
  must_init(al_init(), "allegro");
  must_init(al_init_image_addon(), "allegro");
  must_init(al_init_primitives_addon(), "primitives");
  must_init(al_install_keyboard(), "keyboard");

  // Criando timer em 60 FPS
  ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
  must_init(timer, "timer");

  // Criando fila de eventos do alegro
  ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
  must_init(queue, "queue");

  // Registrando fontes de eventos (teclado e timer)
  al_register_event_source(queue, al_get_keyboard_event_source());
  al_register_event_source(queue, al_get_timer_event_source(timer));
  unsigned char keyboard_keys[ALLEGRO_KEY_MAX];
  ClearKeyboardKeys(keyboard_keys);
  ALLEGRO_EVENT event;

  // Inicializando a struct renderer - para desenho e controle de combate
  Renderer renderer;
  FillRenderer(&renderer);
  al_register_event_source(queue, al_get_display_event_source(renderer.display));

  // Iniciando o timer e em seguida o jogo
  al_start_timer(timer);
  // while 1 --> enquanto não houver algum erro
  while (1) {
    al_wait_for_event(queue, &event);     // Espera o próximo evento acontecer
    int done = 0, print_combat = 0, redraw = 0;

    switch (event.type) {
      // Cria casos para cada tipo de evento (timer, teclas, fechamento de tela)
      case ALLEGRO_EVENT_TIMER:
        redraw = 1;   // redesenhar a tela

        // Atualiza o timer das mensagens que aparecem na tela
        if (renderer.combate->msg.timer > 0) {
            renderer.combate->msg.timer--;
        }

        if (keyboard_keys[ALLEGRO_KEY_Q]) {
          done = 1;     // será usado para encerrar o jogo
          break;
        }
        
        // Limpa a flag de "tecla vista" para o próximo frame
        for (int i = 0; i < ALLEGRO_KEY_MAX; i++) {
          keyboard_keys[i] &= ~GAME_KEY_SEEN;
        }
        break;
      
      // Evento de tecla pressionada
      case ALLEGRO_EVENT_KEY_DOWN:
        keyboard_keys[event.keyboard.keycode] = GAME_KEY_SEEN | GAME_KEY_DOWN;  // registra no array de teclas
        // Em caso de tecla pressionada, controle das ações do jogador:      
        // Separando (para facilitar) o ponteiro para o combate
        controleCombate* combate = renderer.combate;
        
        // Tela de perda ou vitória
        if (strcmp(combate->estado, "GAMEOVER") == 0 || strcmp(combate->estado, "VITORIA") == 0) {
            
            // Tecla R: Reinicia o Jogo
            if (event.keyboard.keycode == ALLEGRO_KEY_R) {
                reiniciarJogo(combate);
            }
            // Tecla Q: Sair
            if (event.keyboard.keycode == ALLEGRO_KEY_Q) {
                done = 1;
            }
        } else if (strcmp(combate->estado, "VEZ_JOGADOR") == 0) {        // Se o estado do jogo for a vez do jogador... 
          
          // ESTADO DE ESCOLHER A CARTA:
          if (combate->selecionandoAlvo == 0){
          
            // separar a qtd de cartas na mão
            int tam_mao = combate->jog_atv.mao->tam;

            // Navegação do usuário pelas setas do teclado (Esquerda/Direita) - índice da carta escolhida
            if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
              if (combate->selected_card_idx > 0) {
                combate->selected_card_idx--;
              }
            }
            if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
              if (combate->selected_card_idx < tam_mao - 1) {
                combate->selected_card_idx++;
              }
            }

            // enter - escolha da carta e alvo
            if (event.keyboard.keycode == ALLEGRO_KEY_ENTER && tam_mao > 0) {
                      Carta c = combate->jog_atv.mao->cartas[combate->selected_card_idx];

                      // Se for defesa aplica a carta
                      if (c.tipo == DEFESA) {
                          jogaCarta(combate, combate->selected_card_idx);
                          
                          // Voltar o índice da carta selecionada em 1 posição (evitar erros se jogar a última carta)
                          if (combate->selected_card_idx >= combate->jog_atv.mao->tam) 
                              combate->selected_card_idx = combate->jog_atv.mao->tam - 1;
                          if (combate->selected_card_idx < 0) combate->selected_card_idx = 0;
                      } 
                      // Se for ataque ou especial --> vai para seleção de inimigo
                      else {
                          // Verifica se tem energia antes de entrar no modo de mira
                          if (c.custo <= combate->jog_atv.energia) {
                              combate->selecionandoAlvo = 1;
                              
                              // Alvo deve estar vivo, senão troca para o outro
                              if(combate->inim_atv.inimigos[combate->indiceInimigoAtual].enemy->ptsVida <= 0){
                                  combate->indiceInimigoAtual = (combate->indiceInimigoAtual + 1) % 2;
                              }
                          }
                          else {
                            exibirMensagem(combate, "Voce nao possui energia suficiente para usar esta carta! Tente outra carta ou passe a vez (ESC).", al_map_rgb(255, 50, 50));
                          }
                      }
            }
                      
            // Se apertada a tecla esc -> passar turno ao inimigo
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
              descartaMao(&combate->jog_atv);
              combate->estado = "VEZ_INIMIGOS";
              combate->selected_card_idx = 0;
            }

            // Outras teclas "cheats"
            // Espaço: Mata todos os inimigos
            if (event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
              for(int i = 0; i < combate->inim_atv.qtd; i++){
                  combate->inim_atv.inimigos[i].enemy->ptsVida = 0;
              }
            }
            // Tecla X: Vida do jogador vira 1
            if (event.keyboard.keycode == ALLEGRO_KEY_X) {
              combate->jog_atv.player.ptsVida = 1;
            }
          
          
          } else {        // no caso de ser o Estado de selecionar inimigo
            if (event.keyboard.keycode == ALLEGRO_KEY_LEFT || event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                    // Alterna entre 0 e 1 (índice dos inimigos do combate)
                    int proximo = (combate->indiceInimigoAtual + 1) % 2;
                    // Só troca se o próximo estiver vivo
                    if (combate->inim_atv.inimigos[proximo].enemy->ptsVida > 0) {
                        combate->indiceInimigoAtual = proximo;
                    }

            }
          

          // Se apertada a tecla enter -> jogar carta no inimigo selecionado
          if (event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
            jogaCarta(combate, combate->selected_card_idx);
            combate->selecionandoAlvo = 0;   // volta pro estado de seleção de carta
            // Voltar o índice da carta selecionada em 1 posição (evitar erros se jogar a última carta)
            if (combate->selected_card_idx >= combate->jog_atv.mao->tam) {
                combate->selected_card_idx = combate->jog_atv.mao->tam - 1;
            }
            // Se o ajuste acima resultar em índice negativo, volta pra 0
            if (combate->selected_card_idx < 0) combate->selected_card_idx = 0;
          }

        }

        // Se apertada a tecla esc -> passar turno ao inimigo
        if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
          combate->selecionandoAlvo = 0;
          descartaMao(&combate->jog_atv);
          combate->estado = "VEZ_INIMIGOS";
          combate->selected_card_idx = 0;
        }
      }
        break;
      
      // Evento de soltar a tecla que estava pressionada
      case ALLEGRO_EVENT_KEY_UP:
        keyboard_keys[event.keyboard.keycode] &= ~GAME_KEY_DOWN;
        break;
      
      // Evento de fechar a janela (clicar em x)
      case ALLEGRO_EVENT_DISPLAY_CLOSE:
        done = true;  // será usado para encerrar o jogo
        break;
    }
    // Encerra o jogo se done = true/1
    if (done) {
      break;
    }

    // IA do Inimigo (Turno do Inimigo)
    // Se o estado mudou para a vez dos inimigos...
    if (strcmp(renderer.combate->estado, "VEZ_INIMIGOS") == 0) {
        // Pequena pausa (visual)
        al_rest(0.5); 
        // Ataque dos inimigos
        turnoInimigos(renderer.combate);
    }

    // Checagem se o jogo acabou (jogador perdeu) antes de continuar o loop
    if (strcmp(renderer.combate->estado, "GAMEOVER") == 0) {
        // TELA DE DERROTA
        printf("YOU LOST! No amount of autotune can save you...\n");
    }

    // Se o comando (a partir da variável redraw) de redesenhar "estiver ativo" - redesenha a tela
    if (redraw) {
      Render(&renderer);
      redraw = 0;
    }

  }
  al_destroy_timer(timer);
  al_destroy_event_queue(queue);
  ClearRenderer(&renderer);
  return 0;
}