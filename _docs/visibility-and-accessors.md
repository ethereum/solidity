---
layout: docs
title: Visibility and Accessors
permalink: /docs/visibility-and-accessors/
---

Since Solidity knows two kinds of function calls (internal
ones that do not create an actual EVM call (also called
a "message call") and external
ones that do), there are four types of visibilities for
functions and state variables.

Functions can be specified as being `external`,
`public`, `internal` or `private`, where the default is
`public`. For state variables, `external` is not possible
and the default is `internal`.

`external`: External functions are part of the contract
interface, which means they can be called from other contracts and
via transactions. An external function `f` cannot be called
internally (i.e. `f()` does not work, but `this.f()` works).
External functions are sometimes more efficient when
they receive large arrays of data.

`public`: Public functions are part of the contract
interface and can be either called internally or via
messages. For public state variables, an automatic accessor
function (see below) is generated.

`internal`: Those functions and state variables can only be
accessed internally (i.e. from within the current contract
or contracts deriving from it), without using `this`.

`private`: Private functions and state variables are only
visible for the contract they are defined in and not in
derived contracts.

The visibility specifier is given after the type for
state variables and between parameter list and
return parameter list for functions.

{% highlight javascript %}
contract c {
  function f(uint a) private returns (uint b) { return a + 1; }
  function setData(uint a) internal { data = a; }
  uint public data;
}
{% endhighlight %}

Other contracts can call `c.data()` to retrieve the value of
data in state storage, but are not able to call `f`.
Contracts derived from `c` can call `setData` to alter the
value of `data` (but only in their own state).

## Accessor Functions

The compiler automatically creates accessor functions for
all public state variables. The contract given below will
have a function called `data` that does not take any
arguments and returns a uint, the value of the state
variable `data`. The initialization of state variables can
be done at declaration.

The accessor functions have external visibility. If the
symbol is accessed internally (i.e. without `this.`),
it is a state variable and if it is accessed externally
(i.e. with `this.`), it is a function.

{% highlight javascript %}
contract test {
     uint public data = 42;
}
{% endhighlight %}

The next example is a bit more complex:

{% highlight javascript %}
contract complex {
  struct Data { uint a; bytes3 b; mapping(uint => uint) map; }
  mapping(uint => mapping(bool => Data[])) public data;
}
{% endhighlight %}

It will generate a function of the following form:
{% highlight javascript %}
function data(uint arg1, bool arg2, uint arg3) returns (uint a, bytes3 b)
{
  a = data[arg1][arg2][arg3].a;
  b = data[arg1][arg2][arg3].b;
}
{% endhighlight %}

Note that the mapping in the struct is omitted because there
is no good way to provide the key for the mapping.
