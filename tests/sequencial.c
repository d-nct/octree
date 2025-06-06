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
      fprintf(stderr, " FALHA: %s:%d | Condição: %s\n", __FILE__, __LINE__, #condicao); \
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
  printf("Executando Teste 3 e 4: Subdivisão e Redistribuição de Pontos...\n");
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
  printf("Executando Teste 5: Subdivisão Recursiva...\n");
  float tamanho[] = {100.0f, 100.0f, 100.0f};
  noctree* no = inicializaNo(inicializaAmostra(0, 0, 0), tamanho);

  // Insere NOCTREE_CAPACIDADE + 1 pontos todos no mesmo octante (octante 7: +x, +y, +z)
  for (int i = 0; i < NOCTREE_CAPACIDADE + 1; i++) {
    insereAmostra(no, inicializaAmostra(i + 1.0f, i + 1.0f, i + 1.0f));
  }

  ASSERT(no->subdividido == true);

  // O filho que contém todos os pontos (octante 7) também deve estar subdividido
  noctree* filho_alvo = no->filhos[7];
  ASSERT(filho_alvo != NULL);
  ASSERT(filho_alvo->subdividido == true);
  ASSERT(filho_alvo->qtPontos == 0);

  // Verifica se os pontos foram para os netos
  int pontos_nos_netos = 0;
  for(int i = 0; i < QT_FILHOS_NOCTREE; i++) {
    if(filho_alvo->filhos[i] != NULL) {
      pontos_nos_netos += filho_alvo->filhos[i]->qtPontos;
    }
  }
  printf("Pts nos netos é %d, mas devia ser %d\n", pontos_nos_netos, NOCTREE_CAPACIDADE + 1);
  ASSERT(pontos_nos_netos == NOCTREE_CAPACIDADE + 1);

  destroiNo(no);
}


// --- Função Principal ---

int main(void) {
  printf("=======================================\n");
  printf("  INICIANDO TESTES DA OCTREE SEQUENCIAL \n");
  printf("=======================================\n");

  // Lembrete importante
  printf("\nAVISO: Estes testes assumem que as correções da análise anterior foram aplicadas.\n\n");

  test_inicializacao();
  test_insercao_simples();
  test_subdivisao_e_redistribuicao();
  test_subdivisao_recursiva();

  print_sumario_testes();

  return total_testes == testes_passaram;
}
