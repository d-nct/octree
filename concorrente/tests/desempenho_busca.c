/**
 * @file Arquivo fonte para analisar o desempenho de Octree
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h> // Para a função sleep()

#include "../src/noctree.h"
#include "timer.h"

/* Define a região que serão geradas as amostras: cubo de aresta 100 */
#define REGIAO_X_MENOS -50
#define REGIAO_X_MAIS   50
#define REGIAO_Y_MENOS -50
#define REGIAO_Y_MAIS   50
#define REGIAO_Z_MENOS -50
#define REGIAO_Z_MAIS   50

/* Define o raio de busca */
#define RAIO_BUSCA       5

/* E a quantidade de buscas */
#define QT_BUSCAS    10000

/* Gera um ponto no R^3 com uma distribuição uniforme */
amostra* geraAmostraUnif(void) {
  /* Gera as posições em X,Y,Z no intervalo possível */
  float pos_x = REGIAO_X_MENOS + (((float)rand() / (float)RAND_MAX) * (REGIAO_X_MAIS - REGIAO_X_MENOS) );
  float pos_y = REGIAO_Y_MENOS + (((float)rand() / (float)RAND_MAX) * (REGIAO_Y_MAIS - REGIAO_Y_MENOS) );
  float pos_z = REGIAO_Z_MENOS + (((float)rand() / (float)RAND_MAX) * (REGIAO_Z_MAIS - REGIAO_Z_MENOS) );

  return inicializaAmostra(pos_x, pos_y, pos_z);
}

/* Gera os pontos aleatórios e insere na octree */
void preencheOctreeUnif(noctree* raiz, int qtPontos) {
  /* Insere os pontos */
  for (int i = 0; i < qtPontos; i++) {
    insereAmostra(raiz, geraAmostraUnif());
  }
}

/* Argumento da thread */
typedef struct {
  pthread_t* tid;
  noctree* raiz;
  long int idThread;
  long int nthreads;
  long long int N;
} dados_thread_produtora_t;

typedef struct {
  pthread_t* tid;
  long int idThread;
  long long int qtAmostras;
} ret_thread_produtora_t;

/* Função que será executada pela thread escritora */
void* rotina_produtora(void* arg) {
  dados_thread_produtora_t* dados = (dados_thread_produtora_t*)arg;

  /* Calcula quantas amostras precisa inserir */
  int qtAmostrasAGerar = dados->N / dados->nthreads;
  if (dados->idThread == 0) qtAmostrasAGerar += dados->N % dados->nthreads; // A última fica com o resto, se houver

  /* Insere as amostras */
  preencheOctreeUnif(dados->raiz, qtAmostrasAGerar);

  /* Gera o retorno */
  ret_thread_produtora_t* ret = (ret_thread_produtora_t*) malloc(sizeof(ret_thread_produtora_t));
  CHECK_MALLOC(ret);

  ret->tid = dados->tid;
  ret->idThread = dados->idThread;
  ret->qtAmostras = qtAmostrasAGerar;

  pthread_exit((void*) ret);
}

/* Argumento da thread */
typedef struct {
  pthread_t* tid;
  noctree* raiz;
  long int idThread;
  long int nthreads;
  long long int numBuscas;
} dados_thread_consumidora_t;

typedef struct {
  pthread_t* tid;
  long int idThread;
  long long int qtAmostrasEncontradas;
  long int qtBuscasRealizadas;
} ret_thread_consumidora_t;

/* Função que será executada pela thread escritora */
void* rotina_consumidora(void* arg) {
  dados_thread_consumidora_t* dados = (dados_thread_consumidora_t*)arg;
  amostra* ptAmostra;
  int qt_encontrados; // Retorno da busca

  /* Gera o retorno */
  ret_thread_consumidora_t* ret = (ret_thread_consumidora_t*) malloc(sizeof(ret_thread_consumidora_t));
  CHECK_MALLOC(ret);
  ret->tid = dados->tid;
  ret->idThread = dados->idThread;

  /* Calcula quantas buscas terá que realizar */
  int qtBuscas = dados->numBuscas / dados->nthreads;
  if (dados->idThread == 0) qtBuscas += dados->numBuscas % dados->nthreads; // A última fica com o resto, se houver

  /* Realiza as buscas */
  for (int i = 0; i < qtBuscas; i++) {
    ptAmostra = geraAmostraUnif(); // Gera uma amostra qualquer
    buscaPorRegiao(dados->raiz, ptAmostra, RAIO_BUSCA, &qt_encontrados); // faz a busca
    // despreza a lista de amostras encontradas, mas em uma aplicação real, utilizaríamos
    ret->qtAmostrasEncontradas += qt_encontrados; // Atualiza o retorno
    ret->qtBuscasRealizadas++;
  }

  pthread_exit((void*) ret);
}


int main(int argc, char *argv[]) {
  srand(time(NULL));

  /* Variáveis auxiliares */
  int nthreadsProd, nthreadsCons;  // Qt de threads (passada linha de comando)
  long long int N;                 // Qt de pontos na octree
  long long int numBuscas;         // Qt de buscas a serem realizadas na octree
  pthread_t *tid;                  // Identificadores das threads no sistema
  double inicioAlvo;               // Marcações de tempo da parte concorrente
  double inicioTotal, fim;         // Marcações de tempo total do programa

  /* Variáveis da árvore */
  noctree* raiz = inicializaNo(inicializaAmostra(0,0,0), (float[]){100,100,100}, 0);
  dados_thread_produtora_t* ptArgProd;
  ret_thread_produtora_t** retsProd;
  dados_thread_consumidora_t* ptArgCons;
  ret_thread_consumidora_t** retsCons;
  
  printf("Tomadas de Tempo:\n");

  GET_TIME(inicioTotal);

  //--le e avalia os parametros de entrada
  if(argc<5) {
    printf("Digite: %s <numero de threads produtoras> <nthreads consumidoras> <N> <numBuscas>\n", argv[0]);
    return 1;
  }
  nthreadsProd = atoi(argv[1]);
  nthreadsCons = atoi(argv[2]);
  N = atoll(argv[3]);
  numBuscas = atoll(argv[4]);

  //--aloca as estruturas
  tid = (pthread_t*) malloc(sizeof(pthread_t)*(nthreadsProd + nthreadsCons));
  CHECK_MALLOC(tid);

  retsProd = (ret_thread_produtora_t**) malloc(sizeof(ret_thread_produtora_t*) * nthreadsProd);
  CHECK_MALLOC(retsProd);

  retsCons = (ret_thread_consumidora_t**) malloc(sizeof(ret_thread_consumidora_t*) * nthreadsCons);
  CHECK_MALLOC(retsCons);

  //--cria as threads
  GET_TIME(inicioAlvo);
  printf("  Início da criação das threads em %lf\n", inicioAlvo - inicioTotal);

  /* Cria as Produtoras, que geram a árvore */
  for(long int t=0; t < nthreadsProd ; t++) {
    /* Cria o argumento */
    ptArgProd = (dados_thread_produtora_t*) malloc(sizeof(dados_thread_produtora_t));
    CHECK_MALLOC(ptArgProd);

    ptArgProd->tid = &tid[t];
    ptArgProd->raiz = raiz;
    ptArgProd->idThread = t;
    ptArgProd->nthreads = nthreadsProd;
    ptArgProd->N = N;

    /* Inicializa a thread */
    if (pthread_create(&tid[t], NULL, rotina_produtora, (void*)ptArgProd)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
    }
  }

  /* Cria as Consumidoras, que buscam na árvore */
  for(long int t=nthreadsProd; t < (nthreadsProd + nthreadsCons) ; t++) {
    /* Cria o argumento */
    ptArgCons = (dados_thread_consumidora_t*) malloc(sizeof(dados_thread_consumidora_t));
    CHECK_MALLOC(ptArgCons);

    ptArgCons->tid = &tid[t];
    ptArgCons->raiz = raiz;
    ptArgCons->idThread = t;
    ptArgCons->nthreads = nthreadsProd;
    ptArgCons->numBuscas = numBuscas;

    /* Inicializa a thread */
    if (pthread_create(&tid[t], NULL, rotina_consumidora, (void*)ptArgCons)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
    }
  }

  /* Espera o fim das produtoras e consumidoras */
  for (int t=0; t < nthreadsProd; t++) {  // Produtoras
    if (pthread_join(tid[t], (void**) &retsCons[t])) {
      printf("--ERRO: pthread_join() \n"); exit(-1); 
    } 
  }
  for (int t=nthreadsProd; t < (nthreadsProd + nthreadsCons); t++) {  // Consumidoras
    if (pthread_join(tid[t], (void**) &retsProd[t - nthreadsProd])) {
      printf("--ERRO: pthread_join() \n"); exit(-1); 
    } 
  }
  GET_TIME(fim);
  printf("  Fim das Threads em %lf\n", fim - inicioTotal);


  printf("\n");
  printf("RESUMO THREADS\n");
  printf("--------------\n");
  for (int t = 0; t < nthreadsProd; t++) { // Produtoras
    printf("Thread #%ld\n", retsProd[t]->idThread);
    printf("  Gerados e inseridos: %lld\n", retsProd[t]->qtAmostras);
  }
  for (int t=nthreadsProd; t < (nthreadsProd + nthreadsCons); t++) {  // Consumidoras
    printf("Thread #%ld\n", retsCons[t - nthreadsProd]->idThread);
    printf("  Buscas realizadas:    %ld\n", retsCons[t - nthreadsProd]->qtBuscasRealizadas);
    printf("  Amostras encontradas: %lld\n", retsCons[t - nthreadsProd]->qtAmostrasEncontradas);
  }

  printf("\n");
  printf("RESUMO PROGRAMA\n");
  printf("---------------\n");
  printf("  Tempo de execução concorrente:  %lf\n", fim - inicioAlvo);
  printf("  Tempo total do programa:        %lf\n", fim - inicioTotal);

  free(tid);
  free(ptArgProd);
  free(ptArgCons);
  for (int i = 0; i < nthreadsProd; i++) free(retsProd[i]);
  for (int i = 0; i < nthreadsCons; i++) free(retsCons[i]);
  free(retsProd);
  free(retsCons);
  destroiNo(raiz);
  return 0;
}
