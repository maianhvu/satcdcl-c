#include "dpll_solve.h"
#include <stdio.h>
#include <stdlib.h>

// Private interface
void find_pure_symbol(
        Formula f, Model m,
        int *clause_eval_result,
        int *p, int *value
        );
void find_unit_clause(
        Formula f, Model m,
        int *clause_eval_result,
        int *p, int *value
        );

// Solving
int dpll_solve(Formula formula, Model model) {
    int clause_idx;
    int formula_eval_min = MODEL_1;
    int *clause_eval_result = (int *) malloc(sizeof(int) * formula->size);
    // Evaluate all clauses
    for (clause_idx = 0; clause_idx < formula->size; ++clause_idx) {
        int eval = model_eval(model, formula, clause_idx);
        if (formula_eval_min > eval) {
            formula_eval_min = eval;
        }
        // "if some clause in clauses is false in model then return false"
        if (formula_eval_min == MODEL_0) {
            return 0;
        }
        clause_eval_result[clause_idx] = eval;
    }
    // "if every clause in clauses is true in model then return true"
    if (formula_eval_min == MODEL_1) {
        printf("All are true\n");
        return 1;
    }
    // Find pure symbol
    int p = -1;
    int value;
    find_pure_symbol(formula, model, clause_eval_result, &p, &value);
    if (p > -1) {
        // Found a pure symbol
        printf("[DEBUG] pure x%d\tassign %c\n", p, model_value_to_char(value));
        model->values[p] = value;
        free(clause_eval_result);
        return dpll_solve(formula, model);
    }

    // Find unit clause
    find_unit_clause(formula, model, clause_eval_result, &p, &value);
    if (p > -1) {
        // Found a unit clause
        printf("[DEBUG] unit x%d\tassign %c\n", p, model_value_to_char(value));
        model->values[p] = value;
        free(clause_eval_result);
        return dpll_solve(formula, model);
    }

    // We don't need it anymore
    free(clause_eval_result);

    // Else find the first unassigned symbol and assign it to
    // two different values
    int variable_idx;
    for (variable_idx = 0; variable_idx < model->size; ++variable_idx) {
        if (model->values[variable_idx] == MODEL_U) {
            break;
        }
    }
    // Cannot find an unassigned variable
    if (model->values[variable_idx] != MODEL_U) {
        printf("Cannot find an unassigned variable");
        return 0;
    }
    // Try first assignment;
    Model clone = model_clone(model);
    clone->values[variable_idx] = MODEL_1;
    if (dpll_solve(formula, clone)) {
        model_transfer(model, clone);
        model_free(clone);
        return 1;
    }
    clone->values[variable_idx] = MODEL_0;
    if (dpll_solve(formula, clone)) {
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
        int *p, int *value
        ) {
    int clause_idx, literal_idx;
    int literal_signs[m->size];
    int variable_idx;
    // Initialize all signs for variables to non-present (0)
    for (variable_idx = 0; variable_idx < m->size; ++variable_idx) {
        literal_signs[variable_idx] = 0;
    }

    for (clause_idx = 0; clause_idx < f->size; ++clause_idx) {
        // Skip non-unassigned clauses
        if (clause_eval_result[clause_idx] != MODEL_U) {
            continue;
        }
        // Traverse the clause and find all unassigned literals
        for (literal_idx = (clause_idx == 0 ? 0 : f->indexes[clause_idx - 1]);
             literal_idx < f->indexes[clause_idx];
             ++literal_idx) {
            // Skip assigned literals
            int value = model_value(m, f->buffer[literal_idx]);
            if (value != MODEL_U) {
                continue;
            }
            // Literal is now unassigned, update its signs
            // Positive sign maps to 0b01, negative sign maps to 0b10
            int variable = abs(f->buffer[literal_idx]) - 1;
            literal_signs[variable] |= (f->buffer[literal_idx] > 0 ? (1 << 0) : (1 << 1));
        }
    }

    // Find the first variable that has only one sign
    for (variable_idx = 0; variable_idx < m->size; ++variable_idx) {
        // If it has only one sign
        if ((literal_signs[variable_idx] & 1) ^ (literal_signs[variable_idx] & (1 << 1))) {
            *p = variable_idx;
            *value = (literal_signs[variable_idx] & 1) ? MODEL_1 : MODEL_0;
            return;
        }
    }
}

void find_unit_clause(
        Formula f, Model m,
        int *clause_eval_result,
        int *p, int *value
        ) {
    int clause_idx, literal_idx;

    for (clause_idx = 0; clause_idx < f->size; ++clause_idx) {
        // Skip non-unassigned clauses
        if (clause_eval_result[clause_idx] != MODEL_U) {
            continue;
        }
        // Traverse the clause and find the first unassigned literal
        int unassigned_count = 0;
        int unassigned_literal_idx = -1;

        for (literal_idx = (clause_idx == 0 ? 0 : f->indexes[clause_idx - 1]);
             literal_idx < f->indexes[clause_idx];
             ++literal_idx) {
            // Skip assigned literals
            if (model_value(m, f->buffer[literal_idx]) == MODEL_U) {
                unassigned_literal_idx = literal_idx;
                ++unassigned_count;
                // End early if there are more than one unassigned literals
                if (unassigned_count > 1) {
                    break;
                }
            }
            // Check if is NOT a unit clause
            if (unassigned_count > 1) {
                continue;
            }
            // Is now confirmed to be a unit clause, extract result from the literal
            *p = abs(f->buffer[unassigned_literal_idx]);
            *value = (f->buffer[unassigned_literal_idx] > 0) ? MODEL_1 : MODEL_0;
            return;
        }
    }
}

