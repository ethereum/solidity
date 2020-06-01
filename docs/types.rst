.. index:: type

.. _types:

*****
Types
*****

Solidity est un langage à typage statique, ce qui signifie que le type de chaque variable (état et locale) doit être spécifié.
Solidity propose plusieurs types élémentaires qui peuvent être combinés pour former des types complexes.

De plus, les types peuvent interagir entre eux dans des expressions contenant des opérateurs. Pour une liste synthétique des différents opérateurs, voir :ref:`order`.

The concept of "undefined" or "null" values does not exist in Solidity, but newly
declared variables always have a :ref:`default value<default-value>` dependent
on its type. To handle any unexpected values, you should use the :ref:`revert function<assert-and-require>` to revert the whole transaction, or return a
tuple with a second ``bool`` value denoting success.

.. include:: types/value-types.rst

.. include:: types/reference-types.rst

.. include:: types/mapping-types.rst

.. include:: types/operators.rst

.. include:: types/conversion.rst
