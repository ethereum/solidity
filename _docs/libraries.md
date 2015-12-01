---
layout: docs
title: Contracts
permalink: /docs/contracts/
---

Libraries are similar to contracts, but their purpose is that they are deployed
only once at a specific address and their code is reused using the `CALLCODE`
feature of the EVM. This means that if library functions are called, their code
is executed in the context of the calling contract, i.e. `this` points to the
calling contract and especially the storage from the calling contract can be
accessed. As a library is an isolated piece of source code, it can only access
state variables of the calling contract if they are explicitly supplied (it
would have to way to name them, otherwise).

The following example illustrates how to use libraries (but
be sure to check out [usign for](../using-for) for a
more advanced example to implement a set).
{% highlight javascript %}
library Set {
  // We define a new struct datatype that will be used to
  // hold its data in the calling contract.
  struct Data { mapping(uint => bool) flags; }
  // Note that the first parameter is of type "storage
  // reference" and thus only its storage address and not
  // its contents is passed as part of the call.  This is a
  // special feature of library functions.  It is idiomatic
  // to call the first parameter 'self', if the function can
  // be seen as a method of that object.
  function insert(Data storage self, uint value)
      returns (bool)
  {
    if (self.flags[value])
      return false; // already there
    self.flags[value] = true;
    return true;
  }
  function remove(Data storage self, uint value)
    returns (bool)
  {
    if (!self.flags[value])
      return false; // not there
    self.flags[value] = false;
    return true;
  }
  function contains(Data storage self, uint value)
    returns (bool)
  {
    return self.flags[value];
  }
}
contract C {
  Set.Data knownValues;
  function register(uint value) {
    // The library functions can be called without a
    // specific instance of the library, since the
    // "instance" will be the current contract.
    if (!Set.insert(knownValues, value))
      throw;
  }
  // In this contract, we can also directly access knownValues.flags, if we want.
}
{% endhighlight %}
Of course, you do not have to follow this way to use
libraries - they can also be used without defining struct
data types, functions also work without any storage
reference parameters, can have multiple storage reference
parameters and in any position.

The calls to `Set.contains`, `Set.insert` and `Set.remove`
are all compiled as calls (`CALLCODE`s) to an external
contract/library. If you use libraries, take care that an
actual external function call is performed, so `msg.sender`
does not point to the original sender anymore but to the the
calling contract and also `msg.value` contains the funds
sent during the call to the library function.

As the compiler cannot know where the library will be
deployed at, these addresses have to be filled into the
final bytecode by a linker (see [Using the Commandline
Compiler](#using-the-commandline-compiler) on how to use the
commandline compiler for linking). If the addresses are not
given as arguments to the compiler, the compiled hex code
will contain placeholders of the form `__Set______` (where
`Set` is the name of the library). The address can be filled
manually by replacing all those 40 symbols by the hex
encoding of the address of the library contract.

Restrictions for libraries in comparison to contracts:

 - no state variables
 - cannot inherit nor be inherited

(these might be lifted at a later point)

### Common pitfalls for libraries

#### The value of `msg.sender`

The value for `msg.sender` will be that of the contract which is calling the library function.

For example, if A calls contract B which internally calls library C, then within the function call of library C, `msg.sender` will be the address of contract B.

The reason for this is that the expression `LibraryName.functionName()`
performs an external function call using `CALLCODE`, which maps to a real EVM
call just like `otherContract.functionName()` or `this.functionName()`.  This
call extends the call depth by one (limited to 1024), stores the caller (the
current contract) as `msg.sender`, and then executes the library contract's
code against the current contracts storage.  This execution occurs in a
completely new memory context meaning that memory types will be copied and
cannot be passed by reference.

It is *in principle* possible to transfer ether using
`LibraryName.functionName.value(x)()`, but as `CALLCODE` is used, the Ether
will just end up at the current contract.
