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
} dados_thread_escrita_t;

typedef struct {
  pthread_t* tid;
  long int idThread;
  double tempoParcial;
  long long int qtAmostras;
} ret_thread_escrita_t;

/* Função que será executada pela thread escritora */
void* rotina_escritora(void* arg) {
  dados_thread_escrita_t* dados = (dados_thread_escrita_t*)arg;
  double inicio, fim;

LOGP("%s: Tentando inserir o ponto (%.1f, %.1f, %.1f)", dados->nome_thread, dados->ponto->x, dados->ponto->y, dados->ponto->z); ENDL;

  /* Calcula quantas amostras precisa inserir */
  int qtAmostrasAGerar = dados->N / dados->nthreads;
  if (dados->idThread == 1) qtAmostrasAGerar += dados->N % dados->nthreads; // A última fica com o resto, se houver

  /* Insere as amostras */
  GET_TIME(inicio);
  preencheOctreeUnif(dados->raiz, qtAmostrasAGerar);
  GET_TIME(fim);

  /* Gera o retorno */
  ret_thread_escrita_t* ret = (ret_thread_escrita_t*) malloc(sizeof(ret_thread_escrita_t));
  CHECK_MALLOC(ret);

  ret->tid = dados->tid;
  ret->idThread = dados->idThread;
  ret->tempoParcial = fim - inicio;
  ret->qtAmostras = qtAmostrasAGerar;

  LOGP("%s: Inserção concluída.", dados->nome_thread); ENDL;
  pthread_exit((void*) ret);
}

int main(int argc, char *argv[]) {
  srand(time(NULL));
  int nthreads;                    // Qt de threads (passada linha de comando)
  long long int N;                 // Qt de pontos na octree
  double inicio, fim, delta, total = 0; // Marcações de tempo
  pthread_t *tid;                  // Identificadores das threads no sistema
  noctree* raiz = inicializaNo(inicializaAmostra(0,0,0), (float[]){100,100,100}, 0);
  dados_thread_escrita_t* ptArgEscrita;
  ret_thread_escrita_t* rets;
  double tempoTotalThreads;
  

  printf("Tomadas de Tempo:\n");

  GET_TIME(inicio)

  //--le e avalia os parametros de entrada
  if(argc<3) {
    printf("Digite: %s <numero de threads> <N>\n", argv[0]);
    return 1;
  }
  nthreads = atoi(argv[1]);
  N = atoll(argv[2]);

  //--aloca as estruturas
  tid = (pthread_t*) malloc(sizeof(pthread_t)*(nthreads+1));
  CHECK_MALLOC(tid);

  rets = (ret_thread_escrita_t*) malloc(sizeof(ret_thread_escrita_t) * nthreads);
  CHECK_MALLOC(rets);

  //--cria as threads
  GET_TIME(fim);
  delta = fim - inicio;
  printf("  Início da criação das threads em %lf\n", fim);
  total += delta;

  GET_TIME(inicio)
  for(long int t=0; t<nthreads; t++) {
    /* Cria o argumento */
    ptArgEscrita = (dados_thread_escrita_t*) malloc(sizeof(dados_thread_escrita_t));
    CHECK_MALLOC(ptArgEscrita);

    ptArgEscrita->tid = &tid[t];
    ptArgEscrita->raiz = raiz;
    ptArgEscrita->idThread = t;
    ptArgEscrita->nthreads = nthreads;
    ptArgEscrita->N = N;

    /* Inicializa a thread */
    if (pthread_create((void*)ptArgEscrita, NULL, rotina_escritora, (void *)t)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
    }
  }

  GET_TIME(fim);
  delta = fim - inicio;
  printf("  Fim da criação das Threads em %lf\n", fim);
  total += delta;

  //--espera todas as threads terminarem
  GET_TIME(inicio)
  for (int t=0; t<nthreads; t++) {
    if (pthread_join(tid[t], (void**) &rets[t])) {
      printf("--ERRO: pthread_join() \n"); exit(-1); 
    } 
  }
  GET_TIME(fim);
  delta = fim - inicio;
  printf("  Fim das Threads em %lf\n", fim);
  total += delta;


  printf("\n");
  printf("RESUMO THREADS\n");
  printf("--------------\n");
  for (int t = 0; t < nthreads; t++) {
    printf("Thread #%ld\n", rets[t].idThread);
    printf("  Gerados e inseridos: %lld\n", rets[t].qtAmostras);
    printf("  Tempo parcial: %lf\n", rets[t].tempoParcial);
    tempoTotalThreads += rets[t].tempoParcial;
  }

  printf("\n");
  printf("RESUMO PROGRAMA\n");
  printf("---------------\n");
  printf("Tempo de processamento das threads: %lf", tempoTotalThreads);
  printf("Tempo total do programa:            %lf", total);

  destroiNo(raiz);
  free(rets);

  return 0;
}
