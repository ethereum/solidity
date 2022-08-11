.. index: variable cleanup

*********************
Cleaning Up Variables
*********************

Ultimately, all values in the EVM are stored in 256 bit words.
Thus, in some cases, when the type of a value has less than 256 bits,
it is necessary to clean the remaining bits.
The Solidity compiler is designed to do such cleaning before any operations
that might be adversely affected by the potential garbage in the remaining bits.
For example, before writing a value to  memory, the remaining bits need
to be cleared because the memory contents can be used for computing
hashes or sent as the data of a message call.  Similarly, before
storing a value in the storage, the remaining bits need to be cleaned
because otherwise the garbled value can be observed.

Note that access via inline assembly is not considered such an operation:
If you use inline assembly to access Solidity variables
shorter than 256 bits, the compiler does not guarantee that
the value is properly cleaned up.

Moreover, we do not clean the bits if the immediately
following operation is not affected.  For instance, since any non-zero
value is considered ``true`` by ``JUMPI`` instruction, we do not clean
the boolean values before they are used as the condition for
``JUMPI``.

In addition to the design principle above, the Solidity compiler
cleans input data when it is loaded onto the stack.

The following table describes the cleaning rules applied to different types,
where ``higher bits`` refers to the remaining bits in case the type has less than 256 bits.

+---------------+---------------+-------------------------+
|Type           |Valid Values   |Cleanup of Invalid Values|
+===============+===============+=========================+
|enum of n      |0 until n - 1  |throws exception         |
|members        |               |                         |
+---------------+---------------+-------------------------+
|bool           |0 or 1         |results in 1             |
+---------------+---------------+-------------------------+
|signed integers|higher bits    |currently silently       |
|               |set to the     |signextends to a valid   |
|               |sign bit       |value, i.e. all higher   |
|               |               |bits are set to the sign |
|               |               |bit; may throw an        |
|               |               |exception in the future  |
+---------------+---------------+-------------------------+
|unsigned       |higher bits    |currently silently masks |
|integers       |zeroed         |to a valid value, i.e.   |
|               |               |all higher bits are set  |
|               |               |to zero; may throw an    |
|               |               |exception in the future  |
+---------------+---------------+-------------------------+

Note that valid and invalid values are dependent on their type size.
Consider ``uint8``, the unsigned 8-bit type, which has the following valid values:

.. code-block:: none

    0000...0000 0000 0000
    0000...0000 0000 0001
    0000...0000 0000 0010
    ....
    0000...0000 1111 1111

Any invalid value will have the higher bits set to zero:

.. code-block:: none

    0101...1101 0010 1010   invalid value
    0000...0000 0010 1010   cleaned value

For ``int8``, the signed 8-bit type, the valid values are:

Negative

.. code-block:: none

    1111...1111 1111 1111
    1111...1111 1111 1110
    ....
    1111...1111 1000 0000

Positive

.. code-block:: none

    0000...0000 0000 0000
    0000...0000 0000 0001
    0000...0000 0000 0010
    ....
    0000...0000 1111 1111

The compiler will ``signextend`` the sign bit, which is 1 for negative and 0 for
positive values, overwriting the higher bits:

Negative

.. code-block:: none

    0010...1010 1111 1111   invalid value
    1111...1111 1111 1111   cleaned value

Positive

.. code-block:: none

    1101...0101 0000 0100   invalid value
    0000...0000 0000 0100   cleaned value
