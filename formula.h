#ifndef _SATCDCL_FORMULA_H
#define _SATCDCL_FORMULA_H

typedef struct Formula {
    int *buffer;
    int *indexes;
    int size;

    // Private properties
    int buffer_size;

    // CDCL Augmentations
    int *learned_clauses_indexes;
    int num_clauses_learned;
} *Formula;

Formula formula_read(int size);
void formula_print(Formula);
void formula_free(Formula);

#endif

