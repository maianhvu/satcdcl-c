#ifndef _SATCDCL_CDCL_SOLVE_H
#define _SATCDCL_CDCL_SOLVE_H

#include "formula.h"
#include "model.h"

int cdcl_solve(Formula formula, Model model);

#endif