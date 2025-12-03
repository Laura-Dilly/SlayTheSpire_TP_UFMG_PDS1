#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>

#include "deck_logic.h"
#include "creatures_logic.h"
#include "constants.h"
#include "utils.h"

typedef struct {
  ALLEGRO_DISPLAY* display;
  ALLEGRO_BITMAP* display_buffer;
  ALLEGRO_FONT* font;
  controleCombate* combate;

} Renderer;


void FillRenderer(Renderer* renderer);

void Render(Renderer* renderer);

void ClearRenderer(Renderer* renderer);

void RenderCard(const Renderer* renderer, const Carta *carta, int x_left, int y_top);

#endif