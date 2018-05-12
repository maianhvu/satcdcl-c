#include "cdcl_solve.h"
#include <stdio.h>
#include <stdlib.h>

// Private interface
void find_pure_symbol(
    Formula f, Model m,
    int *clause_eval_result,
    int *variable, int *value);
void find_unit_clause(
    Formula f, Model m,
    int *clause_eval_result,
    int *variable, int *value);

int unit_propagation(
        Formula, Model,
        int decision_level,
        int *conflicted_clause_idx);

void pick_branching_variable(
    Formula, Model,
    int *variable, int *value);

int conflict_analysis(Formula, Model, int decision_level);
int xi(Formula, Model, int clause_idx, int literal, int decision_level);

// Solving
int cdcl_solve(Formula formula, Model model) {
    // First unit propagation check
    // We don't care what the conflicted clause is, as long as there
    // is a conflict we will return UNSAT.
    if (!unit_propagation(formula, model, 0, NULL)) {
        return 0;
    }
    int decision_level = 0;

    int i;
    int num_conflicts = 0;
    double alpha = 0.4;

    // CHB: TODO: Find out what this is
    int last_conflict_map[model->size];
    for (i = 0; i < model->size; ++i) { last_conflict_map[i] = 0; }
    // CHB: TODO: Find out what this is
    double q_map[model->size];
    for (i = 0; i < model->size; ++i) { q_map[i] = 0; }

    while (model->assigned_count < model->size) {
        int p = -1;
        int value;
        pick_branching_variable(formula, model, &p, &value);
        if (p == -1) {
            printf("[ERROR] A variable must have been picked");
            return 0;
        }

        ++decision_level;
        AssignProps props = {
            .decision_level = decision_level,
            .is_branching_var = 1,
            .antecedent_idx = -1
        };
        model_decision(model, p, value, props);

        // Check for conflict
        int conflicted_clause_idx = -1;
        int conflicted = !unit_propagation(
                formula, model,
                decision_level,
                &conflicted_clause_idx);

        // CHB: Update multiplier
        double chb_multiplier = conflicted ? 1 : 0;
        // Update all assigned variables
        for (i = 0; i < model->size; ++i) {
            // Skip unassigned variables, we only want assigned
            if (model->values[i] == MODEL_U) {
                continue;
            }
            double reward = chb_multiplier / (num_conflicts - last_conflict_map[i] + 1);
            q_map[i] = (1 - alpha) * q_map[i] + alpha * reward;
        }

        // CDCL: Backtrack or exit
        if (conflicted) {
            // Increment conflict count
            ++num_conflicts;

            // CHB: Update alpha
            if (alpha > 0.06) {
                alpha -= 1e-6;
            }

            // CDCL: Conflict analysis
            int beta = conflict_analysis(formula, model, decision_level);
            // UNSAT
            if (beta < 0) {
                return 0;
            } else {
                // Backtrack
                model_backtrack(model, beta);
                decision_level = beta;
            }
        }

    }

    return 1;
}

/**
 * Check if the clause at index `clause_idx` within formula `formula` is a unit
 * clause. If it is, returns the index of the unit literal within the formula's
 * buffer, else return -1.
 * @param f The formula
 * @param m The model to check assignments against
 * @param clause_idx The index of the clause
 * @returns literal index of the unit literal within the formula's buffer,
 *  or -1 if the clause is not unit
 */
int check_unit_clause(Formula f, Model m, int clause_idx) {
    int literal_idx;
    int unit_literal_idx = -1;

    for (literal_idx = (clause_idx == 0 ? 0 : f->indexes[literal_idx - 1]);
         literal_idx < f->indexes[clause_idx];
         ++literal_idx) {
        // Skip assigned literals
        if (model_value(m, f->buffer[literal_idx]) != MODEL_U) {
            continue;
        }
        // If there is already another unit literal before this one,
        // then we have at least two unassigned literals and thus
        // this clause is not a unit clause
        if (unit_literal_idx > -1) {
            return -1;
        }
        unit_literal_idx = literal_idx;
    }

    return unit_literal_idx;
}

int unit_propagation(
        Formula f, Model m,
        int decision_level,
        int *conflicted_clause_idx) {
    int clause_idx;
    int found_unit_clause = 1;
    while (found_unit_clause) {
        found_unit_clause = 0;

        for (clause_idx = 0; clause_idx < f->size; ++clause_idx) {
            int eval = model_eval(m, f, clause_idx);
            // One clause is unsatisfied -> conflict
            if (eval == MODEL_0) {
                *conflicted_clause_idx = clause_idx;
                return 0;
            }
            int unit_literal_idx;
            // If clause is not unit then move on to the next
            if ((unit_literal_idx = check_unit_clause(f, m, clause_idx)) == -1) {
                continue;
            }
            // Clause is now unit, set value
            int variable = f->buffer[unit_literal_idx];
            int value = f->buffer[unit_literal_idx] > 0 ? MODEL_1 : MODEL_0;
            AssignProps props = {
                .decision_level = decision_level,
                .is_branching_var = 0,
                .antecedent_idx = clause_idx
            };
            model_decision(m, variable, value, props);
            found_unit_clause = 1;
            // Check again
            break;
        }
    }
    return 1;
}

void pick_branching_variable(
    Formula f, Model m,
    int *p, int *value) {
    // TODO: Change this
    *p = 0;
    *value = 1;
}

//-----------------------------------------------
// Clause Learning
//-----------------------------------------------
// xi(omega, l, d) = { 1 if (l in omega) and (delta(l) = d) and (alpha(l) != NIL)
//                   { 0 otherwise
// - omega: a clause
// - l: a literal (integer)
// - d: decision level (integer)
// - delta(l) == d returns 1 if decision level of l is d
// - alpha(l) is the antecedent of l
int xi(Formula f, Model m, int clause_idx, int literal, int decision_level) {
    // Check if literal is in clause
    int literal_idx;
    for (literal_idx = (clause_idx == 0 ? 0 : f->indexes[clause_idx - 1]):
         literal_idx < f->indexes[clause_idx];
         ++literal_idx) {
        if (f->buffer[literal_idx] == literal) {
            break;
        }
    }
    if (f->buffer[literal_idx] != literal) {
        return 0;
    }

    AssignProps props = model->assign_props[abs(literal) - 1];
    // Check decision level
    if (props.decision_level != decision_level) {
        return 0;
    }
    // Check antecedent, must be present
    return props.antecedent_idx > -1;
}

int conflict_analysis(Formula formula, Model model, int decision_level) {
    int beta = 0; // TODO: Find conflict level

    return 0; // TODO: Implement this
}


