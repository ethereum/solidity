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
    But most importantly: know when to be inconsistent -- sometimes the style guide just doesn't apply. When in doubt, use your best judgment. Look at other examples and decide what looks best. And don't hesitate to ask!


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
        function spam();
        function ham();
    }


    contract B is A {
        function spam() {
            ...
        }

        function ham() {
            ...
        }
    }

No::

    contract A {
        function spam() {
            ...
        }
        function ham() {
            ...
        }
    }

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

Within a grouping, place the ``constant`` functions last.

Yes::

    contract A {
        function A() {
            ...
        }
        
        function() {
            ...
        }
        
        // External functions
        // ...
        
        // External functions that are constant
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

        // Private functions
        // ...

        // Public functions
        // ...

        function A() {
            ...
        }
        
        function() {
            ...
        }

        // Internal functions
        // ...       
    }

Whitespace in Expressions
=========================

Avoid extraneous whitespace in the following  situations:

Immediately inside parenthesis, brackets or braces, with the exception of single-line function declarations.

Yes::

    spam(ham[1], Coin({name: "ham"}));

No::

    spam( ham[ 1 ], Coin( { name: "ham" } ) );

Exception::

    function singleLine() { spam(); }

Immediately before a comma, semicolon:

Yes::

    function spam(uint i, Coin coin);

No::

    function spam(uint i , Coin coin) ;

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

    function() {
        ...
    }

No::
   
    function () {
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

The opening brace should be preceeded by a single space.

Yes::

    function increment(uint x) returns (uint) {
        return x + 1;
    }

    function increment(uint x) public onlyowner returns (uint) {
        return x + 1;
    }

No::

    function increment(uint x) returns (uint)
    {
        return x + 1;
    }

    function increment(uint x) returns (uint){
        return x + 1;
    }

    function increment(uint x) returns (uint) {
        return x + 1;
        }

    function increment(uint x) returns (uint) {
        return x + 1;}

The visibility modifiers for a function should come before any custom
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
    ) {
        doSomething();
    }

No::

    function thisFunctionHasLotsOfArguments(address a, address b, address c,
        address d, address e, address f) {
        doSomething();
    }

    function thisFunctionHasLotsOfArguments(address a,
                                            address b,
                                            address c,
                                            address d,
                                            address e,
                                            address f) {
        doSomething();
    }

    function thisFunctionHasLotsOfArguments(
        address a,
        address b,
        address c,
        address d,
        address e,
        address f) {
        doSomething();
    }

If a long function declaration has modifiers, then each modifier should be
dropped to it's own line.

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

For constructor functions on inherited contracts whose bases require arguments,
it is recommended to drop the base constructors onto new lines in the same
manner as modifiers if the function declaration is long or hard to read.

Yes::

    contract A is B, C, D {
        function A(uint param1, uint param2, uint param3, uint param4, uint param5)
            B(param1)
            C(param2, param3)
            D(param4)
        {
            // do something with param5
        }
    }

No::

    contract A is B, C, D {
        function A(uint param1, uint param2, uint param3, uint param4, uint param5)
        B(param1)
        C(param2, param3)
        D(param4)
        {
            // do something with param5
        }
    }

    contract A is B, C, D {
        function A(uint param1, uint param2, uint param3, uint param4, uint param5)
            B(param1)
            C(param2, param3)
            D(param4) {
            // do something with param5
        }
    }

When declaring short functions with a single statement, it is permissible to do it on a single line.

Permissible::

    function shortFunction() { doSomething(); }

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

.. note:: When using abbreviations in CapWords, capitalize all the letters of the abbreviation. Thus HTTPServerError is better than HttpServerError.


Names to Avoid
==============

* ``l`` - Lowercase letter el
* ``O`` - Uppercase letter oh
* ``I`` - Uppercase letter eye

Never use any of these for single letter variable names.  They are often
indistinguishable from the numerals one and zero.


Contract and Library Names
==========================

Contracts and libraries should be named using the CapWords style.


Events
======

Events should be named using the CapWords style.


Function Names
==============

Functions should use mixedCase.


Function Arguments
==================

When writing library functions that operate on a custom struct, the struct
should be the first argument and should always be named ``self``.


Local and State Variables
=========================

Use mixedCase.


Constants
=========

Constants should be named with all capital letters with underscores separating
words.  (for example:``MAX_BLOCKS``)


Modifiers
=========

Use mixedCase.


Avoiding Collisions
===================

* ``single_trailing_underscore_``

This convention is suggested when the desired name collides with that of a
built-in or otherwise reserved name.


General Recommendations
=======================

TODO
