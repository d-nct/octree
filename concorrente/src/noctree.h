#ifndef NOCTREE_H
#define NOCTREE_H

#include "system.h"
#include "amostra.h"
#include <math.h>

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
  pthread_rwlock_t lock;               // Lock de leitura/escrita por nó
} noctree;


/* Funções de Geometria
 * -------------------- */

/**
 * Retorna o quadrado da distância euclidiana entre dois pontos.
 */
float dist2(amostra* p1, amostra* p2);

/**
 * Verifica se uma esfera intersecta o cubo representado por um nó.
 * 
 */
int esferaIntersectaCubo(amostra* centro_esfera, float raio, noctree* no);


/* Funções da Octree 
 * ----------------- */

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

/**
 * Realiza um passo da busca recursiva por região. 
 * NÃO DEVE SER CHAMADA PELO USUÁRIO! Poderia ser static, mas preferi não o fazer para manter a documentação organizada no .h.
 *
 * @param no é a raiz da árvore
 * @param centro é o centroide ao redor do qual a busca será realizada
 * @param raio2 é o quadrado do raio que define a esfera de busca ao redor do centroide (é o quadrado para evitar chamadas de  sqrt  desnecessárias)
 * @param resultados é um ponteiro para o vetor de amostras. Tripla indireção geralmente quer dizer que algo está overcomplicated. TODO: refatorar.
 * @param qt_encontrados é um pt para inteiro que dirá quantas amostras foram encontradas ao final da busca (essa variável será alterada!!)
 * @param capacidade do vetor com as amostras encontradas
 *
 * @returns uma lista de amostras
 */
void passoDaBuscaPorRegiao(noctree* no, amostra* centro_busca, float raio2, amostra*** resultados, int* qt_encontrados, int* capacidade);

/**
 * Busca amostras em uma região da árvore.
 *
 * @param no é a raiz da árvore
 * @param centro é o centroide ao redor do qual a busca será realizada
 * @param raio é o tamanho que define a região de busca ao redor do centroid
 * @param qt_encontrados é um pt para inteiro que dirá quantas amostras foram encontradas ao final da busca (essa variável será alterada!!)
 *
 * @returns um vetor com todas as amostras contidas na região
 */
amostra** buscaPorRegiao(noctree* no, amostra* centro, float raio2, int* qt_encontrados);

/**
 * Busca amostras em uma vizinhança da árvore.
 * Dado um ponto, verifica em qual folha ele cairia e retorna as amostras contidas na folha.
 *
 * @param no é a raiz da árvore
 * @param alvo da busca a partir do qual, se encontrará a folha
 * @param qt_encontrados é um pt para inteiro que dirá quantas amostras foram encontradas ao final da busca (essa variável será alterada!!)
 *
 * @returns um vetor com todas as amostras contidas pela folha
 */
amostra** buscaNaFolha(noctree* no, amostra* alvo, int* qt_encontrados);

#endif
