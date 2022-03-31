.. index: variable cleanup

*********************
Zeroing of Padding
*********************

When a value is shorter than 256 bit, in some cases the remaining (padding) bits
must be zeroed. The Solidity compiler is designed to zero any padding bits before operations
that might be adversely affected by garbage in the padding bits.
For example, before writing a value to  memory, the padding bits need
to be cleared because the memory contents could be used for computing
hashes or sent as the data of a message call. Similarly, before
storing a value in storage, padding bits need to be cleaned
because otherwise their value can be observed, and may leak information.

Note that inline assembly does not automatically zero padding bits
of values used therein.
If you use inline assembly to access and modify Solidity variables
shorter than 256 bits, the compiler does not do any automated
cleanup of the padding bits. It is up to you to store zeroed
values back into the source memory or storage location.

Moreover, the assembly emitted by the compiler may not zero the
padding bits of temporary values that are not stored anywhere,
particularly when the following instruction(s) would behave the
same irrespective of the content of the padding bits. Low level
temporary values that are inputs to the `JUMPI` opcode are one
example.

In addition to the design principle above, the Solidity compiler
cleans input data when it is loaded onto the stack.

Different types have different rules for cleaning up invalid values:

+---------------+---------------+-------------------+
|Type           |Valid Values   |Invalid Values Mean|
+===============+===============+===================+
|enum of n      |0 until n - 1  |exception          |
|members        |               |                   |
+---------------+---------------+-------------------+
|bool           |0 or 1         |1                  |
+---------------+---------------+-------------------+
|signed integers|sign-extended  |currently silently |
|               |word           |wraps; in the      |
|               |               |future exceptions  |
|               |               |will be thrown     |
|               |               |                   |
|               |               |                   |
+---------------+---------------+-------------------+
|unsigned       |higher bits    |currently silently |
|integers       |zeroed         |wraps; in the      |
|               |               |future exceptions  |
|               |               |will be thrown     |
+---------------+---------------+-------------------+
