#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* DEFINIÇÕES
   ---------- */

/* Máximo de pontos antes de subdividir um nó (int) */
#define NOCTREE_CAPACIDADE 10 // Coloquei 10 só por colocar qualquer coisa. TODO

#define DIM                3 // X, Y e Z
#define QT_FILHOS_NOCTREE  8 // Quantidade de filhos de cada Nó Octree

/* ERROS
   ---------- */
#define ERRO_ALOCACAO     1

// Macro para logar erros
#define LOG_ERROR(codigo, fmt, ...) \
  do { \
    fprintf(stderr, "[ERRO] %s:%d (%s) - " fmt "\n", \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    exit(codigo); \
  } while (0)

#define CHECK_MALLOC(pt) \
  do { \
    if (pt == NULL) { \
      printf("[ERRO] Alocação de Memória: linha %d\n", __LINE__); \
      exit(ERRO_ALOCACAO); \
    } \
  } while (0)

/* debug! debug!! debug!!! */
/* compilador com a flag -DDEBUG */

#ifdef DEBUG
#define LOGN(fmt, ...)            printf("[l:%d] " fmt, __LINE__, ##__VA_ARGS__)
#define LOGP(fmt, ...)            printf(fmt, ##__VA_ARGS__)
#define ENDL                      printf("\n")
#else
#define LOGN(fmt, ...)    
#define LOGP(fmt, ...)    
#define ENDL
#endif



#endif
