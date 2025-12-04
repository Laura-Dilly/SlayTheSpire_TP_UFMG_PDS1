#include "renderer.h"

#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  // Embaralha inimigos
  NomesInimigos *ni = renderer->combate->listainimigos;
  embaralhaInimigos(ni->nome, ni->imag, ni->totalNomes);
  // Criando grupo de inimigos do nível 1
  renderer->combate->nivel = 1;
  renderer->combate->indiceInimigoAtual = 0;
  renderer->combate->contadorInimigos = 0;
  
  grupoInimigos *gp = geraGrupoInimigos(renderer->combate->listainimigos, &renderer->combate->contadorInimigos);
  renderer->combate->inim_atv = *gp; // Copia a struct para o controle
  free(gp); // Libera o ponteiro temporário, mas mantém os dados copiados na struct

  // Criando o Jogador (FIXO POR ENQUANTO)
  Criatura *opcoes = criaOpcoesJogadores();
  Jogador *j = geraJogador(opcoes, 2); // Escolhe Faustão
  renderer->combate->jog_atv = *j;     // Copia a struct do jogador para a struct de controle geral do combate
  free(j); // Libera ponteiro temporário usado pra criar o jogador

  // Iniciando o estado do jogo
  renderer->combate->estado = "VEZ_JOGADOR";
  renderer->combate->turnoCount = 0;

  // Definindo alvo a selecionar (cartas ou inimigos)
  renderer->combate->selecionandoAlvo = 0;

  // Inicializando por quanto tempo as mensagens definidas aparecerão e o texto e cor padrão delas
  renderer->combate->msg.timer = 0;
  sprintf(renderer->combate->msg.texto, "");
  renderer->combate->msg.cor = al_map_rgb(255, 255, 255);

  // Inicializa o índice da carta selecionada como zero
  renderer->combate->selected_card_idx = 0;

  // Preparando a mão inicial (obs: o baralho já é preparado junto com a criação do jogador)
  iniciaTurnoJogador(&renderer->combate->jog_atv);

}

// Modificada para incluir uma imagem POR NÍVEIS no background
void RenderBackground(Renderer* renderer) {
  al_clear_to_color(al_map_rgb(173,216,230));

}

// Desenhar texto com número do nível do jogo
void RenderLevelInfo(Renderer* renderer) {
    char texto[50];
    sprintf(texto, "NIVEL %02d", renderer->combate->nivel);
    
    // Desenha o texto
    DrawScaledText(renderer->font, al_map_rgb(0, 0, 0), 
                   30, 20, 
                   2.5, 2.5, 
                   ALLEGRO_ALIGN_LEFT, texto);
}


// -----------------------------------------------------------
void RenderDeck(Renderer* renderer, int x_left, int y_top, char *deck_type) {
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

  // Texto de numero de cartas na pilha
  int num_cartas;
  if (strcmp(deck_type,"BARALHO") == 0){
    num_cartas = renderer->combate->jog_atv.pilha->tam;
  } else if (strcmp(deck_type,"DESCARTE") == 0){
    num_cartas = renderer->combate->jog_atv.descarte->tam;
  }

  // Texto de Quantidade de Cartas na Pilha
  if (renderer->combate->jog_atv.pilha != NULL) {
      char qtd[10];
      sprintf(qtd, "%d cartas", num_cartas);
      
      // Texto "BARALHO" (Preto)
        DrawCenteredScaledText(renderer->font, al_map_rgb(0, 0, 0), // COR PRETA
                              (DECK_WIDTH / 3.8),         // X: Centro do deck
                              20,                          // Y: Um pouco abaixo do topo
                               2.0, 2.0, deck_type);

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
  if (percent < 0) percent = 0;   // Visualmente não pode ser negativo

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
  
  // Barra Verde Sobreposta - vida ainda existente
  float green_bar_end = (x_begin - HEALTH_BAR_BACKGROUND_EXTRA) + (total_width * percent);
  al_draw_filled_rounded_rectangle(
          x_begin - HEALTH_BAR_BACKGROUND_EXTRA,
          y_down_left - HEALTH_BAR_BACKGROUND_EXTRA,
          green_bar_end,
          y_down_left - (HEALTH_BAR_HEIGHT + HEALTH_BAR_BACKGROUND_EXTRA),
          HEALTH_BAR_RX, HEALTH_BAR_RY, al_map_rgb(27, 137, 27));
  

  // Texto da vida

  char text[100] = "";
  sprintf(text, "%d/%d", vida, max_vida);
  float x_scale = 2.0, y_scale = 2.0;
  DrawScaledText(font, al_map_rgb(0, 0, 0), (x_begin + x_end) / 2.0 / x_scale,
                 mid_y / y_scale, x_scale, y_scale, ALLEGRO_ALIGN_CENTRE, text);
}

// Desenha escudo
void RenderShield(const Renderer *renderer, Criatura *criatura, int x_begin, int y_begin){
  
  if (criatura->ptsEscudo > 0) {
      // Desenha um ícone de escudo (círculo azul) ao lado da vida
      al_draw_filled_circle(x_begin, y_begin, 25, al_map_rgb(50, 50, 200));   // Preenchimento azul
      al_draw_circle(x_begin, y_begin, 25, al_map_rgb(255, 255, 255), 3);   // Borda Branca

      char txtEscudo[10];
      sprintf(txtEscudo, "%d", criatura->ptsEscudo);
      int escala = 2.0;
      DrawCenteredScaledText(renderer->font, al_map_rgb(255, 255, 255), 
                             x_begin / escala, (y_begin - 10) / escala, escala, escala, txtEscudo);
  }
}



void RenderCreature(const Renderer* renderer, int begin_x, int mid_y,
                    int width, Criatura *criatura, ALLEGRO_COLOR cor, int lado_escudo) {
  
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
      al_draw_circle(center_x, mid_y, raio, cor, 3);  // borda
  }
  
  // Barra de Vida
  float x_bar_begin = begin_x;          // Começa na esquerda do char
  float x_bar_end = begin_x + diametro; // Vai até a direita do char
  float health_bar_y = mid_y + raio + 15; // Logo abaixo dos pés
  RenderHealthBar(x_bar_begin, x_bar_end, health_bar_y, renderer->font, criatura);

  // Escudo
  float escudo_x;  
  if (lado_escudo == 0) {
      // Escudo do lado esquerdo - para o inimigo 1
      escudo_x = x_bar_begin - 30; 
  } else {
      // Escudo do lado esquerdo - para o inimigo 2
      escudo_x = x_bar_end + 30;
  }

  float escudo_y = health_bar_y - 10;
  RenderShield(renderer, criatura, escudo_x, escudo_y);

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
    int y_final = y;
    // Muda altura se for a carta selecionada
    if (i == renderer->combate->selected_card_idx && strcmp(renderer->combate->estado, "VEZ_JOGADOR") == 0) {
        y_final -= 30; // Move para cima
    }
    RenderCard(renderer, &mao->cartas[i], x, y_final);
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
            // Calcula posição x: Primeiro em 1200, Segundo em 1500 etc
            int x_pos = start_x + (i * 300);
            float centro_x = x_pos + radius;

            // Define a cor da borda para inimigo forte
            ALLEGRO_COLOR cor = al_map_rgb(0,0,0);    // preto
            if (inim->tipo == FORTE){
              cor = al_map_rgb(60,24,176);    // roxo
            }

            // Desenha o Inimigo
            RenderCreature(renderer, x_pos, y_pos, radius, inim->enemy, cor, i);
            
            // Desenha o nome do inimigo acima da imagem
            char nome[50];
            sprintf(nome, "%s", inim->enemy->nome);
            int escala = 2.0;
            DrawCenteredScaledText(renderer->font, al_map_rgb(0, 0, 0), 
                                  (x_pos + PLAYER_RADIUS) / escala, (y_pos - PLAYER_RADIUS - 30) / escala, escala, escala, nome);

            // Desenha a próxima ação do inimigo [acima do nome]
            int idxAcao = renderer->combate->turnoCount % inim->qtdAcoes;
            acao proxAcao = inim->acoes[idxAcao];

            char txtAcao[20];
            ALLEGRO_COLOR corTexto;
            ALLEGRO_COLOR corFundo;

            if (proxAcao.tipoAcao == ATAQUE) {
                sprintf(txtAcao, "ATAQUE: %d", proxAcao.efeito);
                corTexto = al_map_rgb(255, 255, 255); // Branco
                corFundo = al_map_rgb(200, 50, 50);   // Vermelho Escuro
            } else {
                sprintf(txtAcao, "DEFESA: %d", proxAcao.efeito);
                corTexto = al_map_rgb(255, 255, 255); // Branco
                corFundo = al_map_rgb(50, 50, 200);   // Azul Escuro
            }

            // Posição: Acima do Nome
            float acao_y = y_pos - radius - 70;
            
            // Fundo (balaozinho) do texto
            al_draw_filled_rounded_rectangle(
                centro_x - 80, acao_y, 
                centro_x + 80, acao_y + 25, 
                5, 5, corFundo
            );
            
            // Desenha o texto da ação
            DrawCenteredScaledText(renderer->font, corTexto, 
                                   centro_x / 2.0, (acao_y + 5) / 2.0, 
                                   2.0, 2.0, txtAcao);


            // Desenha uma mira vermelha em cima do nome no momento de selecionar alvo
            if (renderer->combate->selecionandoAlvo == 1 && renderer->combate->indiceInimigoAtual == i) {
                
                // Cálculos de posição da seta
                float centro_x = x_pos + radius; // Centro horizontal do inimigo
                float ponta_y = y_pos - radius - 100; // Acima da cabeça e do nome
                float largura_seta = 15; // Largura da metade da seta
                float altura_seta = 20;  // Altura da seta

                // Desenha a seta vermelha
                al_draw_filled_triangle(
                    centro_x, ponta_y,                                // Ponto 1: Ponta de baixo
                    centro_x - largura_seta, ponta_y - altura_seta,   // Ponto 2: Canto superior esquerdo
                    centro_x + largura_seta, ponta_y - altura_seta,   // Ponto 3: Canto superior direito
                    al_map_rgb(255, 0, 0)
                );
                
            }

        }
      }
}

void RenderEnergy(Renderer* renderer) {
    int energia = renderer->combate->jog_atv.energia;
    
    // Posição do círculo (acima do deck, na esquerda)
    float centro_x = DRAW_DECK_X + DECK_WIDTH/2;
    float centro_y = DISCARD_DECK_Y - 100; // 100 pixels acima do deck

    // Desenha um círculo amarelo
    al_draw_filled_circle(centro_x, centro_y, 40, al_map_rgb(255, 215, 0)); // Dourado
    al_draw_circle(centro_x, centro_y, 40, al_map_rgb(0, 0, 0), 3); // Borda

    char txt[10];
    sprintf(txt, "%d/3", energia);
    int escala = 2.5;
    // Texto número
    DrawCenteredScaledText(renderer->font, al_map_rgb(0,0,0), centro_x / escala, (centro_y - 10) / escala, escala, escala, txt);
    // Texto "Energia"
    DrawCenteredScaledText(renderer->font, al_map_rgb(0,0,0), centro_x / escala, (centro_y - 65) / escala, escala, escala, "ENERGIA");

}


void RenderMsgTela (Renderer* renderer){
  // Desenha um fundo preto semi-transparente
  al_draw_filled_rectangle(0, 80, DISPLAY_WIDTH, 160, al_map_rgba(0,0,0, 180));

  // Escala (Zoom do texto)
  float scale = 2.0;
  
  ALLEGRO_TRANSFORM t;
  al_identity_transform(&t);
  al_scale_transform(&t, scale, scale);
  al_use_transform(&t); // Aplica o zoom

  // Configurações do Texto:
  // Largura máxima de 90% da tela e Altura é altura da fonte + 2
  float max_width = (DISPLAY_WIDTH * 0.9) / scale; 
  float line_height = al_get_font_line_height(renderer->font) + 4;

  // Desenha o Texto Multilinha - centro superior da tela
  al_draw_multiline_text(
      renderer->font, 
      renderer->combate->msg.cor, 
      (DISPLAY_WIDTH / 2.0) / scale,  // X Centralizado (compensando zoom)
      (100 / scale),                   // Y Um pouco mais pra cima (compensando zoom)
      max_width,                       // Largura máxima antes de quebrar
      line_height,                     // Altura de cada linha
      ALLEGRO_ALIGN_CENTRE,            // Alinhamento Centralizado
      renderer->combate->msg.texto
  );

  // Reseta a transformação para tirar o "zoom" dos próximos desenhos
  al_identity_transform(&t);
  al_use_transform(&t);

 }

// Desenha tela de vitória
void RenderVictory(Renderer* renderer) {
    // Limpa a tela (Fundo Preto)
    al_clear_to_color(al_map_rgb(0, 0, 0));

    // 2. Configura o Texto Gigante
    float escala = 10.0;

    // Desenha "YOU WON!" no centro exato
    DrawCenteredScaledText(renderer->font, 
                           al_map_rgb(50, 255, 50), // Verde Brilhante
                           (DISPLAY_WIDTH / 2.0) / escala, 
                           (DISPLAY_HEIGHT / 2.0) / escala, 
                           escala, escala, 
                           "YOU WON!");
}

// Desenha tela de derrota
void RenderEnd(Renderer* renderer) {
    // Limpa a tela (Fundo Preto)
    al_clear_to_color(al_map_rgb(0, 0, 0));

    // 2. Configura o Texto Gigante
    float escala = 8.0;

    // Desenha "YOU WON!" no centro exato
    DrawCenteredScaledText(renderer->font, 
                           al_map_rgb(50, 255, 50), // Verde Brilhante
                           (DISPLAY_WIDTH / 2.0) / escala, 
                           (DISPLAY_HEIGHT / 2.0) / escala, 
                           escala, escala, 
                           "YOU LOST! ...");

    // Texto Secundário (dividido em 2 chamadas para evitar erro de quebra de linha)
    DrawCenteredScaledText(renderer->font, al_map_rgb(255, 255, 255), 
                           (DISPLAY_WIDTH / 2.0) / 2.0, (DISPLAY_HEIGHT / 2.0) / 2.0, 
                           2.0, 2.0, "No amount of autotune");
    
    DrawCenteredScaledText(renderer->font, al_map_rgb(255, 255, 255), 
                           (DISPLAY_WIDTH / 2.0) / 2.0, ((DISPLAY_HEIGHT / 2.0) + 50) / 2.0, 
                           2.0, 2.0, "can save you now...");
}



void Render(Renderer* renderer) {
  al_set_target_bitmap(renderer->display_buffer);
  if (strcmp(renderer->combate->estado, "GAMEOVER") == 0) {
      RenderEnd(renderer); // Tela de derrota
  } 
  else if (strcmp(renderer->combate->estado, "VITORIA") == 0) {
      RenderVictory(renderer); // Chama a tela de vitória
  } 
  else {
    RenderBackground(renderer);
    RenderLevelInfo(renderer);
    RenderDeck(renderer, DRAW_DECK_X, DRAW_DECK_Y, "BARALHO");
    RenderDeck(renderer, DISCARD_DECK_X, DISCARD_DECK_Y, "DESCARTE");
    RenderCreature(renderer, PLAYER_BEGIN_X, PLAYER_BEGIN_Y + PLAYER_RADIUS,
                  PLAYER_RADIUS, &renderer->combate->jog_atv.player, al_map_rgb(0,0,0), 1);
    RenderEnergy(renderer);
    RenderEnemies(renderer);
    RenderPlayerHand(renderer);
    if (renderer->combate->msg.timer > 0) {
      RenderMsgTela(renderer);
    }
    al_set_target_backbuffer(renderer->display);

    al_draw_scaled_bitmap(renderer->display_buffer, 0, 0, DISPLAY_BUFFER_WIDTH,
                          DISPLAY_BUFFER_HEIGHT, 0, 0, DISPLAY_WIDTH,
                          DISPLAY_HEIGHT, 0);

    al_flip_display();
  }
}

void ClearRenderer(Renderer* renderer) {
  al_destroy_display(renderer->display);
  al_destroy_bitmap(renderer->display_buffer);
  al_destroy_font(renderer->font);
}