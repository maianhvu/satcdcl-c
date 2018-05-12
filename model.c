#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model.h"

Model model_create(int size)
{
    int i;
    Model model = (Model)malloc(sizeof(struct Model));
    model->values = (int *)malloc(sizeof(int) * size);
    model->decision_levels = (int *)malloc(sizeof(int) * size);
    for (i = 0; i < size; ++i)
    {
        model->values[i] = MODEL_U;
        model->decision_levels[i] = -1;
    }
    model->size = size;

    return model;
}

char model_value_to_char(int value)
{
    switch (value)
    {
    case MODEL_1:
        return '1';
    case MODEL_0:
        return '0';
    case MODEL_U:
        return 'u';
    default:
        return '*';
    }
}

void model_print(Model m)
{
    int i;
    for (i = 0; i < m->size; ++i)
    {
        char rep = model_value_to_char(m->values[i]);
        printf("x%d = %c", i + 1, rep);
        if (i == m->size - 1)
        {
            printf("\n");
        }
        else
        {
            printf(", ");
        }
    }
}

void model_print_compact(Model m)
{
    int i;
    for (i = 0; i < m->size; ++i)
    {
        printf("%c", model_value_to_char(m->values[i]));
        if (i < m->size - 1)
        {
            printf(" ");
        }
        else
        {
            printf("\n");
        }
    }
}

void model_free(Model model)
{
    free(model->values);
    free(model);
}

int model_value(Model model, int variable_or_literal)
{
    int index = abs(variable_or_literal) - 1;
    int is_positive = variable_or_literal > 0;
    int value = model->values[index];
    if (!is_positive)
    {
        value = MODEL_1 - value;
    }
    return value;
}

void model_assign(Model model, int variable, int value)
{
    model_decision(model, variable, value, 0);
}

void model_decision(Model model, int variable, int value, int decision_level)
{
    int variable_idx = abs(variable) - 1;
    model->values[variable_idx] = value;
    model->decision_levels[variable_idx] = decision_level;
}

// Clause evaluation
int model_eval(Model model, Formula formula, int clause_index)
{
    int literal_idx;
    int result = 0;
    for (literal_idx = (clause_index == 0 ? 0 : formula->indexes[clause_index - 1]);
         literal_idx < formula->indexes[clause_index];
         ++literal_idx)
    {
        int value = model_value(model, formula->buffer[literal_idx]);
        // Find max
        if (value > result)
        {
            result = value;
        }
        // 1 is max, return immediately
        if (result == MODEL_1)
        {
            break;
        }
    }
    return result;
}

Model model_clone(Model source)
{
    Model clone = (Model)malloc(sizeof(struct Model));
    // Copy values
    clone->values = (int *)malloc(sizeof(int) * source->size);
    memcpy(clone->values, source->values, sizeof(int) * source->size);
    // Copy decision levels
    clone->decision_levels = (int *)malloc(sizeof(int) * source->size);
    memcpy(clone->decision_levels, source->decision_levels, sizeof(int) * source->size);
    // Retain size
    clone->size = source->size;
    return clone;
}

void model_transfer(Model dest, Model src)
{
    int size = dest->size < src->size ? dest->size : src->size;
    memcpy(dest->values, src->values, sizeof(int) * size);
    memcpy(dest->decision_levels, src->decision_levels, sizeof(int) * size);
}

//-----------------------------------------------
// CDCL
//-----------------------------------------------
void model_print_decisions(Model model) {
    int level, variable_idx;
    for (level = 0; 1; ++level) {
        int count = 0;
        for (variable_idx = 0; variable_idx < model->size; ++variable_idx) {
            if (model->decision_levels[variable_idx] != level) {
                continue;
            }
            if (count == 0) {
                printf("Level %d: ", level);
            } else if (count > 0) {
                printf(" ");
            }
            printf("x%d=%c", variable_idx + 1, model_value_to_char(model->values[variable_idx]));
            ++count;
        }
        printf("\n");
        // No more variables
        if (count == 0) {
            break;
        }
    }
}

