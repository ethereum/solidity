.. index:: type

.. _types:

*****
Types
*****

Solidity is a statically typed language, which means that the type of each
variable (state and local) needs to be specified.
Solidity provides several elementary types which can be combined to form complex types.

In addition, types can interact with each other in expressions containing
operators. For a quick reference of the various operators, see :ref:`order`.

The concept of "undefined" or "null" values does not exist in Solidity, but newly
declared variables always have a :ref:`default value<default-value>` dependent
on its type. To handle any unexpected values, you should use the :ref:`revert function<assert-and-require>` to revert the whole transaction, or return a
tuple with a second `bool` value denoting success.

.. index:: ! value type, ! type;value
.. _value-types:

Value Types
===========

The following types are also called value types because variables of these
types will always be passed by value, i.e. they are always copied when they
are used as function arguments or in assignments.

.. index:: ! bool, ! true, ! false

Booleans
--------

``bool``: The possible values are constants ``true`` and ``false``.

Operators:

*  ``!`` (logical negation)
*  ``&&`` (logical conjunction, "and")
*  ``||`` (logical disjunction, "or")
*  ``==`` (equality)
*  ``!=`` (inequality)

The operators ``||`` and ``&&`` apply the common short-circuiting rules. This means that in the expression ``f(x) || g(y)``, if ``f(x)`` evaluates to ``true``, ``g(y)`` will not be evaluated even if it may have side-effects.

.. index:: ! uint, ! int, ! integer

Integers
--------

``int`` / ``uint``: Signed and unsigned integers of various sizes. Keywords ``uint8`` to ``uint256`` in steps of ``8`` (unsigned of 8 up to 256 bits) and ``int8`` to ``int256``. ``uint`` and ``int`` are aliases for ``uint256`` and ``int256``, respectively.

Operators:

* Comparisons: ``<=``, ``<``, ``==``, ``!=``, ``>=``, ``>`` (evaluate to ``bool``)
* Bit operators: ``&``, ``|``, ``^`` (bitwise exclusive or), ``~`` (bitwise negation)
* Shift operators: ``<<`` (left shift), ``>>`` (right shift)
* Arithmetic operators: ``+``, ``-``, unary ``-``, ``*``, ``/``, ``%`` (modulo), ``**`` (exponentiation)


Comparisons
^^^^^^^^^^^

The value of a comparison is the one obtained by comparing the integer value.

Bit operations
^^^^^^^^^^^^^^

Bit operations are performed on the two's complement representation of the number.
This means that, for example ``~int256(0) == int256(-1)``.

Shifts
^^^^^^

The result of a shift operation has the type of the left operand. The
expression ``x << y`` is equivalent to ``x * 2**y``, and, for positive integers,
``x >> y`` is equivalent to ``x / 2**y``. For negative ``x``, ``x >> y``
is equivalent to dividing by a power of ``2`` while rounding down (towards negative infinity).
Shifting by a negative amount throws a runtime exception.

.. warning::
    Before version ``0.5.0`` a right shift ``x >> y`` for negative ``x`` was equivalent to ``x / 2**y``,
    i.e. right shifts used rounding towards zero instead of rounding towards negative infinity.

Addition, Subtraction and Multiplication
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Addition, subtraction and multiplication have the usual semantics.
They wrap in two's complement representation, meaning that
for example ``uint256(0) - uint256(1) == 2**256 - 1``. You have to take these overflows
into account when designing safe smart contracts.

The expression ``-x`` is equivalent to ``(T(0) - x)`` where
``T`` is the type of ``x``. This means that ``-x`` will not be negative
if the type of ``x`` is an unsigned integer type. Also, ``-x`` can be
positive if ``x`` is negative. There is another caveat also resulting
from two's complement representation::

    int x = -2**255;
    assert(-x == x);

This means that even if a number is negative, you cannot assume that
its negation will be positive.


Division
^^^^^^^^

Since the type of the result of an operation is always the type of one of
the operands, division on integers always results in an integer.
In Solidity, division rounds towards zero. This mean that ``int256(-5) / int256(2) == int256(-2)``.

Note that in contrast, division on :ref:`literals<rational_literals>` results in fractional values
of arbitrary precision.

.. note::
  Division by zero causes a failing assert.

Modulo
^^^^^^

The modulo operation ``a % n`` yields the remainder ``r`` after the division of the operand ``a``
by the operand ``n``, where ``q = int(a / n)`` and ``r = a - (n * q)``. This means that modulo
results in the same sign as its left operand (or zero) and ``a % n == -(abs(a) % n)`` holds for negative ``a``:

 * ``int256(5) % int256(2) == int256(1)``
 * ``int256(5) % int256(-2) == int256(1)``
 * ``int256(-5) % int256(2) == int256(-1)``
 * ``int256(-5) % int256(-2) == int256(-1)``

.. note::
  Modulo with zero causes a failing assert.

Exponentiation
^^^^^^^^^^^^^^

Exponentiation is only available for unsigned types. Please take care that the types
you are using are large enough to hold the result and prepare for potential wrapping behaviour.

.. note::
  Note that ``0**0`` is defined by the EVM as ``1``.

.. index:: ! ufixed, ! fixed, ! fixed point number

Fixed Point Numbers
-------------------

.. warning::
    Fixed point numbers are not fully supported by Solidity yet. They can be declared, but
    cannot be assigned to or from.

``fixed`` / ``ufixed``: Signed and unsigned fixed point number of various sizes. Keywords ``ufixedMxN`` and ``fixedMxN``, where ``M`` represents the number of bits taken by
the type and ``N`` represents how many decimal points are available. ``M`` must be divisible by 8 and goes from 8 to 256 bits. ``N`` must be between 0 and 80, inclusive.
``ufixed`` and ``fixed`` are aliases for ``ufixed128x18`` and ``fixed128x18``, respectively.

Operators:

* Comparisons: ``<=``, ``<``, ``==``, ``!=``, ``>=``, ``>`` (evaluate to ``bool``)
* Arithmetic operators: ``+``, ``-``, unary ``-``, ``*``, ``/``, ``%`` (modulo)

.. note::
    The main difference between floating point (``float`` and ``double`` in many languages, more precisely IEEE 754 numbers) and fixed point numbers is
    that the number of bits used for the integer and the fractional part (the part after the decimal dot) is flexible in the former, while it is strictly
    defined in the latter. Generally, in floating point almost the entire space is used to represent the number, while only a small number of bits define
    where the decimal point is.

.. index:: address, balance, send, call, callcode, delegatecall, staticcall, transfer

.. _address:

Address
-------

The address type comes in two flavours, which are largely identical:

 - ``address``: Holds a 20 byte value (size of an Ethereum address).
 - ``address payable``: Same as ``address``, but with the additional members ``transfer`` and ``send``.

The idea behind this distinction is that ``address payable`` is an address you can send Ether to,
while a plain ``address`` cannot be sent Ether.

Type conversions:

Implicit conversions from ``address payable`` to ``address`` are allowed, whereas conversions from ``address`` to ``address payable`` are
not possible (the only way to perform such a conversion is by using an intermediate conversion to ``uint160``).

:ref:`Address literals<address_literals>` can be implicitly converted to ``address payable``.

Explicit conversions to and from ``address`` are allowed for integers, integer literals, ``bytes20`` and contract types with the following
caveat:
Conversions of the form ``address payable(x)`` are not allowed. Instead the result of a conversion of the form ``address(x)``
has the type ``address payable``, if ``x`` is of integer or fixed bytes type, a literal or a contract with a payable fallback function.
If ``x`` is a contract without payable fallback function, then ``address(x)`` will be of type ``address``.
In external function signatures ``address`` is used for both the ``address`` and the ``address payable`` type.

.. note::
    It might very well be that you do not need to care about the distinction between ``address``
    and ``address payable`` and just use ``address`` everywhere. For example,
    if you are using the :ref:`withdrawal pattern<withdrawal_pattern>`, you can (and should) store the
    address itself as ``address``, because you invoke the ``transfer`` function on
    ``msg.sender``, which is an ``address payable``.

Operators:

* ``<=``, ``<``, ``==``, ``!=``, ``>=`` and ``>``

.. warning::
    If you convert a type that uses a larger byte size to an ``address``, for example ``bytes32``, then the ``address`` is truncated.
    To reduce conversion ambiguity version 0.4.24 and higher of the compiler force you make the truncation explicit in the conversion.
    Take for example the address ``0x111122223333444455556666777788889999AAAABBBBCCCCDDDDEEEEFFFFCCCC``.

    You can use ``address(uint160(bytes20(b)))``, which results in ``0x111122223333444455556666777788889999aAaa``,
    or you can use ``address(uint160(uint256(b)))``, which results in ``0x777788889999AaAAbBbbCcccddDdeeeEfFFfCcCc``.

.. note::
    The distinction between ``address`` and ``address payable`` was introduced with version 0.5.0.
    Also starting from that version, contracts do not derive from the address type, but can still be explicitly converted to
    ``address`` or to ``address payable``, if they have a payable fallback function.

.. _members-of-addresses:

Members of Addresses
^^^^^^^^^^^^^^^^^^^^

For a quick reference of all members of address, see :ref:`address_related`.

* ``balance`` and ``transfer``

It is possible to query the balance of an address using the property ``balance``
and to send Ether (in units of wei) to a payable address using the ``transfer`` function:

::

    address payable x = address(0x123);
    address myAddress = address(this);
    if (x.balance < 10 && myAddress.balance >= 10) x.transfer(10);

The ``transfer`` function fails if the balance of the current contract is not large enough
or if the Ether transfer is rejected by the receiving account. The ``transfer`` function
reverts on failure.

.. note::
    If ``x`` is a contract address, its code (more specifically: its :ref:`fallback-function`, if present) will be executed together with the ``transfer`` call (this is a feature of the EVM and cannot be prevented). If that execution runs out of gas or fails in any way, the Ether transfer will be reverted and the current contract will stop with an exception.

* ``send``

Send is the low-level counterpart of ``transfer``. If the execution fails, the current contract will not stop with an exception, but ``send`` will return ``false``.

.. warning::
    There are some dangers in using ``send``: The transfer fails if the call stack depth is at 1024
    (this can always be forced by the caller) and it also fails if the recipient runs out of gas. So in order
    to make safe Ether transfers, always check the return value of ``send``, use ``transfer`` or even better:
    use a pattern where the recipient withdraws the money.

* ``call``, ``delegatecall`` and ``staticcall``

In order to interface with contracts that do not adhere to the ABI,
or to get more direct control over the encoding,
the functions ``call``, ``delegatecall`` and ``staticcall`` are provided.
They all take a single ``bytes memory`` parameter and
return the success condition (as a ``bool``) and the returned data
(``bytes memory``).
The functions ``abi.encode``, ``abi.encodePacked``, ``abi.encodeWithSelector``
and ``abi.encodeWithSignature`` can be used to encode structured data.

Example::

    bytes memory payload = abi.encodeWithSignature("register(string)", "MyName");
    (bool success, bytes memory returnData) = address(nameReg).call(payload);
    require(success);

.. warning::
    All these functions are low-level functions and should be used with care.
    Specifically, any unknown contract might be malicious and if you call it, you
    hand over control to that contract which could in turn call back into
    your contract, so be prepared for changes to your state variables
    when the call returns. The regular way to interact with other contracts
    is to call a function on a contract object (``x.f()``).

.. note::
    Previous versions of Solidity allowed these functions to receive
    arbitrary arguments and would also handle a first argument of type
    ``bytes4`` differently. These edge cases were removed in version 0.5.0.

It is possible to adjust the supplied gas with the ``.gas()`` modifier::

    address(nameReg).call.gas(1000000)(abi.encodeWithSignature("register(string)", "MyName"));

Similarly, the supplied Ether value can be controlled too::

    address(nameReg).call.value(1 ether)(abi.encodeWithSignature("register(string)", "MyName"));

Lastly, these modifiers can be combined. Their order does not matter::

    address(nameReg).call.gas(1000000).value(1 ether)(abi.encodeWithSignature("register(string)", "MyName"));

In a similar way, the function ``delegatecall`` can be used: the difference is that only the code of the given address is used, all other aspects (storage, balance, ...) are taken from the current contract. The purpose of ``delegatecall`` is to use library code which is stored in another contract. The user has to ensure that the layout of storage in both contracts is suitable for delegatecall to be used.

.. note::
    Prior to homestead, only a limited variant called ``callcode`` was available that did not provide access to the original ``msg.sender`` and ``msg.value`` values. This function was removed in version 0.5.0.

Since byzantium ``staticcall`` can be used as well. This is basically the same as ``call``, but will revert if the called function modifies the state in any way.

All three functions ``call``, ``delegatecall`` and ``staticcall`` are very low-level functions and should only be used as a *last resort* as they break the type-safety of Solidity.

The ``.gas()`` option is available on all three methods, while the ``.value()`` option is not supported for ``delegatecall``.

.. note::
    All contracts can be converted to ``address`` type, so it is possible to query the balance of the
    current contract using ``address(this).balance``.

.. index:: ! contract type, ! type; contract

.. _contract_types:

Contract Types
--------------

Every :ref:`contract<contracts>` defines its own type.
You can implicitly convert contracts to contracts they inherit from.
Contracts can be explicitly converted to and from all other contract types
and the ``address`` type.

Explicit conversion to and from the ``address payable`` type
is only possible if the contract type has a payable fallback function.
The conversion is still performed using ``address(x)`` and not
using ``address payable(x)``. You can find more information in the section about
the :ref:`address type<address>`.

.. note::
    Before version 0.5.0, contracts directly derived from the address type
    and there was no distinction between ``address`` and ``address payable``.

If you declare a local variable of contract type (`MyContract c`), you can call
functions on that contract. Take care to assign it from somewhere that is the
same contract type.

You can also instantiate contracts (which means they are newly created). You
can find more details in the :ref:`'Contracts via new'<creating-contracts>`
section.

The data representation of a contract is identical to that of the ``address``
type and this type is also used in the :ref:`ABI<ABI>`.

Contracts do not support any operators.

The members of contract types are the external functions of the contract
including public state variables.

.. index:: byte array, bytes32

Fixed-size byte arrays
----------------------

The value types ``bytes1``, ``bytes2``, ``bytes3``, ..., ``bytes32``
hold a sequence of bytes from one to up to 32.
``byte`` is an alias for ``bytes1``.

Operators:

* Comparisons: ``<=``, ``<``, ``==``, ``!=``, ``>=``, ``>`` (evaluate to ``bool``)
* Bit operators: ``&``, ``|``, ``^`` (bitwise exclusive or), ``~`` (bitwise negation)
* Shift operators: ``<<`` (left shift), ``>>`` (right shift)
* Index access: If ``x`` is of type ``bytesI``, then ``x[k]`` for ``0 <= k < I`` returns the ``k`` th byte (read-only).

The shifting operator works with any integer type as right operand (but
returns the type of the left operand), which denotes the number of bits to shift by.
Shifting by a negative amount causes a runtime exception.

Members:

* ``.length`` yields the fixed length of the byte array (read-only).

.. note::
    The type ``byte[]`` is an array of bytes, but due to padding rules, it wastes
    31 bytes of space for each element (except in storage). It is better to use the ``bytes``
    type instead.

Dynamically-sized byte array
----------------------------

``bytes``:
    Dynamically-sized byte array, see :ref:`arrays`. Not a value-type!
``string``:
    Dynamically-sized UTF-8-encoded string, see :ref:`arrays`. Not a value-type!

.. index:: address, literal;address

.. _address_literals:

Address Literals
----------------

Hexadecimal literals that pass the address checksum test, for example
``0xdCad3a6d3569DF655070DEd06cb7A1b2Ccd1D3AF`` are of ``address payable`` type.
Hexadecimal literals that are between 39 and 41 digits
long and do not pass the checksum test produce
a warning and are treated as regular rational number literals.

.. note::
    The mixed-case address checksum format is defined in `EIP-55 <https://github.com/ethereum/EIPs/blob/master/EIPS/eip-55.md>`_.

.. index:: literal, literal;rational

.. _rational_literals:

Rational and Integer Literals
-----------------------------

Integer literals are formed from a sequence of numbers in the range 0-9.
They are interpreted as decimals. For example, ``69`` means sixty nine.
Octal literals do not exist in Solidity and leading zeros are invalid.

Decimal fraction literals are formed by a ``.`` with at least one number on
one side.  Examples include ``1.``, ``.1`` and ``1.3``.

Scientific notation is also supported, where the base can have fractions, while the exponent cannot.
Examples include ``2e10``, ``-2e10``, ``2e-10``, ``2.5e1``.

Underscores can be used to separate the digits of a numeric literal to aid readability.
For example, decimal ``123_000``, hexadecimal ``0x2eff_abde``, scientific decimal notation ``1_2e345_678`` are all valid.
Underscores are only allowed between two digits and only one consecutive underscore is allowed.
There is no additional semantic meaning added to a number literal containing underscores,
the underscores are ignored.

Number literal expressions retain arbitrary precision until they are converted to a non-literal type (i.e. by
using them together with a non-literal expression or by explicit conversion).
This means that computations do not overflow and divisions do not truncate
in number literal expressions.

For example, ``(2**800 + 1) - 2**800`` results in the constant ``1`` (of type ``uint8``)
although intermediate results would not even fit the machine word size. Furthermore, ``.5 * 8`` results
in the integer ``4`` (although non-integers were used in between).

Any operator that can be applied to integers can also be applied to number literal expressions as
long as the operands are integers. If any of the two is fractional, bit operations are disallowed
and exponentiation is disallowed if the exponent is fractional (because that might result in
a non-rational number).

.. note::
    Solidity has a number literal type for each rational number.
    Integer literals and rational number literals belong to number literal types.
    Moreover, all number literal expressions (i.e. the expressions that
    contain only number literals and operators) belong to number literal
    types.  So the number literal expressions ``1 + 2`` and ``2 + 1`` both
    belong to the same number literal type for the rational number three.

.. warning::
    Division on integer literals used to truncate in Solidity prior to version 0.4.0, but it now converts into a rational number, i.e. ``5 / 2`` is not equal to ``2``, but to ``2.5``.

.. note::
    Number literal expressions are converted into a non-literal type as soon as they are used with non-literal
    expressions. Disregarding types, the value of the expression assigned to ``b``
    below evaluates to an integer. Because ``a`` is of type ``uint128``, the
    expression ``2.5 + a`` has to have a proper type, though. Since there is no common type
    for the type of ``2.5`` and ``uint128``, the Solidity compiler does not accept
    this code.

::

    uint128 a = 1;
    uint128 b = 2.5 + a + 0.5;

.. index:: literal, literal;string, string
.. _string_literals:

String Literals and Types
-------------------------

String literals are written with either double or single-quotes (``"foo"`` or ``'bar'``).  They do not imply trailing zeroes as in C; ``"foo"`` represents three bytes, not four.  As with integer literals, their type can vary, but they are implicitly convertible to ``bytes1``, ..., ``bytes32``, if they fit, to ``bytes`` and to ``string``.

For example, with ``bytes32 samevar = "stringliteral"`` the string literal is interpreted in its raw byte form when assigned to a ``bytes32`` type.

String literals support the following escape characters:

 - ``\<newline>`` (escapes an actual newline)
 - ``\\`` (backslash)
 - ``\'`` (single quote)
 - ``\"`` (double quote)
 - ``\b`` (backspace)
 - ``\f`` (form feed)
 - ``\n`` (newline)
 - ``\r`` (carriage return)
 - ``\t`` (tab)
 - ``\v`` (vertical tab)
 - ``\xNN`` (hex escape, see below)
 - ``\uNNNN`` (unicode escape, see below)

``\xNN`` takes a hex value and inserts the appropriate byte, while ``\uNNNN`` takes a Unicode codepoint and inserts an UTF-8 sequence.

The string in the following example has a length of ten bytes.
It starts with a newline byte, followed by a double quote, a single
quote a backslash character and then (without separator) the
character sequence ``abcdef``.

::

    "\n\"\'\\abc\
    def"

Any unicode line terminator which is not a newline (i.e. LF, VF, FF, CR, NEL, LS, PS) is considered to
terminate the string literal. Newline only terminates the string literal if it is not preceded by a ``\``.

.. index:: literal, bytes

Hexadecimal Literals
--------------------

Hexadecimal literals are prefixed with the keyword ``hex`` and are enclosed in double or single-quotes (``hex"001122FF"``). Their content must be a hexadecimal string and their value will be the binary representation of those values.

Hexadecimal literals behave like :ref:`string literals <string_literals>` and have the same convertibility restrictions.

.. index:: enum

.. _enums:

Enums
-----

Enums are one way to create a user-defined type in Solidity. They are explicitly convertible
to and from all integer types but implicit conversion is not allowed.  The explicit conversion
from integer checks at runtime that the value lies inside the range of the enum and causes a failing assert otherwise.
Enums needs at least one member.

The data representation is the same as for enums in C: The options are represented by
subsequent unsigned integer values starting from ``0``.


::

    pragma solidity >=0.4.16 <0.6.0;

    contract test {
        enum ActionChoices { GoLeft, GoRight, GoStraight, SitStill }
        ActionChoices choice;
        ActionChoices constant defaultChoice = ActionChoices.GoStraight;

        function setGoStraight() public {
            choice = ActionChoices.GoStraight;
        }

        // Since enum types are not part of the ABI, the signature of "getChoice"
        // will automatically be changed to "getChoice() returns (uint8)"
        // for all matters external to Solidity. The integer type used is just
        // large enough to hold all enum values, i.e. if you have more than 256 values,
        // `uint16` will be used and so on.
        function getChoice() public view returns (ActionChoices) {
            return choice;
        }

        function getDefaultChoice() public pure returns (uint) {
            return uint(defaultChoice);
        }
    }

.. index:: ! function type, ! type; function

.. _function_types:

Function Types
--------------

Function types are the types of functions. Variables of function type
can be assigned from functions and function parameters of function type
can be used to pass functions to and return functions from function calls.
Function types come in two flavours - *internal* and *external* functions:

Internal functions can only be called inside the current contract (more specifically,
inside the current code unit, which also includes internal library functions
and inherited functions) because they cannot be executed outside of the
context of the current contract. Calling an internal function is realized
by jumping to its entry label, just like when calling a function of the current
contract internally.

External functions consist of an address and a function signature and they can
be passed via and returned from external function calls.

Function types are notated as follows::

    function (<parameter types>) {internal|external} [pure|view|payable] [returns (<return types>)]

In contrast to the parameter types, the return types cannot be empty - if the
function type should not return anything, the whole ``returns (<return types>)``
part has to be omitted.

By default, function types are internal, so the ``internal`` keyword can be
omitted. Note that this only applies to function types. Visibility has
to be specified explicitly for functions defined in contracts, they
do not have a default.

Conversions:

A value of external function type can be explicitly converted to ``address``
resulting in the address of the contract of the function.

A function type ``A`` is implicitly convertible to a function type ``B`` if and only if
their parameter types are identical, their return types are identical,
their internal/external property is identical and the state mutability of ``A``
is not more restrictive than the state mutability of ``B``. In particular:

 - ``pure`` functions can be converted to ``view`` and ``non-payable`` functions
 - ``view`` functions can be converted to ``non-payable`` functions
 - ``payable`` functions can be converted to ``non-payable`` functions

No other conversions between function types are possible.

The rule about ``payable`` and ``non-payable`` might be a little
confusing, but in essence, if a function is ``payable``, this means that it
also accepts a payment of zero Ether, so it also is ``non-payable``.
On the other hand, a ``non-payable`` function will reject Ether sent to it,
so ``non-payable`` functions cannot be converted to ``payable`` functions.

If a function type variable is not initialised, calling it results
in a failed assertion. The same happens if you call a function after using ``delete``
on it.

If external function types are used outside of the context of Solidity,
they are treated as the ``function`` type, which encodes the address
followed by the function identifier together in a single ``bytes24`` type.

Note that public functions of the current contract can be used both as an
internal and as an external function. To use ``f`` as an internal function,
just use ``f``, if you want to use its external form, use ``this.f``.

Members:

Public (or external) functions also have a special member called ``selector``,
which returns the :ref:`ABI function selector <abi_function_selector>`::

    pragma solidity >=0.4.16 <0.6.0;

    contract Selector {
      function f() public pure returns (bytes4) {
        return this.f.selector;
      }
    }

Example that shows how to use internal function types::

    pragma solidity >=0.4.16 <0.6.0;

    library ArrayUtils {
      // internal functions can be used in internal library functions because
      // they will be part of the same code context
      function map(uint[] memory self, function (uint) pure returns (uint) f)
        internal
        pure
        returns (uint[] memory r)
      {
        r = new uint[](self.length);
        for (uint i = 0; i < self.length; i++) {
          r[i] = f(self[i]);
        }
      }
      function reduce(
        uint[] memory self,
        function (uint, uint) pure returns (uint) f
      )
        internal
        pure
        returns (uint r)
      {
        r = self[0];
        for (uint i = 1; i < self.length; i++) {
          r = f(r, self[i]);
        }
      }
      function range(uint length) internal pure returns (uint[] memory r) {
        r = new uint[](length);
        for (uint i = 0; i < r.length; i++) {
          r[i] = i;
        }
      }
    }

    contract Pyramid {
      using ArrayUtils for *;
      function pyramid(uint l) public pure returns (uint) {
        return ArrayUtils.range(l).map(square).reduce(sum);
      }
      function square(uint x) internal pure returns (uint) {
        return x * x;
      }
      function sum(uint x, uint y) internal pure returns (uint) {
        return x + y;
      }
    }

Another example that uses external function types::

    pragma solidity >=0.4.22 <0.6.0;

    contract Oracle {
      struct Request {
        bytes data;
        function(uint) external callback;
      }
      Request[] requests;
      event NewRequest(uint);
      function query(bytes memory data, function(uint) external callback) public {
        requests.push(Request(data, callback));
        emit NewRequest(requests.length - 1);
      }
      function reply(uint requestID, uint response) public {
        // Here goes the check that the reply comes from a trusted source
        requests[requestID].callback(response);
      }
    }

    contract OracleUser {
      Oracle constant oracle = Oracle(0x1234567); // known contract
      uint exchangeRate;
      function buySomething() public {
        oracle.query("USD", this.oracleResponse);
      }
      function oracleResponse(uint response) public {
        require(
            msg.sender == address(oracle),
            "Only oracle can call this."
        );
        exchangeRate = response;
      }
    }

.. note::
    Lambda or inline functions are planned but not yet supported.

.. include:: types/reference-types.rst

.. index:: !mapping
.. _mapping-types:

Mapping Types
=============

You declare mapping types with the syntax ``mapping(_KeyType => _ValueType)``.
The ``_KeyType`` can be any elementary type. This means it can be any of
the built-in value types plus ``bytes`` and ``string``. User-defined
or complex types like contract types, enums, mappings, structs and any array type
apart from ``bytes`` and ``string`` are not allowed.
``_ValueType`` can be any type, including mappings.

You can think of mappings as `hash tables <https://en.wikipedia.org/wiki/Hash_table>`_, which are virtually initialised
such that every possible key exists and is mapped to a value whose
byte-representation is all zeros, a type's :ref:`default value <default-value>`. The similarity ends there, the key data is not stored in a
mapping, only its ``keccak256`` hash is used to look up the value.

Because of this, mappings do not have a length or a concept of a key or
value being set.

Mappings can only have a data location of ``storage`` and thus
are allowed for state variables, as storage reference types
in functions, or as parameters for library functions.
They cannot be used as parameters or return parameters
of contract functions that are publicly visible.

You can mark variables of mapping type as ``public`` and Solidity creates a
:ref:`getter <visibility-and-getters>` for you. The ``_KeyType`` becomes a
parameter for the getter. If ``_ValueType`` is a value type or a struct,
the getter returns ``_ValueType``.
If ``_ValueType`` is an array or a mapping, the getter has one parameter for
each ``_KeyType``, recursively. For example with a mapping:

::

    pragma solidity >=0.4.0 <0.6.0;

    contract MappingExample {
        mapping(address => uint) public balances;

        function update(uint newBalance) public {
            balances[msg.sender] = newBalance;
        }
    }

    contract MappingUser {
        function f() public returns (uint) {
            MappingExample m = new MappingExample();
            m.update(100);
            return m.balances(address(this));
        }
    }


.. note::
  Mappings are not iterable, but it is possible to implement a data structure
  on top of them. For an example, see `iterable mapping <https://github.com/ethereum/dapp-bin/blob/master/library/iterable_mapping.sol>`_.

.. index:: assignment, ! delete, lvalue

Operators Involving LValues
===========================

If ``a`` is an LValue (i.e. a variable or something that can be assigned to), the following operators are available as shorthands:

``a += e`` is equivalent to ``a = a + e``. The operators ``-=``, ``*=``, ``/=``, ``%=``, ``|=``, ``&=`` and ``^=`` are defined accordingly. ``a++`` and ``a--`` are equivalent to ``a += 1`` / ``a -= 1`` but the expression itself still has the previous value of ``a``. In contrast, ``--a`` and ``++a`` have the same effect on ``a`` but return the value after the change.

delete
------

``delete a`` assigns the initial value for the type to ``a``. I.e. for integers it is
equivalent to ``a = 0``, but it can also be used on arrays, where it assigns a dynamic
array of length zero or a static array of the same length with all elements set to their
initial value. ``delete a[x]`` deletes the item at index ``x`` of the array and leaves
all other elements and the length of the array untouched. This especially means that it leaves
a gap in the array. If you plan to remove items, a mapping is probably a better choice.

For structs, it assigns a struct with all members reset. In other words, the value of ``a`` after ``delete a`` is the same as if ``a`` would be declared without assignment, with the following caveat:

``delete`` has no effect on mappings (as the keys of mappings may be arbitrary and are generally unknown). So if you delete a struct, it will reset all members that are not mappings and also recurse into the members unless they are mappings. However, individual keys and what they map to can be deleted: If ``a`` is a mapping, then ``delete a[x]`` will delete the value stored at ``x``.

It is important to note that ``delete a`` really behaves like an assignment to ``a``, i.e. it stores a new object in ``a``.
This distinction is visible when ``a`` is reference variable: It will only reset ``a`` itself, not the
value it referred to previously.

::

    pragma solidity >=0.4.0 <0.6.0;

    contract DeleteExample {
        uint data;
        uint[] dataArray;

        function f() public {
            uint x = data;
            delete x; // sets x to 0, does not affect data
            delete data; // sets data to 0, does not affect x
            uint[] storage y = dataArray;
            delete dataArray; // this sets dataArray.length to zero, but as uint[] is a complex object, also
            // y is affected which is an alias to the storage object
            // On the other hand: "delete y" is not valid, as assignments to local variables
            // referencing storage objects can only be made from existing storage objects.
            assert(y.length == 0);
        }
    }

.. index:: ! type;conversion, ! cast

.. _types-conversion-elementary-types:

Conversions between Elementary Types
====================================

Implicit Conversions
--------------------

If an operator is applied to different types, the compiler tries to
implicitly convert one of the operands to the type of the other (the same is
true for assignments). In general, an implicit conversion between value-types
is possible if it
makes sense semantically and no information is lost: ``uint8`` is convertible to
``uint16`` and ``int128`` to ``int256``, but ``int8`` is not convertible to ``uint256``
(because ``uint256`` cannot hold e.g. ``-1``).

For more details, please consult the sections about the types themselves.

Explicit Conversions
--------------------

If the compiler does not allow implicit conversion but you know what you are
doing, an explicit type conversion is sometimes possible. Note that this may
give you some unexpected behaviour and allows you to bypass some security
features of the compiler, so be sure to test that the
result is what you want! Take the following example where you are converting
a negative ``int8`` to a ``uint``:

::

    int8 y = -3;
    uint x = uint(y);

At the end of this code snippet, ``x`` will have the value ``0xfffff..fd`` (64 hex
characters), which is -3 in the two's complement representation of 256 bits.

If an integer is explicitly converted to a smaller type, higher-order bits are
cut off::

    uint32 a = 0x12345678;
    uint16 b = uint16(a); // b will be 0x5678 now

If an integer is explicitly converted to a larger type, it is padded on the left (i.e. at the higher order end).
The result of the conversion will compare equal to the original integer::

    uint16 a = 0x1234;
    uint32 b = uint32(a); // b will be 0x00001234 now
    assert(a == b);

Fixed-size bytes types behave differently during conversions. They can be thought of as
sequences of individual bytes and converting to a smaller type will cut off the
sequence::

    bytes2 a = 0x1234;
    bytes1 b = bytes1(a); // b will be 0x12

If a fixed-size bytes type is explicitly converted to a larger type, it is padded on
the right. Accessing the byte at a fixed index will result in the same value before and
after the conversion (if the index is still in range)::

    bytes2 a = 0x1234;
    bytes4 b = bytes4(a); // b will be 0x12340000
    assert(a[0] == b[0]);
    assert(a[1] == b[1]);

Since integers and fixed-size byte arrays behave differently when truncating or
padding, explicit conversions between integers and fixed-size byte arrays are only allowed,
if both have the same size. If you want to convert between integers and fixed-size byte arrays of
different size, you have to use intermediate conversions that make the desired truncation and padding
rules explicit::

    bytes2 a = 0x1234;
    uint32 b = uint16(a); // b will be 0x00001234
    uint32 c = uint32(bytes4(a)); // c will be 0x12340000
    uint8 d = uint8(uint16(a)); // d will be 0x34
    uint8 e = uint8(bytes1(a)); // e will be 0x12

.. _types-conversion-literals:

Conversions between Literals and Elementary Types
=================================================

Integer Types
-------------

Decimal and hexadecimal number literals can be implicitly converted to any integer type
that is large enough to represent it without truncation::

    uint8 a = 12; // fine
    uint32 b = 1234; // fine
    uint16 c = 0x123456; // fails, since it would have to truncate to 0x3456

Fixed-Size Byte Arrays
----------------------

Decimal number literals cannot be implicitly converted to fixed-size byte arrays. Hexadecimal
number literals can be, but only if the number of hex digits exactly fits the size of the bytes
type. As an exception both decimal and hexadecimal literals which have a value of zero can be
converted to any fixed-size bytes type::

    bytes2 a = 54321; // not allowed
    bytes2 b = 0x12; // not allowed
    bytes2 c = 0x123; // not allowed
    bytes2 d = 0x1234; // fine
    bytes2 e = 0x0012; // fine
    bytes4 f = 0; // fine
    bytes4 g = 0x0; // fine

String literals and hex string literals can be implicitly converted to fixed-size byte arrays,
if their number of characters matches the size of the bytes type::

    bytes2 a = hex"1234"; // fine
    bytes2 b = "xy"; // fine
    bytes2 c = hex"12"; // not allowed
    bytes2 d = hex"123"; // not allowed
    bytes2 e = "x"; // not allowed
    bytes2 f = "xyz"; // not allowed

Addresses
---------

As described in :ref:`address_literals`, hex literals of the correct size that pass the checksum
test are of ``address`` type. No other literals can be implicitly converted to the ``address`` type.

Explicit conversions from ``bytes20`` or any integer type to ``address`` result in ``address payable``.
