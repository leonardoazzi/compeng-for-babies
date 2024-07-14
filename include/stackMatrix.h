#ifndef _STACK_MATRIX_H
#define _STACK_MATRIX_H

#include "globals.h"

// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

#endif // STACK_MATRIX