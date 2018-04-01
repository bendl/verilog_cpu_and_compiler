//
// Created by BDL on 03/03/2018.
//

#include "adt/ast.h"

#define AST_SELF_CF(node) \
        node = opt_cf(node)

/// Optimisation: Constant Folding.
///
/// Recursively iterates over AST tree, finding nodes that
/// can be reduced, such as AST_BINs, AST_IFs, and AST_ASSIGNs
struct ast_item *opt_cf(struct ast_item *node);
