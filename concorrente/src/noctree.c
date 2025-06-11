/**
 * @file noctree.c
 * 
 * Implementação das funções do Octree. Para ver a documentação, consulte o header.
 */

#include "noctree.h"


int esferaIntersectaCubo(amostra* centro_esfera, float raio, noctree* no) {
  float raio2 = raio * raio;
  float dist_cubo_x = fmax(0.0f, fabs(centro_esfera->x - no->centro->x) - no->tamanho[0]);
  float dist_cubo_y = fmax(0.0f, fabs(centro_esfera->y - no->centro->y) - no->tamanho[1]);
  float dist_cubo_z = fmax(0.0f, fabs(centro_esfera->z - no->centro->z) - no->tamanho[2]);

  return (dist_cubo_x*dist_cubo_x + dist_cubo_y*dist_cubo_y + dist_cubo_z*dist_cubo_z) < raio2;
}

float dist2(amostra* p1, amostra* p2) {
  float dx = p1->x - p2->x;
  float dy = p1->y - p2->y;
  float dz = p1->z - p2->z;
  return dx*dx + dy*dy + dz*dz;
}


noctree* inicializaNo(amostra* centro, float* tamanho, int profundidade) {
  LOGP("Cheguei no inicializaNo"); ENDL;
  /* Aloca a memória */
  noctree* no = (noctree*) malloc(sizeof(noctree));
  CHECK_MALLOC(no);

  /* Aloca o o vetor de amostras, mas não os pontos em si */
  no->pontos = (amostra**) malloc(sizeof(amostra*) * NOCTREE_CAPACIDADE);
  CHECK_MALLOC(no->pontos);

  for (int i = 0; i < NOCTREE_CAPACIDADE; i++) {
    no->pontos[i] = NULL; /* Não aloca memória */
  }


  no->qtPontos     = 0;            // Qt de amostras no vetor de amostras
  no->centro       = centro;       // Ponto que define o centroide do nó
  no->profundidade = profundidade; // Profundidade do nó na árvore

  /* Configura os tamanhos em X,Y,Z */
  for(int i = 0; i < DIM; i++) {
    no->tamanho[i] = tamanho[i];
  }

  /* Note que os filhos serão inicializados apenas quanto  subdividido == 1 */
  no->subdividido = 0;
  for (int i = 0; i < QT_FILHOS_NOCTREE; i++) {
    no->filhos[i] = NULL; /* Não aloca memória */
  }

  /* Inicializa o lock */
  if (pthread_rwlock_init(&no->lock, NULL) != 0) {
    LOG_ERROR(ERRO_LOCK, "Falha na inicialização do rwlock");
  }

  return no;
}

/* Obs: essa função tem melhorias de desempenho bem claras pedindo para serem
 * otimizadas, mas essa foi a forma que a lógica do código está mais clara.
 * Conscientemente estamos priorizando a legibilidade frente ao desempenho!
 * */
int insereAmostra(noctree* no, amostra* ponto) {
  int status = 1; // Booleano de retorno da função
  int pegueiOLock = 0;

  /* A primeira coisa é pegar o lock de escrita (apenas se é folha) */
  if (!no->subdividido) { // Aí sim há risco de modificação no nó
    pthread_rwlock_wrlock(&no->lock);
    pegueiOLock = 1;
    LOGP(" Peguei o Lock"); ENDL;
  }

  /* Depois segue normalmente a sessão crítica */
  if (no->subdividido) { // Amostra fica sempre nas folhas
    realocaAmostra(no, ponto);
  }
  else if (no->qtPontos < NOCTREE_CAPACIDADE) { // É folha & há espaço
    no->pontos[no->qtPontos] = ponto; // Aloca o ponto
    no->qtPontos++;
  }
  else { // É folha, mas não há espaço -> subdivide (apenas se profundidade não é max)
    /* Caso 1: profundidade não é máxima */
    if (no->profundidade <= NOCTREE_MAX_PROFUNDIDADE) {
      subdividir(no);

      // Reidistribui os pontos nos filhos apropriados
      for (int i = 0; i < NOCTREE_CAPACIDADE; i++) {
        status = status & realocaAmostra(no, no->pontos[i]);
      }
      // Housekeeping o vetor de amostras
      free(no->pontos);
      no->pontos = NULL;
      no->qtPontos = no->capacidade = 0;

      // E insere o ponto passado como argumento
      realocaAmostra(no, ponto);
    }
    /* Caso 2: profundidade é máxima. Decisão de projeto: alocaremos todas as amostras que vierem para esse nó */
    else {
      if (no->qtPontos == no->capacidade) { // Overflow no vetor de amostras
        no->capacidade = no->capacidade << 1; // Dobra a capacidade
        no->pontos = (amostra**) realloc(no->pontos, sizeof(amostra*) * no->capacidade);
      }
      no->pontos[no->qtPontos] = ponto; // Aloca o ponto
      no->qtPontos++;
    }
  }

  /* Solta o lock */
  if (pegueiOLock) {
    pthread_rwlock_unlock(&no->lock);
    LOGP(" Soltei o Lock"); ENDL;
  }
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
    no->filhos[i] = inicializaNo(novoCentro, novoTamanho, no->profundidade+1);

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

  // Libera a memória das amostras guardadas no nó
  for (int i = 0; i < no->qtPontos; i++) {
    free(no->pontos[i]);
  }
  free(no->pontos);

  free(no->centro); // Libera a memória do centro do nó
  pthread_rwlock_destroy(&no->lock); // Destrói o lock
  free(no);
}

void passoDaBuscaPorRegiao(noctree* no, amostra* centro_busca, float raio2, amostra*** resultados, int* qt_encontrados, int* capacidade) {
  pthread_rwlock_rdlock(&no->lock);

  /* Se a região não passa pelo nó, fim da busca nele e seus filhos */
  if (!esferaIntersectaCubo(centro_busca, sqrtf(raio2), no)) {
    pthread_rwlock_unlock(&no->lock);
    return;
  }
  
  /* Ok, passa pelo nó. */

  /* Se não é folha */
  if (no->subdividido) {
    for (int i = 0; i < QT_FILHOS_NOCTREE; i++) {
      if (no->filhos[i]) {
        passoDaBuscaPorRegiao(no->filhos[i], centro_busca, raio2, 
                              resultados, qt_encontrados, capacidade);
      }
    }
  } else { /* Se é folha, registramos apenas se está dentro da regiao */
    for (int i = 0; i < no->qtPontos; i++) {
      if (dist2(no->pontos[i], centro_busca) <= raio2) { 
        // Adiciona o ponto ao vetor de resultados, realocando se necessário
        if (*qt_encontrados >= *capacidade) {
          *capacidade *= 2;
          *resultados = realloc(*resultados, sizeof(amostra*) * (*capacidade));
          // TODO: talvez esse realloc dê problema com as threads
        }
        (*resultados)[*qt_encontrados] = no->pontos[i];
        (*qt_encontrados)++;
      }
    }
  }

  /* Libera o lock */
  pthread_rwlock_unlock(&no->lock);
}


amostra** buscaPorRegiao(noctree* no, amostra* centro, float raio, int* qt_encontrados) {
  int capacidade = 16; // Capacidade inicial do array de resultados
  amostra** resultados = malloc(sizeof(amostra*) * capacidade);
  if (!resultados) return NULL;
  // TODO: acho que deveria ter um lock para esse vetor de resultados também!!

  /* Housekeeping da entrada que faz parte da saída */
  *qt_encontrados = 0;
  float raio2 = raio * raio; /* raio^2 para evitar chamar sqrt */

  /* Passo recursivo */
  passoDaBuscaPorRegiao(no, centro, raio2, &resultados, qt_encontrados, &capacidade);

  /* Tira o espaço livre do vetor */
  if (*qt_encontrados > 0) {
    resultados = realloc(resultados, sizeof(amostra*) * (*qt_encontrados));
  } else {
    free(resultados);
    resultados = NULL;
  }

  return resultados;
}


amostra** buscaNaFolha(noctree* no, amostra* alvo, int* qt_encontrados) {
  /* Sanitiza a entrada */
  *qt_encontrados = 0;

  /* Pega o lock de leitura do nó */
  pthread_rwlock_rdlock(&no->lock);

  /* Se não estamos na folha */
  if (no->subdividido) {
    /* Libera o pai antes de descer para o filho */
    pthread_rwlock_unlock(&no->lock); 

    /* Vemos para qual filho devemos ir */
    int posicao = 0;
    if (alvo->x >= no->centro->x) posicao += 1;
    if (alvo->y >= no->centro->y) posicao += 2;
    if (alvo->z >= no->centro->z) posicao += 4;

    /* Desce para o filho correto */
    if (no->filhos[posicao]) {
      return buscaNaFolha(no->filhos[posicao], alvo, qt_encontrados);
    } else {
      return NULL; /* Se o filho não existe, não tem amostra lá -> lista vazia */
    }
  } 
  /* Se estamos numa folha, então é a folha correta */
  else {
    if (no->qtPontos == 0) { /* Se não há amostras -> lista vazia */
      pthread_rwlock_unlock(&no->lock);
      return NULL;
    }

    amostra** resultados = malloc(sizeof(amostra*) * no->qtPontos);
    CHECK_MALLOC(resultados); /* TODO: seria bom largar o lock antes de chamar exit */
    /*if (!resultados) {*/
    /*  pthread_rwlock_unlock(&no->lock);*/
    /*  return NULL;*/
    /*}*/

    memcpy(resultados, no->pontos, sizeof(amostra*) * no->qtPontos);
    *qt_encontrados = no->qtPontos;

    pthread_rwlock_unlock(&no->lock);
    return resultados;
  }

}

