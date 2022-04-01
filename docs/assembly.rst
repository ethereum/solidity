.. _inline-assembly:

###############
Inline Assembly
###############

.. index:: ! assembly, ! asm, ! evmasm


You can interleave Solidity statements with inline assembly in a language close
to the one of the Ethereum virtual machine. This gives you more fine-grained control,
which is especially useful when you are enhancing the language by writing libraries.

The language used for inline assembly in Solidity is called :ref:`Yul <yul>`
and it is documented in its own section. This section will only cover
how the inline assembly code can interface with the surrounding Solidity code.


.. warning::
    Inline assembly is a way to access the Ethereum Virtual Machine
    at a low level. This bypasses several important safety
    features and checks of Solidity. You should only use it for
    tasks that need it, and only if you are confident with using it.


An inline assembly block is marked by ``assembly { ... }``, where the code inside
the curly braces is code in the :ref:`Yul <yul>` language.

The inline assembly code can access local Solidity variables as explained below.

Different inline assembly blocks share no namespace, i.e. it is not possible
to call a Yul function or access a Yul variable defined in a different inline assembly block.

Example
-------

The following example provides library code to access the code of another contract and
load it into a ``bytes`` variable. This is possible with "plain Solidity" too, by using
``<address>.code``. But the point here is that reusable assembly libraries can enhance the
Solidity language without a compiler change.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;

    library GetCode {
        function at(address addr) public view returns (bytes memory code) {
            assembly {
                // retrieve the size of the code, this needs assembly
                let size := extcodesize(addr)
                // allocate output byte array - this could also be done without assembly
                // by using code = new bytes(size)
                code := mload(0x40)
                // new "memory end" including padding
                mstore(0x40, add(code, and(add(add(size, 0x20), 0x1f), not(0x1f))))
                // store length in memory
                mstore(code, size)
                // actually retrieve the code, this needs assembly
                extcodecopy(addr, add(code, 0x20), 0, size)
            }
        }
    }

Inline assembly is also beneficial in cases where the optimizer fails to produce
efficient code, for example:

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.16 <0.9.0;


    library VectorSum {
        // This function is less efficient because the optimizer currently fails to
        // remove the bounds checks in array access.
        function sumSolidity(uint[] memory data) public pure returns (uint sum) {
            for (uint i = 0; i < data.length; ++i)
                sum += data[i];
        }

        // We know that we only access the array in bounds, so we can avoid the check.
        // 0x20 needs to be added to an array because the first slot contains the
        // array length.
        function sumAsm(uint[] memory data) public pure returns (uint sum) {
            for (uint i = 0; i < data.length; ++i) {
                assembly {
                    sum := add(sum, mload(add(add(data, 0x20), mul(i, 0x20))))
                }
            }
        }

        // Same as above, but accomplish the entire code within inline assembly.
        function sumPureAsm(uint[] memory data) public pure returns (uint sum) {
            assembly {
                // Load the length (first 32 bytes)
                let len := mload(data)

                // Skip over the length field.
                //
                // Keep temporary variable so it can be incremented in place.
                //
                // NOTE: incrementing data would result in an unusable
                //       data variable after this assembly block
                let dataElementLocation := add(data, 0x20)

                // Iterate until the bound is not met.
                for
                    { let end := add(dataElementLocation, mul(len, 0x20)) }
                    lt(dataElementLocation, end)
                    { data := add(dataElementLocation, 0x20) }
                {
                    sum := add(sum, mload(dataElementLocation))
                }
            }
        }
    }



Access to External Variables, Functions and Libraries
-----------------------------------------------------

You can access Solidity variables and other identifiers by using their name.

Local variables of value type are directly usable in inline assembly.
They can both be read and assigned to.

Local variables that refer to memory evaluate to the address of the variable in memory not the value itself.
Such variables can also be assigned to, but note that an assignment will only change the pointer and not the data
and that it is your responsibility to respect Solidity's memory management.
See :ref:`Conventions in Solidity <conventions-in-solidity>`.

Similarly, local variables that refer to statically-sized calldata arrays or calldata structs
evaluate to the address of the variable in calldata, not the value itself.
The variable can also be assigned a new offset, but note that no validation to ensure that
the variable will not point beyond ``calldatasize()`` is performed.

For external function pointers the address and the function selector can be
accessed using ``x.address`` and ``x.selector``.
The selector consists of four right-aligned bytes.
Both values can be assigned to. For example:

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.8.10 <0.9.0;

    contract C {
        // Assigns a new selector and address to the return variable @fun
        function combineToFunctionPointer(address newAddress, uint newSelector) public pure returns (function() external fun) {
            assembly {
                fun.selector := newSelector
                fun.address  := newAddress
            }
        }
    }

For dynamic calldata arrays, you can access
their calldata offset (in bytes) and length (number of elements) using ``x.offset`` and ``x.length``.
Both expressions can also be assigned to, but as for the static case, no validation will be performed
to ensure that the resulting data area is within the bounds of ``calldatasize()``.

For local storage variables or state variables, a single Yul identifier
is not sufficient, since they do not necessarily occupy a single full storage slot.
Therefore, their "address" is composed of a slot and a byte-offset
inside that slot. To retrieve the slot pointed to by the variable ``x``, you
use ``x.slot``, and to retrieve the byte-offset you use ``x.offset``.
Using ``x`` itself will result in an error.

You can also assign to the ``.slot`` part of a local storage variable pointer.
For these (structs, arrays or mappings), the ``.offset`` part is always zero.
It is not possible to assign to the ``.slot`` or ``.offset`` part of a state variable,
though.

Local Solidity variables are available for assignments, for example:

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.7.0 <0.9.0;

    contract C {
        uint b;
        function f(uint x) public view returns (uint r) {
            assembly {
                // We ignore the storage slot offset, we know it is zero
                // in this special case.
                r := mul(x, sload(b.slot))
            }
        }
    }

.. warning::
    If you access variables of a type that spans less than 256 bits
    (for example ``uint64``, ``address``, or ``bytes16``),
    you cannot make any assumptions about bits not part of the
    encoding of the type. Especially, do not assume them to be zero.
    To be safe, always clear the data properly before you use it
    in a context where this is important:
    ``uint32 x = f(); assembly { x := and(x, 0xffffffff) /* now use x */ }``
    To clean signed types, you can use the ``signextend`` opcode:
    ``assembly { signextend(<num_bytes_of_x_minus_one>, x) }``


Since Solidity 0.6.0 the name of a inline assembly variable may not
shadow any declaration visible in the scope of the inline assembly block
(including variable, contract and function declarations).

Since Solidity 0.7.0, variables and functions declared inside the
inline assembly block may not contain ``.``, but using ``.`` is
valid to access Solidity variables from outside the inline assembly block.

Things to Avoid
---------------

Inline assembly might have a quite high-level look, but it actually is extremely
low-level. Function calls, loops, ifs and switches are converted by simple
rewriting rules and after that, the only thing the assembler does for you is re-arranging
functional-style opcodes, counting stack height for
variable access and removing stack slots for assembly-local variables when the end
of their block is reached.

.. _conventions-in-solidity:

Conventions in Solidity
-----------------------

.. _assembly-typed-variables:

Values of Typed Variables
=========================

In contrast to EVM assembly, Solidity has types which are narrower than 256 bits,
e.g. ``uint24``. For efficiency, most arithmetic operations ignore the fact that
types can be shorter than 256
bits, and the higher-order bits are cleaned when necessary,
i.e., shortly before they are written to memory or before comparisons are performed.
This means that if you access such a variable
from within inline assembly, you might have to manually clean the higher-order bits
first.

.. _assembly-memory-management:

Memory Management
=================

This section amounts to a brief recap of :ref:`Reserved Memory Areas and Memory Management Details`
and :ref:`Layout in Memory`. Please read those documents before you attempt to
use memory via inline assembly.

Solidity is a high level language targetting the EVM. It implements complex data
structures (such as multidimensional arrays) that require memory to be carefully
managed so as to avoid collisions. This memory management happens by convention.

Specifically, there is a "free memory pointer" at memory location ``0x40``.
This pointer indicates the first memory location beyond which memory can be written
to without colliding with Solidity data structures (variables) that have been
previously allocated. There is no guarantee that the memory beyond the location
pointed to by the free memory pointed has not been used before and thus
you cannot assume that its contents are zero bytes. For similar reasons, you
cannot rely on that memory not being written to in unexpected ways.

In fact, unless you are using the memory beyond the free memory pointer only within
inline assembly between Solidity statements, you should expect that it may
be arbitrarily changed by other code (in the same way that your code is changing it).
Therefore, you may wish to update the free memory pointer so that it points beyond
the memory you want to use.

This amounts to "allocating" memory for yourself. There is no built-in mechanism
to release or free allocated memory.

The following assembly snippet allocates `length` bytes which you can then safely
use indefinitely:

.. code-block:: yul

    function allocate(length) -> pos {
      pos := mload(0x40)
      mstore(0x40, add(pos, length))
    }

In addition to the memory beyond the free memory pointer, the memory between ``0x00``
and ``0x3F`` (inclusive, i.e. the first 64 bytes of memory) is "scratch space"
available for short-term use.

If you are accessing or writing to Solidity variables and data structures in memory
or storage via inline assembly, you should read these relevant documents first:

  * :ref:`Reserved Memory Areas and Memory Management Details`
  * :ref:`Layout in Memory`
  * :ref:`Layout of State Variables in Storage`
  

Memory Safety
=============

To understand this section, you should first read :ref:`Reserved Memory Areas and Memory Management Details`

The compiler generates code that relies on memory to remain in a well-defined state at all times.
Memory management is especially important given :ref:`the new code generation pipeline via Yul IR <ir-breaking-changes>`.
This code generation path may move local variables from stack to memory to avoid stack-too-deep errors and
it may perform additional memory optimizations. It will not work correctly unless memory remains in a well-defined state.

Because Solidity cannot enforce its memory management and layout conventions inside inline assembly blocks,
it defends against potentially invalid memory states caused by inline assembly by disabling
various optimizations and memory movements (including those that help manage the stack) in the presence
of any inline assembly block that contains a memory operation or assigns to solidity variables in memory.

However, to prevent this, you can specifically annotate an assembly block as "memory-safe":

.. code-block:: solidity

    assembly ("memory-safe") {
        ...
    }

Memory safety (for current versions of Solidity) can be boiled down to not accessing memory that is
forbidden. Forbidden memory includes:
  * the zero slot at memory location ``0x60``
  * any memory that would not safely be readable by a Solidity statement (outside an inline assembly block)

Exceptions include:
  * the scratch area between ``0x00`` and ``0x3F`` (0 to 64, inclusive)
  * the scratch area beyond what the free memory pointer points to (see :ref:`Reserved Memory Areas and Memory Management Details`)
  * memory you have allocated yourself by increasing the free memory pointer (it should never be decreased)

You should only mark an inline assembly block as "memory-safe" if it also does not cause operations
after it to be unsafe. This could happen if Solidity generated code that runs later, accesses forbidden memory
because of the values you wrote. This can happen by writing invalid values into Solidity
variables of reference type, or by changing other values relied upon by Solidity's data structures in memory.

For example, as described in :ref:`Layout of Arrays In Memory`, dynamically sized arrays (e.g. `uint[] varArray;`)
encode their length into the first memory slot pointed to by that variable. It would be unsafe for inline assembly
to change that length. It would also be unsafe for inline assembly to change the value of the array variable
itself (the pointer) such that it points to some arbitrary location that is not also a dynamically sized array.
Similarly, multi-dimensional arrays are arrays of pointers to arrays. It would be unsafe for inline assembly to
change the memory location pointed to by an element of a multi-dimensional array.

This description is intentionally somewhat ambiguous. It may technically not lead to unexpected
behaviors to shorten the length of an array, or the repoint a multi-dimensional array element to another
memory area with equivalent structure. However, it is better for you to consider these unsafe operations.
Similarly, there may technically be no consequence from reading (vs writing) "any" memory location, even one in
a forbidden area, but it is better for you to consider this too, an unsafe operation.

In brief, if it's outside of the scratch areas or memory you've allocated yourself, don't write to it in ways
that you can't using Solidity statements (vs inline assembly).

Memory safety must be maintained even if the inline assembly block immediately reverts or terminates. For example,
the following inline assembly should not be marked memory safe because `returndatasize()` may be larger than
64, causing the `returndatacopy()` to write beyond the end of the 64 byte scratch area located at memory location 0.

.. code-block:: solidity

    assembly {
      returndatacopy(0, 0, returndatasize()); // could overwrite the free memory pointer at 0x40
      revert(0, returndatasize());
    }

By contrast, the following inline assembly block is memory safe:

.. code-block:: solidity

    assembly ("memory-safe") {
      let p := mload(0x40);
      returndatacopy(p, 0, returndatasize());
      revert(p, returndatasize());
    }

If your inline assembly memory operations are parameterized with a zero length, they are memory safe. For example,
this code is safe:

.. code-block:: solidity

    assembly ("memory-safe") {
      revert(0, 0)
    }

It is also unsafe to cause Solidity statements to access the scratch areas, or to cause a memory variable of
reference type to point to a location that does not actually store a value of that type. For example:

.. code-block:: solidity

    bytes memory x;
    assembly {
      x := 0x40
    }
    x[0x20] = 0x42;

Inline assembly that neither involves any operations that access memory nor assigns to any solidity variables
in memory is automatically considered memory-safe and does not need to be annotated.

.. warning::
    It is your responsibility to make sure that the assembly actually satisfies the memory model. If you annotate
    an assembly block as memory-safe, but violate one of the memory assumptions, this **will** lead to incorrect and
    undefined behaviour that cannot easily be discovered by testing.

In case you are developing a library that is meant to be compatible across multiple versions
of solidity, you can use a special comment to annotate an assembly block as memory-safe:

.. code-block:: solidity

    /// @solidity memory-safe-assembly
    assembly {
        ...
    }

Note that we will disallow the annotation via comment in a future breaking release, so if you are not concerned with
backwards-compatibility with older compiler versions, prefer using the dialect string.
