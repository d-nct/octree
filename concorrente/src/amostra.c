#include "amostra.h"

amostra* inicializaAmostra(float x, float y, float z) {
  LOGP("Cheguei no inicializaAmostra"); ENDL;
  amostra* novaAmostra = (amostra*)malloc(sizeof(amostra));
  CHECK_MALLOC(novaAmostra);

  /* Preenche */
  novaAmostra->x = x;
  novaAmostra->y = y;
  novaAmostra->z = z;

  return novaAmostra;
  LOGP("Fim do inicializaAmostra"); ENDL;
}
