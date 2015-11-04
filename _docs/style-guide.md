---
layout: docs
title: Style Guide
permalink: /docs/style-guide/
---

## Introduction

This guide is intended to provide coding conventions for writing solidity code.
This guide should be thought of as an evolving document that will change over
time as useful conventions are found and old conventions are rendered obsolete.

Many projects will implement their own style guides.  In the event of
conflicts, project specific style guides take precidence.

The structure and many of the recommendations within this style guide were
taken from python's
[pep8 style guide](https://www.python.org/dev/peps/pep-0008/)

## Things this guide is not

The goal of this guide is to be the *right* way or the *best* way to write
solidity code.

The goal of this guide is consistency.  A quote from python's
[pep8](https://www.python.org/dev/peps/pep-0008/#a-foolish-consistency-is-the-hobgoblin-of-little-minds)
captures this concept well.

>  A style guide is about consistency. Consistency with this style guide is important. Consistency within a project is more important. Consistency within one module or function is most important.
> But most importantly: know when to be inconsistent -- sometimes the style guide just doesn't apply. When in doubt, use your best judgment. Look at other examples and decide what looks best. And don't hesitate to ask! 


## Code Layout


### Indentation

Use 4 spaces per indentation level.

### Tabs or Spaces

Spaces are the preferred indentation method.

Mixing tabs and spaces should be avoided.

### Blank Lines

Surround top level declarations in solidity source with two blank lines.

Yes:
```
contract A {
    ...
}


contract B {
    ...
}


contract C {
    ...
}
```

No:
```
contract A {
    ...
}
contract B {
    ...
}

contract C {
    ...
}
```

Within a contract surround function declarations with a single blank line.

Blank lines may be omitted between groups of related one-liners (such as stub functions for an abstract contract)

Yes:
```
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
```

No:
```
contract A {
    function spam() {
        ...
    }
    function ham() {
        ...
    }
}
```

### Source File Encoding

UTF-8 or ASCII encoding is preferred.

### Imports

Import statements should always be placed at the top of the file.

Yes:
```
import "owned";


contract A {
    ...
}


contract B is owned {
    ...
}
```

No:
```
contract A {
    ...
}


import "owned";


contract B is owned {
    ...
}
```

### Whitespace in Expressions

Avoid extraneous whitespace in the following  situations:

* Immediately inside parenthesis, brackets or braces.

    ```
    Yes: spam(ham[1], Coin({name: "ham"}));
    No: spam( ham[ 1 ], Coin( { name: "ham" } ) );
    ```

* Immediately before a comma, semicolon:

    ```
    Yes: function spam(uint i, Coin coin);
    No: function spam(uint i , Coin coin) ;
    ```

* More than one space around an assignment or other operator to align with
  another:

    ```
    Yes:

    x = 1;
    y = 2;
    long_variable = 3;

    No:

    x             = 1;
    y             = 2;
    long_variable = 3;
    ```


### Control Structures

The braces denoting the body of a contract, library, struct, function should:

* open on the same line as the declaration
* close on their own line at the same indentation level as the beginning of the
  declaration.
* The opening brace should be proceeded by a single space.

Yes:
```
contract Coin {
    struct Bank {
        address owner;
        uint balance;
    }
}
```

No:
```
contract Coin
{
    struct Bank {
        address owner;
        uint balance;
    }
}
```

The same recommendations apply to the control structures `if`, `else`, `while`,
and `for`.

Additionally there should be a single space between the control structures
`if`, `while`, and `for` and the parenthetic block representing the
conditional, as well as a single space between the conditional parenthetic
block and the opening brace.

Yes:
```
if (...) {
    ...
}

for (...) {
    ...
}
```

No:
```
if (...)
{
    ...
}

while(...){
}

for (...) {
    ...;}
```

For control structures who's body contains a single statement, omitting the
braces is ok *if* the statement is contained on a single line.

Yes:
```
if (x < 10)
    x += 1;
```

No:
```
if (x < 10)
    someArray.push(Coin({
        name: 'spam',
        value: 42
    }));
```

For `if` blocks which have an `else` or `else if` clause, the `else` should be
placed on it's own line following the previous closing parenthesis.  The
parenthesis for the else block should follow the same rules as the other
conditional control structures.

Yes:
```
if (x < 3) {
    x += 1;
}
else {
    x -= 1;
}


if (x < 3)
    x += 1;
else
    x -= 1;
```

No:
```
if (x < 3) {
    x += 1;
} else {
    x -= 1;
}
```

### Variable Declarations

Declarations of array variables should not have a space between the type and
the brackets.

```
Yes: uint[] x;
No:  uint [] x;
```

### Other Recommendations

* Surround operators with a single space on either side.

    ```
    Yes:

    x = 3;
    x = 100 / 10;
    x += 3 + 4;
    x |= y && z;

    No:

    x=3;
    x = 100/10;
    x += 3+4;
    x |= y&&z;
    ```

* Operators with a higher priority than others can exclude surrounding
  whitespace in order to denote precidence.  This is meant to allow for
  improved readability for complex statement. You should always use the same
  amount of whitespace on either side of an operator:

    ```
    Yes:

    x = 2**3 + 5;
    x = 2*y + 3*z;
    x = (a+b) * (a-b);

    No:

    x = 2** 3 + 5;
    x = y+z;
    x +=1;
    ```

## Naming Conventions

TODO

## General Recommendations

TODO
