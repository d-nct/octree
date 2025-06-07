/**
 * @file Arquivo com testes para a versão concorrente da implementação.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h> // Para a função sleep()

#include "framework.h"
#include "../src/noctree.h"

/* Variáveis do framework de testes */
extern int total_testes;
extern int testes_passaram;


/* Funções e Estruturas Auxiliares para Testes Concorrentes
*  -------------------------------------------------------- */

/* Argumento da thread */
typedef struct {
  noctree* raiz;
  amostra* ponto;
  const char* nome_thread;
} dados_thread_escrita_t;

/* Função que será executada pela thread escritora */
void* rotina_escritora(void* arg) {
  dados_thread_escrita_t* dados = (dados_thread_escrita_t*)arg;
  LOGP("%s: Tentando inserir o ponto (%.1f, %.1f, %.1f)", dados->nome_thread, dados->ponto->x, dados->ponto->y, dados->ponto->z); ENDL;
  insereAmostra(dados->raiz, dados->ponto);
  LOGP("%s: Inserção concluída.", dados->nome_thread); ENDL;
  return NULL;
}

/* Função auxiliar para ver se um ponto existe na árvore */
bool encontra_ponto(noctree* no, amostra* ponto_busca) {
  int qt_encontrados = 0;
  amostra** pontos_na_folha = buscaNaFolha(no, ponto_busca, &qt_encontrados);
  if (pontos_na_folha == NULL) return false;

  LOGP("  Procurando pt em %d encontrados. Achei na ", qt_encontrados);
  bool encontrado = false;
  for (int i = 0; i < qt_encontrados; i++) {
    if ((pontos_na_folha[i]->x == ponto_busca->x) &&
        (pontos_na_folha[i]->y == ponto_busca->y) &&
        (pontos_na_folha[i]->z == ponto_busca->z)) {

      encontrado = true;
      LOGP("%d-ésima tentantiva.", i); ENDL;
      break;
    }
  }

  free(pontos_na_folha);
  return encontrado;
}


/* INÍCIO DOS CASOS DE TESTE
*  ------------------------- */

// --- Testes de CORRETUDE (Single-Thread) ---

void test_corretude_insercao_simples() {
  printf("Executando Teste 1: Corretude - Inserção Simples com Lock...\n");
  noctree* raiz = inicializaNo(inicializaAmostra(0,0,0), (float[]){100,100,100});

  amostra* p1 = inicializaAmostra(10, 20, 30);
  insereAmostra(raiz, p1);

  ASSERT(raiz->qtPontos == 1);
  ASSERT(raiz->pontos[0] == p1);

  destroiNo(raiz);
}

void test_corretude_busca_em_folha() {
  printf("Executando Teste 2: Corretude - Busca em Folha com Lock...\n");
  noctree* raiz = inicializaNo(inicializaAmostra(0,0,0), (float[]){100,100,100});
  amostra* p1 = inicializaAmostra(10, 10, 10);
  amostra* p2 = inicializaAmostra(20, 20, 20);
  insereAmostra(raiz, p1);
  insereAmostra(raiz, p2);

  int qt_encontrados = 0;
  amostra** resultados = buscaNaFolha(raiz, p1, &qt_encontrados);

  ASSERT(qt_encontrados == 2);
  ASSERT(resultados != NULL);
  if (resultados) {
    bool p1_encontrado = (resultados[0] == p1 || resultados[1] == p1);
    bool p2_encontrado = (resultados[0] == p2 || resultados[1] == p2);
    ASSERT(p1_encontrado && p2_encontrado);
    free(resultados);
  }

  destroiNo(raiz);
}

// --- Testes de THREAD-SAFENESS (Multi-Thread) ---

void test_safeness_escritores_concorrentes() {
  printf("Executando Teste 3: Thread-Safety - Dois Escritores Concorrentes...\n");
  noctree* raiz = inicializaNo(inicializaAmostra(0,0,0), (float[]){100,100,100});
  pthread_t th1, th2;

  amostra* p_th1 = inicializaAmostra(25, 25, 25);
  amostra* p_th2 = inicializaAmostra(-25, -25, -25);

  dados_thread_escrita_t dados1 = {raiz, p_th1, "Thread 1 (Escritor)"};
  dados_thread_escrita_t dados2 = {raiz, p_th2, "Thread 2 (Escritor)"};

  pthread_create(&th1, NULL, rotina_escritora, &dados1);
  pthread_create(&th2, NULL, rotina_escritora, &dados2);

  pthread_join(th1, NULL);
  pthread_join(th2, NULL);

  LOGP("Ambas as threads terminaram. Verificando o estado final da árvore..."); ENDL;

  ASSERT(raiz->subdividido == false);
  ASSERT(encontra_ponto(raiz, p_th1));
  ASSERT(encontra_ponto(raiz, p_th2));

  destroiNo(raiz);
}

void* rotina_leitora(void* arg) {
  dados_thread_escrita_t* dados = (dados_thread_escrita_t*)arg;
  LOGP("%s: Iniciando buscas repetidas...", dados->nome_thread); ENDL;
  int qt = 0;
  // Faz 5 buscas para aumentar a chance de conflito com o escritor
  for(int i = 0; i < 5; i++) {
    amostra** res = buscaNaFolha(dados->raiz, dados->ponto, &qt);
    if (res) free(res);
    usleep(10000); // Pequena pausa
  }
  LOGP("%s: Buscas concluídas.", dados->nome_thread); ENDL;
  return (void*)(intptr_t)1; // Retorna 1 para indicar sucesso
}

void test_safeness_leitor_e_escritor() {
  printf("Executando Teste 4: Thread-Safety - Leitores e um Escritor Concorrentes...\n");
  noctree* raiz = inicializaNo(inicializaAmostra(0,0,0), (float[]){100,100,100});
  pthread_t th_escritor, th_leitor1, th_leitor2;

  // Pré-popula a árvore
  amostra* p_inicial = inicializaAmostra(1,1,1);
  insereAmostra(raiz, p_inicial);

  // Dados para as threads
  amostra* p_escritor = inicializaAmostra(2,2,2);
  dados_thread_escrita_t dados_escritor = {raiz, p_escritor, "Thread ESCRITOR"};
  dados_thread_escrita_t dados_leitor1 = {raiz, p_inicial, "Thread LEITOR 1"};
  dados_thread_escrita_t dados_leitor2 = {raiz, p_inicial, "Thread LEITOR 2"};

  // Dispara as threads leitoras primeiro
  pthread_create(&th_leitor1, NULL, rotina_leitora, &dados_leitor1);
  pthread_create(&th_leitor2, NULL, rotina_leitora, &dados_leitor2);
  usleep(5000); // Dá uma pequena chance para as leitoras começarem
  pthread_create(&th_escritor, NULL, rotina_escritora, &dados_escritor);

  void* leitor1_status, *leitor2_status;
  pthread_join(th_escritor, NULL);
  pthread_join(th_leitor1, &leitor1_status);
  pthread_join(th_leitor2, &leitor2_status);

  LOGP("Todas as threads terminaram. Verificando o estado final..."); ENDL;
  ASSERT(raiz->qtPontos == 2);
  ASSERT(encontra_ponto(raiz, p_inicial));
  ASSERT(encontra_ponto(raiz, p_escritor));
  ASSERT((intptr_t)leitor1_status == 1 && (intptr_t)leitor2_status == 1); // Verifica se as leitoras retornaram sucesso

  destroiNo(raiz);
}

void test_subdivisao_concorrente_manual() {
  printf("Executando Teste 5: Thread-Safety - Subdivisão Concorrente Manual...\n");
  noctree* raiz = inicializaNo(inicializaAmostra(0,0,0), (float[]){100,100,100});
  // NOCTREE_CAPACIDADE é 10. Vamos usar 11 threads para forçar a subdivisão.
  pthread_t threads[11];
  dados_thread_escrita_t dados[11];
  amostra* pontos[11];

  // Criando os pontos manualmente
  pontos[0]  = inicializaAmostra(1,1,1);    // filho 7
  pontos[1]  = inicializaAmostra(2,2,2);    // filho 7
  pontos[2]  = inicializaAmostra(-3,-3,-3); // filho 0
  pontos[3]  = inicializaAmostra(4,4,-4);   // filho 4
  pontos[4]  = inicializaAmostra(-5,5,-5);  // filho 5
  pontos[5]  = inicializaAmostra(6,-6,6);   // filho 2
  pontos[6]  = inicializaAmostra(-7,-7,7);  // filho 3
  pontos[7]  = inicializaAmostra(8,-8,-8);  // filho 1
  pontos[8]  = inicializaAmostra(9,9,9);    // filho 7
  pontos[9]  = inicializaAmostra(10,10,10); // filho 7
  pontos[10] = inicializaAmostra(-11,-11,-11); // Força a subdivisão. filho 0

  // Disparando 11 threads, cada uma para inserir um ponto
  for (int i = 0; i < 11; i++) {
    dados[i] = (dados_thread_escrita_t){raiz, pontos[i], "Thread"};
    pthread_create(&threads[i], NULL, rotina_escritora, &dados[i]);
  }

  // Esperando todas as threads terminarem
  for (int i = 0; i < 11; i++) {
    pthread_join(threads[i], NULL);
  }

  LOGP("Todas as threads de inserção terminaram. Verificando..."); ENDL;
  ASSERT(raiz->subdividido == true);
  ASSERT(raiz->qtPontos == 0); // O nó raiz deve estar vazio

  // Verifica a qt de pts em cada filho
  ASSERT((raiz->filhos[0])->qtPontos == 2); LOGP("Achei: %d pts no filho 0. Queria 2", (raiz->filhos[0])->qtPontos); ENDL;
  ASSERT((raiz->filhos[1])->qtPontos == 1); LOGP("Achei: %d pts no filho 1. Queria 1", (raiz->filhos[1])->qtPontos); ENDL;
  ASSERT((raiz->filhos[2])->qtPontos == 1); LOGP("Achei: %d pts no filho 2. Queria 1", (raiz->filhos[2])->qtPontos); ENDL;
  ASSERT((raiz->filhos[3])->qtPontos == 1); LOGP("Achei: %d pts no filho 3. Queria 1", (raiz->filhos[3])->qtPontos); ENDL;
  ASSERT((raiz->filhos[4])->qtPontos == 1); LOGP("Achei: %d pts no filho 4. Queria 1", (raiz->filhos[4])->qtPontos); ENDL;
  ASSERT((raiz->filhos[5])->qtPontos == 1); LOGP("Achei: %d pts no filho 5. Queria 1", (raiz->filhos[5])->qtPontos); ENDL;
  ASSERT((raiz->filhos[6])->qtPontos == 0); LOGP("Achei: %d pts no filho 6. Queria 0", (raiz->filhos[6])->qtPontos); ENDL;
  ASSERT((raiz->filhos[7])->qtPontos == 4); LOGP("Achei: %d pts no filho 7. Queria 4", (raiz->filhos[7])->qtPontos); ENDL;

  // Verifica manualmente se todos os 11 pontos estão na árvore
  ASSERT(encontra_ponto(raiz, pontos[0]));
  ASSERT(encontra_ponto(raiz, pontos[1]));
  ASSERT(encontra_ponto(raiz, pontos[2]));
  ASSERT(encontra_ponto(raiz, pontos[3]));
  ASSERT(encontra_ponto(raiz, pontos[4]));
  ASSERT(encontra_ponto(raiz, pontos[5]));
  ASSERT(encontra_ponto(raiz, pontos[6]));
  ASSERT(encontra_ponto(raiz, pontos[7]));
  ASSERT(encontra_ponto(raiz, pontos[8]));
  ASSERT(encontra_ponto(raiz, pontos[9]));
  ASSERT(encontra_ponto(raiz, pontos[10]));

  destroiNo(raiz);
}


// =========== FUNÇÃO PRINCIPAL ===========

int main() {
  printf("========================================\n");
  printf(" INICIANDO TESTES DA OCTREE CONCORRENTE \n");
  printf("========================================\n");

  /* Executa os testes */
  test_corretude_insercao_simples();
  test_corretude_busca_em_folha();
  test_safeness_escritores_concorrentes();
  test_safeness_leitor_e_escritor();
  test_subdivisao_concorrente_manual();

  /* Interface com o usuário */
  print_sumario_testes();
  return total_testes == testes_passaram;
}
