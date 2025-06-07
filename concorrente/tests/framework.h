#ifndef FRAMEWORK_H

#include <stdio.h>

#define LOG_INFO(fmt, ...) do { printf("[INFO] " fmt "\n", ##__VA_ARGS__); } while (0)

#define LOG_TEST_FAIL(fmt, ...) do { fprintf(stdout, "[FALHA] %s:%d - " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); } while (0)

#define ASSERT(condicao) \
    do { \
        total_testes++; \
        if (condicao) { \
            testes_passaram++; \
        } else { \
            LOG_TEST_FAIL("Condição não atendida: %s", #condicao); \
        } \
    } while (0)

void print_sumario_testes();

#endif // !FRAMEWORK_H
