---
layout: docs
title: A Simple Smart Contract
link_title: A Simple Contract
permalink: /docs/simple-smart-contract/
---

Let us begin with the most basic example. It is fine if you do not understand everything
right now, we will go into more detail later.

## Storage

{% include open_link gist="a4532ce30246847b371b" %}
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

A contract in the sense of Solidity is a collection of code (its functions) and
data (its *state*) that resides at a specific address on the Ethereum
blockchain. The line `uint storedData;` declares a state variable called `storedData` of
type `uint` (unsigned integer of 256 bits). You can think of it as a single slot
in a database that can be queried and altered by calling functions of the
code that manages the database. In the case of Ethereum, this is always the owning
contract. And in this case, the functions `set` and `get` can be used to modify
or retrieve the value of the variable.

To access a state variable, you do not need the prefix `this.` as is common in
other languages.

This contract does not yet do much apart from (due to the infrastructure
built by Ethereum) allowing anyone to store a single number that is accessible by
anyone in the world without (feasible) a way to prevent you from publishing
this number. Of course, anyone could just call `set` again with a different value
and overwrite your number, but the number will still be stored in the history
of the blockchain. Later, we will see how you can impose access restrictions
so that only you can alter the number.

## Subcurrency Example

The following contract will implement the simplest form of a
cryptocurrency. It is possible to generate coins out of thin air, but
only the person that created the contract will be able to do that (it is trivial
to implement a different issuance scheme).
Furthermore, anyone can send coins to each other without any need for
registering with username and password - all you need is an Ethereum keypair.


<div class="note info">
<h5>Not a nice example for browser-solidity</h5>
<p>If you use <a href="https://chriseth.github.io/browser-solidity">browser-solidity</a>
to try this example, you cannot change the address where you call
functions from. So you will always be the "minter", you can mint coins and send
them somewhere, but you cannot impersonate someone else. This might change in
the future.</p>
</div>

{% include open_link gist="ad490694f3e5b3de47ab" %}
{% highlight javascript %}
contract Coin {
    // The keyword "public" makes those variables
    // readable from outside.
    address public minter;
    mapping (address => uint) public balances;

    // Events allow light clients to react on
    // changes efficiently.
    event Sent(address from, address to, uint amount);

    // This is the constructor whose code is
    // run only when the contract is created.
    function Coin() {
        minter = msg.sender;
    }
    function mint(address receiver, uint amount) {
        if (msg.sender != minter) return;
        balances[receiver] += amount;
    }
    function send(address receiver, uint amount) {
        if (balances[msg.sender] < amount) return;
        balances[msg.sender] -= amount;
        balances[receiver] += amount;
        Sent(msg.sender, receiver, amount);
    }
}
{% endhighlight %}

This contract introduces some new concepts, let us go through them one by one

The line `address public minter;` declares a state variable of type address
that is publicly accessible. The `address` type is a 160 bit value
that does not allow any arithmetic operations. It is suitable for
storing addresses of contracts or keypairs belonging to external
persons. The keyword `public` automatically generates a function that
allows you to access the current value of the state variable.
Without this keyword, other contracts have no way to access the variable
and only the code of this contract can write to it.
The function will look something like this
{% highlight javascript %}
function minter() returns (address) { return minter; }
{% endhighlight %}
Of course, adding a function exactly like that will not work
because we would have a
function and a state variable with the same name, but hopefully, you
get the idea - the compiler figures that out for you.

The next line, `mapping (address => uint) public balances;` also
creates a public state variable, but it of a more complex datatype.
The type maps addresses to unsigned integers.
Mappings can be seen as hashtables which are
virtually initialized such that every possible key exists and is mapped to a
value whose byte-representation is all zeros. This analogy does not go
too far, though, as it is neither possible to obtain a list of all keys of
a mapping, nor a list of all values. So either keep in mind (or
better, keep a list or use a more advanced data type) what you
added to the mapping or use it in a context where this is not needed,
like this one. The accessor function created by the `public` keyword
is a bit more complex in this case. It roughly looks like the
following:
{% highlight javascript %}
function balances(address _account) returns (uint balance) {
    return balances[_account];
}
{% endhighlight %}
As you see, you can use this function to easily query the balance of a
single account.

The line `event Sent(address from, address to, uint value);` declares
a so-called "event" which is fired in the last line of the function
`send`. User interfaces (as well as server appliances of course) can
listen for those events being fired on the blockchain without much
cost. As soon as it is fired, the listener will also receive the
arguments `from`, `to` and `value`, which makes it easy to track
transactions. In order to listen for this event, you would use
{% highlight javascript %}
Coin.Sent().watch({}, '', function(error, result) {
    if (!error) {
        console.log("Coin transfer: " + result.args.amount +
            " coins were sent from " + result.args.from +
            " to " + result.args.to + ".");
        console.log("Balances now:\n" +
            "Sender: " + Coin.balances.call(result.args.from) +
            "Receiver: " + Coin.balances.call(result.args.to));
    }
}
{% endhighlight %}
Note how the automatically generated function `balances` is called from
the user interface.

The special function `Coin` is the
constructor which is run during creation of the contract and
cannot be called afterwards. It permanently stores the address of the person creating the
contract: `msg` (together with `tx` and `block`) is a magic global variable that
contains some properties which allow access to the blockchain. `msg.sender` is
always the address where the current (external) function call came from.

Finally, the functions that will actually end up with the contract and can be called
by users and contracts alike are `mint` and `send`.
If `mint` is called by anyone except the account that created the contract,
nothing will happen. On the other hand, `send` can be used by anyone (who already
has some of these coins) to send coins to anyone else. Note that if you use
this contract to send coins to an address, you will not see anything when you
look at that address on a blockchain explorer, because the fact that you sent
coins and the changed balances are only stored in the data storage of this
particular coin contract. By the use of events it is relatively easy to create
a "blockchain explorer" that tracks transactions and balances of your new coin.

