#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "renderer.h"
#include "utils.h"
#include "deck_logic.h"
#include <string.h>

// Retorna um array com 3 criaturas preenchidas - serão as opções iniciais de escolha do jogador
Criatura* criaOpcoesJogadores() {
    
    Criatura *opcoes = (Criatura *) malloc(3 * sizeof(Criatura));
    if (opcoes == NULL) {
        printf("Falha ao alocar memoria para opcoes de jogadores.\n");
        exit(EXIT_FAILURE);
    }

    // --- OPÇÃO 1: SHAKIRA ---
    opcoes[0].nome = strdup("Shakira");
    opcoes[0].vidaMax = 100;
    opcoes[0].ptsVida = 100;
    opcoes[0].ptsEscudo = 0;
    opcoes[0].img = al_load_bitmap("img/players/shakira.png"); 

    // --- OPÇÃO 2: ANITTA ---
    opcoes[1].nome = strdup("Anitta");
    opcoes[1].vidaMax = 100;
    opcoes[1].ptsVida = 100;
    opcoes[1].ptsEscudo = 0;
    opcoes[1].img = al_load_bitmap("img/players/anitta.png");

    // --- OPÇÃO 3: FAUSTÃO ---
    opcoes[2].nome = strdup("Faustão");
    opcoes[2].vidaMax = 100;
    opcoes[2].ptsVida = 100;
    opcoes[2].ptsEscudo = 0;
    opcoes[2].img = al_load_bitmap("img/players/faustao.png");

    // Checar se as imagens carregaram
    for(int i = 0; i < 3; i++) {
        if(opcoes[i].img == NULL) {
            printf("AVISO: Nao foi possivel carregar a imagem de (%s).\n", opcoes[i].nome);
        }
    }

    return opcoes;
}

// Preenche o jogador de acordo com a seleção
Jogador* geraJogador(Criatura *opcoes, int escolha) {
    
    if (escolha < 0 || escolha > 2) {
        printf("Erro: Escolha de jogador invalida.\n");
        exit(EXIT_FAILURE);
    }

    Jogador *jog = (Jogador *) malloc(sizeof(Jogador));
    if (jog == NULL) {
        exit(EXIT_FAILURE);
    }
    
    jog->player = opcoes[escolha];
    jog->energia = 3; 
    jog->energiaMax = 3;
    jog->pilha = geraBaralhoInicial(); // Baralho de compras cheio
    embaralhaCartas(jog->pilha);
    jog->mao = criaPilhaVazia();     // Mão vazia
    jog->descarte = criaPilhaVazia(); // Descarte vazio
    jog->vitorias = 0;

    // Libera memória das opções não escolhidas
    for (int i = 0; i < 3; i++) {
        if (i != escolha) {
            if (opcoes[i].img != NULL) {
                al_destroy_bitmap(opcoes[i].img);
            }
            if (opcoes[i].nome != NULL) {
                free(opcoes[i].nome);
            }
        }
    }
    free(opcoes);

    return jog;
}

//Liberar jogador [ao fechar o jogo]
void liberaJogador(Jogador *j) {
    if (j == NULL) return;

    // Libera a Criatura (Imagem e Nome)
    if (j->player.img != NULL) {
        al_destroy_bitmap(j->player.img);
    }
    if (j->player.nome != NULL) {
        free(j->player.nome);
    }

    // Libera os Baralhos do jogador
    liberaBaralho(j->pilha);
    liberaBaralho(j->mao);
    liberaBaralho(j->descarte);

    // Libera a struct do jogador
    free(j);
}

// Cria struct de nomes dos inimigos com base em arquivos txt
NomesInimigos *geraNomesInimigos(void) {
    
    FILE *nomes = fopen("data/enemy_names.txt", "r");
    FILE *imgs = fopen("data/enemy_imgs.txt", "r");

    if (nomes == NULL || imgs == NULL) {
        printf("Erro FATAL: Nao foi possivel abrir arquivos de inimigos (names/imgs).\n");
        if (nomes) fclose(nomes);
        if (imgs) fclose(imgs);
        exit(EXIT_FAILURE);
    }

    NomesInimigos *ni = (NomesInimigos *) malloc(sizeof(NomesInimigos));

    // LEITURA DOS NOMES
    ni->nome = malloc(17 * sizeof(char*));
    char linha[256];
    int idx = 0;
    while(fgets(linha, sizeof(linha), nomes) != NULL && idx < 17){
        linha[strcspn(linha, "\n")] = '\0';
        ni->nome[idx] = malloc(strlen(linha) + 1);
        strcpy(ni->nome[idx], linha);
        idx++;
    }
    ni->totalNomes = idx;

    // LEITURA DOS NOMES DE ARQUIVOS DAS IMAGENS
    ni->imag = malloc(17 * sizeof(char*));    
    int idx_img = 0;
    while(fgets(linha, sizeof(linha), imgs) != NULL && idx_img < 17){
        linha[strcspn(linha, "\n")] = '\0';
        linha[strcspn(linha, "\r")] = '\0';  
        
        ni->imag[idx_img] = malloc(strlen(linha) + 1);
        strcpy(ni->imag[idx_img], linha);
        idx_img++;
    }


    fclose(nomes);
    fclose(imgs);

    return ni;
}


// Libera struct de nomes e nomes de imagens de inimigos
void liberaNomesInimigos(NomesInimigos *ni) {
    if (ni == NULL) return;

    // Libera nomes
    if (ni->nome) {
        for (int i = 0; i < ni->totalNomes; i++) {
            free(ni->nome[i]);
        }
        free(ni->nome);
    }

    // Libera nomes de imagens
    if (ni->imag) {
        for (int i = 0; i < ni->totalNomes; i++) {
            free(ni->imag[i]);
        }
        free(ni->imag);
    }
    free(ni);
}

// Embaralha nome (e imagens) de inimigos para escolha aleatória
void embaralhaInimigos(char **inimigos, char **imagens, int tamanho) {
    for (int i = 0; i < tamanho; i++) {
        // Sorteia uma posição aleatória
        int r = rand() % tamanho;

        // Troca o nome da posição atual (i) com a sorteada (r)
        char *temp = inimigos[i];
        inimigos[i] = inimigos[r];
        inimigos[r] = temp;

        // Troca o nome do arquivo da posição atual (i) com a sorteada (r)
        char *temp_img = imagens[i];
        imagens[i] = imagens[r];
        imagens[r] = temp_img;
    }
}

// Cria um inimigo
Inimigo *geraInimigo (inimigoTipo type, char *name, char *imag){
    // lê o tipo do inimigo, o nome do inimigo e o nome do arquivo de imagem

    Inimigo *en = (Inimigo *) malloc(sizeof(Inimigo));
    if (en == NULL){
        printf("Erro ao criar estrutura do inimigo.");
        exit(EXIT_FAILURE);
    }

    // Aloca espaço para a struct Criatura do inimigo
    en->enemy = (Criatura *) malloc(sizeof(Criatura));
    if (en->enemy == NULL) {
        printf("Erro ao criar struct de criatura do inimigo.");
        free(en);
        exit(EXIT_FAILURE);
    }

    //Gera Nome do inimigo
    en->enemy->nome = (char *) malloc(strlen(name) + 1);
    if (en->enemy->nome == NULL) {
        printf("Erro ao criar nome do inimigo.");
        free(en);
        exit(EXIT_FAILURE);
    }
    strcpy(en->enemy->nome, name);

    
    // Gera imagem do inimigo
    en->enemy->img = al_load_bitmap(imag);
    if(en->enemy->img == NULL) {
        printf("AVISO: Nao foi possivel carregar a imagem do inimigo (%s).\n", en->enemy->nome);
    }

    // Gera tipo do Inimigo
    en->tipo = type;

    // Gera pts de Vida do inimigo
    if (en->tipo == FRACO){
        en->enemy->ptsVida = (rand() % 21 + 10);
        en->qtdAcoes = (rand() % 2 + 1);
    } else {
        en->enemy->ptsVida = (rand() % 61 + 40);
        en->qtdAcoes = (rand() % 2 + 2);
    }

    en->enemy->vidaMax = en->enemy->ptsVida;
    en->enemy->ptsEscudo = 0;


    // Gera rol de ações do inimigo
    int ataques = (rand() % (en->qtdAcoes) + 1);
    int defesas = en->qtdAcoes - ataques;

    // Aloca espaço para a struct acoes do inimigo
    en->acoes = (acao *) malloc(en->qtdAcoes * sizeof(acao));
    if (en->acoes == NULL) {
        printf("Erro ao criar ações do inimigo.");
        exit(EXIT_FAILURE);
    }

    // Cria as ações (primeiro as de ataque e depois as de defesa)
    int contadorCustoUm = 0;
    int i = 0;
    for (i = 0; i < ataques; i++){
        en->acoes[i].tipoAcao = ATAQUE;

        // Regras de custo a depender do tipo do jogador
        if (en->tipo == FORTE) {
            // Se for forte só pode ter uma ação de custo 1
            if (contadorCustoUm >= 1) {
                en->acoes[i].custo = (rand() % 2) + 2; 
            } else {
                en->acoes[i].custo = (rand() % 3 + 1);
                if (en->acoes[i].custo == 1){
                    contadorCustoUm++;
                }                
            }
        }

        if (en->tipo == FRACO) {
            en->acoes[i].custo = (rand() % 2); 
        }

        // Regras de efeito a depender do custo - cartas de ATAQUE       
        switch (en->acoes[i].custo){
        case 0:
            en->acoes[i].efeito = (rand() % 5 + 1);     // efeitos possíveis: 1, 2, 3, 4 e 5
            break;
        case 1:
            en->acoes[i].efeito = (rand() % 5 + 6);     // efeitos possíveis: 6, 7, 8, 9 e 10
            break;
        case 2:
            en->acoes[i].efeito = (rand() % 5 + 11);     // efeitos possíveis: 11, 12, 13, 14 e 15
            break;
        case 3:
            en->acoes[i].efeito = (rand() % 15 + 16);     // efeitos possíveis: 16, 17, 18, 19, 20 ... 30
            break;
        }

    }

    for (; i < en->qtdAcoes; i++){
        en->acoes[i].tipoAcao = DEFESA;

        // Regras de custo a depender do tipo do jogador
        if (en->tipo == FORTE) {
            // Se for forte só pode ter uma ação de custo 1
            if (contadorCustoUm >= 1) {
                en->acoes[i].custo = (rand() % 2) + 2; 
            } else {
                en->acoes[i].custo = (rand() % 3 + 1);
                if (en->acoes[i].custo == 1){
                    contadorCustoUm++;
                }                
            }
        }

        if (en->tipo == FRACO) {
            en->acoes[i].custo = (rand() % 2); 
        }
        
        // Regras de efeito a depender do custo - cartas de DEFESA
        switch (en->acoes[i].custo)
        {
        case 0:
            en->acoes[i].efeito = (rand() % 5 + 1);     // efeitos possíveis: 1, 2, 3, 4 e 5
            break;
        case 1:
            en->acoes[i].efeito = (rand() % 5 + 6);     // efeitos possíveis: 6, 7, 8, 9 e 10
            break;
        case 2:
            en->acoes[i].efeito = (rand() % 5 + 11);     // efeitos possíveis: 11, 12, 13, 14 e 15
            break;
        case 3:
            en->acoes[i].efeito = (rand() % 15 + 16);     // efeitos possíveis: 16, 17, 18, 19, 20 ... 30
            break;
        }

    }

    return en;
}

// Gera o grupo de 2 inimigos para o combate atual
grupoInimigos *geraGrupoInimigos(NomesInimigos *ni, int *idxGlobal) {
    //recebe a lista de inimigos do controle do jogo e um ponteiro para o índice o inimigo atual

    // Aloca espaço para o grupo de inimigos do turno
    grupoInimigos *gp = (grupoInimigos *) malloc(sizeof(grupoInimigos));
    gp->qtd = 2;
    gp->inimigos = (Inimigo *) malloc(gp->qtd * sizeof(Inimigo));

    
    //DEFINIÇÃO DO TIPO DOS INIMIGOS
    // Cria vetor de enumeração de tipo dos inimigos
    inimigoTipo tipos[2];
    
    // Lógica de Probabilidade (5% de chance de Forte, no máximo 1 forte por turno)
    // Rola dados para o primeiro (0 a 99. Se menor que 5 - probabilidade de 5% - é Forte)
    int roll = rand() % 100; 
    if (roll < 5){
        tipos[0] = FORTE;
    } else{
        tipos[0] = FRACO; 
    } 

    //Define tipo do segundo
    //Se o primeiro é forte o segundo é fraco (max 1 forte por turno)
    if (tipos[0] == FORTE){
        tipos[1] = FRACO;
    } else {
        roll = rand() % 100;
        if (roll < 5){
        tipos[1] = FORTE;
        } else{
        tipos[1] = FRACO; 
        }
    }
    
    // GERAÇÃO DOS INIMIGOS
    for (int i = 0; i < 2; i++) {
        // Pega o nome/imagem da lista usando o índice global
        char *nomeAtual = ni->nome[*idxGlobal];
        char *imgAtual = ni->imag[*idxGlobal];

        // Gera o inimigo
        Inimigo *temp = geraInimigo(tipos[i], nomeAtual, imgAtual);
        gp->inimigos[i] = *temp;
        free(temp);

        // Incrementa o índice global para o próximo inimigo gerado ser diferente
        (*idxGlobal)++;
    }

    return gp;
}

// Liberar memória alocada por turno para o grupo de 2 inimigos
void liberaGrupoInimigos(grupoInimigos *gp) {
    if (gp == NULL) return;

    if (gp->inimigos != NULL) {
        for (int i = 0; i < gp->qtd; i++) {            
            if (gp->inimigos[i].enemy != NULL) {
                //Libera img do inimigo
                if (gp->inimigos[i].enemy->img != NULL) 
                    al_destroy_bitmap(gp->inimigos[i].enemy->img);
                //Libera nome do inimigo            
                if (gp->inimigos[i].enemy->nome != NULL) 
                    free(gp->inimigos[i].enemy->nome);
                //Libera o inimigo
                free(gp->inimigos[i].enemy);
            }

            // Libera o vetor de Ações
            if (gp->inimigos[i].acoes != NULL) {
                free(gp->inimigos[i].acoes);
            }
        }
        // Libera o vetor de inimigos
        free(gp->inimigos);
    }
    // Libera a struct do grupo
    free(gp);
}