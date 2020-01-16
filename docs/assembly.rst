.. _inline-assembly:

###############
Inline Assembly
###############

.. index:: ! assembly, ! asm, ! evmasm


You can interleave Solidity statements with inline assembly in a language close
to the one of the Ethereum virtual machine. This gives you more fine-grained control,
which is especially useful when you are enhancing the language by writing libraries.

The language used for inline assembly in Solidity is called `Yul <yul>`_
and it is documented in its own section. This section will only cover
how the inline assembly code can interface with the surrounding Solidity code.


.. warning::
    Inline assembly is a way to access the Ethereum Virtual Machine
    at a low level. This bypasses several important safety
    features and checks of Solidity. You should only use it for
    tasks that need it, and only if you are confident with using it.


An inline assembly block is marked by ``assembly { ... }``, where the code inside
the curly braces is code in the `Yul <yul>`_ language.

The inline assembly code can access local Solidity variables as explained below.

Different inline assembly blocks share no namespace, i.e. it is not possible
to call a Yul function or access a Yul variable defined in a different inline assembly block.

Example
-------

The following example provides library code to access the code of another contract and
load it into a ``bytes`` variable. This is not possible with "plain Solidity" and the
idea is that reusable assembly libraries can enhance the Solidity language
without a compiler change.

.. code::

    pragma solidity >=0.4.0 <0.7.0;

    library GetCode {
        function at(address _addr) public view returns (bytes memory o_code) {
            assembly {
                // retrieve the size of the code, this needs assembly
                let size := extcodesize(_addr)
                // allocate output byte array - this could also be done without assembly
                // by using o_code = new bytes(size)
                o_code := mload(0x40)
                // new "memory end" including padding
                mstore(0x40, add(o_code, and(add(add(size, 0x20), 0x1f), not(0x1f))))
                // store length in memory
                mstore(o_code, size)
                // actually retrieve the code, this needs assembly
                extcodecopy(_addr, add(o_code, 0x20), 0, size)
            }
        }
    }

Inline assembly is also beneficial in cases where the optimizer fails to produce
efficient code, for example:

.. code::

    pragma solidity >=0.4.16 <0.7.0;


    library VectorSum {
        // This function is less efficient because the optimizer currently fails to
        // remove the bounds checks in array access.
        function sumSolidity(uint[] memory _data) public pure returns (uint sum) {
            for (uint i = 0; i < _data.length; ++i)
                sum += _data[i];
        }

        // We know that we only access the array in bounds, so we can avoid the check.
        // 0x20 needs to be added to an array because the first slot contains the
        // array length.
        function sumAsm(uint[] memory _data) public pure returns (uint sum) {
            for (uint i = 0; i < _data.length; ++i) {
                assembly {
                    sum := add(sum, mload(add(add(_data, 0x20), mul(i, 0x20))))
                }
            }
        }

        // Same as above, but accomplish the entire code within inline assembly.
        function sumPureAsm(uint[] memory _data) public pure returns (uint sum) {
            assembly {
                // Load the length (first 32 bytes)
                let len := mload(_data)

                // Skip over the length field.
                //
                // Keep temporary variable so it can be incremented in place.
                //
                // NOTE: incrementing _data would result in an unusable
                //       _data variable after this assembly block
                let data := add(_data, 0x20)

                // Iterate until the bound is not met.
                for
                    { let end := add(data, mul(len, 0x20)) }
                    lt(data, end)
                    { data := add(data, 0x20) }
                {
                    sum := add(sum, mload(data))
                }
            }
        }
    }



Access to External Variables, Functions and Libraries
-----------------------------------------------------

You can access Solidity variables and other identifiers by using their name.

Local variables of value type are directly usable in inline assembly.

Local variables that refer to memory or calldata evaluate to the
address of the variable in memory, resp. calldata, not the value itself.

For local storage variables or state variables, a single Yul identifier
is not sufficient, since they do not necessarily occupy a single full storage slot.
Therefore, their "address" is composed of a slot and a byte-offset
inside that slot. To retrieve the slot pointed to by the variable ``x``, you
use ``x_slot``, and to retrieve the byte-offset you use ``x_offset``.

Local Solidity variables are available for assignments, for example:

.. code::

    pragma solidity >=0.4.11 <0.7.0;

    contract C {
        uint b;
        function f(uint x) public view returns (uint r) {
            assembly {
                // We ignore the storage slot offset, we know it is zero
                // in this special case.
                r := mul(x, sload(b_slot))
            }
        }
    }

.. warning::
    If you access variables of a type that spans less than 256 bits
    (for example ``uint64``, ``address``, ``bytes16`` or ``byte``),
    you cannot make any assumptions about bits not part of the
    encoding of the type. Especially, do not assume them to be zero.
    To be safe, always clear the data properly before you use it
    in a context where this is important:
    ``uint32 x = f(); assembly { x := and(x, 0xffffffff) /* now use x */ }``
    To clean signed types, you can use the ``signextend`` opcode:
    ``assembly { signextend(<num_bytes_of_x_minus_one>, x) }``


Since Solidity 0.6.0 the name of a inline assembly variable may not end in ``_offset`` or ``_slot``
and it may not shadow any declaration visible in the scope of the inline assembly block
(including variable, contract and function declarations). Similarly, if the name of a declared
variable contains a dot ``.``, the prefix up to the ``.`` may not conflict with any
declaration visible in the scope of the inline assembly block.


Assignments are possible to assembly-local variables and to function-local
variables. Take care that when you assign to variables that point to
memory or storage, you will only change the pointer and not the data.



Things to Avoid
---------------

Inline assembly might have a quite high-level look, but it actually is extremely
low-level. Function calls, loops, ifs and switches are converted by simple
rewriting rules and after that, the only thing the assembler does for you is re-arranging
functional-style opcodes, counting stack height for
variable access and removing stack slots for assembly-local variables when the end
of their block is reached.

Conventions in Solidity
-----------------------

In contrast to EVM assembly, Solidity has types which are narrower than 256 bits,
e.g. ``uint24``. For efficiency, most arithmetic operations ignore the fact that
types can be shorter than 256
bits, and the higher-order bits are cleaned when necessary,
i.e., shortly before they are written to memory or before comparisons are performed.
This means that if you access such a variable
from within inline assembly, you might have to manually clean the higher-order bits
first.

Solidity manages memory in the following way. There is a "free memory pointer"
at position ``0x40`` in memory. If you want to allocate memory, use the memory
starting from where this pointer points at and update it.
There is no guarantee that the memory has not been used before and thus
you cannot assume that its contents are zero bytes.
There is no built-in mechanism to release or free allocated memory.
Here is an assembly snippet you can use for allocating memory that follows the process outlined above::

    function allocate(length) -> pos {
      pos := mload(0x40)
      mstore(0x40, add(pos, length))
    }

The first 64 bytes of memory can be used as "scratch space" for short-term
allocation. The 32 bytes after the free memory pointer (i.e., starting at ``0x60``)
are meant to be zero permanently and is used as the initial value for
empty dynamic memory arrays.
This means that the allocatable memory starts at ``0x80``, which is the initial value
of the free memory pointer.

Elements in memory arrays in Solidity always occupy multiples of 32 bytes (this is
even true for ``byte[]``, but not for ``bytes`` and ``string``). Multi-dimensional memory
arrays are pointers to memory arrays. The length of a dynamic array is stored at the
first slot of the array and followed by the array elements.

.. warning::
    Statically-sized memory arrays do not have a length field, but it might be added later
    to allow better convertibility between statically- and dynamically-sized arrays, so
    do not rely on this.

