#include <stdio.h>
#include "formula.h"
#include "model.h"

#define USE_CDCL 0

#if USE_CDCL
#else
#include "dpll_solve.h"
#endif

int main(int argc, char* argv[]) {
    char *line = NULL;
    size_t line_length;
    int n = 0, k = 0;
    while (getline(&line, &line_length, stdin) != -1) {
        char directive;
        sscanf(line, "%c", &directive);
        if (directive != 'p') continue;
        char temp[4];
        sscanf(line, "%c %s %d %d", &directive, temp, &n, &k);
        break;
    }
    if (n <= 0 || k <= 0) {
        printf("Invalid format.\n");
        return 0;
    }
    // Initialize formula
    Formula formula = formula_read(k);
    formula_print(formula);

    Model model = model_create(n);
    model_print(model);

    int sat;
#if USE_CDCL
#else
    sat = dpll_solve(formula, model);
#endif

    if (sat) {
        printf("SAT\n");
        model_print(model);
    } else {
        printf("UNSAT\n");
    }

    model_free(model);
    formula_free(formula);
    return 0;
}

