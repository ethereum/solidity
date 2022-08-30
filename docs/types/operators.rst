.. index:: ! operator

Operators
=========

Arithmetic and bit operators can be applied even if the two operands do not have the same type.
For example, you can compute ``y = x + z``, where ``x`` is a ``uint8`` and ``z`` has
the type ``uint32``. In these cases, the following mechanism will be used to determine
the type in which the operation is computed (this is important in case of overflow)
and the type of the operator's result:

1. If the type of the right operand can be implicitly converted to the type of the left
   operand, use the type of the left operand,
2. if the type of the left operand can be implicitly converted to the type of the right
   operand, use the type of the right operand,
3. otherwise, the operation is not allowed.

In case one of the operands is a :ref:`literal number <rational_literals>` it is first converted to its
"mobile type", which is the smallest type that can hold the value
(unsigned types of the same bit-width are considered "smaller" than the signed types).
If both are literal numbers, the operation is computed with effectively unlimited precision in
that the expression is evaluated to whatever precision is necessary so that none is lost
when the result is used with a non-literal type.

The operator's result type is the same as the type the operation is performed in,
except for comparison operators where the result is always ``bool``.

The operators ``**`` (exponentiation), ``<<``  and ``>>`` use the type of the
left operand for the operation and the result.

Ternary Operator
----------------
The ternary operator is used in expressions of the form ``<expression> ? <trueExpression> : <falseExpression>``.
It evaluates one of the latter two given expressions depending upon the result of the evaluation of the main ``<expression>``.
If ``<expression>`` evaluates to ``true``, then ``<trueExpression>`` will be evaluated, otherwise ``<falseExpression>`` is evaluated.

The result of the ternary operator does not have a rational number type, even if all of its operands are rational number literals.
The result type is determined from the types of the two operands in the same way as above, converting to their mobile type first if required.

As a consequence, ``255 + (true ? 1 : 0)`` will revert due to arithmetic overflow.
The reason is that ``(true ? 1 : 0)`` is of ``uint8`` type, which forces the addition to be performed in ``uint8`` as well,
and 256 exceeds the range allowed for this type.

Another consequence is that an expression like ``1.5 + 1.5`` is valid but ``1.5 + (true ? 1.5 : 2.5)`` is not.
This is because the former is a rational expression evaluated in unlimited precision and only its final value matters.
The latter involves a conversion of a fractional rational number to an integer, which is currently disallowed.

.. index:: assignment, lvalue, ! compound operators

Compound and Increment/Decrement Operators
------------------------------------------

If ``a`` is an LValue (i.e. a variable or something that can be assigned to), the
following operators are available as shorthands:

``a += e`` is equivalent to ``a = a + e``. The operators ``-=``, ``*=``, ``/=``, ``%=``,
``|=``, ``&=``, ``^=``, ``<<=`` and ``>>=`` are defined accordingly. ``a++`` and ``a--`` are equivalent
to ``a += 1`` / ``a -= 1`` but the expression itself still has the previous value
of ``a``. In contrast, ``--a`` and ``++a`` have the same effect on ``a`` but
return the value after the change.

.. index:: !delete

.. _delete:

delete
------

``delete a`` assigns the initial value for the type to ``a``. I.e. for integers it is
equivalent to ``a = 0``, but it can also be used on arrays, where it assigns a dynamic
array of length zero or a static array of the same length with all elements set to their
initial value. ``delete a[x]`` deletes the item at index ``x`` of the array and leaves
all other elements and the length of the array untouched. This especially means that it leaves
a gap in the array. If you plan to remove items, a :ref:`mapping <mapping-types>` is probably a better choice.

For structs, it assigns a struct with all members reset. In other words,
the value of ``a`` after ``delete a`` is the same as if ``a`` would be declared
without assignment, with the following caveat:

``delete`` has no effect on mappings (as the keys of mappings may be arbitrary and
are generally unknown). So if you delete a struct, it will reset all members that
are not mappings and also recurse into the members unless they are mappings.
However, individual keys and what they map to can be deleted: If ``a`` is a
mapping, then ``delete a[x]`` will delete the value stored at ``x``.

It is important to note that ``delete a`` really behaves like an
assignment to ``a``, i.e. it stores a new object in ``a``.
This distinction is visible when ``a`` is reference variable: It
will only reset ``a`` itself, not the
value it referred to previously.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.4.0 <0.9.0;

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

.. index:: ! operator; precedence
.. _order:

Order of Precedence of Operators
--------------------------------

.. include:: types/operator-precedence-table.rst
