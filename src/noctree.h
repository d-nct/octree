#ifndef NOCTREE_H
#define NOCTREE_H

#include "Amostra.h"

/**
 * Cria a estrutura de dados Noctree, que é um Nó da Octree
 */
typedef struct _Noctree {
    amostra* pontos; // Vetor com pontos contido no nó. Aqui na verdade é uma lista TODO
    amostra* centro; // Ponto central do cubo
    float[3] tamanho; // Dimensões X, Y, Z do cubo
    // float tamanho_x; // Dimensões fixas do cubo (eixo X)
    // float tamanho_y; // Dimensões fixas do cubo (eixo Y)
    // float tamanho_z; // Dimensões fixas do cubo (eixo Z)
    struct _Noctree[8] filhos; // 8 filhos do Nóctree
    bool subdividido; // Se o nó foi subdividido
} noctree;

/**
 * Inicializa um nó vazio da Octree.
 * 
 * @param centro É o centro do cubo.
 * @param tamanho Vetor coma as dimensões do cubo em X, Y e Z.
 * @return Ponteiro para o nó da Octree vazio.
 */
noctree* inicializaNo(amostra* centro, float* tamanho) {
    /* Aloca a memória */
    noctree* no = (noctree*) malloc(sizeof(noctree));
    CHECK_MALLOC(no);

    /* Inicializa */
    // Inicializar lista de pontos
    no->centro      = centro;
    no->tamanho     = tamanho;
    no->filhos      = [NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL]
    no->subdividido = 0;
}

#endif