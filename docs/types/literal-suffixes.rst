.. index:: ! literal suffix
.. _literal_suffixes:

Literal Suffixes
================

While Solidity :ref:`provides implicit conversions<types-conversion-literals>` from its literals to selected types,
there are types for which no built-in conversions are available (most notably the
:ref:`user-defined value types<user-defined-value-types>`).

To fill this gap, it is possible to define custom conversions as *literal suffixes*.

.. code-block:: solidity
    :force:

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity ^0.8.21;

    type Coin is uint;
    using {add as +, eq as ==} for Coin global;

    function add(Coin a, Coin b) pure returns (Coin) {
        return Coin.wrap(Coin.unwrap(a) + Coin.unwrap(b));
    }

    function eq(Coin a, Coin b) pure returns (bool) {
        return Coin.unwrap(a) == Coin.unwrap(b);
    }

    function c(uint mantissa, uint exponent) pure suffix returns (Coin) {
        return Coin.wrap(mantissa * 10**6 / 10**exponent);
    }

    function uc(uint microValue) pure suffix returns (Coin) {
        return Coin.wrap(microValue);
    }

    contract CoinSeller {
        mapping(address => Coin) balances;

        function buy() public payable {
            require(msg.value == 1 ether);
            assert(1.5 c == 1_500_000 uc);
            balances[msg.sender] = balances[msg.sender] + 1.5 c;
        }
    }

.. index:: function;call
.. _calling_suffix_functions:

Calling Suffix Functions
------------------------

There are two ways to call a suffix function: a *suffix call* and a *function call*.

.. index:: ! literal suffix; suffix call syntax

Suffix Call Syntax
^^^^^^^^^^^^^^^^^^

A suffix call has the same syntax as a literal with a :ref:`denomination<denominations>`:

.. code-block:: solidity

    42 suffix;
    1.23 suffix;
    0x1234 suffix;
    'abc' suffix;
    hex"12ff" suffix;
    unicode"😃" suffix;
    true suffix;

The literal passed as input to the suffix function must be immediately followed by the name of the suffix.
The two must be separated by whitespace unless it is a string, unicode or hexadecimal string literal,
in which case the whitespace is optional (i.e. ``'abc'suffix`` is also allowed).

This call syntax supports only a single literal argument.
Variables or expressions (even as simple as wrapping the literal in parentheses) are not allowed.
:ref:`Suffix functions defined with two parameters<suffix_function_parameters>` are also invoked with
one literal - the :ref:`decomposition<fractional_decomposition>` of the literal into two values is
performed implicitly by the compiler.

.. warning::
    There are no negative number literals in Solidity.
    A literal with a minus sign is an expression.
    ``-123 suffix`` is equivalent to ``-(123 suffix)``, so ``suffix`` does not receive ``-123`` as input.
    The argument is instead ``123``, and the negation is applied to the returned value.

.. note::
    :ref:`String literal concatenation<string_literal_concatenation>` produces a single literal at
    compilation time and therefore is not treated as an expression.
    This means that e.g., ``"abc" "def" suffix`` is a valid suffix call.

.. note::
    Fractional decomposition is performed at compilation time.
    Using the suffix call syntax does not incur any extra gas cost compared to the equivalent
    function call.

.. index:: ! literal suffix; function call syntax, overload
.. _calling_suffix_functions_with_function_call_syntax:

Function Call Syntax
^^^^^^^^^^^^^^^^^^^^

Suffix definitions are in all respects valid free functions, and this includes the ability to call
them directly:

.. code-block:: solidity

    suffix(42);
    suffix(123, 2);
    suffix(0x1234);
    suffix('abc');
    suffix(hex"12ff");
    suffix(unicode"😃");
    suffix(true);

This also makes it possible to call such functions with arguments which are not literals.

Note that the fractional decomposition is not performed for this kind of call -
:ref:`two-parameter suffix functions<suffix_function_parameters>` must be explicitly called with
two arguments.

Regardless of the call syntax used and in contrast to applying a denomination, the result of the
call is itself not considered a literal.
As a consequence, it cannot be used as input of another suffix call, and calculations on it are performed
within its type rather than in arbitrary precision (as is the case with calculations on rational number
literals):

.. code-block:: solidity

    123 suffix1 suffix2;    // This will not compile.
    suffix1(123) suffix2;   // This will not compile.

Such calls are possible only with the function call syntax:

.. code-block:: solidity

    suffix2(123 suffix1);   // This is fine.
    suffix2(suffix1(123));  // This is fine.

.. note::
    As all free functions, suffix definitions can be :ref:`overloaded<overload-function>`.
    Overloaded suffixes, however, cannot be invoked using the suffix call syntax.

.. index:: ! literal suffix;definition, function;free

Defining Suffix Functions
-------------------------

Literal suffixes can be defined by applying the built-in ``suffix`` modifier to a :ref:`free function<functions>`.

Only pure functions can be used as suffixes.
This means that suffixes cannot read or modify blockchain state.
As with all pure functions, however, they can perform pure external calls.

.. index:: literal;address
.. _suffix_function_parameters:

Suffix Function Parameters
^^^^^^^^^^^^^^^^^^^^^^^^^^

A suffix function must accept and return exactly one value.
As a special case, suffixes on :ref:`rational literals<rational_literals>` can optionally accept two arguments,
produced by the :ref:`fractional decomposition<fractional_decomposition>` of such a literal.

Suffixes can only have parameters of types for which an implicit conversion from a literal exists.
For single-parameter suffixes, this includes the following types:

+-------------------------------------------------------------+----------------------------------------------------------------+
| Parameter type                                              | Accepted literals                                              |
+=============================================================+================================================================+
| ``bool``                                                    | - :ref:`Boolean<booleans>` literals                            |
+-------------------------------------------------------------+----------------------------------------------------------------+
| ``uint8``, ..., ``uint256``, ``int8``, ..., ``int256``      | - :ref:`Rational literals<rational_literals>` (including zero) |
|                                                             | - Hexadecimal number literals                                  |
+-------------------------------------------------------------+----------------------------------------------------------------+
| ``address``                                                 | - :ref:`Address literals<address_literals>`                    |
+-------------------------------------------------------------+----------------------------------------------------------------+
| ``bytes1``, ..., ``bytes32``                                | - Hexadecimal number literals (not for ``bytes20``)            |
|                                                             | - :ref:`Hexadecimal string literals<hexadecimal_literals>`     |
|                                                             | - :ref:`String literals<string_literals>`                      |
|                                                             | - :ref:`Unicode literals<unicode_literals>`                    |
|                                                             | - :ref:`Zero<rational_literals>`                               |
+-------------------------------------------------------------+----------------------------------------------------------------+
| ``bytes``                                                   | - :ref:`Hexadecimal string literals<hexadecimal_literals>`     |
|                                                             | - :ref:`String literals<string_literals>`                      |
|                                                             | - :ref:`Unicode literals<unicode_literals>`                    |
+-------------------------------------------------------------+----------------------------------------------------------------+
| ``string``                                                  | - :ref:`String literals<string_literals>`                      |
|                                                             | - :ref:`Unicode literals<unicode_literals>`                    |
+-------------------------------------------------------------+----------------------------------------------------------------+

For two-parameter suffix functions, the first parameter (representing the mantissa) can be of any integer type.
The second parameter (the exponent) must be of an unsigned integer type.

.. note::
    :ref:`The function call syntax<calling_suffix_functions_with_function_call_syntax>` is the only
    way to pass a negative integer value into a suffix function.
    Despite this, signed integer types are allowed for suffix parameters.
    They are still useful in cases where it is desirable to limit the range of the parameter or to
    avoid explicit conversions when the return type is signed.

.. note::
    40-digit literals prefixed with ``0x`` such as, for example, ``0xdCad3a6d3569DF655070DEd06cb7A1b2Ccd1D3AF``
    always represent ``address`` literals in the language.
    To invoke a suffix accepting ``bytes20`` you must use one of the other literal kinds implicitly
    convertible to ``bytes20``, e.g., a hexadecimal string literal
    (``hex"dCad3a6d3569DF655070DEd06cb7A1b2Ccd1D3AF"``).

.. note::
    Suffix functions accepting ``address payable`` are not allowed since address literals are never payable.

Suffix functions may not accept or return reference types with ``storage`` or ``calldata`` locations.

.. index:: ! fractional decomposition
.. _fractional_decomposition:

Fractional Decomposition
------------------------

To allow defining suffixes that work with fractional literals, like ``1.23``, the language allows
a special form of a suffix definition.
Such a suffix can be considered a more general form of a suffix taking a single integer argument.

A single-parameter suffix can be applied only to those rational number literals which represent
integers.
Let us consider the following suffix definition:

.. code-block:: solidity

    function kg(uint grams) pure suffix returns (uint) {
        return 1000 * grams;
    }

The ``kg`` suffix can receive integer values like ``123 kg``, ``1.23e2 kg``, or ``12300e-2 kg``.
However, invoking such a suffix with a fractional number (e.g., ``1.23 kg``) triggers an error.
We can fix that by adding an *exponent* parameter:

.. code-block:: solidity

    function kg(uint mantissa, uint exponent) pure suffix returns (uint) {
        return 1000 * mantissa / 10**exponent;
    }

When defined this way, the suffix can handle all the literals it could previously, while ``1.23 kg``
also becomes a valid expression, equivalent to ``kg(123, 2)``.

More generally, the argument of such a suffix call is decomposed into two integer values (``mantissa``
and ``exponent``), such that:

#. ``mantissa * 10**-exponent`` is equal to the value of the literal.
#. ``exponent`` is the smallest possible non-negative integer value satisfying the equation.

The two rules provide unambiguous decomposition in all cases.
For example:

- ``123000`` is decomposed into ``123000 * 10**-0`` (i.e. ``123000`` for ``mantissa`` and ``0`` for ``exponent``).
  Not ``123 * 10**3`` or ``123000000 * 10**-3``.

  In general, when the suffix is invoked on an integer, ``mantissa`` is always equal to that integer
  and ``exponent`` is ``0``.
- ``1.23`` is decomposed into ``123 * 10**-2``, not ``1.23 * 10**-0`` or ``123000 * 10**-5``.

  In general, when the suffix is invoked on a fractional number, ``exponent`` is the exponent of the
  lowest positive power of ``10`` that multiplied by the literal produces an integer value.
  ``mantissa`` is the result of that multiplication.

``exponent`` is never negative and therefore must have an unsigned integer type.
