#ifndef NOCTREE_H
#define NOCTREE_H

#include "system.h"
#include "amostra.h"

/**
 * Cria a estrutura de dados Noctree, que é um Nó da Octree
 */
typedef struct _Noctree {
	amostra* pontos[NOCTREE_CAPACIDADE]; // Vetor com pontos contido no nó.
	int qtPontos;                        // Tamanho do vetor com os pontos
	amostra* centro;                     // Ponto central do cubo
	float tamanho[DIM];                  // Dimensões X, Y, Z do cubo
	struct _Noctree *filhos[QT_FILHOS_NOCTREE]; // 8 filhos do Nóctree
	int subdividido;                     // 1 se o nó foi subdividido; 0 c.c.
  pthread_rwlock_t lock;
} noctree;

/**
 * Inicializa um nó vazio da Octree.
 * 
 * @param centro É o centro do cubo.
 * @param tamanho Vetor coma as dimensões do cubo em X, Y e Z.
 *
 * @return Ponteiro para o nó da Octree vazio.
 */
noctree* inicializaNo(amostra* centro, float* tamanho);

/**
 * Insere uma amostra em um nó da Octree.
 *
 * @param no É o nó da Octree que será a nova moradia do ponto.
 * @param ponto É uma amostra do LIDAR.
 * 
 * @return 1, se ok
 * 			   0, c.c.
 */
int insereAmostra(noctree* no, amostra* ponto);

/**
 * Subdivide um nó da Octree em 8 octantes.
 *
 * @param no É o nó a ser subdividido.
 */
void subdividir(noctree* no);

/**
 * Redistribui uma amostra para o nó apropriado.
 * 
 * @param no É o nó pai.
 * @param ponto É a amostra a ser realocada
 * 
 * @return 1, se ok
 * 			   0, c.c.
 */
int realocaAmostra(noctree* no, amostra* ponto);

/**
 * Baseado em um nó e seu tamanho, calcula o novo centro do octante para seu i-ésimo filho.
 */
amostra* calculaCentroDoOctante(noctree* no, float* tamanho, int i);

/**
 * Destrói um nó da octree de forma segura. Isto é, desalocando o que foi alocado.
 */
void destroiNo(noctree* no);

#endif
