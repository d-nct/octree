#include "amostra.h"
#include <stdlib.h>

amostra* inicializaAmostra(float x, float y, float z) {
    amostra* novaAmostra = (amostra*)malloc(sizeof(amostra));
    CHECK_MALLOC(novaAmostra);

    /* Preenche */
    novaAmostra->x = x;
    novaAmostra->y = y;
    novaAmostra->z = z;

    return novaAmostra;
}