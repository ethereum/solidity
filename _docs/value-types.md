---
layout: docs
title: Value Types
permalink: /docs/value-types/
---

The following types are also called value types because variables of these
types will always be passed by value, i.e. they are always copied when they
are used as function arguments or in assignments.

**Booleans**  
`bool`: The possible values are constants `true` and `false`.

Operators:  

*  `!` (logical negation)
*  `&&` (logical conjunction, "and")
*  `||` (logical disjunction, "or")
*  `==` (equality)
*  `!=` (inequality)

> The operators `||` and `&&` apply the common short-circuiting rules. This means that in the expression `f(x) || g(y)`, if `f(x)` evaluates to `true`, `g(y)` will not be evaluated even if it may have side-effects.

**Integers**  
`int•` / `uint•`: Signed and unsigned integers of various sizes. Keywords `uint8` to `uint256` in steps of `8` (unsigned of 8 up to 256 bits) and `int8` to `int256`. `uint` and `int` are aliases for `uint256` and `int256`, respectively.

Operators:  

* Comparisons: `<=`, `<`, `==`, `!=`, `>=`, `>` (evaluate to `bool`)  
* Bit operators: `&`, `|`, `^` (bitwise exclusive or), `~` (bitwise negation)  
* Arithmetic operators: `+`, `-`, unary `-`, unary `+`, `*`, `/`, `%` (remainder), `**` (exponentiation)

**Address**  
`address`: Holds a 20 byte value (size of an Ethereum address). Address types also have members(see [Functions on addresses](#functions-on-addresses)) and serve as base for all contracts.

Operators:  

* `<=`, `<`, `==`, `!=`, `>=` and `>`

**Fixed-size byte arrays**  
`bytes1`, `bytes2`, `bytes3`, ..., `bytes32`. `byte` is an alias for `bytes1`.  

Operators:  

* Comparisons: `<=`, `<`, `==`, `!=`, `>=`, `>` (evaluate to `bool`)  
* Bit operators: `&`, `|`, `^` (bitwise exclusive or), `~` (bitwise negation)  

**Dynamically-sized byte array**  
`bytes`: Dynamically-sized byte array, see [arrays](#arrays). Not a value-type!  
`string`: Dynamically-sized UTF8-encoded string, see [arrays](#arrays). Not a value-type!

**Integer Literals**  
Integer Literals are arbitrary precision integers until they are used together with a non-literal. In `var x = 1 - 2;`, for example, the value of `1 - 2` is `-1`, which is assigned to `x` and thus `x` receives the type `int8` -- the smallest type that contains `-1`, although the natural types of `1` and `2` are actually `uint8`.    

It is even possible to temporarily exceed the maximum of 256 bits as long as only integer literals are used for the computation: `var x = (0xffffffffffffffffffff * 0xffffffffffffffffffff) * 0;` Here, `x` will have the value `0` and thus the type `uint8`.

**String Literals**  
String Literals are written with double quotes (`"abc"`). As with integer literals, their type can vary, but they are implicitly convertible to `bytes•` if they fit, to `bytes` and to `string`.

## Operators Involving LValues

If `a` is an LValue (i.e. a variable or something that can be assigned to), the following operators are available as shorthands:

`a += e` is equivalent to `a = a + e`. The operators `-=`, `*=`, `/=`, `%=`, `a |=`, `&=` and `^=` are defined accordingly. `a++` and `a--` are equivalent to `a += 1` / `a -= 1` but the expression itself still has the previous value of `a`. In contrast, `--a` and `++a` have the same effect on `a` but return the value after the change.

**delete**

`delete a` assigns the initial value for the type to `a`. I.e. for integers it is equivalent to `a = 0`, but it can also be used on arrays, where it assigns a dynamic array of length zero or a static array of the same length with all elements reset. For structs, it assigns a struct with all members reset.

`delete` has no effect on whole mappings (as the keys of mappings may be arbitrary and are generally unknown). So if you delete a struct, it will reset all members that are not mappings and also recurse into the members unless they are mappings. However, individual keys and what they map to can be deleted.

It is important to note that `delete a` really behaves like an assignment to `a`, i.e. it stores a new object in `a`.
{% highlight javascript %}
contract DeleteExample {
  uint data;
  uint[] dataArray;
  function f() {
    uint x = data;
    delete x; // sets x to 0, does not affect data
    delete data; // sets data to 0, does not affect x which still holds a copy
    uint[] y = dataArray;
    delete dataArray; // this sets dataArray.length to zero, but as uint[] is a complex object, also
    // y is affected which is an alias to the storage object
    // On the other hand: "delete y" is not valid, as assignments to local variables
    // referencing storage objects can only be made from existing storage objects.
  }
}
{% endhighlight %}

## Conversions between Elementary Types

### Implicit Conversions

If an operator is applied to different types, the compiler tries to
implicitly convert one of the operands to the type of the other (the same is
true for assignments). In general, an implicit conversion between value-types
is possible if it
makes sense semantically and no information is lost: `uint8` is convertible to
`uint16` and `int128` to `int256`, but `int8` is not convertible to `uint256`
(because `uint256` cannot hold e.g. `-1`).
Furthermore, unsigned integers can be converted to bytes of the same or larger
size, but not vice-versa. Any type that can be converted to `uint160` can also
be converted to `address`.

### Explicit Conversions

If the compiler does not allow implicit conversion but you know what you are
doing, an explicit type conversion is sometimes possible:

{% highlight javascript %}
int8 y = -3;
uint x = uint(y);
{% endhighlight %}

At the end of this code snippet, `x` will have the value `0xfffff..fd` (64 hex
characters), which is -3 in two's complement representation of 256 bits.

If a type is explicitly converted to a smaller type, higher-order bits are
cut off:

{% highlight javascript %}
uint32 a = 0x12345678;
uint16 b = uint16(a); // b will be 0x5678 now
{% endhighlight %}

### Type Deduction

For convenience, it is not always necessary to explicitly specify the type of a
variable, the compiler automatically infers it from the type of the first
expression that is assigned to the variable:
{% highlight javascript %}
uint20 x = 0x123;
var y = x;
{% endhighlight %}
Here, the type of `y` will be `uint20`. Using `var` is not possible for function
parameters or return parameters.

Beware that currently, the type is only deduced from the first assignment, so
the loop in the following snippet is infinite, as `i` will have the type
`uint8` and any value of this type is smaller than `2000`.
{% highlight javascript %}
for (var i = 0; i < 2000; i++)
{
    // do something
}
{% endhighlight %}

## Functions on addresses

It is possible to query the balance of an address using the property `balance`
and to send Ether (in units of wei) to an address using the `send` function:

{% highlight javascript %}
address x = 0x123;
address myAddress = this;
if (x.balance < 10 && myAddress.balance >= 10) x.send(10);
{% endhighlight %}

Beware that if `x` is a contract address, its code (more specifically: its fallback function, if present) will be executed together with the `send` call (this is a limitation of the EVM and cannot be prevented). If that execution runs out of gas or fails in any way, the Ether transfer will be reverted. In this case, `send` returns `false`.

Furthermore, to interface with contracts that do not adhere to the ABI (like the classic NameReg contract),
the function `call` is provided which takes an arbitrary number of arguments of any type. These arguments are padded to 32 bytes and concatenated. One exception is the case where the first argument is encoded to exactly four bytes. In this case, it is not padded to allow the use of function signatures here.

{% highlight javascript %}
address nameReg = 0x72ba7d8e73fe8eb666ea66babc8116a41bfb10e2;
nameReg.call("register", "MyName");
nameReg.call(bytes4(sha3("fun(uint256)")), a);
{% endhighlight %}

`call` returns a boolean indicating whether the invoked function terminated (`true`) or caused an EVM exception (`false`). It is not possible to access the actual data returned (for this we would need to know the encoding and size in advance).

In a similar way, the function `callcode` can be used: The difference is that only the code of the given address is used, all other aspects (storage, balance, ...) are taken from the current contract. The purpose of `callcode` is to use library code which is stored in another contract. The user has to ensure that the layout of storage in both contracts is suitable for callcode to be used.

Both `call` and `callcode` are very low-level functions and should only be used as a *last resort* as they break the type-safety of Solidity.

Note that contracts inherit all members of address, so it is possible to query the balance of the
current contract using `this.balance`.

## Enums

Enums are one way to create a user-defined type in Solidity. They are explicitly convertible
to and from all integer types but implicit conversion is not allowed.

{% highlight javascript %}
contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, SitStill }
    ActionChoices choice;
    ActionChoices constant defaultChoice = ActionChoices.GoStraight;
    function setGoStraight()
    {
        choice = ActionChoices.GoStraight;
    }
    // Since enum types are not part of the ABI, the signature of "getChoice"
    // will automatically be changed to "getChoice() returns (uint8)"
    // for all matters external to Solidity. The integer type used is just
    // large enough to hold all enum values, i.e. if you have more values,
    // `uint16` will be used and so on.
    function getChoice() returns (ActionChoices)
    {
        return choice;
    }
    function getDefaultChoice() returns (uint)
    {
        return uint(defaultChoice);
    }
}
{% endhighlight %}
