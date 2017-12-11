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

The benefit of this stage is that function definitions can be lookup up more easily.

## Function Grouper

The function grouper has to be applied after the disambiguator and the function hoister.
Its effect is that all topmost elements that are not function definitions are moved
into a single block which is the first satement of the root block.

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

## Variable Eliminator

## Unused Declaration Pruner

## Function Unifier
