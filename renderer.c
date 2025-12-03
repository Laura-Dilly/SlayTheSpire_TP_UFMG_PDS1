#include "renderer.h"

#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "utils.h"
#include "deck_logic.h"
#include "creatures_logic.h"

void DrawScaledText(ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x, float y,
                    float xscale, float yscale, int alignment,
                    const char* text) {
  ALLEGRO_TRANSFORM transform;
  al_identity_transform(&transform);  // Start with an identity transform
  al_scale_transform(
      &transform, xscale,
      yscale);  // Apply scaling (e.g., sx=2.0, sy=2.0 for double size)
  al_use_transform(&transform);  // Use the transform for subsequent drawing

  al_draw_text(font, color, x, y, alignment, text);
  // al_draw_text(font, color, x, y, alignment, text);
  al_identity_transform(&transform);
  al_use_transform(&transform);  // Use the transform for subsequent drawing
}

void DrawCenteredScaledText(ALLEGRO_FONT* font, ALLEGRO_COLOR color, float x,
                            float y, float xscale, float yscale, char* text) {
  DrawScaledText(font, color, x, y, xscale, yscale, ALLEGRO_ALIGN_CENTRE, text);
}


// Preenche a struct renderer
void FillRenderer(Renderer* renderer) {
  al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
  al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
  al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

  renderer->display = al_create_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);
  must_init(renderer->display, "display");

  renderer->display_buffer =
      al_create_bitmap(DISPLAY_BUFFER_WIDTH, DISPLAY_BUFFER_HEIGHT);
  must_init(renderer->display_buffer, "display buffer");

  renderer->font = al_create_builtin_font();
  must_init(renderer->font, "font");

  // Inicialização do combate
  renderer->combate = malloc(sizeof(controleCombate));
  must_init(renderer->combate != NULL, "control structure (controleCombate)");

  // Carregando os inimigos
  renderer->combate->listainimigos = geraNomesInimigos();
  // Criando grupo de inimigos do nível 1
  renderer->combate->nivel = 1;
  renderer->combate->indiceInimigoAtual = 0;
  int idxGlobal = 0; // Controle do índice do inimigo
  grupoInimigos *gp = geraGrupoInimigos(renderer->combate->listainimigos, &idxGlobal);
  renderer->combate->inim_atv = *gp; // Copia a struct para o controle
  free(gp); // Libera o ponteiro temporário, mas mantém os dados copiados na struct

  // Criando o Jogador (FIXO POR ENQUANTO)
  Criatura *opcoes = criaOpcoesJogadores();
  Jogador *j = geraJogador(opcoes, 0); // Escolhe Shakira
  renderer->combate->jog_atv = *j;     // Copia a struct do jogador para a struct de controle geral do combate
  free(j); // Libera ponteiro temporário usado pra criar o jogador

  // Iniciando o estado do jogo
  renderer->combate->estado = "VEZ_JOGADOR";
  renderer->combate->turnoCount = 0;

  // Preparando a mão inicial (obs: o baralho já é preparado junto com a criação do jogador)
  iniciaTurnoJogador(&renderer->combate->jog_atv);

}

// Modificada para incluir uma imagem POR NÍVEIS no background
void RenderBackground(Renderer* renderer) {
  al_clear_to_color(al_map_rgb(173,216,230));
}


// -----------------------------------------------------------
void RenderDeck(Renderer* renderer, int x_left, int y_top) {
  // Salva o alvo atual [tela atual]
  ALLEGRO_BITMAP* prev_bmp_target = al_get_target_bitmap();

  // Cria uma tela temporária para desenho e muda o alvo para ela
  ALLEGRO_BITMAP* deck_bitmap = al_create_bitmap(DECK_WIDTH, DECK_HEIGHT);
  al_set_target_bitmap(deck_bitmap);

  // Desenhos na tela: retangulo branco de cantos arredondados do tamanho do deck
  al_draw_filled_rounded_rectangle(0,0, DECK_WIDTH, DECK_HEIGHT, 5, 5,
                                   al_map_rgb(255, 255, 255));

  al_draw_rounded_rectangle(0,0, DECK_WIDTH, DECK_HEIGHT, 5, 5,
                                   al_map_rgb(0, 0, 0), 3);

  // Texto de Quantidade de Cartas na Pilha
  if (renderer->combate->jog_atv.pilha != NULL) {
      char qtd[10];
      sprintf(qtd, "%d cartas", renderer->combate->jog_atv.pilha->tam);
      
      // Texto "BARALHO" (Preto)
        DrawCenteredScaledText(renderer->font, al_map_rgb(0, 0, 0), // COR PRETA
                              (DECK_WIDTH / 3.8),         // X: Centro do deck
                              20,                          // Y: Um pouco abaixo do topo
                               2.0, 2.0, "BARALHO");

        // Número da quantidade (Preto e Grande)
        DrawCenteredScaledText(renderer->font, al_map_rgb(0, 0, 0), // COR PRETA
                               (DECK_WIDTH / 3.5),         // X: Centro do deck
                               50,         // Y: Meio do deck
                               1.8, 1.8, qtd);
  }

  // Restaura o alvo
  al_set_target_bitmap(prev_bmp_target);
  
  // Desenha no alvo a imagem do deck_bitmap
  al_draw_scaled_bitmap(deck_bitmap, 0, 0, DECK_WIDTH, DECK_HEIGHT, x_left,
                        y_top, DECK_WIDTH, DECK_HEIGHT, 0);
  
  // Limpa memória
  al_destroy_bitmap(deck_bitmap);
}


// ----------------------------------------------
void RenderHealthBar(float x_begin, float x_end, float y_down_left,
                     ALLEGRO_FONT* font, Criatura *criatura) {
  
  float mid_y = y_down_left - (HEALTH_BAR_HEIGHT * 0.78);

  //Desenhar vida
  int vida = criatura->ptsVida;
  int max_vida = criatura->vidaMax;
  float percent = (float) vida/ (float) max_vida;
  float total_width = x_end - x_begin;
  float vida_width = total_width * percent;    // cálculo da largura da barra de pts de vida

  // Barra vida total
  al_draw_filled_rounded_rectangle(
          x_begin - HEALTH_BAR_BACKGROUND_EXTRA,
          y_down_left - HEALTH_BAR_BACKGROUND_EXTRA,
          x_end - HEALTH_BAR_BACKGROUND_EXTRA,
          y_down_left - HEALTH_BAR_HEIGHT + HEALTH_BAR_BACKGROUND_EXTRA,
          HEALTH_BAR_RX, HEALTH_BAR_RY, al_map_rgb(137, 27, 27));
  
  // Desenhar Borda Preta - Barra vida Total
  al_draw_rounded_rectangle(
      x_begin - HEALTH_BAR_BACKGROUND_EXTRA,
      y_down_left - HEALTH_BAR_BACKGROUND_EXTRA,
      x_end - HEALTH_BAR_BACKGROUND_EXTRA,
      y_down_left - HEALTH_BAR_HEIGHT + HEALTH_BAR_BACKGROUND_EXTRA,
      HEALTH_BAR_RX, HEALTH_BAR_RY, al_map_rgb(0, 0, 0), 3);
  
  // Barra vida ainda existente
  al_draw_filled_rounded_rectangle(
          x_begin - HEALTH_BAR_BACKGROUND_EXTRA,
          y_down_left - HEALTH_BAR_BACKGROUND_EXTRA,
          x_end - HEALTH_BAR_BACKGROUND_EXTRA,
          y_down_left - (HEALTH_BAR_HEIGHT + HEALTH_BAR_BACKGROUND_EXTRA) * percent,
          HEALTH_BAR_RX, HEALTH_BAR_RY, al_map_rgb(27, 137, 27));
  

  // Texto da vida

  char text[100] = "";
  sprintf(text, "%d/%d", vida, max_vida);
  float x_scale = 2.0, y_scale = 2.0;
  DrawScaledText(font, al_map_rgb(0, 0, 0), (x_begin + x_end) / 2.0 / x_scale,
                 mid_y / y_scale, x_scale, y_scale, ALLEGRO_ALIGN_CENTRE, text);
}



void RenderCreature(const Renderer* renderer, int begin_x, int mid_y,
                    int width, Criatura *criatura) {
  
  // Variáveis auxiliares para tamanho da criatura
  int raio = width;
  int diametro = raio * 2;
  float center_x = begin_x + raio;    // Coordenada X do CENTRO da criatura

  // Desenho da imagem
  if (criatura->img != NULL) {
      int bw = al_get_bitmap_width(criatura->img);    // Pega largura original
      int bh = al_get_bitmap_height(criatura->img);   // Pega altura original
      
      // Desenha com escala ajustada para caber exatamente no quadrado do diâmetro
      al_draw_scaled_bitmap(criatura->img,
                            0, 0, bw, bh,              // Origem
                            begin_x, mid_y - raio,     // Destino X, Y (canto superior esquerdo)
                            diametro, diametro,        // Largura/Altura final
                            0);
  } else {
      // Círculo (Caso não tenha ou não encontre imagem)
      al_draw_filled_circle(center_x, mid_y, raio, al_map_rgb(255, 255, 255));
      al_draw_circle(center_x, mid_y, raio, al_map_rgb(0,0,0), 3);
  }
  
  // Barra de Vida
  float x_bar_begin = begin_x;          // Começa na esquerda do char
  float x_bar_end = begin_x + diametro; // Vai até a direita do char
  float health_bar_y = mid_y + raio + 15; // Logo abaixo dos pés
  
  RenderHealthBar(x_bar_begin, x_bar_end, health_bar_y, 
                  renderer->font, criatura);
}

void RenderCard(const Renderer* renderer, const Carta *carta, int x_left, int y_top) {
  // Cria o bitmap temporário da carta
  ALLEGRO_BITMAP* card_bitmap = al_create_bitmap(CARD_WIDTH, CARD_HEIGHT);
  al_set_target_bitmap(card_bitmap);

  // Define as cores de fundo e borda da carta
  ALLEGRO_COLOR corFundo = al_map_rgb(255, 255, 255);  // Branco - cor padrão
  ALLEGRO_COLOR corBorda = al_map_rgb(0, 0, 0); // Preto - cor padrão
  if (carta->tipo == ATAQUE) corFundo = al_map_rgb(255, 200, 200); // Avermelhado
  if (carta->tipo == DEFESA) corFundo = al_map_rgb(200, 200, 255); // Azulado
  if (carta->tipo == ESPECIAL){
    corFundo = al_map_rgb(255, 255, 200); // Amarelado
    corBorda = al_map_rgb(255, 215, 0); // Borda dourada
  }

  // Desenha o retângulo colorido da carta em sua devida posição
  al_draw_filled_rounded_rectangle(0, 0, CARD_WIDTH, CARD_HEIGHT, 5, 5, corFundo);
  // Desenha a borda
  al_draw_rounded_rectangle(0, 0, CARD_WIDTH, CARD_HEIGHT, 5, 5, corBorda, 2);
  
  // Variáveis auxiliares para texto
  ALLEGRO_COLOR corTexto = al_map_rgb(0, 0, 0);  // Preto - cor padrão
  char text[100];
  float xscale = 1.5, yscale = 1.5;

  // Desenha o texto de:
  // NOME da carta
  // Define a largura máxima do texto (largura da carta - margens)
  float max_width = (CARD_WIDTH - 20) / xscale; 
  
  // Altura da linha (baseada na fonte)
  int line_height = al_get_font_line_height(renderer->font);

  // TRANSFORMAÇÃO PARA ESCALA
  ALLEGRO_TRANSFORM t;
  al_identity_transform(&t);
  al_scale_transform(&t, xscale, yscale);
  al_use_transform(&t);

  al_draw_multiline_text(
      renderer->font, 
      corTexto, 
      (CARD_WIDTH / 2.0) / xscale, // X Centralizado
      20 / yscale,                 // Y Topo
      max_width,                       // Largura Máxima antes de quebrar
      line_height,                     // Altura da linha
      ALLEGRO_ALIGN_CENTRE, 
      carta->nome
  );

  // Reseta transformação
  al_identity_transform(&t);
  al_use_transform(&t);


  // CUSTO da carta
  sprintf(text, "Custo: %d", carta->custo);
  DrawScaledText(renderer->font, corTexto, (CARD_WIDTH * 0.1) / xscale,
                 (CARD_HEIGHT * 0.4) / yscale, xscale, yscale,
                 ALLEGRO_ALIGN_LEFT, text);

  // EFEITO da carta
  if (carta->tipo == ATAQUE) {
      sprintf(text, "Dano: -%d", carta->efeito);
  } else if (carta->tipo == DEFESA) {
      sprintf(text, "+%d - escudo", carta->efeito);
  } else {
      sprintf(text, "Dano: -%d", carta->efeito);
  }
  DrawScaledText(renderer->font, corTexto, (CARD_WIDTH * 0.1) / xscale,
                 (CARD_HEIGHT * 0.55) / yscale, xscale, yscale,
                 ALLEGRO_ALIGN_LEFT, text);

  // TIPO da carta (no rodapé)
  char *textoTipo;
  if(carta->tipo == ATAQUE) textoTipo = "ATAQUE";
  else if(carta->tipo == DEFESA) textoTipo = "DEFESA";
  else textoTipo = "ESPECIAL";
  DrawScaledText(renderer->font, corTexto, (CARD_WIDTH * 0.1) / xscale,
                 (CARD_HEIGHT * 0.7) / yscale, xscale, yscale,
                 ALLEGRO_ALIGN_LEFT, textoTipo);



  // Finalização:
  // Volta a desenhar na tela principal (buffer)
  al_set_target_bitmap(renderer->display_buffer);
  // Desenha o bitmap da carta na posição correta (x_left, y_top)
  al_draw_scaled_bitmap(card_bitmap, 0, 0, CARD_WIDTH, CARD_HEIGHT, x_left,
                        y_top, CARD_WIDTH, CARD_HEIGHT, 0);
  // Libera espaço (do bitmap temporário)
  al_destroy_bitmap(card_bitmap);
}


// 
void RenderPlayerHand(Renderer* renderer) {
  // Separando o grupo de cartas que compõe a mão do jogador
  grupoCarta *mao = renderer->combate->jog_atv.mao;
  if (mao->tam == 0) return;

  // Definições de espaço (em caso da mão ser muito grande)
  int largura_tela_util = DISPLAY_WIDTH - 2 * HAND_BEGIN_X; // Deixa 2x o tamanho de HAND_BEGIN_X como margem
  int start_x = HAND_BEGIN_X;
  int y = HAND_BEGIN_Y;
  // Lógica de Sobreposição
  int passo_x = CARD_WIDTH + 15; // Distância padrão entre cartas

  // Se o total de cartas ultrapassa a tela útil, então 
  int largura_total_necessaria = (mao->tam * passo_x);
  if (largura_total_necessaria > largura_tela_util) {
      // Cálculo: (Espaço Disponivel - Uma Carta) / (Resto das cartas)
      passo_x = (largura_tela_util - CARD_WIDTH) / (mao->tam - 1);
  }

  // Criando um loop que desenha cada carta da mão
  for (int i = 0; i < mao->tam; i++){
    int x = start_x + (i * passo_x);
    RenderCard(renderer, &mao->cartas[i], x, y);
  }

}

void RenderEnemies(Renderer* renderer) {
    grupoInimigos *gp = &renderer->combate->inim_atv;
    
    // Configuração para o lado DIREITO da tela
    int start_x = 1200; 
    int y_pos = PLAYER_BEGIN_Y + PLAYER_RADIUS; // Mesma base do player
    int radius = PLAYER_RADIUS;

    for(int i = 0; i < gp->qtd; i++) {
        Inimigo *inim = &gp->inimigos[i];
        
        // Só desenha se estiver vivo
        if(inim->enemy->ptsVida > 0) {
            // Calcula posição: Primeiro em 1200, Segundo em 1500...
            int x_pos = start_x + (i * 300);

            // Desenha o Inimigo
            RenderCreature(renderer, x_pos, y_pos, radius, inim->enemy);
        }
      }
}

void RenderEnergy(Renderer* renderer) {
    // Texto de Energia
    char txtEnergia[20];
    sprintf(txtEnergia, "Energia: %d / 3", renderer->combate->jog_atv.energia);
    DrawScaledText(renderer->font, al_map_rgb(0,0,0), 50, 950, 2.5, 2.5, ALLEGRO_ALIGN_LEFT, txtEnergia);

    // Botão de Passar Turno (Canto Inferior Direito)
    // Coordenadas: X=1600 a 1850, Y=920 a 1000
    al_draw_filled_rounded_rectangle(1600, 20, 1850, 100, 10, 10, al_map_rgb(255, 255, 255)); // Fundo branco
    al_draw_rounded_rectangle(1600, 20, 1850, 100, 10, 10, al_map_rgb(0,0,0), 2); // Borda preta
    
    DrawCenteredScaledText(renderer->font, al_map_rgb(255,255,255), 
                           1725, 945, 2.0, 2.0, "ENCERRAR TURNO");
}

void Render(Renderer* renderer) {
  al_set_target_bitmap(renderer->display_buffer);
  RenderBackground(renderer);
  RenderDeck(renderer, DRAW_DECK_X, DRAW_DECK_Y);
  RenderCreature(renderer, PLAYER_BEGIN_X, PLAYER_BEGIN_Y + PLAYER_RADIUS,
                 PLAYER_RADIUS, &renderer->combate->jog_atv.player);
  RenderEnergy(renderer);
  RenderEnemies(renderer);
  RenderPlayerHand(renderer);
  al_set_target_backbuffer(renderer->display);

  al_draw_scaled_bitmap(renderer->display_buffer, 0, 0, DISPLAY_BUFFER_WIDTH,
                        DISPLAY_BUFFER_HEIGHT, 0, 0, DISPLAY_WIDTH,
                        DISPLAY_HEIGHT, 0);

  al_flip_display();
}

void ClearRenderer(Renderer* renderer) {
  al_destroy_display(renderer->display);
  al_destroy_bitmap(renderer->display_buffer);
  al_destroy_font(renderer->font);
}