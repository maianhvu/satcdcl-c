#include <stdio.h>
#include <stdlib.h>
#include "formula.h"

Formula formula_read(int size) {
    int i, j;
    int *buffer = (int *) malloc(sizeof(int) * 1000);
    int *indexes = (int *) malloc(sizeof(int) * size);
    int cursor = 0;
    for (i = 0; i < size; ++i) {
        int num = 0;
        for (j = 0; 1; ++j) {
            scanf("%d", &num);
            if (num == 0) {
                cursor += j;
                indexes[i] = cursor;
                break;
            }
            buffer[cursor + j] = num;
        }
    }
    Formula f = (Formula) malloc(sizeof(struct Formula));
    f->buffer = buffer;
    f->indexes = indexes;
    f->size = size;
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
