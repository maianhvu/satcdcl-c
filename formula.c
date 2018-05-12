#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "formula.h"

static double const FORMULA_BUFFER_RESIZE_THRESH = 0.9;

Formula formula_read(int size) {
    int clause_idx, clause_size;
    int buffer_size = 1000;

    // Initialize first buffer
    int *buffer = (int *) malloc(sizeof(int) * buffer_size);
    double average_clause_size = 0;

    int *indexes = (int *) malloc(sizeof(int) * size);
    int cursor = 0;

    for (clause_idx = 0; clause_idx < size; ++clause_idx) {
        int num = 0;
        for (clause_size = 0; 1; ++clause_size) {
            scanf("%d", &num);
            if (num == 0) {
                cursor += clause_size;
                indexes[clause_idx] = cursor;
                // Update average clause size, so that when we resize we can guess
                // by how much we should resize the buffer
                if (clause_idx == 0) {
                    average_clause_size = clause_size;
                } else {
                    average_clause_size = ((average_clause_size * clause_idx) + clause_size) / (clause_idx + 1);
                }
                break;
            }
            // Resize the buffer if needed
            if (cursor + clause_size >= buffer_size) {
                buffer_size = ceil(average_clause_size * (size - clause_idx) + buffer_size);
                int *new_buffer = (int *) malloc(sizeof(int) * buffer_size);
                memcpy(new_buffer, buffer, sizeof(int) * (cursor + clause_size));
                free(buffer);
                buffer = new_buffer;
            }
            // Copy literal to buffer
            buffer[cursor + clause_size] = num;
        }

    }
    Formula f = (Formula) malloc(sizeof(struct Formula));
    f->buffer = buffer;
    f->indexes = indexes;
    f->size = size;
    f->buffer_size = buffer_size;
    return f;
}

void formula_print(Formula f) {
    int i, j;
    for (i = 0; i < f->size; ++i) {
        printf("%d. (", i);
        for (j = (i == 0 ? 0 : f->indexes[i - 1]); j < f->indexes[i]; ++j) {
            int literal = f->buffer[j];
            if (literal < 0) {
                printf("~");
            }
            printf("x%d", abs(literal));
            if (j < f->indexes[i] - 1) {
                printf(" + ");
            }
        }
        printf(")\n");
    }
}
void formula_free(Formula f) {
    free(f->buffer);
    free(f->indexes);
    free(f);
}
