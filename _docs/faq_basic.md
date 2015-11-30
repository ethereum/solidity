---
layout: docs
title: Frequently Asked Questions (Basics)
link_title: Basics
permalink: /docs/faq/basics/
---

This list was originally compiled by [fivedogit](mailto:fivedogit@gmail.com).

### What is Solidity?

Solidity is the DEV-created (i.e. Ethereum Foundation-created),
Javascript-inspired language that can be used to create smart contracts
on the Ethereum blockchain. There are other
languages you can use as well (LLL, Serpent, etc). The main points in
favour of Solidity is that it is statically typed and offers many
advanced features like inheritance, libraries, complex
user-defined types and a bytecode optimizer.

Solidity contracts can be compiled a few different ways (see below) and the
resulting output can be cut/pasted into a geth console to deploy them to the
Ethereum blockchain.

There are some [contract examples](https://github.com/fivedogit/solidity-baby-steps/tree/master/contracts/) by fivedogit and 
there should be a [test contract](https://github.com/ethereum/solidity/blob/develop/test/libsolidity/SolidityEndToEndTest.cpp) for every single feature of Solidity.

### How do I compile contracts?

Probably the fastest way is the [online compiler](https://chriseth.github.io/browser-solidity/).

You can also use the `solc` binary which comes with cpp-ethereum to compile
contracts or an emerging option is to use Mix, the IDE.


### Create and publish the most basic contract possible

A quite simple contract is the [greeter](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/05_greeter.sol).

### Is it possible to do something on a specific block number? (e.g. publish a contract or execute a transaction)

Transactions are not guaranteed to happen on the next block or any future
specific block, since it is up to the miners to include transactions and not up
to the submitter of the transaction. This applies to function calls/transactions and contract
creation transactions.

If you want to schedule future calls of your contract, you can use the
[alarm clock](http://www.ethereum-alarm-clock.com/).

### What is the transaction "payload"?

This is just the bytecode "data" sent along with the request.

### Is there a decompiler available?

There is no decompiler to Solidity. This is in principle possible
to some degree, but for example variable names will be lost and
great effort will be necessary to make it look similar to
the original source code.

Bytecode can be decompiled to opcodes, a service that is provided by
several blockchain explorers.

Contracts on the blockchain should have their original source
code published if they are to be used by third parties.

### Does selfdestruct() free up space in the blockchain?

It removes the contract bytecode and storage from the current block
into the future, but since the blockchain stores every single block (i.e.
all history), this will not actually free up space on full/achive nodes.

### Create a contract that can be killed and return funds

First, a word of warning: Killing contracts sounds like a good idea, because "cleaning up"
is always good, but as seen above, it does not really clean up. Furthermore,
if Ether is sent to removed contracts, the Ether will be forever lost.

If you want to deactivate your contracts, rather **disable** them by changing some
internal state which causes all functions to throw. This will make it impossible
to use the contract and ether sent to the contract will be returned automatically.

Now to answering the question: Inside a constructor, `msg.sender` is the
creator. Save it. Then `selfdestruct(creator);` to kill and return funds.

[example](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/05_greeter.sol)

Note that if you `import "mortal"` at the top of your contracts and declare
`contract SomeContract is mortal { ...` and compile with a compiler that already
has it (which includes [browser-solidity](https://chriseth.github.io/browser-solidity/)), then
`kill()` is taken care of for you. Once a contract is "mortal", then you can
`contractname.kill.sendTransaction({from:eth.coinbase})`, just the same as my
examples.

### Store Ether in a contract

The trick is to create the contract with `{from:someaddress, value: web3.toWei(3,"ether")...}`

See [endowment_retriever.sol](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/30_endowment_retriever.sol).

### Use a non-constant function (req sendTransaction) to increment a variable in a contract

See [value_incrementer.sol](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/20_value_incrementer.sol).

### Get contract address in Solidity

Short answer: The global variable `this` is the contract address.

See [basic_info_getter](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/15_basic_info_getter.sol).

Long answer: `this` is a variable representing the current contract.
Its type is the type of the contract. Since any contract type basically inherits from the
`address` type, `this` is always convertible to `address` and in this case contains
its own address.

### What is the difference between a function marked constant and one that is not?

`constant` functions can perform some action and return a value, but cannot
change state (this is not yet enforced by the compiler). In other words, a
constant function cannot save or update any variables within the contract or wider
blockchain. These functions are called using `c.someFunction(...)` from
geth or any other web3.js environment.

"non-constant" functions (those lacking the `constant` specifier) must be called
with `c.someMethod.sendTransaction({from:eth.accounts[x], gas: 1000000});`
That is, because they can change state, they have to have a gas
payment sent along to get the work done.

### Get a contract to return its funds to you (not using selfdestruct(...)). 

This example demonstrates how to send funds from a contract to an address. 

See [endowment_retriever](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/30_endowment_retriever.sol).

### What is a mapping and how do we use them?

A mapping is very similar to a K->V hashmap.
If you have a state variable of type `mapping (string -> uint) x;`, then you can
access the value by `x["somekeystring"]`.

### How can I get the length of a mapping?

Mappings are a rather low-level data structure. It does not store the keys
and it is not possible to know which or how many values are "set". Actually,
all values to all possible keys are set by default, they are just
initialised with the zero value.

In this sense, the attribute `length` for a mapping does not really apply.

If you want to have a "sized mapping", you can use the iterable mapping
(see below) or just a dynamically-sized array of structs.

### Are mappings iterable?

Mappings themselves are not iterable, but you can use a higher-level
datastructure on top of it, for example the [iterable mapping](https://github.com/ethereum/dapp-bin/blob/master/library/iterable_mapping.sol).

### Can you return an array or a string from a solidity function call?

Yes. See [array_receiver_and_returner.sol](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/60_array_receiver_and_returner.sol).

What is problematic, though, is returning any variably-sized data (e.g. a
        variably-sized array like `uint[]`) from a fuction **called from within Solidity**.
This is a limitation of the EVM and will be solved with the next protocol update.

Returning variably-sized data as part of an external transaction or call is fine.

### How do you represent double/float in Solidity?

This is not yet possible.

### Is it possible to in-line initialize an array like so: string32[] myarray = ["a", "b"];

This is not yet possible.

### What are events and why do we need them?

Let us suppose that you need a contract to alert the outside world when
something happens. The contract can fire an event, which can be listened to
from web3 (inside geth or a web application). The main advantage of events
is that they are stored in a special way on the blockchain so that it
is very easy to search for them.

### What are the different function visibilities?

The visibility specifiers do not only change the visibility but also
the way functions can be called. In general, functions in the
same contract can also be called internally (which is cheaper
        and allows for memory types to be passed by reference). This
is done if you just use `f(1,2)`. If you use `this.f(1,2)`
or `otherContract.f(1,2)`, the function is called externally.

Internal function calls have the advantage that you can use
all Solidity types as parameters, but you have to stick to the
simpler ABI types for external calls.

* external: all, only externally

* public: all (this is the default), externally and internally

* internal: only this contract and contracts deriving from it, only internally

* private: only this contract, only internally


### Do contract constructors have to be publicly visible?

You can use the visibility specifiers, but they do not yet have any effect.
The constructor is removed from the contract code once it is deployed,

### Can a contract have multiple constructors?

No, a contract can have only one constructor.

More specifically, it can only have one function whose name matches
that of the constructor.

Having multiple constructors with different number of arguments
or argument types, as it is possible in other languages
is not allowed in Solidity.

### Is a constructor required?

No. If there is no constructor, a generic one without arguments and no actions will be used.

### Are timestamps (now, block.timestamp) reliable? 

This depends on what you mean by "reliable".
In general, they are supplied by miners and are therefore vulnerable.

Unless someone really messes up the blockchain or the clock on
your computer, you can make the following assumptions:

You publish a transaction at a time X, this transaction contains same
code that calls `now` and is included in a block whose timestamp is Y
and this block is included into the canonical chain (published) at a time Z.

The value of `now` will be identical to Y and X <= Y <= Z.

Never use `now` or `block.hash` as a source of randomness, unless you know
what you are doing!

### Can a contract function return a struct?

Yes, but only in "internal" function calls.

### If I return an enum, I only get integer values in web3.js. How to get the named values?

Enums are not supported by the ABI, they are just supported by Solidity.
You have to do the mapping yourself for now, we might provide some help
later.

### What is the deal with "function () { ... }" inside Solidity contracts? How can a function not have a name?

This function is called "fallback function" and it
is called when someone just sent Ether to the contract without
providing any data or if someone messed up the types so that they tried to
call a function that does not exist.

The default behaviour (if no fallback function is explicitly given) in
these situations is to just accept the call and do nothing.
This is desireable in many cases, but should only be used if there is
a way to pull out Ether from a contract.

If the contract is not meant to receive Ether with simple transfers, you
should implement the fallback function as

`function() { throw; }`

this will cause all transactions to this contract that do not call an
existing function to be reverted, so that all Ether is sent back. 

Another use of the fallback function is to e.g. register that your
contract received ether by using an event.

*Attention*: If you implement the fallback function take care that it uses as
little gas as possible, because `send()` will only supply a limited amount.

### Is it possible to pass arguments to the fallback function?

The fallback function cannot take parameters.

Under special circumstances, you can send data. If you take care
that none of the other functions is invoked, you can access the data
by `msg.data`.

### Can state variables be initialized in-line?

Yes, this is possible for most types (even for structs), but not for arrays.

Examples:
{% highlight Javascript %}
contract C {
    struct S { uint a; uint b; }
    S public x = S(1, 2);
    string name = "Ada";
}
contract D {
    C c = new C();
}
{% endhighlight %}

### What is the "modifier" keyword?

Modifiers are a way to prepend or append code to a function in order
to add guards, initialisation or cleanup functionality in a concise way.

For examples, see the [features.sol](https://github.com/ethereum/dapp-bin/blob/master/library/features.sol).

### How do structs work?

See [struct_and_for_loop_tester.sol](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/65_struct_and_for_loop_tester.sol).

### How do for loops work?

Very similar to JavaScript. There is one point to watch out for, though:

If you use `for (var i = 0; i < a.length; i ++) { a[i] = i; }`, then
the type of `i` will be inferred only from `0`, whose type is `uint8`.
This means that if `a` has more than `255` elements, your loop will
not terminate because `i` can only hold values up to `255`.

Better use `for (uint i = 0; i < a.length...`

See [struct_and_for_loop_tester.sol](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/65_struct_and_for_loop_tester.sol).

### What character set does Solidity use?

Solidity is character set agnostic concerning strings in the source code, although
utf-8 is recommended. Identifiers (variables, functions, ...) can only use
ASCII.

### What are some examples of basic string manipulation (substring, indexOf, charAt, etc)?

There are some string utility functions at [stringUtils.sol](https://github.com/ethereum/dapp-bin/blob/master/library/stringUtils.sol)
which will be extended in the future.

For now, if you want to modify a string (even when you only want to know its length),
you should always convert it to a `bytes` first:

{% highlight JavaScript %}
contract C {
    string s;
    function append(byte c) {
        bytes(s).push(c);
    }
    function set(uint i, byte c) {
        bytes(s)[i] = c;
    }
}
{% endhighlight %}


### Can I concatenate two strings?

You have to do it manually for now.

### Why is the low-level function .call() less favorable than instantiating a contract with a variable (ContractB b;) and executing its functions (b.doSomething();)?

If you use actual functions, the compiler will tell you if the types
or your arguments do not match, if the function does not exist
or is not visible and it will do the packing of the
arguments for you.

See [ping.sol](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/45_ping.sol) and
[pong.sol](https://github.com/fivedogit/solidity-baby-steps/blob/master/contracts/45_pong.sol).

### Is unused gas automatically refunded?

Yes and it is immediate, i.e. done as part of the transaction.

### When returning a value of say "uint" type, is it possible to return an "undefined" or "null"-like value?

This is not possible, because all types use up the full value range.

You have the option to `throw` on error, which will also revert the whole
transaction, which might be a good idea if you ran into an unexpected
situation.

If you do not want to throw, you can return a pair:
{% highlight JavaScript %}
contract C {
    uint[] counters;
    function getCounter(uint index)
        returns (uint counter, bool error) {
            if (index >= counters.length) return (0, true);
            else return (counters[index], false);
        }
    function checkCounter(uint index) {
        var (counter, error) = getCounter(index);
        if (error) { ... }
        else { ... }
    }
}
{% endhighlight %}


### Are comments included with deployed contracts and do they increase deployment gas?

No, everything that is not needed for execution is removed during compilation.
This includes, among others, comments, variable names and type names.

### What happens if you send ether along with a function call to a contract?

It gets added to the total balance of the contract, just like when you send ether when creating a contract.

### Is it possible to get a tx receipt for a transaction executed contract-to-contract?

No, a function call from one contract to another does not create its own transaction,
you have to look in the overall transaction. This is also the reason why several
block explorer do not show Ether sent between contracts correctly.

### What is the memory keyword? What does it do?

The Ethereum Virtual Machine has three areas where it can store items.

The first is "storage", where all the contract state variables reside.
Every contract has its own storage and it is persistent between function calls
and quite expensive to use.

The second is "memory", this is used to hold temporary values. It
is erased between (external) function calls and is cheaper to use.

The third one is the stack, which is used to hold small local variables.
It is almost free to use, but can only hold a limited amount of values.

For almost all types, you cannot specify where they should be stored, because
they are copied everytime they are used.

The types where the so-called storage location is important are structs
and arrays. If you e.g. pass such variables in function calls, their
data is not copied if it can stay in memory or stay in storage.
This means that you can modify their content in the called function
and these modifications will still be visible in the caller.

There are defaults for the storage location depending on which type
of variable it concerns:

* state variables are always in storage
* function arguments are always in memory
* local variables always reference storage

Example:

{% highlight JavaScript %}
contract C {
    uint[] data1;
    uint[] data2;
    function appendOne() {
        append(data1);
    }
    function appendTwo() {
        append(data2);
    }
    function append(uint[] storage d) {
        d.push(1);
    }
}
{% endhighlight %}
The function `append` can work both on `data1` and `data2` and its modifications will be
stored permanently. If you remove the `storage` keyword, the default
is to use `memory` for function arguments. This has the effect that
at the point where `append(data1)` or `append(data2)` is called, an
independent copy of the state variable is created in memory and
`append` operates on this copy (which does not support `.push` - but that
        is another issue). The modifications to this independent copy do not
carry back to `data1` or `data2`.

A common mistake is to declare a local variable and assume that it will
be created in memory, although it will be created in storage:
{% highlight JavaScript %}
/// THIS CONTRACT CONTAINS AN ERROR
contract C {
    uint someVariable;
    uint[] data;
    function f() {
        uint[] x;
        x.push(2);
        data = x;
    }
}
{% endhighlight %}
The type of the local variable `x` is `uint[] storage`, but since
storage is not dynamically allocated, it has to be assigned from
a state variable before it can be used. So no space in storage will be
allocated for `x`, but instead it functions only as an alias for
a pre-existing variable in storage.

What will happen is that the compiler interprets `x` as a storage
pointer and will make it point to the storage slot `0` by default.
This has the effect that `someVariable` (which resides at storage
slot `0`) is modified by `x.push(2)`.

The correct way to do this is the following:
{% highlight JavaScript %}
contract C {
    uint someVariable;
    uint[] data;
    function f() {
        uint[] x = data;
        x.push(2);
    }
}
{% endhighlight %}

### Can a regular (i.e. non-contract) ethereum account be closed permanently like a contract can?

No. Non-contract accounts "exist" as long as the private key is known by
someone or can be generated in some way.
