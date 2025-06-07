#include "framework.h"

int total_testes = 0;
int testes_passaram = 0;

void print_sumario_testes() {
    printf("\n--- SUMÁRIO DOS TESTES ---\n");
    printf("Total de Testes:     %d\n", total_testes);
    printf("Testes que Passaram: %d\n", testes_passaram);
    printf("Testes que Falharam: %d\n", total_testes - testes_passaram);
    printf("--------------------------\n");
}
