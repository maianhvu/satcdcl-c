#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model.h"

AssignProps ASSIGN_PROPS_DEFAULT = {
    .decision_level = -1,
    .is_branching_var = 0,
    .antecedent_idx = -1
};

Model model_create(int size)
{
    int i;
    Model model = (Model)malloc(sizeof(struct Model));
    model->values = (int *)malloc(sizeof(int) * size);
    model->assign_props = (AssignProps *)malloc(sizeof(AssignProps) * size);
    for (i = 0; i < size; ++i)
    {
        model->values[i] = MODEL_U;
        model->assign_props[i] = ASSIGN_PROPS_DEFAULT;
    }
    model->size = size;
    model->assigned_count = 0;

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
    free(model->assign_props);
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
    AssignProps assign_props;
    assign_props.decision_level = 0;
    assign_props.is_branching_var = 0;
    assign_props.antecedent_idx = -1;
    model_decision(model, variable, value, assign_props);
}

void model_decision(Model model, int variable, int value, AssignProps assign_props)
{
    int variable_idx = abs(variable) - 1;

    if (value == MODEL_U) {
        printf("[ERROR] Cannot assign 'unassigned', use model_backtrack() instead");
        return;
    }

    // Check if already assigned
    if (model->values[variable_idx] != MODEL_U) {
        // Skip if already assigned
        return;
    }

    // Increment assigned count
    model->assigned_count = model->assigned_count + 1;

    model->values[variable_idx] = value;
    model->assign_props[variable_idx] = assign_props;
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
    // Copy assign properties
    clone->assign_props = (AssignProps *)malloc(sizeof(AssignProps) * source->size);
    memcpy(clone->assign_props, source->assign_props, sizeof(AssignProps) * source->size);
    // Retain size
    clone->size = source->size;
    return clone;
}

void model_transfer(Model dest, Model src)
{
    int size = dest->size < src->size ? dest->size : src->size;
    memcpy(dest->values, src->values, sizeof(int) * size);
    memcpy(dest->assign_props,
            src->assign_props,
            sizeof(AssignProps) * size);
}

//-----------------------------------------------
// CDCL
//-----------------------------------------------
void model_print_decisions(Model model) {
    int level, variable_idx;
    for (level = 0; 1; ++level) {
        int count = 0;
        for (variable_idx = 0; variable_idx < model->size; ++variable_idx) {
            if (model->assign_props[variable_idx].decision_level != level) {
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

void model_backtrack(Model m, int decision_level) {
    int variable_idx;
    for (variable_idx = 0; variable_idx < m->size; ++variable_idx) {
        // Skip variables with decision level smaller than the supplied number
        if (m->assign_props[variable_idx].decision_level < decision_level) {
            continue;
        }
        m->values[variable_idx] = MODEL_U;
        m->assign_props[variable_idx] = ASSIGN_PROPS_DEFAULT;
        m->assigned_count = m->assigned_count - 1;
    }
}

