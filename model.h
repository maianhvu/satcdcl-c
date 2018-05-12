#ifndef _SATCDCL_MODEL_H
#define _SATCDCL_MODEL_H

#include "formula.h"

static int const MODEL_1 = 2;
static int const MODEL_0 = 0;
static int const MODEL_U = 1;

typedef struct Model {
    int *values;
    int size;
    int *decision_levels;
} *Model;

Model model_create(int size);
char model_value_to_char(int value);
void model_print(Model);
void model_print_compact(Model);
void model_free(Model);
int model_value(Model, int variable_or_literal);
int model_eval(Model, Formula, int clause_index);
Model model_clone(Model);
void model_transfer(Model dest, Model src);

void model_assign(Model, int variable, int value);
void model_decision(Model, int variable, int value, int decision_level);

#endif

