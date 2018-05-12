#ifndef _SATCDCL_FORMULA_H
#define _SATCDCL_FORMULA_H

typedef struct Formula {
    int *buffer;
    int *indexes;
    int size;
} *Formula;

Formula formula_read(int size);
void formula_print(Formula);
void formula_free(Formula);

#endif

