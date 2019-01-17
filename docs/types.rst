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

.. include:: types/value-types.rst

.. include:: types/reference-types.rst

.. include:: types/mapping-types.rst

.. include:: types/operators.rst

.. include:: types/conversion.rst