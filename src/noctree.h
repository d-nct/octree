#ifndef NOCTREE_H
#define NOCTREE_H

#include "amostra.h"
#include "system.h"

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
noctree* inicializaNo(amostra* centro, float* tamanho);

/**
 * Insere uma amostra em um nó da Octree.
 *
 * @param no É o nó da Octree que será a nova moradia do ponto.
 * @param ponto É uma amostra do LIDAR.
 */
bool insereAmostra(noctree* no, amostra* ponto);

/**
 * Subdivide um nó da Octree em 8 octantes.
 *
 * @param no É o nó a ser subdividido.
 */
void subdividir(noctree* no);

/**
 * Redistribui uma amostra para o nó apropriado.
 */
bool realocaAmostra(noctree* no, amostra* ponto);

#endif