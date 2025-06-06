#ifndef AMOSTRA_H
#define AMOSTRA_H

#include "system.h"

/**
 * Estrutura para armazenar uma amostra do LIDAR.
 */
typedef struct _Amostra {
    float x;
    float y;
    float z;
} amostra;

/**
 * Inicializa uma amostra do LIDAR.
 */
amostra* inicializaAmostra(float x, float y, float z);

#endif
