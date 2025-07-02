/**
 * @file Arquivo fonte para analisar o desempenho de Octree
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

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
} dados_thread_escritora_t;

typedef struct {
  pthread_t* tid;
  long int idThread;
  long long int qtAmostras;
} ret_thread_escritora_t;

/* Função que será executada pela thread escritora */
void* rotina_escritora(void* arg) {
  dados_thread_escritora_t* dados = (dados_thread_escritora_t*)arg;

  /* Calcula quantas amostras precisa inserir */
  int qtAmostrasAGerar = dados->N / dados->nthreads;
  if (dados->idThread == 0) qtAmostrasAGerar += dados->N % dados->nthreads; // A última fica com o resto, se houver

  /* Insere as amostras */
  preencheOctreeUnif(dados->raiz, qtAmostrasAGerar);

  /* Gera o retorno */
  ret_thread_escritora_t* ret = (ret_thread_escritora_t*) malloc(sizeof(ret_thread_escritora_t));
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
} dados_thread_leitora_t;

typedef struct {
  pthread_t* tid;
  long int idThread;
  long long int qtAmostrasEncontradas;
  long int qtBuscasRealizadas;
} ret_thread_leitora_t;

/* Função que será executada pela thread escritora */
void* rotina_leitora(void* arg) {
  dados_thread_leitora_t* dados = (dados_thread_leitora_t*)arg;
  amostra* ptAmostra;
  int qt_encontrados; // Retorno da busca

  /* Gera o retorno */
  ret_thread_leitora_t* ret = (ret_thread_leitora_t*) malloc(sizeof(ret_thread_leitora_t));
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
  int nthreadsEsc, nthreadsLeit;   // Qt de threads (passada linha de comando)
  long long int N;                 // Qt de pontos na octree
  long long int numBuscas;         // Qt de buscas a serem realizadas na octree
  pthread_t *tid;                  // Identificadores das threads no sistema
  double inicio, fim;              // Marcacoes de tempo
  double t_setup, t_criacao, t_busca; // Tomadas de tempo

  /* Variáveis da árvore */
  noctree* raiz = inicializaNo(inicializaAmostra(0,0,0), (float[]){100,100,100}, 0);
  dados_thread_escritora_t* ptArgEsc;
  ret_thread_escritora_t** retsEsc;
  dados_thread_leitora_t* ptArgLeit;
  ret_thread_leitora_t** retsLeit;
  
  printf("Tomadas de Tempo:\n");

  GET_TIME(inicio);

  //--le e avalia os parametros de entrada
  if(argc<5) {
    printf("Digite: %s <numero de threads escritoras> <nthreads leitoras> <N> <numBuscas>\n", argv[0]);
    return 1;
  }
  nthreadsEsc = atoi(argv[1]);
  nthreadsLeit = atoi(argv[2]);
  N = atoll(argv[3]);
  numBuscas = atoll(argv[4]);

  //--aloca as estruturas
  tid = (pthread_t*) malloc(sizeof(pthread_t)*(nthreadsEsc + nthreadsLeit));
  CHECK_MALLOC(tid);

  retsEsc = (ret_thread_escritora_t**) malloc(sizeof(ret_thread_escritora_t*) * nthreadsEsc);
  CHECK_MALLOC(retsEsc);

  retsLeit = (ret_thread_leitora_t**) malloc(sizeof(ret_thread_leitora_t*) * nthreadsLeit);
  CHECK_MALLOC(retsLeit);

  //--cria as threads
  GET_TIME(fim);
  t_setup = fim - inicio;
  printf("  Início da criação das threads em %lf\n", t_setup);

  /* Cria as Escritoras, que geram a árvore */
  for(long int t=0; t < nthreadsEsc ; t++) {
    /* Cria o argumento */
    ptArgEsc = (dados_thread_escritora_t*) malloc(sizeof(dados_thread_escritora_t));
    CHECK_MALLOC(ptArgEsc);

    ptArgEsc->tid = &tid[t];
    ptArgEsc->raiz = raiz;
    ptArgEsc->idThread = t;
    ptArgEsc->nthreads = nthreadsEsc;
    ptArgEsc->N = N;

    /* Inicializa a thread */
    if (pthread_create(&tid[t], NULL, rotina_escritora, (void*)ptArgEsc)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
    }
  }

  /* Espera o fim das escritoras */
  for (int t=0; t < nthreadsEsc; t++) {
    if (pthread_join(tid[t], (void**) &retsEsc[t])) {
      printf("--ERRO: pthread_join() \n"); exit(-1);
    }
  }

  GET_TIME(fim);
  t_criacao = fim - inicio - t_setup; // É o delta
  printf("  Início das buscas em %lf\n", t_criacao);

  /* Cria as Leitoras, que buscam na árvore */
  for(long int t=nthreadsEsc; t < (nthreadsEsc + nthreadsLeit) ; t++) {
    /* Cria o argumento */
    ptArgLeit = (dados_thread_leitora_t*) malloc(sizeof(dados_thread_leitora_t));
    CHECK_MALLOC(ptArgLeit);

    ptArgLeit->tid = &tid[t];
    ptArgLeit->raiz = raiz;
    ptArgLeit->idThread = t;
    ptArgLeit->nthreads = nthreadsLeit;
    ptArgLeit->numBuscas = numBuscas;

    /* Inicializa a thread */
    if (pthread_create(&tid[t], NULL, rotina_leitora, (void*)ptArgLeit)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
    }
  }

  /* Espera o fim das leitoras */
  for (int t=nthreadsEsc; t < (nthreadsEsc + nthreadsLeit); t++) {
    if (pthread_join(tid[t], (void**) &retsLeit[t - nthreadsEsc])) {
      printf("--ERRO: pthread_join() \n"); exit(-1);
    }
  }

  GET_TIME(fim);
  t_busca = fim - t_criacao - t_setup - inicio;
  fim = fim - inicio;
  printf("  Fim das buscas em %lf\n", fim);


  printf("\n");
  printf("RESUMO THREADS\n");
  printf("--------------\n");
  for (int t = 0; t < nthreadsEsc; t++) { // escritoras
    printf("Thread #%ld\n", retsEsc[t]->idThread);
    printf("  Gerados e inseridos: %lld\n", retsEsc[t]->qtAmostras);
  }
  for (int t=nthreadsEsc; t < (nthreadsEsc + nthreadsLeit); t++) {  // Leitoras
    printf("Thread #%ld\n", retsLeit[t - nthreadsEsc]->idThread);
    printf("  Buscas realizadas:    %ld\n", retsLeit[t - nthreadsEsc]->qtBuscasRealizadas);
    printf("  Amostras encontradas: %lld\n", retsLeit[t - nthreadsEsc]->qtAmostrasEncontradas);
  }

  printf("\n");
  printf("RESUMO PROGRAMA\n");
  printf("---------------\n");
  printf("  Tempo de setup do programa:  %lf seg\n", t_setup);
  printf("  Tempo de criacao da octree:  %lf seg\n", t_criacao);
  printf("  Tempo de buscas na octree:   %lf seg\n", t_busca);
  printf("  Tempo total do programa:     %lf seg\n", fim);

  free(tid);
  free(ptArgEsc);
  free(ptArgLeit);
  for (int i = 0; i < nthreadsEsc; i++) free(retsEsc[i]);
  for (int i = 0; i < nthreadsLeit; i++) free(retsLeit[i]);
  free(retsEsc);
  free(retsLeit);
  destroiNo(raiz);
  return 0;
}
