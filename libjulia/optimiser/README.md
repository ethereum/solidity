## IULIA Optimiser

The iulia optimiser consists of several stages and components that all transform
the AST in a semantically equivalent way. The goal is to end up either with code
that is shorter or at least only marginally longer but will allow further
optimisation steps.

The optimiser currently follows a purely greedy strategy and does not do any
backtracking.

## Disambiguator

The disambiguator takes an AST and returns a fresh copy where all identifiers have
names unique to the input AST. This is a prerequisite for all other optimiser stages.
One of the benefits is that identifier lookup does not need to take scopes into account
and we can basically ignore the result of the analysis phase.

All subsequent stages have the property that all names stay unique. This means if
a new identifier needs to be introduced, a new unique name is generated.

## Function Hoister

The function hoister moves all function definitions to the topmost block. This is
a semantically equivalent transformation as long as it is performed after the
disambiguation stage. The reason is that moving a definition upwards cannot decrease
its visibility and it is impossible to reference variables defined in a different function.

The benefit of this stage is that function definitions can be looked up more easily.

## Function Grouper

The function grouper has to be applied after the disambiguator and the function hoister.
Its effect is that all topmost elements that are not function definitions are moved
into a single block which is the first statement of the root block.

After this step, a program has the following normal form:

	{ I F... }

Where I is a block that does not contain any function definitions (not even recursively)
and F is a list of function definitions such that no function contains a function definition.

## Functional Inliner

The functional inliner depends on the disambiguator, the function hoister and function grouper.
It performs function inlining such that the result of the inlining is an expression. This can
only be done if the body of the function to be inlined has the form ``{ r := E }`` where ``r``
is the single return value of the function, ``E`` is an expression and all arguments in the
function call are so-called movable expressions. A movable expression is either a literal, a
variable or a function call (or EVM opcode) which does not have side-effects and also does not
depend on any side-effects.

As an example, neither ``mload`` nor ``mstore`` would be allowed.

## Full Function Inliner

## Rematerialisation

The rematerialisation stage tries to replace variable references by the expression that
was last assigned to the variable. This is of course only beneficial if this expression
is comparatively cheap to evaluate. Furthermore, it is only semantically equivalent if
the value of the expression did not change between the point of assignment and the
point of use. The main benefit of this stage is that it can save stack slots if it
leads to a variable being eliminated completely (see below), but it can also
save a DUP opcode on the EVM if the expression is very cheap.

The algorithm only allows movable expressions (see above for a definition) in this case.
Expressions that contain other variables are also disallowed if one of those variables
have been assigned to in the meantime. This is also not applied to variables where
assignment and use span across loops and conditionals.

## Unused Definition Pruner

If a variable or function is not referenced, it is removed from the code.
If there are two assignments to a variable where the first one is a movable expression
and the variable is not used between the two assignments (and the second is not inside
a loop or conditional, the first one is not inside), the first assignment is removed.


## Function Unifier

## Expression Simplifier

This step can only be applied for the EVM-flavoured dialect of iulia. It applies
simple rules like ``x + 0 == x`` to simplify expressions.

## Ineffective Statement Remover

This step removes statements that have no side-effects.

## WebAssembly specific

### Main Function

Changes the topmost block to be a function with a specific name ("main") which has no
inputs nor outputs.

Depends on the Function Grouper.
