#ifndef NOCTREE_H
#define NOCTREE_H

#include "Amostra.h"

/**
 * Cria a estrutura de dados Noctree, que é um Nó da Octree
 */
typedef struct _Noctree {
    amostra[NOCTREE_CAPACIDADE] pontos; // Vetor com pontos contido no nó.
    int qtPontos; // Tamanho do vetor com os pontos
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
    for (int i = 0; i < NOCTREE_CAPACIDADE; i++) {
        no->pontos[i] = 0;
    }
    no->qtPontos    = 0;
    no->centro      = centro;
    no->tamanho     = tamanho;
    no->filhos      = [NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL];
    no->subdividido = 0;
}

/**
 * Insere uma amostra em um nó da Octree.
 *
 * @param no É o nó da Octree que será a nova moradia do ponto.
 * @param ponto É uma amostra do LIDAR.
 */
bool insereAmostra(noctree* no, amostra* ponto) {
    if (!no->subdividido) { // Insere nesse nó se houver espaço
        if (no->qtPontos < NOCTREE_CAPACIDADE) { // Se não está cheio
            no->pontos[no->qtPontos] = ponto;
            no->qtPontos++;
            return 1;
        } else { // Divide
            subdividir(no); // TODO

            // Reinsere os pontos nos filhos apropriados
            for (int i = 0; i < no->qtPontos; i++) {
                realocaAmostra(no, no->pontos[i])
            }
            
            // Limpa o vetor de amostras
            no->qtPontos = 0;
        }
    }
    // Se já subdividiu, insere no filho aequado
    return realocaAmostra(no, ponto);
}

/**
 * Subdivide um nó da Octree em 8 octantes.
 *
 * @param no É o nó a ser subdividido.
 */
void subdividir(noctree* no) {
    amostra novoCentro;
    float[3] novoTamanho;

    /* Cria os filhos */
    for (int i=0; i < 8; i++) {
        /* Calcula o novo centro */
        novoCentro = calculaCentroDoOctante(no->centro, no->tamanho, i);
        
        /* Calcula os novos tamanhos */
        novoTamanho[0] = no->tamanho[0] / 2; // Eixo X
        novoTamanho[1] = no->tamanho[1] / 2; // Eixo Y
        novoTamanho[2] = no->tamanho[2] / 2; // Eixo Z

        /* Inicializa o filho correspondente */
        no->filhos[i] = inicializaNo(novoCentro, novoTamanho);
    }

    /* Marca como dividido */
    no->subdividido = 1;
}

bool realocaAmostra(noctree* no, amostra* ponto) {
    int posicao = 0;

    // Calcula a posição
    if (ponto->x >= no->centro->x) indice += 1;
    if (ponto->y >= no->centro->y) indice += 2;
    if (ponto->z >= no->centro->z) indice += 4;

    // TODO esse check não tá bom. A ideia é ver se o nó tá 0 bala. COmo ver isso?
    if (posicao >= no->qtPontos) {
        return insereAmostra(no->filhos[posicao], ponto)
    } else {
        // TODO ver o que fazer se o nó não estiver 0
        return 0;
    }
}

#endif