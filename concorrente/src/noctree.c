/**
 * @file noctree.c
 * 
 * Implementação das funções do Octree. Para ver a documentação, consulte o header.
 */

#include "noctree.h"

noctree* inicializaNo(amostra* centro, float* tamanho) {
  LOGP("Cheguei no inicializaNo"); ENDL;
  /* Aloca a memória */
  noctree* no = (noctree*) malloc(sizeof(noctree));
  CHECK_MALLOC(no);

  /* Inicializa */
  for (int i = 0; i < NOCTREE_CAPACIDADE; i++) {
    no->pontos[i] = NULL; /* Não aloca memória */
  }

  for (int i = 0; i < QT_FILHOS_NOCTREE; i++) {
    no->filhos[i] = NULL; /* Não aloca memória */
  }

  no->qtPontos = 0;

  no->centro   = centro;

  for(int i = 0; i < DIM; i++) {
    no->tamanho[i] = tamanho[i];
  }

  /* Note que os filhos serão inicializados apenas quanto  subdividido == 1 */
  no->subdividido = 0;

  /* Inicializa o lock */
  if (pthread_rwlock_init(&no->lock, NULL) != 0) {
    LOG_ERROR(ERRO_LOCK, "Falha na inicialização do rwlock");
  }

  return no;
}

int insereAmostra(noctree* no, amostra* ponto) {
  int status = 1;

  /* A primeira coisa é pegar o lock de escrita */
  pthread_rwlock_wrlock(&no->lock);

  /* Depois segue normalmente a sessão crítica */
  if (no->subdividido) { // Amostra fica sempre nas folhas
    return realocaAmostra(no, ponto);
  }
  else if (no->qtPontos < NOCTREE_CAPACIDADE) { // É folha & há espaço
    no->pontos[no->qtPontos] = ponto;
    no->qtPontos++;
    return status;
  }
  else { // É folha, mas não há espaço -> subdivide
    subdividir(no);

    // Reidistribui os pontos nos filhos apropriados
    for (int i = 0; i < NOCTREE_CAPACIDADE; i++) {
      status = status & realocaAmostra(no, no->pontos[i]);
      no->pontos[i] = NULL;

      // Limpa o vetor de amostras
      no->qtPontos = 0;
    }
  }

  /* Solta o lock */
  pthread_rwlock_unlock(&no->lock);
  return status;
}

void subdividir(noctree* no) {
  LOGP("Subdividindo nó com centro (%.2f, %.2f, %.2f) e tamanho (%.2f)",
             no->centro->x, no->centro->y, no->centro->z, no->tamanho[0]); ENDL;

  float novoTamanho[DIM];

  /* Calcula os novos tamanhos */
  for (int j=0; j < DIM; j++) {
    novoTamanho[j] = no->tamanho[j] / 2.0f; // Eixos X,Y,Z
  }

  /* Cria os filhos */
  for (int i=0; i < QT_FILHOS_NOCTREE; i++) {
    /* Calcula o novo centro */
    amostra *novoCentro = calculaCentroDoOctante(no, novoTamanho, i);

    /* Inicializa o filho correspondente */
    no->filhos[i] = inicializaNo(novoCentro, novoTamanho);

    LOGP(" -> Filho %d criado com centro (%.2f, %.2f, %.2f) e tamanho (%.2f)",
             i, novoCentro->x, novoCentro->y, novoCentro->z, novoTamanho[0]); ENDL;

  }

  /* Marca como dividido */
  no->subdividido = 1;
}

/* Não precisa de lock, posi só é chamada por insere, que é bloqueante */
int realocaAmostra(noctree* no, amostra* ponto) {
  int posicao = 0;

  // Calcula a posição
  if (ponto->x >= no->centro->x) posicao += 1;
  if (ponto->y >= no->centro->y) posicao += 2;
  if (ponto->z >= no->centro->z) posicao += 4;

  LOGP("Realocando ponto (%.2f, %.2f, %.2f) para o filho de índice %d do nó com centro (%.2f, %.2f, %.2f)",
           ponto->x, ponto->y, ponto->z, posicao, no->centro->x, no->centro->y, no->centro->z); ENDL;


  return insereAmostra(no->filhos[posicao], ponto);
}

amostra* calculaCentroDoOctante(noctree* no, float* tamanho, int i) {
  amostra* centro = inicializaAmostra(0, 0, 0);

  /* Calcula o centro do octante baseado no nó e no tamanho */
  centro->x = no->centro->x + ((i & 1) ? tamanho[0] / 2 : -tamanho[0] / 2);
  centro->y = no->centro->y + ((i & 2) ? tamanho[1] / 2 : -tamanho[1] / 2);
  centro->z = no->centro->z + ((i & 4) ? tamanho[2] / 2 : -tamanho[2] / 2);

  return centro;
}

void destroiNo(noctree* no) {
  if (no == NULL) return;

  if (no->subdividido) {
    for (int i = 0; i < QT_FILHOS_NOCTREE; i++) {
      destroiNo(no->filhos[i]);
    }
  }

  // Libera a memória das amostras no nó
  for (int i = 0; i < no->qtPontos; i++) {
    free(no->pontos[i]);
  }

  // Libera o lock
  pthread_rwlock_destroy(&no->lock);

  // Libera a memória do centro do nó
  free(no->centro);
  free(no);
}

/*amostra** buscaPorRegiao(noctree* no, amostra* centro, float raio, int* qt_encontrados) {*/
/*  /* Pega o lock de leitura -> bloqueia escrita */*/
/*  pthread_rwlock_rdlock(&no->lock);*/
/**/
/**/
/*  /* Libera para escrita */*/
/*  pthread_rwlock_unlock(&no->lock);*/
/*}*/
