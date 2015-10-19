---
layout: docs
title: Solidity Documentation
permalink: /docs/home/
---
See also [Russian version (русский перевод)](https://github.com/ethereum/wiki/wiki/%D0%A0%D1%83%D0%BA%D0%BE%D0%B2%D0%BE%D0%B4%D1%81%D1%82%D0%B2%D0%BE-%D0%BF%D0%BE-Solidity)  

***
Solidity is a high-level language whose syntax is similar to that of JavaScript and it is designed to compile to code for the Ethereum Virtual Machine. This
tutorial starts with a basic introduction to Solidity and assumes some knowledge of
the Ethereum Virtual Machine and programming in general. It tries to explain all features present in the language but does not cover features like
the [natural language specification](Ethereum-Natural-Specification-Format)
or formal verification and is also not meant as a final specification
of the language.

You can start using [Solidity in your browser](http://chriseth.github.io/cpp-ethereum),
with no need to download or compile anything. This application only supports
compilation - if you want to run the code or inject it into the blockchain, you
have to use a client like [Geth](https://github.com/ethereum/go-ethereum) or [AlethZero](https://github.com/ethereum/alethzero).

# Some Examples

Let us begin with some examples. It is fine if you do not understand everything
right now, we will go into more detail later.

## Storage

{% highlight javascript %}
contract SimpleStorage {
    uint storedData;
    function set(uint x) {
        storedData = x;
    }
    function get() constant returns (uint retVal) {
        return storedData;
    }
}
{% endhighlight %}

`uint storedData` declares a state variable called `storedData` of type `uint`
(unsigned integer of 256 bits) whose position in storage is automatically
allocated by the compiler. The functions `set` and `get` can be used to modify
or retrieve the value of the variable.

## Subcurrency Example

{% highlight javascript %}
contract Coin {
    address minter;
    mapping (address => uint) balances;

    event Send(address from, address to, uint value);

    function Coin() {
        minter = msg.sender;
    }
    function mint(address owner, uint amount) {
        if (msg.sender != minter) return;
        balances[owner] += amount;
    }
    function send(address receiver, uint amount) {
        if (balances[msg.sender] < amount) return;
        balances[msg.sender] -= amount;
        balances[receiver] += amount;
        Send(msg.sender, receiver, amount);
    }
    function queryBalance(address addr) constant returns (uint balance) {
        return balances[addr];
    }
}
{% endhighlight %}

This contract introduces some new concepts. One of them is the `address` type,
which is a 160 bit value that does not allow any arithmetic operations.
Furthermore, the state variable `balances` is of a complex datatype that maps
addresses to unsigned integers. Mappings can be seen as hashtables which are
virtually initialized such that every possible key exists and is mapped to a
value whose byte-representation is all zeros. The special function `Coin` is the
constructor which is run during creation of the contract and
cannot be called afterwards. It permanently stores the address of the person creating the
contract: Together with `tx` and `block`, `msg` is a magic global variable that
contains some properties which allow access to the world outside of the contract.
The function `queryBalance` is declared `constant` and thus is not allowed to
modify the state of the contract (note that this is not yet enforced, though).
In Solidity, return "parameters" are named and essentially create a local
variable. So to return the balance, we could also just use `balance =
balances[addr];` without any return statement.
Events like `Send` allow external clients to search the blockchain more efficiently.
If an event is invoked like in the function `send`, this fact is permanently stored in the blockchain, but more on this later.




