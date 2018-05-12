#include "cdcl_solve.h"
#include <stdio.h>
#include <stdlib.h>

// Private interface
void find_pure_symbol(
    Formula f, Model m,
    int *clause_eval_result,
    int *p, int *value);
void find_unit_clause(
    Formula f, Model m,
    int *clause_eval_result,
    int *p, int *value);
int cdcl_solve_internal(Formula formula, Model model, int decision_level);

// Solving
int cdcl_solve(Formula formula, Model model) {
    return cdcl_solve_internal(formula, model, 0);
}

int cdcl_solve_internal(Formula formula, Model model, int decision_level)
{
    int clause_idx;
    int formula_eval_min = MODEL_1;
    int *clause_eval_result = (int *)malloc(sizeof(int) * formula->size);
    // Evaluate all clauses
    for (clause_idx = 0; clause_idx < formula->size; ++clause_idx)
    {
        int eval = model_eval(model, formula, clause_idx);
        if (formula_eval_min > eval)
        {
            formula_eval_min = eval;
        }
        // "if some clause in clauses is false in model then return false"
        if (formula_eval_min == MODEL_0)
        {
            return 0;
        }
        clause_eval_result[clause_idx] = eval;
    }
    // "if every clause in clauses is true in model then return true"
    if (formula_eval_min == MODEL_1)
    {
        return 1;
    }
    // Find pure symbol
    int p = -1;
    int value;
    find_pure_symbol(formula, model, clause_eval_result, &p, &value);
    if (p > -1)
    {
        // Found a pure symbol
        model_decision(model, p, value, decision_level);
        free(clause_eval_result);
        return cdcl_solve(formula, model);
    }

    // Find unit clause
    find_unit_clause(formula, model, clause_eval_result, &p, &value);
    if (p > -1)
    {
        // Found a unit clause
        model_decision(model, p, value, decision_level);
        free(clause_eval_result);
        return cdcl_solve(formula, model);
    }

    // We don't need it anymore
    free(clause_eval_result);

    // Else find the first unassigned symbol and assign it to
    // two different values
    int variable_idx;
    for (variable_idx = 0; variable_idx < model->size; ++variable_idx)
    {
        if (model->values[variable_idx] == MODEL_U)
        {
            break;
        }
    }
    // Cannot find an unassigned variable
    if (model->values[variable_idx] != MODEL_U)
    {
        return 0;
    }

    int new_decision_level = decision_level + 1;
    // Try first assignment;
    Model clone = model_clone(model);
    model_decision(clone, variable_idx + 1, MODEL_1, new_decision_level);
    if (cdcl_solve_internal(formula, clone, new_decision_level))
    {
        model_transfer(model, clone);
        model_free(clone);
        return 1;
    }

    // Try second assignment

    // Remove old clone
    model_free(clone);
    // Create a new clone
    clone = model_clone(model);
    // Assign the negative assignment
    model_decision(clone, variable_idx + 1, MODEL_0, new_decision_level);
    if (cdcl_solve_internal(formula, clone, new_decision_level))
    {
        model_transfer(model, clone);
        model_free(clone);
        return 1;
    }
    model_free(clone);
    return 0;
}

void find_pure_symbol(
    Formula f, Model m,
    int *clause_eval_result,
    int *p, int *value)
{
    int clause_idx, literal_idx;
    int literal_signs[m->size];
    int variable_idx;
    // Initialize all signs for variables to non-present (0)
    for (variable_idx = 0; variable_idx < m->size; ++variable_idx)
    {
        literal_signs[variable_idx] = 0;
    }

    for (clause_idx = 0; clause_idx < f->size; ++clause_idx)
    {
        // Skip non-unassigned clauses
        if (clause_eval_result[clause_idx] != MODEL_U)
        {
            continue;
        }
        // Traverse the clause and find all unassigned literals
        for (literal_idx = (clause_idx == 0 ? 0 : f->indexes[clause_idx - 1]);
             literal_idx < f->indexes[clause_idx];
             ++literal_idx)
        {
            // Skip assigned literals
            int value = model_value(m, f->buffer[literal_idx]);
            if (value != MODEL_U)
            {
                continue;
            }
            // Literal is now unassigned, update its signs
            // Positive sign maps to 0b01, negative sign maps to 0b10
            int variable = abs(f->buffer[literal_idx]) - 1;
            literal_signs[variable] |= (f->buffer[literal_idx] > 0 ? (1 << 0) : (1 << 1));
        }
    }

    // Find the first variable that has only one sign
    for (variable_idx = 0; variable_idx < m->size; ++variable_idx)
    {
        // If it has only one sign
        if ((literal_signs[variable_idx] & 1) ^ ((literal_signs[variable_idx] >> 1) & 1))
        {
            *p = (variable_idx + 1);
            *value = (literal_signs[variable_idx] & 1) ? MODEL_1 : MODEL_0;
            return;
        }
    }
}

void find_unit_clause(
    Formula f, Model m,
    int *clause_eval_result,
    int *p, int *value)
{
    int clause_idx, literal_idx;

    for (clause_idx = 0; clause_idx < f->size; ++clause_idx)
    {
        // Skip non-unassigned clauses
        if (clause_eval_result[clause_idx] != MODEL_U)
        {
            continue;
        }
        // Traverse the clause and find the first unassigned literal
        int unassigned_count = 0;
        int unassigned_literal_idx = -1;

        for (literal_idx = (clause_idx == 0 ? 0 : f->indexes[clause_idx - 1]);
             literal_idx < f->indexes[clause_idx];
             ++literal_idx)
        {
            // Skip assigned literals
            if (model_value(m, f->buffer[literal_idx]) == MODEL_U)
            {
                unassigned_literal_idx = literal_idx;
                ++unassigned_count;
                // End early if there are more than one unassigned literals
                if (unassigned_count > 1)
                {
                    break;
                }
            }
        }

        // Check if is NOT a unit clause
        if (unassigned_count != 1)
        {
            continue;
        }
        // Is now confirmed to be a unit clause, extract result from the literal
        *p = abs(f->buffer[unassigned_literal_idx]);
        *value = (f->buffer[unassigned_literal_idx] > 0) ? MODEL_1 : MODEL_0;
        return;
    }
}
