.. index:: style, coding style

#############
Style Guide
#############

************
Introduction
************

This guide is intended to provide coding conventions for writing solidity code.
This guide should be thought of as an evolving document that will change over
time as useful conventions are found and old conventions are rendered obsolete.

Many projects will implement their own style guides.  In the event of
conflicts, project specific style guides take precedence.

The structure and many of the recommendations within this style guide were
taken from python's
`pep8 style guide <https://www.python.org/dev/peps/pep-0008/>`_.

The goal of this guide is *not* to be the right way or the best way to write
solidity code.  The goal of this guide is *consistency*.  A quote from python's
`pep8 <https://www.python.org/dev/peps/pep-0008/#a-foolish-consistency-is-the-hobgoblin-of-little-minds>`_
captures this concept well.

    A style guide is about consistency. Consistency with this style guide is important. Consistency within a project is more important. Consistency within one module or function is most important.
    But most importantly: know when to be inconsistent -- sometimes the style guide just doesn't apply. When in doubt, use your best judgement. Look at other examples and decide what looks best. And don't hesitate to ask!


***********
Code Layout
***********


Indentation
===========

Use 4 spaces per indentation level.

Tabs or Spaces
==============

Spaces are the preferred indentation method.

Mixing tabs and spaces should be avoided.

Blank Lines
===========

Surround top level declarations in solidity source with two blank lines.

Yes::

    contract A {
        ...
    }


    contract B {
        ...
    }


    contract C {
        ...
    }

No::

    contract A {
        ...
    }
    contract B {
        ...
    }

    contract C {
        ...
    }

Within a contract surround function declarations with a single blank line.

Blank lines may be omitted between groups of related one-liners (such as stub functions for an abstract contract)

Yes::

    contract A {
        function spam() public;
        function ham() public;
    }


    contract B is A {
        function spam() public {
            ...
        }

        function ham() public {
            ...
        }
    }

No::

    contract A {
        function spam() public {
            ...
        }
        function ham() public {
            ...
        }
    }

.. _maximum_line_length:

Maximum Line Length
===================

Keeping lines under the `PEP 8 recommendation <https://www.python.org/dev/peps/pep-0008/#maximum-line-length>`_ to a maximum of 79 (or 99)
characters helps readers easily parse the code.

Wrapped lines should conform to the following guidelines.

1. The first argument should not be attached to the opening parenthesis.
2. One, and only one, indent should be used.
3. Each argument should fall on its own line.
4. The terminating element, :code:`);`, should be placed on the final line by itself.

Function Calls

Yes::

    thisFunctionCallIsReallyLong(
        longArgument1,
        longArgument2,
        longArgument3
    );

No::

    thisFunctionCallIsReallyLong(longArgument1,
                                  longArgument2,
                                  longArgument3
    );

    thisFunctionCallIsReallyLong(longArgument1,
        longArgument2,
        longArgument3
    );

    thisFunctionCallIsReallyLong(
        longArgument1, longArgument2,
        longArgument3
    );

    thisFunctionCallIsReallyLong(
    longArgument1,
    longArgument2,
    longArgument3
    );

    thisFunctionCallIsReallyLong(
        longArgument1,
        longArgument2,
        longArgument3);

Assignment Statements

Yes::

    thisIsALongNestedMapping[being][set][to_some_value] = someFunction(
        argument1,
        argument2,
        argument3,
        argument4
    );

No::

    thisIsALongNestedMapping[being][set][to_some_value] = someFunction(argument1,
                                                                       argument2,
                                                                       argument3,
                                                                       argument4);

Event Definitions and Event Emitters

Yes::

    event LongAndLotsOfArgs(
        adress sender,
        adress recipient,
        uint256 publicKey,
        uint256 amount,
        bytes32[] options
    );

    LongAndLotsOfArgs(
        sender,
        recipient,
        publicKey,
        amount,
        options
    );

No::

    event LongAndLotsOfArgs(adress sender,
                            adress recipient,
                            uint256 publicKey,
                            uint256 amount,
                            bytes32[] options);

    LongAndLotsOfArgs(sender,
                      recipient,
                      publicKey,
                      amount,
                      options);

Source File Encoding
====================

UTF-8 or ASCII encoding is preferred.

Imports
=======

Import statements should always be placed at the top of the file.

Yes::

    import "owned";


    contract A {
        ...
    }


    contract B is owned {
        ...
    }

No::

    contract A {
        ...
    }


    import "owned";


    contract B is owned {
        ...
    }

Order of Functions
==================

Ordering helps readers identify which functions they can call and to find the constructor and fallback definitions easier.

Functions should be grouped according to their visibility and ordered:

- constructor
- fallback function (if exists)
- external
- public
- internal
- private

Within a grouping, place the ``view`` and ``pure`` functions last.

Yes::

    contract A {
        constructor() public {
            ...
        }

        function() external {
            ...
        }

        // External functions
        // ...

        // External functions that are view
        // ...

        // External functions that are pure
        // ...

        // Public functions
        // ...

        // Internal functions
        // ...

        // Private functions
        // ...
    }

No::

    contract A {

        // External functions
        // ...

        function() external {
            ...
        }

        // Private functions
        // ...

        // Public functions
        // ...

        constructor() public {
            ...
        }

        // Internal functions
        // ...
    }

Whitespace in Expressions
=========================

Avoid extraneous whitespace in the following  situations:

Immediately inside parenthesis, brackets or braces, with the exception of single line function declarations.

Yes::

    spam(ham[1], Coin({name: "ham"}));

No::

    spam( ham[ 1 ], Coin( { name: "ham" } ) );

Exception::

    function singleLine() public { spam(); }

Immediately before a comma, semicolon:

Yes::

    function spam(uint i, Coin coin) public;

No::

    function spam(uint i , Coin coin) public ;

More than one space around an assignment or other operator to align with
  another:

Yes::

    x = 1;
    y = 2;
    long_variable = 3;

No::

    x             = 1;
    y             = 2;
    long_variable = 3;

Don't include a whitespace in the fallback function:

Yes::

    function() external {
        ...
    }

No::

    function () external {
        ...
    }

Control Structures
==================

The braces denoting the body of a contract, library, functions and structs
should:

* open on the same line as the declaration
* close on their own line at the same indentation level as the beginning of the
  declaration.
* The opening brace should be proceeded by a single space.

Yes::

    contract Coin {
        struct Bank {
            address owner;
            uint balance;
        }
    }

No::

    contract Coin
    {
        struct Bank {
            address owner;
            uint balance;
        }
    }

The same recommendations apply to the control structures ``if``, ``else``, ``while``,
and ``for``.

Additionally there should be a single space between the control structures
``if``, ``while``, and ``for`` and the parenthetic block representing the
conditional, as well as a single space between the conditional parenthetic
block and the opening brace.

Yes::

    if (...) {
        ...
    }

    for (...) {
        ...
    }

No::

    if (...)
    {
        ...
    }

    while(...){
    }

    for (...) {
        ...;}

For control structures whose body contains a single statement, omitting the
braces is ok *if* the statement is contained on a single line.

Yes::

    if (x < 10)
        x += 1;

No::

    if (x < 10)
        someArray.push(Coin({
            name: 'spam',
            value: 42
        }));

For ``if`` blocks which have an ``else`` or ``else if`` clause, the ``else`` should be
placed on the same line as the ``if``'s closing brace. This is an exception compared
to the rules of other block-like structures.

Yes::

    if (x < 3) {
        x += 1;
    } else if (x > 7) {
        x -= 1;
    } else {
        x = 5;
    }


    if (x < 3)
        x += 1;
    else
        x -= 1;

No::

    if (x < 3) {
        x += 1;
    }
    else {
        x -= 1;
    }

Function Declaration
====================

For short function declarations, it is recommended for the opening brace of the
function body to be kept on the same line as the function declaration.

The closing brace should be at the same indentation level as the function
declaration.

The opening brace should be preceded by a single space.

Yes::

    function increment(uint x) public pure returns (uint) {
        return x + 1;
    }

    function increment(uint x) public pure onlyowner returns (uint) {
        return x + 1;
    }

No::

    function increment(uint x) public pure returns (uint)
    {
        return x + 1;
    }

    function increment(uint x) public pure returns (uint){
        return x + 1;
    }

    function increment(uint x) public pure returns (uint) {
        return x + 1;
        }

    function increment(uint x) public pure returns (uint) {
        return x + 1;}

You should explicitly label the visibility of all functions, including constructors.

Yes::

    function explicitlyPublic(uint val) public {
        doSomething();
    }

No::

    function implicitlyPublic(uint val) {
        doSomething();
    }

The visibility modifier for a function should come before any custom
modifiers.

Yes::

    function kill() public onlyowner {
        selfdestruct(owner);
    }

No::

    function kill() onlyowner public {
        selfdestruct(owner);
    }

For long function declarations, it is recommended to drop each argument onto
it's own line at the same indentation level as the function body.  The closing
parenthesis and opening bracket should be placed on their own line as well at
the same indentation level as the function declaration.

Yes::

    function thisFunctionHasLotsOfArguments(
        address a,
        address b,
        address c,
        address d,
        address e,
        address f
    )
        public
    {
        doSomething();
    }

No::

    function thisFunctionHasLotsOfArguments(address a, address b, address c,
        address d, address e, address f) public {
        doSomething();
    }

    function thisFunctionHasLotsOfArguments(address a,
                                            address b,
                                            address c,
                                            address d,
                                            address e,
                                            address f) public {
        doSomething();
    }

    function thisFunctionHasLotsOfArguments(
        address a,
        address b,
        address c,
        address d,
        address e,
        address f) public {
        doSomething();
    }

If a long function declaration has modifiers, then each modifier should be
dropped to its own line.

Yes::

    function thisFunctionNameIsReallyLong(address x, address y, address z)
        public
        onlyowner
        priced
        returns (address)
    {
        doSomething();
    }

    function thisFunctionNameIsReallyLong(
        address x,
        address y,
        address z,
    )
        public
        onlyowner
        priced
        returns (address)
    {
        doSomething();
    }

No::

    function thisFunctionNameIsReallyLong(address x, address y, address z)
                                          public
                                          onlyowner
                                          priced
                                          returns (address) {
        doSomething();
    }

    function thisFunctionNameIsReallyLong(address x, address y, address z)
        public onlyowner priced returns (address)
    {
        doSomething();
    }

    function thisFunctionNameIsReallyLong(address x, address y, address z)
        public
        onlyowner
        priced
        returns (address) {
        doSomething();
    }

Multiline output parameters and return statements should follow the same style recommended for wrapping long lines found in the :ref:`Maximum Line Length <maximum_line_length>` section.

Yes::

    function thisFunctionNameIsReallyLong(
        address a,
        address b,
        address c
    )
        public
        returns (
            address someAddressName,
            uint256 LongArgument,
            uint256 Argument
        )
    {
        doSomething()

        return (
            veryLongReturnArg1,
            veryLongReturnArg2,
            veryLongReturnArg3
        );
    }

No::

    function thisFunctionNameIsReallyLong(
        address a,
        address b,
        address c
    )
        public
        returns (address someAddressName,
                 uint256 LongArgument,
                 uint256 Argument)
    {
        doSomething()

        return (veryLongReturnArg1,
                veryLongReturnArg1,
                veryLongReturnArg1);
    }

For constructor functions on inherited contracts whose bases require arguments,
it is recommended to drop the base constructors onto new lines in the same
manner as modifiers if the function declaration is long or hard to read.

Yes::

    contract A is B, C, D {
        constructor(uint param1, uint param2, uint param3, uint param4, uint param5)
            B(param1)
            C(param2, param3)
            D(param4)
            public
        {
            // do something with param5
        }
    }

No::

    contract A is B, C, D {
        constructor(uint param1, uint param2, uint param3, uint param4, uint param5)
        B(param1)
        C(param2, param3)
        D(param4)
        public
        {
            // do something with param5
        }
    }

    contract A is B, C, D {
        constructor(uint param1, uint param2, uint param3, uint param4, uint param5)
            B(param1)
            C(param2, param3)
            D(param4)
            public {
            // do something with param5
        }
    }

When declaring short functions with a single statement, it is permissible to do it on a single line.

Permissible::

    function shortFunction() public { doSomething(); }

These guidelines for function declarations are intended to improve readability.
Authors should use their best judgement as this guide does not try to cover all
possible permutations for function declarations.

Mappings
========

TODO

Variable Declarations
=====================

Declarations of array variables should not have a space between the type and
the brackets.

Yes::

    uint[] x;

No::

    uint [] x;


Other Recommendations
=====================

* Strings should be quoted with double-quotes instead of single-quotes.

Yes::

    str = "foo";
    str = "Hamlet says, 'To be or not to be...'";

No::

    str = 'bar';
    str = '"Be yourself; everyone else is already taken." -Oscar Wilde';

* Surround operators with a single space on either side.

Yes::

    x = 3;
    x = 100 / 10;
    x += 3 + 4;
    x |= y && z;

No::

    x=3;
    x = 100/10;
    x += 3+4;
    x |= y&&z;

* Operators with a higher priority than others can exclude surrounding
  whitespace in order to denote precedence.  This is meant to allow for
  improved readability for complex statement. You should always use the same
  amount of whitespace on either side of an operator:

Yes::

    x = 2**3 + 5;
    x = 2*y + 3*z;
    x = (a+b) * (a-b);

No::

    x = 2** 3 + 5;
    x = y+z;
    x +=1;


******************
Naming Conventions
******************

Naming conventions are powerful when adopted and used broadly.  The use of
different conventions can convey significant *meta* information that would
otherwise not be immediately available.

The naming recommendations given here are intended to improve the readability,
and thus they are not rules, but rather guidelines to try and help convey the
most information through the names of things.

Lastly, consistency within a codebase should always supercede any conventions
outlined in this document.


Naming Styles
=============

To avoid confusion, the following names will be used to refer to different
naming styles.

* ``b`` (single lowercase letter)
* ``B`` (single uppercase letter)
* ``lowercase``
* ``lower_case_with_underscores``
* ``UPPERCASE``
* ``UPPER_CASE_WITH_UNDERSCORES``
* ``CapitalizedWords`` (or CapWords)
* ``mixedCase`` (differs from CapitalizedWords by initial lowercase character!)
* ``Capitalized_Words_With_Underscores``

.. note:: When using initialisms in CapWords, capitalize all the letters of the initialisms. Thus HTTPServerError is better than HttpServerError. When using initialisms is mixedCase, capitalize all the letters of the initialisms, except keep the first one lower case if it is the beginning of the name. Thus xmlHTTPRequest is better than XMLHTTPRequest.


Names to Avoid
==============

* ``l`` - Lowercase letter el
* ``O`` - Uppercase letter oh
* ``I`` - Uppercase letter eye

Never use any of these for single letter variable names.  They are often
indistinguishable from the numerals one and zero.


Contract and Library Names
==========================

Contracts and libraries should be named using the CapWords style. Examples: ``SimpleToken``, ``SmartBank``, ``CertificateHashRepository``, ``Player``.


Struct Names
==========================

Structs should be named using the CapWords style. Examples: ``MyCoin``, ``Position``, ``PositionXY``.


Event Names
===========

Events should be named using the CapWords style. Examples: ``Deposit``, ``Transfer``, ``Approval``, ``BeforeTransfer``, ``AfterTransfer``.


Function Names
==============

Functions other than constructors should use mixedCase. Examples: ``getBalance``, ``transfer``, ``verifyOwner``, ``addMember``, ``changeOwner``.


Function Argument Names
=======================

Function arguments should use mixedCase. Examples: ``initialSupply``, ``account``, ``recipientAddress``, ``senderAddress``, ``newOwner``.

When writing library functions that operate on a custom struct, the struct
should be the first argument and should always be named ``self``.


Local and State Variable Names
==============================

Use mixedCase. Examples: ``totalSupply``, ``remainingSupply``, ``balancesOf``, ``creatorAddress``, ``isPreSale``, ``tokenExchangeRate``.


Constants
=========

Constants should be named with all capital letters with underscores separating
words. Examples: ``MAX_BLOCKS``, ``TOKEN_NAME``, ``TOKEN_TICKER``, ``CONTRACT_VERSION``.


Modifier Names
==============

Use mixedCase. Examples: ``onlyBy``, ``onlyAfter``, ``onlyDuringThePreSale``.


Enums
=====

Enums, in the style of simple type declarations, should be named using the CapWords style. Examples: ``TokenGroup``, ``Frame``, ``HashStyle``, ``CharacterLocation``.


Avoiding Naming Collisions
==========================

* ``single_trailing_underscore_``

This convention is suggested when the desired name collides with that of a
built-in or otherwise reserved name.


General Recommendations
=======================

TODO
