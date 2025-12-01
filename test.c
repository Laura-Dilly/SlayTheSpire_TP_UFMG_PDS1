#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Inclui os cabeçalhos do projeto
#include "constants.h"

// Assume-se que deck_logic.h contenha os protótipos: 
// grupoCarta *geraBaralhoInicial(void);
// void liberaBaralho(grupoCarta *baralho);
#include "deck_logic.h" 

// Protótipo da função auxiliar (para o main saber que ela existe)
const char* getTipoString(CartaTipo tipo);

int main() {
    // NOTA: O seu geraBaralhoInicial() no deck_logic.c já chama srand(time(NULL)).
    // Portanto, não é estritamente necessário chamar aqui novamente, mas
    // para garantir que o teste main tenha entropia caso a lógica mude, mantemos:
    srand(time(NULL));

    printf("=== INICIANDO TESTE DO BARALHO ===\n");

    // 1. Gera o baralho completo (usa a função compilada de deck_logic.c)
    grupoCarta *meuBaralho = geraBaralhoInicial();

    if (meuBaralho == NULL) {
        printf("Erro fatal: Baralho retornou NULL.\n");
        return 1;
    }

    printf("Baralho gerado com sucesso! Tamanho: %d cartas.\n\n", meuBaralho->tam);

    // 2. Imprime todo o baralho para conferência
    printf("%-4s | %-10s | %-20s | %-5s | %-6s\n", "IDX", "TIPO", "NOME", "CUSTO", "EFEITO");
    printf("----------------------------------------------------------------\n");

    for (int i = 0; i < meuBaralho->tam; i++) {
        Carta c = meuBaralho->cartas[i];
        
        printf("%02d   | %-10s | %-20s | %-5d | %-6d\n", 
               i, 
               getTipoString(c.tipo), 
               c.nome, 
               c.custo, 
               c.efeito);
    }
    printf("----------------------------------------------------------------\n");

    // 3. Limpeza de Memória
    // A função correta definida em deck_logic.c é liberaBaralho, não destroiBaralho
    liberaBaralho(meuBaralho);
    
    printf("\nMemória liberada. Fim do teste.\n");

    return 0;
}

// --- IMPLEMENTAÇÃO DA FUNÇÃO AUXILIAR ---
const char* getTipoString(CartaTipo tipo) {
    switch (tipo) {
        case ATAQUE: return "ATAQUE";
        case DEFESA: return "DEFESA";
        case ESPECIAL: return "ESPECIAL";
        default: return "DESCONHECIDO";
    }
}