/**
 * @file noctree.c
 * 
 * Implementação das funções do Octree. Para ver a documentação, consulte o header.
 */

#include "noctree.h"

noctree* inicializaNo(amostra* centro, float* tamanho) {
	/* Aloca a memória */
	noctree* no = (noctree*) malloc(sizeof(noctree));
	CHECK_MALLOC(no);

	/* Inicializa */
	for (int i = 0; i < NOCTREE_CAPACIDADE; i++) {
		no->pontos[i] = inicializaAmostra(0,0,0);
	}

	no->qtPontos    = 0;

	no->centro      = centro;

    for(int i = 0; i < DIM; i++) {
        no->tamanho[i] = tamanho[i];
    }

    /* Note que os filhos serão inicializados apenas quanto  subdividido == 1 */
	no->subdividido = 0;

    return no;
}

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
				realocaAmostra(no, no->pontos[i]);
			}
			
			// Limpa o vetor de amostras
			no->qtPontos = 0;
		}
	}
	// Se já subdividiu, insere no filho aequado
	return realocaAmostra(no, ponto);
}

void subdividir(noctree* no) {
	amostra** novoCentro;
	float novoTamanho[DIM];

	/* Cria os filhos */
	for (int i=0; i < NOCTREE_CAPACIDADE; i++) {
		/* Calcula o novo centro */
		*novoCentro = calculaCentroDoOctante(no->centro, no->tamanho, i);
		
		/* Calcula os novos tamanhos */
        for (int j=0; j < DIM; j++) {
            novoTamanho[j] = no->tamanho[j] / 2; // Eixos X,Y,Z
        }

		/* Inicializa o filho correspondente */
		no->filhos[i] = inicializaNo(*novoCentro, novoTamanho);
	}

	/* Marca como dividido */
	no->subdividido = 1;
}

bool realocaAmostra(noctree* no, amostra* ponto) {
	int posicao = 0;

	// Calcula a posição
	if (ponto->x >= no->centro->x) posicao += 1;
	if (ponto->y >= no->centro->y) posicao += 2;
	if (ponto->z >= no->centro->z) posicao += 4;

	// TODO esse check não tá bom. A ideia é ver se o nó tá 0 bala. COmo ver isso?
	if (posicao >= no->qtPontos) {
		return insereAmostra(&no->filhos[posicao], ponto);
	} else {
		// TODO ver o que fazer se o nó não estiver 0
		return 0;
	}
}
