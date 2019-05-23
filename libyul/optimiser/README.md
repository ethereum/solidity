Note that the Yul optimiser is still in research phase. Because of that,
the following description might not fully reflect the current or even
planned state of the optimiser.

Table of Contents:

- [Preprocessing](#preprocessing)
- [Pseudo-SSA Transformation](#pseudo-ssa-transformation)
- [Tools](#tools)
- [Expression-Scale Simplifications](#expression-scale-simplifications)
- [Statement-Scale Simplifications](#statement-scale-simplifications)
- [Function Inlining](#function-inlining)
- [Cleanup](#cleanup)
- [Webassembly-sepcific](#webassembly-specific)


# Yul Optimiser

The Yul optimiser consists of several stages and components that all transform
the AST in a semantically equivalent way. The goal is to end up either with code
that is shorter or at least only marginally longer but will allow further
optimisation steps.

The optimiser currently follows a purely greedy strategy and does not do any
backtracking.

All components of the optimiser are explained below, where
the following transformation steps are the main components:

 - [SSA Transform](#ssa-transform)
 - [Common Subexpression Eliminator](#common-subexpression-eliminator)
 - [Expression Simplifier](#expression-simplifier)
 - [Redundant Assign Eliminator](#redundant-assign-eliminator)
 - [Full Function Inliner](#full-function-inliner)

## Preprocessing

The preprocessing components perform transformations to get the program
into a certain normal form that is easier to work with. This normal
form is kept during the rest of the optimisation process.

### Disambiguator

The disambiguator takes an AST and returns a fresh copy where all identifiers have
names unique to the input AST. This is a prerequisite for all other optimiser stages.
One of the benefits is that identifier lookup does not need to take scopes into account
and we can basically ignore the result of the analysis phase.

All subsequent stages have the property that all names stay unique. This means if
a new identifier needs to be introduced, a new unique name is generated.

### Function Hoister

The function hoister moves all function definitions to the end of the topmost block. This is
a semantically equivalent transformation as long as it is performed after the
disambiguation stage. The reason is that moving a definition to a higher-level block cannot decrease
its visibility and it is impossible to reference variables defined in a different function.

The benefit of this stage is that function definitions can be looked up more easily
and functions can be optimised in isolation without having to traverse the AST.

### Function Grouper

The function grouper has to be applied after the disambiguator and the function hoister.
Its effect is that all topmost elements that are not function definitions are moved
into a single block which is the first statement of the root block.

After this step, a program has the following normal form:

	{ I F... }

Where I is a (potentially empty) block that does not contain any function definitions (not even recursively)
and F is a list of function definitions such that no function contains a function definition.

The benefit of this stage is that we always know where the list of function begins.

### For Loop Condition Into Body

This transformation moves the iteration condition of a for-loop into loop body.
We need this transformation because [expression splitter](#expression-splitter) won't
apply to iteration condition expressions (the `C` in the following example).

    for { Init... } C { Post... } {
        Body...
    }

is transformed to

    for { Init... } 1 { Post... } {
        if iszero(C) { break }
        Body...
    }

### For Loop Init Rewriter

This transformation moves the initialization part of a for-loop to before
the loop:

    for { Init... } C { Post... } {
        Body...
    }

is transformed to

    {
        Init...
        for {} C { Post... } {
            Body...
        }
    }

This eases the rest of the optimisation process because we can ignore
the complicated scoping rules of the for loop initialisation block.

## Pseudo-SSA Transformation

The purpose of this components is to get the program into a longer form,
so that other components can more easily work with it. The final representation
will be similar to a static-single-assignment (SSA) form, with the difference
that it does not make use of explicit "phi" functions which combines the values
from different branches of control flow because such a feature does not exist
in the Yul language. Instead, assignments to existing variables are
used.

An example transformation is the following:

    {
        let a := calldataload(0)
        let b := calldataload(0x20)
        if gt(a, 0) {
            b := mul(b, 0x20)
        }
        a := add(a, 1)
        sstore(a, add(b, 0x20))
    }

When all the following transformation steps are applied, the program will look
as follows:

    {
        let _1 := 0
        let a_1 := calldataload(_1)
        let _2 := 0x20
        let b_1 := calldataload(_2)
        let b := b_1
        let _3 := 0
        let _4 := gt(a_1, _3)
        if _4 {
            let _5 := 0x20
            let b_2 := mul(b_1, _5)
            b := b_2
        }
        let a_2 := add(a_1, 1)
        let _6 := 0x20
        let _7 := add(b, _6)
        sstore(a_2, _7)
    }

Note that the only variable that is re-assigned in this snippet is ``b``.
This re-assignment cannot be avoided because ``b`` has different values
depending on the control flow. All other variables never change their
value once they are defined. The advantage of this property is that
variables can be freely moved around and references to them
can be exchanged by their initial value (and vice-versa),
as long as these values are still valid in the new context.

Of course, the code here is far from being optimised. To the contrary, it is much
longer. The hope is that this code will be easier to work with and furthermore,
there are optimiser steps that undo these changes and make the code more
compact again at the end.

### Expression Splitter

The expression splitter turns expressions like ``add(mload(x), mul(mload(y), 0x20))``
into a sequence of declarations of unique variables that are assigned sub-expressions
of that expression so that each function call has only variables or literals
as arguments.

The above would be transformed into

    {
        let _1 := mload(y)
        let _2 := mul(_1, 0x20)
        let _3 := mload(x)
        let z := add(_3, _2)
    }

Note that this transformation does not change the order of opcodes or function calls.

It is not applied to loop conditions, because the loop control flow does not allow
this "outlining" of the inner expressions in all cases. We can sidestep this limitation by applying
[for loop condition into body](#for-loop-condition-into-body) to move the iteration condition into loop body.

The final program should be in a form such that (with the exception of loop conditions)
function calls cannot appear nested inside expressions
and all function call arguments have to be constants or variables.

The benefits of this form are that it is much easier to re-order the sequence of opcodes
and it is also easier to perform function call inlining. Furthermore, it is simpler
to replace individual parts of expressions or re-organize the "expression tree".
The drawback is that such code is much harder to read for humans.

### SSA Transform

This stage tries to replace repeated assignments to
existing variables by declarations of new variables as much as
possible.
The reassignments are still there, but all references to the
reassigned variables are replaced by the newly declared variables.

Example:

    {
        let a := 1
        mstore(a, 2)
        a := 3
    }

is transformed to

    {
        let a_1 := 1
        let a := a_1
        mstore(a_1, 2)
        let a_3 := 3
        a := a_3
    }

Exact semantics:

For any variable ``a`` that is assigned to somewhere in the code
(variables that are declared with value and never re-assigned
are not modified) perform the following transforms:
 - replace ``let a := v`` by ``let a_i := v   let a := a_i``
 - replace ``a := v`` by ``let a_i := v   a := a_i``
where ``i`` is a number such that ``a_i`` is yet unused.

Furthermore, always record the current value of ``i`` used for ``a`` and replace each
reference to ``a`` by ``a_i``.
The current value mapping is cleared for a variable ``a`` at the end of each block
in which it was assigned to and at the end of the for loop init block if it is assigned
inside the for loop body or post block.

After this stage, the Redundant Assign Eliminator is recommended to remove the unnecessary
intermediate assignments.

This stage provides best results if the Expression Splitter and the Common Subexpression Eliminator
are run right before it, because then it does not generate excessive amounts of variables.
On the other hand, the Common Subexpression Eliminator could be more efficient if run after the
SSA transform.

### Redundant Assign Eliminator

The SSA transform always generates an assignment of the form ``a := a_i``, even though
these might be unnecessary in many cases, like the following example:

    {
        let a := 1
        a := mload(a)
        a := sload(a)
        sstore(a, 1)
    }

The SSA transform converts this snippet to the following:

    {
        let a_1 := 1
        a := a_1
        let a_2 := mload(a_1)
        a := a_2
        let a_3 := sload(a_2)
        a := a_3
        sstore(a_3, 1)
    }

The Redundant Assign Eliminator removes all the three assignments to ``a``, because
the value of ``a`` is not used and thus turn this
snippet into strict SSA form:

    {
        let a_1 := 1
        let a_2 := mload(a_1)
        let a_3 := sload(a_2)
        sstore(a_3, 1)
    }

Of course the intricate parts of determining whether an assignment is redundant or not
are connected to joining control flow.

The component works as follows in detail:

The AST is traversed twice: in an information gathering step and in the
actual removal step. During information gathering, we maintain a
mapping from assignment statements to the three states
"unused", "undecided" and "used" which signifies whether the assigned
value will be used later by a reference to the variable.

When an assignment is visited, it is added to the mapping in the "undecided" state
(see remark about for loops below) and every other assignment to the same variable
that is still in the "undecided" state is changed to "unused".
When a variable is referenced, the state of any assignment to that variable still
in the "undecided" state is changed to "used".

At points where control flow splits, a copy
of the mapping is handed over to each branch. At points where control flow
joins, the two mappings coming from the two branches are combined in the following way:
Statements that are only in one mapping or have the same state are used unchanged.
Conflicting values are resolved in the following way:

 - "unused", "undecided" -> "undecided"
 - "unused", "used" -> "used"
 - "undecided, "used" -> "used"

For for-loops, the condition, body and post-part are visited twice, taking
the joining control-flow at the condition into account.
In other words, we create three control flow paths: Zero runs of the loop,
one run and two runs and then combine them at the end.

Simulating a third run or even more is unnecessary, which can be seen as follows:

A state of an assignment at the beginning of the iteration will deterministically
result in a state of that assignment at the end of the iteration. Let this
state mapping function be called `f`. The combination of the three different
states `unused`, `undecided` and `used` as explained above is the `max`
operation where `unused = 0`, `undecided = 1` and `used = 2`.

The proper way would be to compute

    max(s, f(s), f(f(s)), f(f(f(s))), ...)

as state after the loop. Since `f` just has a range of three different values,
iterating it has to reach a cycle after at most three iterations,
and thus `f(f(f(s)))` has to equal one of `s`, `f(s)`, or `f(f(s))`
and thus

    max(s, f(s), f(f(s))) = max(s, f(s), f(f(s)), f(f(f(s))), ...).

In summary, running the loop at most twice is enough because there are only three
different states.

For switch statements that have a "default"-case, there is no control-flow
part that skips the switch.

When a variable goes out of scope, all statements still in the "undecided"
state are changed to "unused", unless the variable is the return
parameter of a function - there, the state changes to "used".

In the second traversal, all assignments that are in the "unused" state are removed.

This step is usually run right after the SSA transform to complete
the generation of the pseudo-SSA.

## Tools

### Movability

Movability is a property of an expression. It roughly means that the expression
is side-effect free and its evaluation only depends on the values of variables
and the call-constant state of the environment. Most expressions are movable.
The following parts make an expression non-movable:

 - function calls (might be relaxed in the future if all statements in the function are movable)
 - opcodes that (can) have side-effects (like ``call`` or ``selfdestruct``)
 - opcodes that read or write memory, storage or external state information
 - opcodes that depend on the current PC, memory size or returndata size

### Dataflow Analyzer

The Dataflow Analyzer is not an optimizer step itself but is used as a tool
by other components. While traversing the AST, it tracks the current value of
each variable, as long as that value is a movable expression.
It records the variables that are part of the expression
that is currently assigned to each other variable. Upon each assignment to
a variable ``a``, the current stored value of ``a`` is updated and
all stored values of all variables ``b`` are cleared whenever ``a`` is part
of the currently stored expression for ``b``.

At control-flow joins, knowledge about variables is cleared if they have or would be assigned
in any of the control-flow paths. For instance, upon entering a
for loop, all variables are cleared that will be assigned during the
body or the post block.

## Expression-Scale Simplifications

These simplification passes change expressions and replace them by equivalent
and hopefully simpler expressions.

### Common Subexpression Eliminator

This step uses the Dataflow Analyzer and replaces subexpressions that
syntactically match the current value of a variable by a reference to
that variable. This is an equivalence transform because such subexpressions have
to be movable.

All subexpressions that are identifiers themselves are replaced by their
current value if the value is an identifier.

The combination of the two rules above allow to compute a local value
numbering, which means that if two variables have the same
value, one of them will always be unused. The Unused Pruner or the
Redundant Assign Eliminator will then be able to fully eliminate such
variables.

This step is especially efficient if the expression splitter is run
before. If the code is in pseudo-SSA form,
the values of variables are available for a longer time and thus we
have a higher chance of expressions to be replaceable.

The expression simplifier will be able to perform better replacements
if the common subexpression eliminator was run right before it.

### Expression Simplifier

The Expression Simplifier uses the Dataflow Analyzer and makes use
of a list of equivalence transforms on expressions like ``X + 0 -> X``
to simplify the code.

It tries to match patterns like ``X + 0`` on each subexpression.
During the matching procedure, it resolves variables to their currently
assigned expressions to be able to match more deeply nested patterns
even when the code is in pseudo-SSA form.

Some of the patterns like ``X - X -> 0`` can only be applied as long
as the expression ``X`` is movable, because otherwise it would remove its potential side-effects.
Since variable references are always movable, even if their current
value might not be, the Expression Simplifier is again more powerful
in split or pseudo-SSA form.

## Statement-Scale Simplifications

### Unused Pruner

This step removes the definitions of all functions that are never referenced.

It also removes the declaration of variables that are never referenced.
If the declaration assigns a value that is not movable, the expression is retained,
but its value is discarded.

All movable expression statements (expressions that are not assigned) are removed.

### Structural Simplifier

This is a general step that performs various kinds of simplifications on
a structural level:

 - replace if statement with empty body by ``pop(condition)``
 - replace if statement with true condition by its body
 - remove if statement with false condition
 - turn switch with single case into if
 - replace switch with only default case by ``pop(expression)`` and body
 - replace switch with literal expression by matching case body
 - replace for loop with false condition by its initialization part

This component uses the Dataflow Analyzer.

### Equivalent Function Combiner

If two functions are syntactically equivalent, while allowing variable
renaming but not any re-ordering, then any reference to one of the
functions is replaced by the other.

The actual removal of the function is performed by the Unused Pruner.

### Block Flattener

This stage eliminates nested blocks by inserting the statement in the
inner block at the appropriate place in the outer block:

    {
        let x := 2
        {
            let y := 3
            mstore(x, y)
        }
    }

is transformed to

    {
        let x := 2
        let y := 3
        mstore(x, y)
    }

As long as the code is disambiguated, this does not cause a problem because
the scopes of variables can only grow.

## Function Inlining

### Functional Inliner

The functional inliner performs restricted function inlining. In particular,
the result of this inlining is always a single expression. This can
only be done if the function to be inlined has the form ``function f(...) -> r { r := E }`` where
``E`` is an expression that does not reference ``r`` and all arguments in the
function call are movable expressions. The function call is directly replaced
by ``E``, substituting the function call arguments. Because this can cause the
function call arguments to be duplicated, removed or re-ordered, they have
to be movable.

### Full Function Inliner

The Full Function Inliner replaces certain calls of certain functions
by the function's body. This is not very helpful in most cases, because
it just increases the code size but does not have a benefit. Furthermore,
code is usually very expensive and we would often rather have shorter
code than more efficient code. In same cases, though, inlining a function
can have positive effects on subsequent optimizer steps. This is the case
if one of the function arguments is a constant, for example.

During inlining, a heuristic is used to tell if the function call
should be inlined or not.
The current heuristic does not inline into "large" functions unless
the called function is tiny. Functions that are only used once
are inlined, as well as medium-sized functions, while function
calls with constant arguments allow slightly larger functions.


In the future, we might want to have a backtracking component
that, instead of inlining a function right away, only specializes it,
which means that a copy of the function is generated where
a certain parameter is always replaced by a constant. After that,
we can run the optimizer on this specialized function. If it
results in heavy gains, the specialized function is kept,
otherwise the original function is used instead.

## Cleanup

The cleanup is performed at the end of the optimizer run. It tries
to combine split expressions into deeply nested ones again and also
improves the "compilability" for stack machines by eliminating
variables as much as possible.

### Expression Joiner

This is the opposite operation of the expression splitter. It turns a sequence of
variable declarations that have exactly one reference into a complex expression.
This stage fully preserves the order of function calls and opcode executions.
It does not make use of any information concerning the commutability of opcodes;
if moving the value of a variable to its place of use would change the order
of any function call or opcode execution, the transformation is not performed.

Note that the component will not move the assigned value of a variable assignment
or a variable that is referenced more than once.

The snippet ``let x := add(0, 2) let y := mul(x, mload(2))`` is not transformed,
because it would cause the order of the call to the opcodes ``add`` and
``mload`` to be swapped - even though this would not make a difference
because ``add`` is movable.

When reordering opcodes like that, variable references and literals are ignored.
Because of that, the snippet ``let x := add(0, 2) let y := mul(x, 3)`` is
transformed to ``let y := mul(add(0, 2), 3)``, even though the ``add`` opcode
would be executed after the evaluation of the literal ``3``.

### SSA Reverser

This is a tiny step that helps in reversing the effects of the SSA transform
if it is combined with the Common Subexpression Eliminator and the
Unused Pruner.

The SSA form we generate is detrimental to code generation on the EVM and
WebAssembly alike because it generates many local variables. It would
be better to just re-use existing variables with assignments instead of
fresh variable declarations.

The SSA transform rewrites

    a := E
    mstore(a, 1)

to

    let a_1 := E
    a := a_1
    mstore(a_1, 1)

The problem is that instead of ``a``, the variable ``a_1`` is used
whenever ``a`` was referenced. The SSA transform changes statements
of this form by just swapping out the declaration and the assignment. The above
snippet is turned into

    a := E
    let a_1 := a
    mstore(a_1, 1)

This is a very simple equivalence transform, but when we now run the
Common Subexpression Eliminator, it will replace all occurrences of ``a_1``
by ``a`` (until ``a`` is re-assigned). The Unused Pruner will then
eliminate the variable ``a_1`` altogether and thus fully reverse the
SSA transform.

### Stack Compressor

One problem that makes code generation for the Ethereum Virtual Machine
hard is the fact that there is a hard limit of 16 slots for reaching
down the expression stack. This more or less translates to a limit
of 16 local variables. The stack compressor takes Yul code and
compiles it to EVM bytecode. Whenever the stack difference is too
large, it records the function this happened in.

For each function that caused such a problem, the Rematerialiser
is called with a special request to aggressively eliminate specific
variables sorted by the cost of their values.

On failure, this procedure is repeated multiple times.

### Rematerialiser

The rematerialisation stage tries to replace variable references by the expression that
was last assigned to the variable. This is of course only beneficial if this expression
is comparatively cheap to evaluate. Furthermore, it is only semantically equivalent if
the value of the expression did not change between the point of assignment and the
point of use. The main benefit of this stage is that it can save stack slots if it
leads to a variable being eliminated completely (see below), but it can also
save a DUP opcode on the EVM if the expression is very cheap.

The Rematerialiser uses the Dataflow Analyzer to track the current values of variables,
which are always movable.
If the value is very cheap or the variable was explicitly requested to be eliminated,
the variable reference is replaced by its current value.

## WebAssembly specific

### Main Function

Changes the topmost block to be a function with a specific name ("main") which has no
inputs nor outputs.

Depends on the Function Grouper.
