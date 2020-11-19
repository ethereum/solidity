
.. index: calldata layout

*******************
Layout of Call Data
*******************

The input data for a function call is assumed to be in the format defined by the :ref:`ABI
specification <ABI>`. Among others, the ABI specification requires arguments to be padded to multiples of 32
bytes. The internal function calls use a different convention.

Arguments for the constructor of a contract are directly appended at the end of the
contract's code, also in ABI encoding. The constructor will access them through a hard-coded offset, and
not by using the ``codesize`` opcode, since this of course changes when appending
data to the code.

