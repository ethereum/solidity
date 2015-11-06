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
{% highlight javascript %}
contract A {
    ...
}


contract B {
    ...
}


contract C {
    ...
}
{% endhighlight %}

No:
{% highlight javascript %}
contract A {
    ...
}
contract B {
    ...
}

contract C {
    ...
}
{% endhighlight %}

Within a contract surround function declarations with a single blank line.

Blank lines may be omitted between groups of related one-liners (such as stub functions for an abstract contract)

Yes:
{% highlight javascript %}
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
{% endhighlight %}

No:
{% highlight javascript %}
contract A {
    function spam() {
        ...
    }
    function ham() {
        ...
    }
}
{% endhighlight %}

### Source File Encoding

UTF-8 or ASCII encoding is preferred.

### Imports

Import statements should always be placed at the top of the file.

Yes:
{% highlight javascript %}
import "owned";


contract A {
    ...
}


contract B is owned {
    ...
}
{% endhighlight %}

No:
{% highlight javascript %}
contract A {
    ...
}


import "owned";


contract B is owned {
    ...
}
{% endhighlight %}

### Whitespace in Expressions

Avoid extraneous whitespace in the following  situations:

* Immediately inside parenthesis, brackets or braces.

    {% highlight javascript %}
    Yes: spam(ham[1], Coin({name: "ham"}));
    No: spam( ham[ 1 ], Coin( { name: "ham" } ) );
    {% endhighlight %}

* Immediately before a comma, semicolon:

    {% highlight javascript %}
    Yes: function spam(uint i, Coin coin);
    No: function spam(uint i , Coin coin) ;
    {% endhighlight %}

* More than one space around an assignment or other operator to align with
  another:

    {% highlight javascript %}
    Yes:

    x = 1;
    y = 2;
    long_variable = 3;

    No:

    x             = 1;
    y             = 2;
    long_variable = 3;
    {% endhighlight %}


### Control Structures

The braces denoting the body of a contract, library, struct, function should:

* open on the same line as the declaration
* close on their own line at the same indentation level as the beginning of the
  declaration.
* The opening brace should be proceeded by a single space.

Yes:
{% highlight javascript %}
contract Coin {
    struct Bank {
        address owner;
        uint balance;
    }
}
{% endhighlight %}

No:
{% highlight javascript %}
contract Coin
{
    struct Bank {
        address owner;
        uint balance;
    }
}
{% endhighlight %}

The same recommendations apply to the control structures `if`, `else`, `while`,
and `for`.

Additionally there should be a single space between the control structures
`if`, `while`, and `for` and the parenthetic block representing the
conditional, as well as a single space between the conditional parenthetic
block and the opening brace.

Yes:
{% highlight javascript %}
if (...) {
    ...
}

for (...) {
    ...
}
{% endhighlight %}

No:
{% highlight javascript %}
if (...)
{
    ...
}

while(...){
}

for (...) {
    ...;}
{% endhighlight %}

For control structures who's body contains a single statement, omitting the
braces is ok *if* the statement is contained on a single line.

Yes:
{% highlight javascript %}
if (x < 10)
    x += 1;
{% endhighlight %}

No:
{% highlight javascript %}
if (x < 10)
    someArray.push(Coin({
        name: 'spam',
        value: 42
    }));
{% endhighlight %}

For `if` blocks which have an `else` or `else if` clause, the `else` should be
placed on it's own line following the previous closing parenthesis.  The
parenthesis for the else block should follow the same rules as the other
conditional control structures.

Yes:
{% highlight javascript %}
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
{% endhighlight %}

No:
{% highlight javascript %}
if (x < 3) {
    x += 1;
} else {
    x -= 1;
}
{% endhighlight %}

### Function Modifiers

TODO

### Mappings

TODO

### Variable Declarations

Declarations of array variables should not have a space between the type and
the brackets.

{% highlight javascript %}
Yes: uint[] x;
No:  uint [] x;
{% endhighlight %}

### Other Recommendations

* Surround operators with a single space on either side.

    {% highlight javascript %}
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
    {% endhighlight %}

* Operators with a higher priority than others can exclude surrounding
  whitespace in order to denote precidence.  This is meant to allow for
  improved readability for complex statement. You should always use the same
  amount of whitespace on either side of an operator:

    {% highlight javascript %}
    Yes:

    x = 2**3 + 5;
    x = 2*y + 3*z;
    x = (a+b) * (a-b);

    No:

    x = 2** 3 + 5;
    x = y+z;
    x +=1;
    {% endhighlight %}

## Naming Conventions

TODO

## General Recommendations

TODO
