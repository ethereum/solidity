---
layout: docs
title: Frequently Asked Questions (Advanced)
link_title: Advanced
permalink: /docs/faq/advanced/
---

This list was originally compiled by [fivedogit](mailto:fivedogit@gmail.com).

### How do you get a random number in a contract? (Implement a self-returning gambling contract.)

Getting randomness right is often the crucial part in a crypto project and
most failures result from bad random number generators.

If you do not want it to be safe, you build something similar to the [coin flipper](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/35_coin_flipper.sol)
but otherwise, rather use a contract that supplies randomness, like the [RANDAO](https://github.com/randao/randao).

### Get return value from non-constant function from another contract

The key point is that the calling contract needs to know about the function it intends to call.

See [ping.sol](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/45_ping.sol)
and [pong.sol](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/45_pong.sol).

### Get contract to do something when it is first mined

Use the constructor. Anything inside it will be executed when the contract is first mined.

See [replicator.sol](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/50_replicator.sol).

### Can a contract create another contract?

Yes, see [replicator.sol](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/50_replicator.sol).

Note that the full code of the created contract has to be included in the creator contract.
This also means that cyclic creations are not possible (because the contract would have
to contain its own code) - at least not in a general way.

### How do you create 2-dimensional arrays?

See [2D_array.sol](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/55_2D_array.sol).

Note that filling a 10x10 square of `uint8` + contract creation took more than `800,000`
gas at the time of this writing. 17x17 took `2,000,000` gas. With the limit at
3.14 million... well, there’s a pretty low ceiling for what you can create right
now.

Note that merely "creating" the array is free, the costs are in filling it.

Note2: Optimizing storage access can pull the gas costs down considerably, because
32 `uint8` values can be stored in a single slot. The problem is that these optimizations
currently do not work across loops and also have a problem with bounds checking.
You might get much better results in the future, though.

### What does p.recipient.call.value(p.amount)(p.data) do?

Every external function call in Solidity can be modified in two ways:

1. You can add Ether together with the call
2. You can limit the amount of gas available to the call

This is done by "calling a function on the function":

`f.gas(2).value(20)()` calls the modified function `f` and thereby sending 20
Wei and limiting the gas to 2 (so this function call will most likely go out of
gas and return your 20 Wei).

In the above example, the low-level function `call` is used to invoke another
contract with `p.data` as payload and `p.amount` Wei is sent with that call.

### Can a contract function accept a two-dimensional array?

This is not yet implemented for external calls and dynamic arrays - 
you can only use one level of dynamic arrays.

### What is the relationship between bytes32 and string? Why is it that ‘bytes32 somevar = "stringliteral";’ works and what does the saved 32-byte hex value mean?

The type `bytes32` can hold 32 (raw) bytes. In the assignment `bytes32 samevar = "stringliteral";`,
the string literal is interpreted in its raw byte form and if you inspect `somevar` and
see a 32-byte hex value, this is just `"stringliteral"` in hex.

The type `bytes` is similar, only that it can change its length.

Finally, `string` is basically identical to `bytes` only that it is assumed
to hold the utf-8 encoding of a real string. Since `string` stores the
data in utf-8 encoding it is quite expensive to compute the number of
characters in the string (the encoding of some characters takes more
than a single byte). Because of that, `string s; s.length` is not yet
supported and not even index access `s[2]`. But if you want to access
the low-level byte encoding of the string, you can use
`bytes(s).length` and `bytes(s)[2]` which will result in the number
of bytes in the utf-8 encoding of the string (not the number of
characters) and the second byte (not character) of the utf-8 encoded
string, respectively.


### Can a contract pass an array (static size) or string or bytes (dynamic size) to another contract?

Sure. Take care that if you cross the memory / storage boundary,
independent copies will be created:
{% highlight JavaScript %}
contract C {
  uint[20] x;
  function f() {
    g(x);
    h(x);
  }
  function g(uint[20] y) {
    y[2] = 3;
  }
  function h(uint[20] storage y) {
    y[3] = 4;
  }
{% endhighlight %}
The call to `g(x)` will not have an effect on `x` because it needs
to create an independent copy of the storage value in memory
(the default storage location is memory). On the other hand,
`h(x)` successfully modifies `x` because only a reference
and not a copy is passed.

### Sometimes, when I try to change the length of an array with ex: "arrayname.length = 7;" I get a compiler error "Value must be an lvalue". Why?

You can resize a dynamic array in storage (i.e. an array declared at the
contract level) with `arrayname.length = <some new length>;`. If you get the
"lvalue" error, you are probably doing one of two things wrong.

1. You might be trying to resize an array in "memory", or

2. You might be trying to resize a non-dynamic array.

{% highlight JavaScript %}
int8[] memory memArr;       // Case 1
memArr.length++;            // illegal
int8[5] storageArr;         // Case 2
somearray.length++;         // legal
int8[5] storage storageArr2; // Explicit case 2
somearray2.length++;         // legal
{% endhighlight %}

**Important note:** In Solidity, array dimensions are declared backwards from the way you
might be used to declaring them in C or Java, but they are access as in
C or Java.

For example, `int8[][5] somearray;` are 5 dynamic `int8` arrays.

The reason for this is that `T[5]` is always an array of 5 `T`s,
no matter whether `T` itself is an array or not (this is not the
case in C or Java).

### Is it possible to return an array of strings ( string[] ) from a Solidity function?

Not yet, as this requires two levels of dynamic arrays (`string` is a dynamic array itself).

### If you issue a call for an array, it is possible to retrieve the whole array? Or must you write a helper function for that?

The automatic accessor function for a public state variable of array type only returns
individual elements. If you want to return the complete array, you have to
manually write a function to do that.


###  What could have happened if an account has storage value/s but no code?  Example: http://test.ether.camp/account/5f740b3a43fbb99724ce93a879805f4dc89178b5

The last thing a constructor does is returning the code of the contract.
The gas costs for this depend on the length of the code and it might be
that the supplied gas is not enough. This situation is the only one
where an "out of gas" exception does not revert changes to the state,
i.e. in this case the initialisation of the state variables.

https://github.com/ethereum/wiki/wiki/Subtleties

After a successful CREATE operation's sub-execution, if the operation returns x, 5 * len(x) gas is subtracted from the remaining gas before the contract is created. If the remaining gas is less than 5 * len(x), then no gas is subtracted, the code of the created contract becomes the empty string, but this is not treated as an exceptional condition - no reverts happen.


### How do I use .send()?

If you want to send 20 Ether from a contract to the address `x`, you use `x.send(20 ether);`.
Here, `x` can be a plain address or a contract. If the contract already explicitly defines
a function `send` (and thus overwrites the special function), you can use `address(x).send(20 ether);`.

## More Questions?

If you have more questions or your question is not answered here, please talk to us on
[gitter](https://gitter.im/ethereum/solidity) or file an [issue](https://github.com/ethereum/solidity/issues).
