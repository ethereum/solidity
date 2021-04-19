.. index:: optimizer, common subexpression elimination, constant propagation

*************
The Optimiser
*************

This section discusses the optimiser that was first added to Solidity,
which operates on opcode streams. For information on the new Yul-based optimiser,
please see the `readme on github <https://github.com/ethereum/solidity/blob/develop/libyul/optimiser/README.md>`_.

The Solidity optimiser operates on assembly. It splits the sequence of instructions into basic blocks
at ``JUMPs`` and ``JUMPDESTs``. Inside these blocks, the optimiser
analyses the instructions and records every modification to the stack,
memory, or storage as an expression which consists of an instruction and
a list of arguments which are pointers to other expressions. The optimiser
uses a component called "CommonSubexpressionEliminator" that amongst other
tasks, finds expressions that are always equal (on every input) and combines
them into an expression class. The optimiser first tries to find each new
expression in a list of already known expressions. If this does not work,
it simplifies the expression according to rules like
``constant + constant = sum_of_constants`` or ``X * 1 = X``. Since this is
a recursive process, we can also apply the latter rule if the second factor
is a more complex expression where we know that it always evaluates to one.
Modifications to storage and memory locations have to erase knowledge about
storage and memory locations which are not known to be different. If we first
write to location x and then to location y and both are input variables, the
second could overwrite the first, so we do not know what is stored at x after
we wrote to y. If simplification of the expression x - y evaluates to a
non-zero constant, we know that we can keep our knowledge about what is stored at x.

After this process, we know which expressions have to be on the stack at
the end, and have a list of modifications to memory and storage. This information
is stored together with the basic blocks and is used to link them. Furthermore,
knowledge about the stack, storage and memory configuration is forwarded to
the next block(s). If we know the targets of all ``JUMP`` and ``JUMPI`` instructions,
we can build a complete control flow graph of the program. If there is only
one target we do not know (this can happen as in principle, jump targets can
be computed from inputs), we have to erase all knowledge about the input state
of a block as it can be the target of the unknown ``JUMP``. If the optimiser
finds a ``JUMPI`` whose condition evaluates to a constant, it transforms it
to an unconditional jump.

As the last step, the code in each block is re-generated. The optimiser creates
a dependency graph from the expressions on the stack at the end of the block,
and it drops every operation that is not part of this graph. It generates code
that applies the modifications to memory and storage in the order they were
made in the original code (dropping modifications which were found not to be
needed). Finally, it generates all values that are required to be on the
stack in the correct place.

These steps are applied to each basic block and the newly generated code
is used as replacement if it is smaller. If a basic block is split at a
``JUMPI`` and during the analysis, the condition evaluates to a constant,
the ``JUMPI`` is replaced depending on the value of the constant. Thus code like

::

    uint x = 7;
    data[7] = 9;
    if (data[x] != x + 2)
      return 2;
    else
      return 1;

still simplifies to code which you can compile even though the instructions contained
a jump in the beginning of the process:

::

    data[7] = 9;
    return 1;

Simple Inlining
---------------

Since Solidity version 0.8.2, there is another optimizer step that replaces certain
jumps to blocks containing "simple" instructions ending with a "jump" by a copy of these instructions.
This corresponds to inlining of simple, small Solidity or Yul functions. In particular, the sequence
``PUSHTAG(tag) JUMP`` may be replaced, whenever the ``JUMP`` is marked as jump "into" a
function and behind ``tag`` there is a basic block (as described above for the
"CommonSubexpressionEliminator") that ends in another ``JUMP`` which is marked as a jump
"out of" a function.
In particular, consider the following prototypical example of assembly generated for a
call to an internal Solidity function:

.. code-block:: text

      tag_return
      tag_f
      jump      // in
    tag_return:
      ...opcodes after call to f...

    tag_f:
      ...body of function f...
      jump      // out

As long as the body of the function is a continuous basic block, the "Inliner" can replace ``tag_f jump`` by
the block at ``tag_f`` resulting in:

.. code-block:: text

      tag_return
      ...body of function f...
      jump
    tag_return:
      ...opcodes after call to f...

    tag_f:
      ...body of function f...
      jump      // out

Now ideally, the other optimiser steps described above will result in the return tag push being moved
towards the remaining jump resulting in:

.. code-block:: text

      ...body of function f...
      tag_return
      jump
    tag_return:
      ...opcodes after call to f...

    tag_f:
      ...body of function f...
      jump      // out

In this situation the "PeepholeOptimizer" will remove the return jump. Ideally, all of this can be done
for all references to ``tag_f`` leaving it unused, s.t. it can be removed, yielding:

.. code-block:: text

      ...body of function f...
      ...opcodes after call to f...

So the call to function ``f`` is inlined and the original definition of ``f`` can be removed.

Inlining like this is attempted, whenever a heuristics suggests that inlining is cheaper over the lifetime of a
contract than not inlining. This heuristics depends on the size of the function body, the
number of other references to its tag (approximating the number of calls to the function) and
the expected number of executions of the contract (the global optimiser parameter "runs").
