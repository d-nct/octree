/**
 * @file sequencial.c
 * 
 * Implementação de testes para a versão sequencial da NocTree.
 */

/* Sessão de Importações
 * --------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../src/noctree.h"
/*#include "../src/system.h" // Para NOCTREE_CAPACIDADE*/

/* Ambiente de Testes */
int total_testes = 0;
int testes_passaram = 0;

#define ASSERT(condicao) \
  do { \
    total_testes++; \
    if (condicao) { \
      testes_passaram++; \
    } else { \
      fprintf(stdout, " FALHA: %s:%d | Condição: %s\n", __FILE__, __LINE__, #condicao); \
    } \
  } while (0)

void print_sumario_testes() {
  printf("\n--- SUMÁRIO DOS TESTES ---\n");
  printf("Total de Testes: %d\n", total_testes);
  printf("Testes que Passaram: %d\n", testes_passaram);
  printf("Testes que Falharam: %d\n", total_testes - testes_passaram);
  printf("--------------------------\n");
}


/* Definição das funções auxiliares */

/**
 * Verifica se um ponto específico existe na lista de pontos de um nó folha
 */ 
bool encontra_ponto_no_no(noctree* no, amostra* ponto) {
  if (no == NULL || no->subdividido) return false;
  for (int i = 0; i < no->qtPontos; i++) {
    if (no->pontos[i] == ponto) {
      return true;
    }
  }
  return false;
}

/* Casos de Teste
*  -------------- */

void test_inicializacao() {
  printf("Executando Teste 1: Inicialização do Nó... \n");
  float tamanho[] = {100.0f, 100.0f, 100.0f};
  amostra *amostraZero = inicializaAmostra(0,0,0);
  noctree* no = inicializaNo(amostraZero, tamanho);

  ASSERT(no != NULL);
  ASSERT(no->qtPontos == 0);
  ASSERT(no->subdividido == false);
  for (int i = 0; i < QT_FILHOS_NOCTREE; i++) {
    ASSERT(no->filhos[i] == NULL);
  }

  destroiNo(no);
}

void test_insercao_simples() {
  printf("Executando Teste 2: Inserção Simples (sem subdivisão)...\n");
  float tamanho[] = {100.0f, 100.0f, 100.0f};
  noctree* no = inicializaNo(inicializaAmostra(0, 0, 0), tamanho);

  amostra* p1 = inicializaAmostra(10, 10, 10);
  amostra* p2 = inicializaAmostra(-10, -10, -10);

  insereAmostra(no, p1);
  insereAmostra(no, p2);

  ASSERT(no->qtPontos == 2);
  ASSERT(no->subdividido == false);
  ASSERT(no->pontos[0] == p1);
  ASSERT(no->pontos[1] == p2);

  destroiNo(no);
}

void test_subdivisao_e_redistribuicao() {
  printf("Executando Teste 3: Subdivisão e Redistribuição Simples de Pontos...\n");
  float tamanho[] = {100.0f, 100.0f, 100.0f};
  noctree* no = inicializaNo(inicializaAmostra(0, 0, 0), tamanho);
  amostra* pontos[NOCTREE_CAPACIDADE + 1];

  // Cria NOCTREE_CAPACIDADE + 1 pontos
  for (int i = 0; i < NOCTREE_CAPACIDADE + 1; i++) {
    // Cria pontos em octantes diferentes para testar a redistribuição
    float x = (i % 2 == 0) ? i * 2.0f : -i * 2.0f;
    float y = (i % 3 == 0) ? i * 2.0f : -i * 2.0f;
    float z = (i % 4 == 0) ? i * 2.0f : -i * 2.0f;
    pontos[i] = inicializaAmostra(x, y, z);
    insereAmostra(no, pontos[i]);
  }

  ASSERT(no->subdividido == true);
  ASSERT(no->qtPontos == 0); // Pontos do pai foram movidos

  int pontos_nos_filhos = 0;
  for (int i = 0; i < QT_FILHOS_NOCTREE; i++) {
    ASSERT(no->filhos[i] != NULL);
    if(no->filhos[i] != NULL) {
      pontos_nos_filhos += no->filhos[i]->qtPontos;
    }
  }
  ASSERT(pontos_nos_filhos == NOCTREE_CAPACIDADE + 1);

  destroiNo(no);
}

void test_subdivisao_recursiva() {
  printf("Executando Teste 4: Subdivisão Recursiva...\n");
  float tamanho[] = {100.0f, 100.0f, 100.0f};
  noctree* raiz = inicializaNo(inicializaAmostra(0, 0, 0), tamanho);

  // --- Parte 1: Inserção Manual dos Pontos ---

  /* Esse cara cai no filho 0 */
  amostra* p_solitario = inicializaAmostra(-1, -1, -1);

  // Estes 11 pontos serão direcionados para o filho 7.
  // Os 10 primeiros irão preenchê-lo, e o 11º forçará sua subdivisão.
  // As coordenadas foram escolhidas para se distribuirem entre os netos.
  amostra* p1 = inicializaAmostra(10, 10, 10); // irá para o neto 0 (do filho 7)
  amostra* p2 = inicializaAmostra(40, 10, 10); // irá para o neto 1
  amostra* p3 = inicializaAmostra(10, 40, 10); // irá para o neto 2
  amostra* p4 = inicializaAmostra(40, 40, 10); // irá para o neto 3
  amostra* p5 = inicializaAmostra(10, 10, 40); // irá para o neto 4
  amostra* p6 = inicializaAmostra(40, 10, 40); // irá para o neto 5
  amostra* p7 = inicializaAmostra(10, 40, 40); // irá para o neto 6
  amostra* p8 = inicializaAmostra(45, 45, 45); // irá para o neto 7
  amostra* p9 = inicializaAmostra(15, 15, 15); // irá para o neto 0
  amostra* p10 = inicializaAmostra(5, 5, 5);   // irá para o neto 0 (10º ponto no filho 7)
  amostra* p11_gatilho = inicializaAmostra(2, 2, 2); // irá para o neto 0 (11º ponto, força a subdivisão do filho 7)

  // A inserção ocorre na raiz para todos os pontos
  insereAmostra(raiz, p1);
  insereAmostra(raiz, p2);
  insereAmostra(raiz, p3);
  insereAmostra(raiz, p4);
  insereAmostra(raiz, p5);
  insereAmostra(raiz, p6);
  insereAmostra(raiz, p7);
  insereAmostra(raiz, p8);
  insereAmostra(raiz, p9);
  insereAmostra(raiz, p10);
  // Até aqui, a raiz tem 10 pontos. O próximo a subdivide.
  insereAmostra(raiz, p_solitario);
  // Agora a raiz está subdividida. O próximo ponto vai para o filho 7 e o enche.
  insereAmostra(raiz, p11_gatilho);


  // --- Parte 2: Verificação Manual da Posição de Cada Ponto ---

  LOGP("Verificando a estrutura da árvore..."); ENDL;
  // A raiz deve estar subdividida
  ASSERT(raiz->subdividido == true);

  /* O filho 0 deve existir e conter o p_solitario, pois não foi subdividido */
  noctree* filho_0 = raiz->filhos[0];
  ASSERT(filho_0 != NULL);
  ASSERT(filho_0->subdividido == false);
  ASSERT(encontra_ponto_no_no(filho_0, p_solitario));

  /* O filho 7 deve existir e ter sido subdividido */
  noctree* filho_7 = raiz->filhos[7];
  ASSERT(filho_7 != NULL);
  ASSERT(filho_7->subdividido == true);
  ASSERT(filho_7->qtPontos == 0); // Vazio, pois redistribuiu para os netos

  LOGP("Verificando a posição de cada ponto nos netos...");ENDL;
  noctree* neto_0 = filho_7->filhos[0];
  noctree* neto_1 = filho_7->filhos[1];
  noctree* neto_2 = filho_7->filhos[2];
  noctree* neto_3 = filho_7->filhos[3];
  noctree* neto_4 = filho_7->filhos[4];
  noctree* neto_5 = filho_7->filhos[5];
  noctree* neto_6 = filho_7->filhos[6];
  noctree* neto_7 = filho_7->filhos[7];

  /* Verifica se o ponto que deveria cair no nó, caiu */
  ASSERT(neto_0 != NULL);
  ASSERT(neto_1 != NULL);
  ASSERT(neto_2 != NULL);
  ASSERT(neto_3 != NULL);
  ASSERT(neto_4 != NULL);
  ASSERT(neto_5 != NULL);
  ASSERT(neto_6 != NULL);
  ASSERT(neto_7 != NULL);

  /* Verifica se cada ponto caiu no lugar certo */
  ASSERT(encontra_ponto_no_no(neto_0, p1));
  ASSERT(encontra_ponto_no_no(neto_1, p2));
  ASSERT(encontra_ponto_no_no(neto_2, p3));
  ASSERT(encontra_ponto_no_no(neto_3, p4));
  ASSERT(encontra_ponto_no_no(neto_4, p5));
  ASSERT(encontra_ponto_no_no(neto_5, p6));
  ASSERT(encontra_ponto_no_no(neto_6, p7));
  ASSERT(encontra_ponto_no_no(neto_7, p8));
  ASSERT(encontra_ponto_no_no(neto_0, p9));
  ASSERT(encontra_ponto_no_no(neto_0, p10));
  ASSERT(encontra_ponto_no_no(neto_0, p11_gatilho));

  /* E a contagem de pontos */
  ASSERT(neto_0->qtPontos == 4);
  ASSERT(neto_1->qtPontos == 1);
  ASSERT(neto_2->qtPontos == 1);
  ASSERT(neto_3->qtPontos == 1);
  ASSERT(neto_4->qtPontos == 1);
  ASSERT(neto_5->qtPontos == 1);
  ASSERT(neto_6->qtPontos == 1);
  ASSERT(neto_7->qtPontos == 1);

  destroiNo(raiz);
}


// --- Função Principal ---

int main(void) {
  printf("=======================================\n");
  printf("  INICIANDO TESTES DA OCTREE SEQUENCIAL \n");
  printf("=======================================\n");

  /* Roda os testes */
  test_inicializacao();
  test_insercao_simples();
  test_subdivisao_e_redistribuicao();
  test_subdivisao_recursiva();

  /* Interface */
  print_sumario_testes();

  return total_testes == testes_passaram;
}
