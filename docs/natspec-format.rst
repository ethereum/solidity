.. _natspec:

##############
NatSpec Format
##############

Solidity contracts can use a special form of comments to provide rich
documentation for functions, return variables and more. This special form is
named the Ethereum Natural Language Specification Format (NatSpec).

This documentation is segmented into developer-focused messages and end-user-facing
messages. These messages may be shown to the end user (the human) at the
time that they will interact with the contract (i.e. sign a transaction).

It is recommended that Solidity contracts are fully annontated using NatSpec for
all public interfaces (everything in the ABI).

NatSpec includes the formatting for comments that the smart contract author will
use, and which are understood by the Solidity compiler. Also detailed below is
output of the Solidity compiler, which extracts these comments into a machine-readable
format.

.. _header-doc-example:

Documentation Example
=====================

Documentation is inserted above each ``class``, ``interface`` and
``function`` using the doxygen notation format.

-  For Solidity you may choose ``///`` for single or multi-line
   comments, or ``/**`` and ending with ``*/``.

-  For Vyper, use ``"""`` indented to the inner contents with bare
   comments. See `Vyper
   documentation <https://vyper.readthedocs.io/en/latest/structure-of-a-contract.html#natspec-metadata>`__.

The following example shows a contract and a function using all available tags.
Note: NatSpec currently does NOT apply to public state variables (see
`solidity#3418 <https://github.com/ethereum/solidity/issues/3418>`__),
even if they are declared public and therefore do affect the ABI. Note:
The Solidity compiler only interprets tags if they are external or
public. You are welcome to use similar comments for your internal and
private functions, but those will not be parsed.

.. code:: solidity

   pragma solidity ^0.5.6;

   /// @title A simulator for trees
   /// @author Larry A. Gardner
   /// @notice You can use this contract for only the most basic simulation
   /// @dev All function calls are currently implemented without side effects
   contract Tree {
       /// @author Mary A. Botanist
       /// @notice Calculate tree age in years, rounded up, for live trees
       /// @dev The Alexandr N. Tetearing algorithm could increase precision
       /// @param rings The number of rings from dendrochronological sample
       /// @return age in years, rounded up for partial years
       function age(uint256 rings) external pure returns (uint256) {
           return rings + 1;
       }
   }

.. _header-tags:

Tags
====

All tags are optional. The following table explains the purpose of each
NatSpec tag and where it may be used. As a special case, if no tags are
used then the Solidity compiler will interpret a `///` or `/**` comment
in the same way as if it were tagged with `@notice`.

=========== =============================================================================== =============================
Tag                                                                                         Context
=========== =============================================================================== =============================
``@title``  A title that should describe the contract/interface                             contract, interface
``@author`` The name of the author                                                          contract, interface, function
``@notice`` Explain to an end user what this does                                           contract, interface, function
``@dev``    Explain to a developer any extra details                                        contract, interface, function
``@param``  Documents a parameter just like in doxygen (must be followed by parameter name) function
``@return`` Documents the return type of a contract's function                              function
=========== =============================================================================== =============================

If your function returns multiple values, like ``(int quotient, int remainder)``
then use multiple ``@return`` statements in the same format as the
``@param`` statements.

.. _header-dynamic:

Dynamic expressions
-------------------

The Solidity compiler will pass through NatSpec documentation from your Solidity
source code to the JSON output as described in this guide. The consumer of this
JSON output, for example the end-user client software, may present this to the end-user directly or it may apply some pre-processing.

For example, some client software will render:

.. code:: solidity

   /// @notice This function will multiply `a` by 7

to the end-user as:

.. code:: text

    This function will multiply 10 by 7

if a function is being called and the input ``a`` is assigned a value of 7.

Specifying these dynamic expressions is outside the scope of the Solidity
documentation and you may read more at
`the radspec project <https://github.com/aragon/radspec>`__.

.. _header-inheritance:

Inheritance Notes
-----------------

Currently it is undefined whether a contract with a function having no
NatSpec will inherit the NatSpec of a parent contract/interface for that
same function.

.. _header-output:

Documentation Output
====================

When parsed by the compiler, documentation such as the one from the
above example will produce two different JSON files. One is meant to be
consumed by the end user as a notice when a function is executed and the
other to be used by the developer.

If the above contract is saved as ``ex1.sol`` then you can generate the
documentation using:

.. code::

   solc --userdoc --devdoc ex1.sol

And the output is below.

.. _header-user-doc:

User Documentation
------------------

The above documentation will produce the following user documentation
JSON file as output:

.. code::

    {
      "methods" :
      {
        "age(uint256)" :
        {
          "notice" : "Calculate tree age in years, rounded up, for live trees"
        }
      },
      "notice" : "You can use this contract for only the most basic simulation"
    }

Note that the key by which to find the methods is the function's
canonical signature as defined in the `Contract
ABI <Ethereum-Contract-ABI#signature>`__ and not simply the function's
name.

.. _header-developer-doc:

Developer Documentation
-----------------------

Apart from the user documentation file, a developer documentation JSON
file should also be produced and should look like this:

.. code::

    {
      "author" : "Larry A. Gardner",
      "details" : "All function calls are currently implemented without side effects",
      "methods" :
      {
        "age(uint256)" :
        {
          "author" : "Mary A. Botanist",
          "details" : "The Alexandr N. Tetearing algorithm could increase precision",
          "params" :
          {
            "rings" : "The number of rings from dendrochronological sample"
          },
          "return" : "age in years, rounded up for partial years"
        }
      },
      "title" : "A simulator for trees"
    }

