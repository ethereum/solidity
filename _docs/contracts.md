---
layout: docs
title: Contracts
permalink: /docs/contracts/
---
# Contracts

## Interfacing with other Contracts

There are two ways to interface with other contracts: Either call a method of a contract whose address is known or create a new contract. Both uses are shown in the example below. Note that (obviously) the source code of a contract to be created needs to be known, which means that it has to come before the contract that creates it (and cyclic dependencies are not possible since the bytecode of the new contract is actually contained in the bytecode of the creating contract).

{% highlight javascript %}
contract OwnedToken {
  // TokenCreator is a contract type that is defined below. It is fine to reference it
  // as long as it is not used to create a new contract.
  TokenCreator creator;
  address owner;
  bytes32 name;
  function OwnedToken(bytes32 _name) {
    address nameReg = 0x72ba7d8e73fe8eb666ea66babc8116a41bfb10e2;
    nameReg.call("register", _name); // This is an unsafe raw call to another contract.
    owner = msg.sender;
    // We do an explicit type conversion from `address` to `TokenCreator` and assume that the type of
    // the calling contract is TokenCreator, there is no real way to check.
    creator = TokenCreator(msg.sender);
    name = _name;
  }
  function changeName(bytes32 newName) {
    // Only the creator can alter the name -- contracts are implicitly convertible to addresses.
    if (msg.sender == creator) name = newName;
  }
  function transfer(address newOwner) {
    // Only the current owner can transfer the token.
    if (msg.sender != owner) return;
    // We also want to ask the creator if the transfer is fine.
    // Note that this calls a function of the contract defined below.
    // If the call fails (e.g. due to out-of-gas), the execution here stops
    // immediately (the ability to catch this will be added later).
    if (creator.isTokenTransferOK(owner, newOwner))
      owner = newOwner;
  }
}
contract TokenCreator {
  function createToken(bytes32 name) returns (OwnedToken tokenAddress) {
    // Create a new Token contract and return its address.
    // From the JavaScript side, the return type is simply "address", as this is the closest
    // type available in the ABI.
    // To get the address, this can only be called by another contract (not eth_call).
    return new OwnedToken(name);
  }
  function changeName(OwnedToken tokenAddress, bytes32 name) {
    // Again, the external type of "tokenAddress" is simply "address".
    tokenAddress.changeName(name);
  }
  function isTokenTransferOK(address currentOwner, address newOwner) returns (bool ok) {
    // Check some arbitrary condition.
    address tokenAddress = msg.sender;
    return (sha3(newOwner) & 0xff) == (bytes20(tokenAddress) & 0xff);
  }
}
{% endhighlight %}

## Libraries

Libraries are similar to contracts, but their purpose is that they are deployed only once at a specific address and their code is reused using the `CALLCODE` feature of the EVM. This means that if library functions are called, their code is executed in the context of the calling contract, i.e. `this` points to the calling contract and especially the storage from the calling contract can be accessed. As a library is an isolated piece of source code, it can only access state variables of the calling contract if they are explicitly supplied (it would have to way to name them, otherwise).

The following example illustrates how to use libraries.
{% highlight javascript %}
library Set {
  // We define a new struct datatype that will be used to hold its data in the calling contract.
  struct Data { mapping(uint => bool) flags; }
  // Note that the first parameter is of type "storage reference" and
  // thus only its storage address and not its contents is passed as part of the call.
  // This is a special feature of library functions.
  // It is idiomatic to call the first parameter 'self', if the function can be seen
  // as a method of that object.
  function insert(Data storage self, uint value) returns (bool) {
    if (self.flags[value])
      return false; // already there
    self.flags[value] = true;
    return true;
  }
  function remove(Data storage self, uint value) returns (bool) {
    if (!self.flags[value])
      return false; // not there
    self.flags[value] = false;
    return true;
  }
  function contains(Data storage self, uint value) returns (bool) {
    return self.flags[value];
  }
}
contract C {
  Set.Data knownValues;
  function register(uint value) {
    // The library functions can be called without a specific instance of the library,
    // since the "instance" will be the current contract.
    if (!Set.insert(knownValues, value))
      throw;
  }
  // In this contract, we can also directly access knownValues.flags, if we want.
}
{% endhighlight %}
Of course, you do not have to follow this way to use libraries - they can also be used without defining struct data types, functions also work without any storage reference parameters, can have multiple storage reference parameters and in any position.

The calls to `Set.contains`, `Set.insert` and `Set.remove` are all compiled as calls (`CALLCODE`s) to an external contract/library. If you use libraries, take care that an actual external function call is performed, so `msg.sender` does not point to the original sender anymore but to the the calling contract and also `msg.value` contains the funds sent during the call to the library function.

As the compiler cannot know where the library will be deployed at, these addresses have to be filled into the final bytecode by a linker (see [Using the Commandline Compiler](#using-the-commandline-compiler) on how to use the commandline compiler for linking). If the addresses are not given as arguments to the compiler, the compiled hex code will contain placeholders of the form `__Set______` (where `Set` is the name of the library). The address can be filled manually by replacing all those 40 symbols by the hex encoding of the address of the library contract.

Restrictions for libraries in comparison to contracts:

 - no state variables
 - cannot inherit nor be inherited

(these might be lifted at a later point)

## Constructor Arguments

A Solidity contract expects constructor arguments after the end of the contract data itself.
This means that you pass the arguments to a contract by putting them after the
compiled bytes as returned by the compiler in the usual ABI format.

If you use web3.js's `MyContract.new()`, you do not have to care about this, though.

## Contract Inheritance

Solidity supports multiple inheritance by copying code including polymorphism.
Details are given in the following example.

{% highlight javascript %}
contract owned {
    function owned() { owner = msg.sender; }
    address owner;
}

// Use "is" to derive from another contract. Derived contracts can access all non-private members
// including internal functions and state variables. These cannot be accessed externally via
// `this`, though.
contract mortal is owned {
    function kill() { if (msg.sender == owner) suicide(owner); }
}

// These are only provided to make the interface known to the compiler.
// Note the bodiless functions. If a contract does not implement all functions
// it can only be used as an interface.
contract Config { function lookup(uint id) returns (address adr); }
contract NameReg { function register(bytes32 name); function unregister(); }

// Multiple inheritance is possible. Note that "owned" is also a base class of
// "mortal", yet there is only a single instance of "owned" (as for virtual
// inheritance in C++).
contract named is owned, mortal {
    function named(bytes32 name) {
        address ConfigAddress = 0xd5f9d8d94886e70b06e474c3fb14fd43e2f23970;
        NameReg(Config(ConfigAddress).lookup(1)).register(name);
    }

// Functions can be overridden, both local and message-based function calls take
// these overrides into account.
    function kill() {
        if (msg.sender == owner) {
            address ConfigAddress = 0xd5f9d8d94886e70b06e474c3fb14fd43e2f23970;
            NameReg(Config(ConfigAddress).lookup(1)).unregister();
// It is still possible to call a specific overridden function.
            mortal.kill();
        }
    }
}

// If a constructor takes an argument, it needs to be provided in the header (or modifier-invocation-style at the constructor of the derived contract (see below)).
contract PriceFeed is owned, mortal, named("GoldFeed") {
   function updateInfo(uint newInfo) {
      if (msg.sender == owner) info = newInfo;
   }

   function get() constant returns(uint r) { return info; }

   uint info;
}
{% endhighlight %}

Note that above, we call `mortal.kill()` to "forward" the destruction request. The way this is done
is problematic, as seen in the following example:
{% highlight javascript %}
contract mortal is owned {
    function kill() { if (msg.sender == owner) suicide(owner); }
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

A call to `Final.kill()` will call `Base2.kill` as the most derived override, but this
function will bypass `Base1.kill`, basically because it does not even know about `Base1`.
The way around this is to use `super`:
{% highlight javascript %}
contract mortal is owned {
    function kill() { if (msg.sender == owner) suicide(owner); }
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

If `Base1` calls a function of `super`, it does not simply call this function on one of its
base contracts, it rather calls this function on the next base contract in the final
inheritance graph, so it will call `Base2.kill()` (note that the final inheritance sequence is
-- starting with the most derived contract: Final, Base1, Base2, mortal, owned). Note that the actual function that
is called when using super is not known in the context of the class where it is used,
although its type is known. This is similar for ordinary virtual method lookup.

### Arguments for Base Constructors

Derived contracts need to provide all arguments needed for the base constructors. This can be done at two places:

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

Either directly in the inheritance list (`is Base(7)`) or in the way a modifier would be invoked as part of the header of the derived constructor (`Base(_y * _y)`). The first way to do it is more convenient if the constructor argument is a constant and defines the behaviour of the contract or describes it. The second way has to be used if the constructor arguments of the base depend on those of the derived contract. If, as in this silly example, both places are used, the modifier-style argument takes precedence.


### Multiple Inheritance and Linearization

Languages that allow multiple inheritance have to deal with several problems, one of them being the [Diamond Problem](https://en.wikipedia.org/wiki/Multiple_inheritance#The_diamond_problem). Solidity follows the path of Python and uses "[C3 Linearization](https://en.wikipedia.org/wiki/C3_linearization)" to force a specific order in the DAG of base classes. This results in the desirable property of monotonicity but disallows some inheritance graphs. Especially, the order in which the base classes are given in the `is` directive is important. In the following code, Solidity will give the error "Linearization of inheritance graph impossible".
{% highlight javascript %}
contract X {}
contract A is X {}
contract C is A, X {}
{% endhighlight %}
The reason for this is that `C` requests `X` to override `A` (by specifying `A, X` in this order), but `A` itself requests to override `X`, which is a contradiction that cannot be resolved.

A simple rule to remember is to specify the base classes in the order from "most base-like" to "most derived".

## Abstract Contracts

Contract functions can lack an implementation as in the following example (note that the function declaration header is terminated by `;`).
{% highlight javascript %}
contract feline {
  function utterance() returns (bytes32);
}
{% endhighlight %}
Such contracts cannot be compiled (even if they contain implemented functions alongside non-implemented functions), but they can be used as base contracts:
{% highlight javascript %}
contract Cat is feline {
  function utterance() returns (bytes32) { return "miaow"; }
}
{% endhighlight %}
If a contract inherits from an abstract contract and does not implement all non-implemented functions by overriding, it will itself be abstract.

## Visibility Specifiers

Functions and state variables can be specified as being `public`, `internal` or `private`, where the default for functions is `public` and `internal` for state variables. In addition, functions can also be specified as `external`.

`external`: External functions are part of the contract interface and they can be called from other contracts and via transactions. An external function `f` cannot be called internally (i.e. `f()` does not work, but `this.f()` works). Furthermore, all function parameters are immutable.

`public`: Public functions are part of the contract interface and can be either called internally or via messages. For public state variables, an automatic accessor function (see below) is generated.

`internal`: Those functions and state variables can only be accessed internally, i.e. from within the current contract or contracts deriving from it without using `this`.

`private`: Private functions and state variables are only visible for the contract they are defined in and not in derived contracts.

{% highlight javascript %}
contract c {
  function f(uint a) private returns (uint b) { return a + 1; }
  function setData(uint a) internal { data = a; }
  uint public data;
}
{% endhighlight %}

Other contracts can call `c.data()` to retrieve the value of data in state storage, but are not able to call `f`. Contracts derived from `c` can call `setData` to alter the value of `data` (but only in their own state).

## Accessor Functions

The compiler automatically creates accessor functions for all public state variables. The contract given below will have a function called `data` that does not take any arguments and returns a uint, the value of the state variable `data`. The initialization of state variables can be done at declaration.

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

Note that the mapping in the struct is omitted because there is no good way to provide the key for the mapping.

## Fallback Functions

A contract can have exactly one unnamed
function. This function cannot have arguments and is executed on a call to the contract if
none of the other functions matches the given function identifier (or if no data was supplied at all).

{% highlight javascript %}
contract Test {
  function() { x = 1; }
  uint x;
}

contract Caller {
  function callTest(address testAddress) {
    Test(testAddress).call(0xabcdef01); // hash does not exist
    // results in Test(testAddress).x becoming == 1.
  }
}
{% endhighlight %}

## Function Modifiers

Modifiers can be used to easily change the behaviour of functions, for example to automatically check a condition prior to executing the function. They are inheritable properties of contracts and may be overridden by derived contracts.

{% highlight javascript %}
contract owned {
  function owned() { owner = msg.sender; }
  address owner;

  // This contract only defines a modifier but does not use it - it will
  // be used in derived contracts.
  // The function body is inserted where the special symbol "_" in the
  // definition of a modifier appears.
  modifier onlyowner { if (msg.sender == owner) _ }
}
contract mortal is owned {
  // This contract inherits the "onlyowner"-modifier from "owned" and
  // applies it to the "kill"-function, which causes that calls to "kill"
  // only have an effect if they are made by the stored owner.
  function kill() onlyowner {
    suicide(owner);
  }
}
contract priced {
  // Modifiers can receive arguments:
  modifier costs(uint price) { if (msg.value >= price) _ }
}
contract Register is priced, owned {
  mapping (address => bool) registeredAddresses;
  uint price;
  function Register(uint initialPrice) { price = initialPrice; }
  function register() costs(price) {
    registeredAddresses[msg.sender] = true;
  }
  function changePrice(uint _price) onlyowner {
    price = _price;
  }
}
{% endhighlight %}

Multiple modifiers can be applied to a function by specifying them in a whitespace-separated list and will be evaluated in order. Explicit returns from a modifier or function body immediately leave the whole function, while control flow reaching the end of a function or modifier body continues after the "_" in the preceding modifier. Arbitrary expressions are allowed for modifier arguments and in this context, all symbols visible from the function are visible in the modifier. Symbols introduced in the modifier are not visible in the function (as they might change by overriding).

## Constants

State variables of value type can be declared as constant.
{% highlight javascript %}
contract C {
  uint constant x = 32;
  bytes3 constant text = "abc";
}
{% endhighlight %}

This has the effect that the compiler does not reserve a storage slot
for these variables and every occurrence is replaced by their constant value.

## Events

Events allow the convenient usage of the EVM logging facilities. Events are inheritable members of contracts. When they are called, they cause the arguments to be stored in the transaction's log. Up to three parameters can receive the attribute `indexed` which will cause the respective arguments to be treated as log topics instead of data. The hash of the signature of the event is one of the topics except if you declared the event with `anonymous` specifier. All non-indexed arguments will be stored in the data part of the log. Example:

{% highlight javascript %}
contract ClientReceipt {
  event Deposit(address indexed _from, bytes32 indexed _id, uint _value);
  function deposit(bytes32 _id) {
    Deposit(msg.sender, _id, msg.value);
  }
}
{% endhighlight %}
Here, the call to `Deposit` will behave identical to
`log3(msg.value, 0x50cb9fe53daa9737b786ab3646f04d0150dc50ef4e75f59509d83667ad5adb20, sha3(msg.sender), _id);`. Note that the large hex number is equal to the sha3-hash of "Deposit(address,bytes32,uint256)", the event's signature.

### Additional Resources for Understanding Events:

- Javascript documentation: <https://github.com/ethereum/wiki/wiki/JavaScript-API#contract-events>
- Example usage of events: <https://github.com/debris/smart-exchange/blob/master/lib/contracts/SmartExchange.sol>
- How to access them in js: <https://github.com/debris/smart-exchange/blob/master/lib/exchange_transactions.js>

