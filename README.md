# Slay the Spire - TP UFMG PDS1

## Descrição
Este projeto é uma implementação simplificada do jogo Slay the Spire, desenvolvida em linguagem C utilizando a biblioteca gráfica Allegro 5.

## Sobre o Jogo
Trata-se de um jogo de cartas e estratégia por turnos, onde o jogador deve enfrentar 10 níveis de combate consecutivos, gerenciando sua energia, pontos de vida e baralho de cartas.

## Estrutura do Projeto
```
SlayTheSpire_TP_UFMG_PDS1/
├── main.c
├── constants.h
├── renderer.c / renderer.h
├── utils.c / utils.h
├── deck_logic.c / deck_logic.h
├── creatures_logic.c / creatures_logic.h
├── Makefile
├── game.out (gerado após compilação)
├── data/
│   ├── card_names.txt
│   ├── special_cards.txt
│   ├── enemy_names.txt
│   └── enemy_imgs.txt
└── img/
    ├── players/ (anitta.png, faustao.png, shakira.png)
    └── (demais imagens de inimigos listadas em enemy_imgs.txt)
```

## Requisitos
Para compilar e executar o jogo, é necessário ter instalado:
- Compilador GCC.
- Biblioteca Allegro 5 e seus addons

## Como Compilar
Linux/MAC:
```bash
make game
```
Windows:
```bash
make game-win
```

## Como Executar
```bash
./game.exe
```

## Manual do Jogador

### Objetivo
Derrotar todos os inimigos em 10 níveis de combate consecutivos. Se os pontos de vida do jogador chegarem a 0, o jogo termina.

### Interface
- Barra de Vida: Mostra vida atual/máximo.
- Energia: Círculo amarelo no canto esquerdo. O jogador possui 3 energias por turno.
- Mão: Cartas disponíveis para jogar no turno atual (centro inferior).
- Pilhas: Pilha de Compras e Descarte (esquerda).
- Inimigos: Situados à direita. Um balão acima deles indica a próxima ação (Ataque ou Defesa) e o valor do efeito.

### Cartas
- Ataque (Vermelha): Causa dano a um inimigo selecionado.
- Defesa (Azul): Adiciona escudo ao jogador (reseta no início do próximo turno).
- Especial (Amarela): Custo 0. Descarta a mão atual e compra 5 novas cartas.

### Controles
- Setas (← / →): Navegar entre as cartas da mão ou selecionar o inimigo alvo.
- ENTER: Confirmar uso da carta.
- ESC: Encerrar o turno do jogador (passa a vez para os inimigos).
- Q: Sair do jogo.
- ESPAÇO: Mata todos os inimigos instantaneamente.
- X: Reduz a vida do jogador para 1.
- C: Aplica dano aleatório (1-3) a um inimigo aleatório.

## Licença
Projeto acadêmico UFMG