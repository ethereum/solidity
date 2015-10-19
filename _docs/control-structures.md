---
layout: docs
title: Control Structures
permalink: /docs/control-structures/
---
# Control Structures

Most of the control structures from C/JavaScript are available in Solidity
except for `switch` and `goto`. So
there is: `if`, `else`, `while`, `for`, `break`, `continue`, `return`, with
the usual semantics known from C / JavaScript.

Parentheses can *not* be omitted for conditionals, but curly brances can be omitted
around single-statement bodies.

Note that there is no type conversion from non-boolean to boolean types as
there is in C and JavaScript, so `if (1) { ... }` is _not_ valid Solidity.

## Function Calls

### Internal Function Calls

Functions of the current contract can be called directly ("internally"), also recursively, as seen in
this nonsensical example:

{% highlight javascript %}
contract c {
  function g(uint a) returns (uint ret) { return f(); }
  function f() returns (uint ret) { return g(7) + f(); }
}
{% endhighlight %}

These function calls are translated into simple jumps inside the EVM. This has
the effect that the current memory is not cleared, i.e. passing memory references
to internally-called functions is very efficient. Only functions of the same
contract can be called internally.

### External Function Calls

The expression `this.g(8);` is also a valid function call, but this time, the function
will be called "externally", via a message call and not directly via jumps.
Functions of other contracts have to be called externally. For an external call,
all function arguments have to be copied to memory.

When calling functions
of other contracts, the amount of Wei sent with the call and the gas can be specified:
{% highlight javascript %}
contract InfoFeed {
  function info() returns (uint ret) { return 42; }
}
contract Consumer {
  InfoFeed feed;
  function setFeed(address addr) { feed = InfoFeed(addr); }
  function callFeed() { feed.info.value(10).gas(800)(); }
}
{% endhighlight %}
Note that the expression `InfoFeed(addr)` performs an explicit type conversion stating
that "we know that the type of the contract at the given address is `InfoFeed`" and
this does not execute a constructor. We could also have used `function setFeed(InfoFeed _feed) { feed = _feed; }` directly.  Be careful about the fact that `feed.info.value(10).gas(800)`
only (locally) sets the value and amount of gas sent with the function call and only the
parentheses at the end perform the actual call.

### Named Calls and Anonymous Function Parameters

Function call arguments can also be given by name, in any order, and the names
of unused parameters (especially return parameters) can be omitted.

{% highlight javascript %}
contract c {
  function f(uint key, uint value) { ... }
  function g() {
    // named arguments
    f({value: 2, key: 3});
  }
  // omitted parameters
  function func(uint k, uint) returns(uint) {
    return k;
  }
}
{% endhighlight %}

## Order of Evaluation of Expressions

The evaluation order of expressions is not specified (more formally, the order
in which the children of one node in the expression tree are evaluated is not
specified, but they are of course evaluated before the node itself). It is only
guaranteed that statements are executed in order and short-circuiting for
boolean expressions is done.

## Assignment

### Destructuring Assignments and Returning Multiple Values

Solidity internally allows tuple types, i.e. a list of objects of potentially different types whose size is a constant at compile-time. Those tuples can be used to return multiple values at the same time and also assign them to multiple variables (or LValues in general) at the same time:

{% highlight javascript %}
contract C {
  uint[] data;
  function f() returns (uint, bool, uint) {
    return (7, true, 2);
  }
  function g() {
    // Declares and assigns the variables. Specifying the type explicitly is not possible.
    var (x, b, y) = f();
    // Assigns to a pre-existing variable.
    (x, y) = (2, 7);
    // Common trick to swap values -- does not work for non-value storage types.
    (x, y) = (y, x);
    // Components can be left out (also for variable declarations).
    // If the tuple ends in an empty component,
    // the rest of the values are discarded.
    (data.length,) = f(); // Sets the length to 7
    // The same can be done on the left side.
    (,data[3]) = f(); // Sets data[3] to 2
    // Components can only be left out at the left-hand-side of assignments, with
    // one exception:
    (x,) = (1,);
    // (1,) is the only way to specify a 1-component tuple, because (1) is
    // equivalent to 1.
  }
}
{% endhighlight %}

### Complications for Arrays and Structs

The semantics of assignment are a bit more complicated for non-value types like arrays and structs.
Assigning *to* a state variable always creates an independent copy. On the other hand, assigning to a local variable creates an independent copy only for elementary types, i.e. static types that fit into 32 bytes. If structs or arrays (including `bytes` and `string`) are assigned from a state variable to a local variable, the local variable holds a reference to the original state variable. A second assignment to the local variable does not modify the state but only changes the reference. Assignments to members (or elements) of the local variable *do* change the state.

## Exceptions

There are some cases where exceptions are thrown automatically (see below). You can use the `throw` instruction to throw an exception manually. The effect of an exception is that the currently executing call is stopped and reverted (i.e. all changes to the state and balances are undone) and the exception is also "bubbled up" through Solidity function calls (exceptions are `send` and the low-level functions `call` and `callcode`, those return `false` in case of an exception).

Catching exceptions is not yet possible.

In the following example, we show how `throw` can be used to easily revert an Ether transfer and also how to check the return value of `send`:
{% highlight javascript %}
contract Sharer {
    function sendHalf(address addr) returns (uint balance) {
        if (!addr.send(msg.value/2))
            throw; // also reverts the transfer to Sharer
        return this.balance;
    }
}
{% endhighlight %}

Currently, there are three situations, where exceptions happen automatically in Solidity:

1. If you access an array beyond its length (i.e. `x[i]` where `i >= x.length`)
2. If a function called via a message call does not finish properly (i.e. it runs out of gas or throws an exception itself).
3. If a non-existent function on a library is called or Ether is sent to a library.

Internally, Solidity performs an "invalid jump" when an exception is thrown and thus causes the EVM to revert all changes made to the state. The reason for this is that there is no safe way to continue execution, because an expected effect did not occur. Because we want to retain the atomicity of transactions, the safest thing to do is to revert all changes and make the whole transaction (or at least call) without effect.

