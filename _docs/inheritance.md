---
layout: docs
title: Inheritance
permalink: /docs/inheritance/
---

Solidity supports multiple inheritance by copying code including polymorphism.

All function calls are virtual, which means that the most derived function
is called, except when the contract is explicitly given.

Even if a contract inherits from multiple other contracts, only a single
contract is created on the blockchain, the code from the base contracts
is always copied into the final contract.

The general inheritance system is very similar to
[Python's](https://docs.python.org/3/tutorial/classes.html#inheritance),
especially concerning multiple inheritance.

Details are given in the following example.

{% highlight javascript %}
contract owned {
    function owned() { owner = msg.sender; }
    address owner;
}

// Use "is" to derive from another contract. Derived
// contracts can access all non-private members including
// internal functions and state variables. These cannot be
// accessed externally via `this`, though.
contract mortal is owned {
    function kill() {
      if (msg.sender == owner) selfdestruct(owner);
    }
}

// These abstract contracts are only provided to make the
// interface known to the compiler. Note the function
// without body. If a contract does not implement all
// functions it can only be used as an interface.
contract Config {
    function lookup(uint id) returns (address adr);
}
contract NameReg {
    function register(bytes32 name);
    function unregister();
 }

// Multiple inheritance is possible. Note that "owned" is
// also a base class of "mortal", yet there is only a single
// instance of "owned" (as for virtual inheritance in C++).
contract named is owned, mortal {
    function named(bytes32 name) {
        Config config = Config(0xd5f9d8d94886e70b06e474c3fb14fd43e2f23970);
        NameReg(config.lookup(1)).register(name);
    }

    // Functions can be overridden, both local and
    // message-based function calls take these overrides
    // into account.
    function kill() {
        if (msg.sender == owner) {
            Config config = Config(0xd5f9d8d94886e70b06e474c3fb14fd43e2f23970);
            NameReg(config.lookup(1)).unregister();
            // It is still possible to call a specific
            // overridden function.
            mortal.kill();
        }
    }
}

// If a constructor takes an argument, it needs to be
// provided in the header (or modifier-invocation-style at
// the constructor of the derived contract (see below)).
contract PriceFeed is owned, mortal, named("GoldFeed") {
   function updateInfo(uint newInfo) {
      if (msg.sender == owner) info = newInfo;
   }

   function get() constant returns(uint r) { return info; }

   uint info;
}
{% endhighlight %}

Note that above, we call `mortal.kill()` to "forward" the
destruction request. The way this is done is problematic, as
seen in the following example:
{% highlight javascript %}
contract mortal is owned {
    function kill() {
        if (msg.sender == owner) selfdestruct(owner);
    }
}
contract Base1 is mortal {
    function kill() { /* do cleanup 1 */ mortal.kill(); }
}
contract Base2 is mortal {
    function kill() { /* do cleanup 2 */ mortal.kill(); }
}
contract Final is Base1, Base2 {
}
{% endhighlight %}

A call to `Final.kill()` will call `Base2.kill` as the most
derived override, but this function will bypass
`Base1.kill`, basically because it does not even know about
`Base1`.  The way around this is to use `super`:
{% highlight javascript %}
contract mortal is owned {
    function kill() {
        if (msg.sender == owner) selfdestruct(owner);
    }
}
contract Base1 is mortal {
    function kill() { /* do cleanup 1 */ super.kill(); }
}
contract Base2 is mortal {
    function kill() { /* do cleanup 2 */ super.kill(); }
}
contract Final is Base2, Base1 {
}
{% endhighlight %}

If `Base1` calls a function of `super`, it does not simply
call this function on one of its base contracts, it rather
calls this function on the next base contract in the final
inheritance graph, so it will call `Base2.kill()` (note that
the final inheritance sequence is -- starting with the most
derived contract: Final, Base1, Base2, mortal, owned).
The actual function that is called when using super is
not known in the context of the class where it is used,
although its type is known. This is similar for ordinary
virtual method lookup.

### Arguments for Base Constructors

Derived contracts need to provide all arguments needed for
the base constructors. This can be done at two places:

{% highlight javascript %}
contract Base {
  uint x;
  function Base(uint _x) { x = _x; }
}
contract Derived is Base(7) {
  function Derived(uint _y) Base(_y * _y) {
  }
}
{% endhighlight %}

Either directly in the inheritance list (`is Base(7)`) or in
the way a modifier would be invoked as part of the header of
the derived constructor (`Base(_y * _y)`). The first way to
do it is more convenient if the constructor argument is a
constant and defines the behaviour of the contract or
describes it. The second way has to be used if the
constructor arguments of the base depend on those of the
derived contract. If, as in this silly example, both places
are used, the modifier-style argument takes precedence.


### Multiple Inheritance and Linearization

Languages that allow multiple inheritance have to deal with
several problems, one of them being the [Diamond Problem](https://en.wikipedia.org/wiki/Multiple_inheritance#The_diamond_problem).
Solidity follows the path of Python and uses "[C3 Linearization](https://en.wikipedia.org/wiki/C3_linearization)"
to force a specific order in the DAG of base classes. This
results in the desirable property of monotonicity but
disallows some inheritance graphs. Especially, the order in
which the base classes are given in the `is` directive is
important. In the following code, Solidity will give the
error "Linearization of inheritance graph impossible".

{% highlight javascript %}
contract X {}
contract A is X {}
contract C is A, X {}
{% endhighlight %}
The reason for this is that `C` requests `X` to override `A`
(by specifying `A, X` in this order), but `A` itself
requests to override `X`, which is a contradiction that
cannot be resolved.

A simple rule to remember is to specify the base classes in
the order from "most base-like" to "most derived".
