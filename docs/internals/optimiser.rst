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
