#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "constants.h"
// Estamos incluindo os .c aqui apenas para facilitar a compilação neste exemplo 
// (Em um projeto real, você incluiria os .h e compilaria os objetos separadamente)
// Se você for compilar via terminal listando os arquivos, troque por #include "deck_logic.h" e "creatures_logic.h"
#include "deck_logic.h" 
#include "creatures_logic.h" 


// --- FUNÇÕES AUXILIARES DE TEXTO ---

const char* getTipoString(CartaTipo tipo) {
    switch (tipo) {
        case ATAQUE: return "ATAQUE";
        case DEFESA: return "DEFESA";
        case ESPECIAL: return "ESPECIAL";
        default: return "UNK";
    }
}

const char* getInimigoTipoString(inimigoTipo tipo) {
    return (tipo == FORTE) ? "FORTE" : "FRACO";
}

const char* getAcaoTipoString(CartaTipo tipo) {
    return (tipo == ATAQUE) ? "ATACAR" : "DEFENDER";
}


int main() {
    // 1. Inicializações Obrigatórias
    srand(time(NULL));

    if (!al_init()) {
        printf("Falha ao inicializar Allegro.\n");
        return 1;
    }
    if (!al_init_image_addon()) {
        printf("Falha ao inicializar add-on de imagem.\n");
        return 1;
    }

    printf("=== SISTEMA DE TESTE: CRIATURAS E BARALHO ===\n\n");

    // ---------------------------------------------------------
    // ETAPA 1: ESCOLHA DO JOGADOR
    // ---------------------------------------------------------
    printf("--- SELECAO DE PERSONAGEM ---\n");
    Criatura *opcoes = criaOpcoesJogadores();

    for (int i = 0; i < 3; i++) {
        printf("[%d] %s (HP: %d)\n", i, opcoes[i].nome, opcoes[i].ptsVida);
    }

    int escolha = -1;
    while (escolha < 0 || escolha > 2) {
        printf("\nEscolha seu heroi (0-2): ");
        if (scanf("%d", &escolha) != 1) {
            while(getchar() != '\n'); // Limpa buffer se digitar letra
            escolha = -1;
        }
    }

    // ---------------------------------------------------------
    // ETAPA 2: GERAÇÃO DO JOGADOR
    // ---------------------------------------------------------
    printf("\n--- GERANDO JOGADOR ---\n");
    // geraJogador já cuida de liberar as opções não escolhidas
    Jogador *player = geraJogador(opcoes, escolha);

    printf("Jogador Criado: %s\n", player->player.nome);
    printf("Vida: %d/%d | Energia: %d/%d\n", 
            player->player.ptsVida, player->player.vidaMax,
            player->energia, player->energiaMax);
    
    printf("Baralho de Compras: %d cartas.\n", player->pilha->tam);
    
    // Mostra algumas cartas do baralho do jogador para provar que deck_logic funcionou
    printf("Topo do baralho do jogador:\n");
    for(int i=0; i < 5 && i < player->pilha->tam; i++){
         printf("  - %s (%s) Custo: %d\n", 
                player->pilha->cartas[i].nome, 
                getTipoString(player->pilha->cartas[i].tipo),
                player->pilha->cartas[i].custo);
    }

    // ---------------------------------------------------------
    // ETAPA 3: GERAÇÃO DE INIMIGOS (SETUP)
    // ---------------------------------------------------------
    printf("\n--- PREPARANDO INIMIGOS ---\n");
    NomesInimigos *poolInimigos = geraNomesInimigos();
    
    // Embaralha para garantir aleatoriedade
    embaralhaInimigos(poolInimigos->nome, poolInimigos->imag, poolInimigos->totalNomes);
    
    int indiceGlobalInimigos = 0;
    printf("Pool de nomes carregado (%d nomes).\n", poolInimigos->totalNomes);

    // ---------------------------------------------------------
    // ETAPA 4: GERAÇÃO DE GRUPO DE COMBATE (TURNO 1)
    // ---------------------------------------------------------
    printf("\n--- GERANDO GRUPO DE COMBATE (TURNO 1) ---\n");
    
    grupoInimigos *grupoAtual = geraGrupoInimigos(poolInimigos, &indiceGlobalInimigos);

    for (int i = 0; i < grupoAtual->qtd; i++) {
        Inimigo *ini = &grupoAtual->inimigos[i]; // Acessa via endereço para facilitar leitura
        
        printf("\n[INIMIGO %d]\n", i+1);
        printf("Nome: %s\n", ini->enemy->nome);
        printf("Tipo: %s\n", getInimigoTipoString(ini->tipo));
        printf("Vida: %d/%d | Escudo: %d\n", ini->enemy->ptsVida, ini->enemy->vidaMax, ini->enemy->ptsEscudo);
        printf("Acoes Disponiveis (%d):\n", ini->qtdAcoes);

        // Lista as ações deste inimigo
        for(int j = 0; j < ini->qtdAcoes; j++) {
            printf("   > %s | Custo: %d | Efeito: %d\n", 
                   getAcaoTipoString(ini->acoes[j].tipoAcao),
                   ini->acoes[j].custo,
                   ini->acoes[j].efeito);
        }
    }

    // ---------------------------------------------------------
    // ETAPA 5: LIMPEZA DE MEMÓRIA
    // ---------------------------------------------------------
    printf("\n--- LIMPANDO MEMORIA ---\n");
    
    liberaJogador(player);
    printf("Jogador liberado.\n");

    liberaGrupoInimigos(grupoAtual);
    printf("Grupo de inimigos liberado.\n");

    liberaNomesInimigos(poolInimigos);
    printf("Pool de nomes liberado.\n");

    printf("\nTeste concluido com sucesso.\n");
    
    // Segura a tela se estiver no Windows
    // system("pause"); 
    
    return 0;
}