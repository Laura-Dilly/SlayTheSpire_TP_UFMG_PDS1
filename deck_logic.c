#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "constants.h"
#include "renderer.h"
#include "utils.h"
#include <string.h>

// Lê nomes de cartas dos arquivos card_names.txt e special_cards.txt
// Retorna struct com arrays alocados (caller deve liberar com liberaNomesCartas)
NomesCartas geraNomesCarta(void){
    
    FILE *arq = fopen("data/card_names.txt", "r");
    FILE *specials = fopen("data/special_cards.txt", "r");

    if(arq == NULL || specials == NULL){
        printf("Erro ao abrir arquivos de nomes de cartas.\n");
        exit(EXIT_FAILURE);
    }

    NomesCartas namescards;

    // CARD_NAMES - 50 nomes
    namescards.names = malloc(50 * sizeof(char*));
    char linha[256];
    
    int idx = 0;
    while(fgets(linha, sizeof(linha), arq) != NULL && idx < 50){
        linha[strcspn(linha, "\n")] = '\0';      
        namescards.names[idx] = malloc(strlen(linha) + 1);
        strcpy(namescards.names[idx], linha);
        idx++;
    }
    namescards.names_count = idx;
    
    // SPECIAL_CARDS - 10 nomes (aloca e lê, NÃO libera)
    namescards.special_names = malloc(10 * sizeof(char*));
    char linha_s[256];
    
    idx = 0;
    while(fgets(linha_s, sizeof(linha_s), specials) != NULL && idx < 10){
        linha_s[strcspn(linha_s, "\n")] = '\0';      
        namescards.special_names[idx] = malloc(strlen(linha_s) + 1);
        strcpy(namescards.special_names[idx], linha_s);
        idx++;
    }
    namescards.special_count = idx;
    
    fclose(arq);
    fclose(specials);
    
    return namescards;
}

// Função para embalhar strings em um vetor de strings (será usada para NomesCartas)
void embaralhaNomes(char **vetor, int tamanho) {
    for (int i = 0; i < tamanho; i++) {
        // Sorteia uma posição aleatória
        int r = rand() % tamanho;

        // Troca o nome da posição atual (i) com a sorteada (r)
        char *temp = vetor[i];
        vetor[i] = vetor[r];
        vetor[r] = temp;
    }
}

// Libera memória alocada por geraNomesCarta()
void liberaNomesCartas(NomesCartas *nc){
    if(nc->names){
        for(int i = 0; i < nc->names_count; i++){
            free(nc->names[i]);
        }
        free(nc->names);
    }
    if(nc->special_names){
        for(int i = 0; i < nc->special_count; i++){
            free(nc->special_names[i]);
        }
        free(nc->special_names);
    }
}


Carta *geraCarta (CartaTipo type, char *name, int cost){
    // lê o tipo da carta, o nome da carta e o custo dela

    Carta *card = (Carta *) malloc(sizeof(Carta));
    if (card == NULL){
        printf("Erro ao criar estrutura da carta.");
        exit(EXIT_FAILURE);
    }

    //Gera Nome da carta
    card->nome = (char *) malloc(strlen(name) + 1);
    if (card->nome == NULL) {
        printf("Erro ao criar nome da carta.");
        free(card);
        exit(EXIT_FAILURE);
    }
    strcpy(card->nome, name);

    //Gera Tipo da carta
    card->tipo = type;

    //Gera custo da carta
    card->custo = cost;

    //Gera efeito da carta
    if ((card-> tipo == ATAQUE) || (card->tipo == DEFESA)){
        switch (card->custo)
        {
        case 0:
            card->efeito = (rand() % 5 + 1);     // efeitos possíveis: 1, 2, 3, 4 e 5
            break;
        case 1:
            card->efeito = (rand() % 5 + 6);     // efeitos possíveis: 6, 7, 8, 9 e 10
            break;
        case 2:
            card->efeito = (rand() % 5 + 11);     // efeitos possíveis: 11, 12, 13, 14 e 15
            break;
        case 3:
            card->efeito = (rand() % 15 + 16);     // efeitos possíveis: 16, 17, 18, 19, 20 ... 30
            break;
        }
    } else {
        card->efeito = 0;     // efeitos da especial: 0
    }

    return card;

}


// Preenche um novo baralho de 20 cartas
grupoCarta *geraBaralhoInicial (void){
    
    srand(time(NULL));

    // Criar espaço do baralho de 20 cartas
    grupoCarta *baralho = (grupoCarta *) malloc(sizeof(grupoCarta));
    if (baralho == NULL) {
        printf("Erro ao criar estrutura do baralho.");
        exit(EXIT_FAILURE); 
    }
    baralho->tam = 20;
    baralho->cartas = (Carta *) malloc(baralho->tam * sizeof(Carta));
    if (baralho->cartas == NULL) {
        printf("Erro ao criar array de cartas do baralho.");
        free(baralho);
        exit(EXIT_FAILURE); 
    }

    // Gerar cartas do baralho:

    //Gerar a lista de nomes possíveis e embaralhar os nomes para escolha aleatória
    NomesCartas nomescartas = geraNomesCarta();
    embaralhaNomes(nomescartas.names, nomescartas.names_count);
    embaralhaNomes(nomescartas.special_names, nomescartas.special_count);   

    // CARTAS DE ATAQUE [10 cartas]
    CartaTipo type = ATAQUE;
    int i = 0;
    // 1 de custo 0, 1 de custo 1, 1 de custo 2 e 1 de custo 3
    for (int c = 0; c < 4; c++) {
        Carta *new_card = geraCarta(type, nomescartas.names[i], c);
        if (new_card != NULL) {
            baralho->cartas[i] = *new_card;
            free(new_card);
            i++; 
        }
    }
    // 2 cartas de custo 1
    for (int j = 0; j < 2; j++) {
        Carta *new_card = geraCarta(type, nomescartas.names[i], 1);
        if (new_card != NULL) {
            baralho->cartas[i] = *new_card;
            free(new_card);
            i++; 
        }
    }
    // 4 cartas de custo aleatório (entre 0 e 3)
    for (int g = 0; g < 4; g++) {
        int r = (rand() % 4);
        Carta *new_card = geraCarta(type, nomescartas.names[i], r);
        if (new_card != NULL) {
            baralho->cartas[i] = *new_card;
            free(new_card);
            i++; 
        }
    } 

    // CARTAS DE DEFESA [8 cartas]
    type = DEFESA;
    // 1 de custo 0, 1 de custo 1, 1 de custo 2 e 1 de custo 3
    for (int c = 0; c < 4; c++) {
        Carta *new_card = geraCarta(type, nomescartas.names[i], c);
        if (new_card != NULL) {
            baralho->cartas[i] = *new_card;
            free(new_card);
            i++; 
        }
    }
    // 2 cartas de custo 1
    for (int j = 0; j < 2; j++) {
        Carta *new_card = geraCarta(type, nomescartas.names[i], 1);
        if (new_card != NULL) {
            baralho->cartas[i] = *new_card;
            free(new_card);
            i++; 
        }
    }
    // 2 cartas de custo aleatório (entre 0 e 2)
    for (int g = 0; g < 2; g++) {
        int r = (rand() % 3);
        Carta *new_card = geraCarta(type, nomescartas.names[i], r);
        if (new_card != NULL) {
            baralho->cartas[i] = *new_card;
            free(new_card);
            i++; 
        }
    } 

    // CARTAS ESPECIAIS [2 cartas]
    type = ESPECIAL;
    for (int g = 0; g < 2; g++) {
        Carta *new_card = geraCarta(type, nomescartas.special_names[g], 0);
        if (new_card != NULL) {
            baralho->cartas[i] = *new_card;
            free(new_card);
            i++; 
        }
    }

    // Libera a memória da lista auxiliar de nomes
    liberaNomesCartas(&nomescartas);

    // Retorna o baralho
    return baralho;

}

// Função Auxiliar: Cria uma pilha vazia (para mão e descarte)
grupoCarta* criaPilhaVazia() {
    grupoCarta *gc = (grupoCarta *) malloc(sizeof(grupoCarta));
    gc->tam = 0;
    gc->cartas = NULL; // Ainda não tem cartas
    return gc;
}

void liberaBaralho(grupoCarta *baralho) {
    if (baralho == NULL) return;

    // Libera o nome de cada carta
    if (baralho->cartas != NULL) {
        for (int i = 0; i < baralho->tam; i++) {
            if (baralho->cartas[i].nome != NULL) {
                free(baralho->cartas[i].nome);
            }
        }
        
        // Libera o vetor de cartas
        free(baralho->cartas);
    }

    // Libera a estrutura principal do baralho
    free(baralho);
}



// Lógica de compra e descarte de cartas

// Função para embaralhar cartas (quando tem que recriar a pilha de compras)
void embaralhaCartas(grupoCarta *baralho) {
    if (baralho == NULL || baralho->tam <= 1) return;

    for (int i = 0; i < baralho->tam; i++) {
        
        int r = rand() % baralho->tam;
        
        // Troca as cartas de posição
        Carta temp = baralho->cartas[i];
        baralho->cartas[i] = baralho->cartas[r];
        baralho->cartas[r] = temp;
    }
}

// Move todas as cartas do descarte para a pilha e embaralha
void reciclaPilha(Jogador *j) {
    if (j->descarte->tam == 0) return; // Nada para reciclar

    int tamAtual = j->pilha->tam;
    int tamAdicional = j->descarte->tam;
    int novoTamanho = tamAtual + tamAdicional;

    // Realoca a pilha de compras para caber as cartas do descarte
    j->pilha->cartas = (Carta *) realloc(j->pilha->cartas, novoTamanho * sizeof(Carta));
    if (j->pilha->cartas == NULL && novoTamanho > 0) {
        printf("Erro ao realocar pilha na reciclagem.\n");
        exit(EXIT_FAILURE);
    }

    // Copia as cartas do descarte para o final da pilha
    for (int i = 0; i < tamAdicional; i++) {
        j->pilha->cartas[tamAtual + i] = j->descarte->cartas[i];
    }
    j->pilha->tam = novoTamanho;

    // Esvazia a pilha de descarte
    free(j->descarte->cartas);
    j->descarte->cartas = NULL;
    j->descarte->tam = 0;

    // Embaralha a nova pilha de compras
    embaralhaCartas(j->pilha);
}

// Função para comprar uma quantidade qtd de cartas
void compraCartas(Jogador *j, int qtd) {
    for (int i = 0; i < qtd; i++) {
        
        if (j->pilha->tam == 0) {
            reciclaPilha(j);
            // conferência caso não tivesse cartas no descarte - não foi reciclado nada
            if (j->pilha->tam == 0) break; 
        }

        // Pega a carta do final da pilha
        int idxTopo = j->pilha->tam - 1;
        Carta cartaMovida = j->pilha->cartas[idxTopo];

        // Adiciona a carta movida na mão
        j->mao->tam++;
        j->mao->cartas = (Carta *) realloc(j->mao->cartas, j->mao->tam * sizeof(Carta));
        j->mao->cartas[j->mao->tam - 1] = cartaMovida;

        // Remove da pilha (Reduz o tamanho e realoca)
        j->pilha->tam--;
        if (j->pilha->tam > 0) {
            j->pilha->cartas = (Carta *) realloc(j->pilha->cartas, j->pilha->tam * sizeof(Carta));
        } else {
            // Se tamanho virou 0, libera o ponteiro e depois recicla a pilha
            free(j->pilha->cartas);
            j->pilha->cartas = NULL;
            reciclaPilha(j);
        }
    }
}


// Função para descartar todas as cartas da mão do jogaddor
void descartaMao (Jogador *j){

    if (j->mao->tam == 0) return; // Nada para descartar

    int tamAtual = j->descarte->tam;
    int tamAdicional = j->mao->tam;
    int novoTamanho = tamAtual + tamAdicional;

    // Realoca as cartas da mao para a pilha de descarte
    j->descarte->cartas = (Carta *) realloc(j->descarte->cartas, novoTamanho * sizeof(Carta));
    if (j->descarte->cartas == NULL && novoTamanho > 0) {
        printf("Erro ao realocar cartas da mao no descarte.\n");
        exit(EXIT_FAILURE);
    }

    // Copia as cartas da mao para o descarte
    for (int i = 0; i < tamAdicional; i++) {
        j->descarte->cartas[tamAtual + i] = j->mao->cartas[i];
    }
    j->descarte->tam = novoTamanho;

    // Esvazia a pilha de descarte
    free(j->mao->cartas);
    j->mao->cartas = NULL;
    j->mao->tam = 0;
}  