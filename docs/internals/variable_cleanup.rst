.. index: variable cleanup

*************************
Cleaning of Padding Bits
*************************

When a value is shorter than 256 bit, in some cases the remaining (padding) bits
must be cleaned up, by sign extending the shorter value. The Solidity
compiler is designed to clean up padding bits before operations
that might be adversely affected by garbage in those padding bits.
For example, before writing a value to  memory, the padding bits need
to be cleaned because the memory contents could be used for computing
hashes or sent as the data of a message call. Similarly, before
storing a value in storage, padding bits need to be cleaned
because otherwise their value can be observed.

Note that inline assembly instructions do not automatically clean padding bits
of values used therein.
Furthermore, if you use inline assembly to access and modify Solidity variables
shorter than 256 bits, the compiler does not do any automated
cleanup of the padding bits either. It is up to you to store cleaned
values back into the source memory or storage location.

Moreover, the assembly emitted by the compiler may not zero the
padding bits of values,
particularly when the following instruction(s) would behave the
same irrespective of the content of the padding bits. Low level
temporary values that are inputs to the `JUMPI` opcode are one
example.

In addition, the Solidity compiler
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
