.. index:: ! contract

##########
Contracts
##########

Contracts in Solidity are similar to classes in object-oriented languages. They
contain persistent data in state variables and functions that can modify these
variables. Calling a function on a different contract (instance) will perform
an EVM function call and thus switch the context such that state variables are
inaccessible.

.. index:: ! contract;creation, constructor

******************
Creating Contracts
******************

Contracts can be created "from outside" via Ethereum transactions or from within Solidity contracts.

IDEs, such as `Remix <https://remix.ethereum.org/>`_, make the creation process seamless using UI elements.

Creating contracts programatically on Ethereum is best done via using the JavaScript API `web3.js <https://github.com/ethereum/web3.js>`_.
As of today it has a method called `web3.eth.Contract <https://web3js.readthedocs.io/en/1.0/web3-eth-contract.html#new-contract>`_
to facilitate contract creation.

When a contract is created, its constructor (a function with the same
name as the contract) is executed once.
A constructor is optional. Only one constructor is allowed, and this means
overloading is not supported.

.. index:: constructor;arguments

Internally, constructor arguments are passed :ref:`ABI encoded <ABI>` after the code of
the contract itself, but you do not have to care about this if you use ``web3.js``.

If a contract wants to create another contract, the source code
(and the binary) of the created contract has to be known to the creator.
This means that cyclic creation dependencies are impossible.

::

    pragma solidity ^0.4.22;

    contract OwnedToken {
        // TokenCreator is a contract type that is defined below.
        // It is fine to reference it as long as it is not used
        // to create a new contract.
        TokenCreator creator;
        address owner;
        bytes32 name;

        // This is the constructor which registers the
        // creator and the assigned name.
        constructor(bytes32 _name) public {
            // State variables are accessed via their name
            // and not via e.g. this.owner. This also applies
            // to functions and especially in the constructors,
            // you can only call them like that ("internally"),
            // because the contract itself does not exist yet.
            owner = msg.sender;
            // We do an explicit type conversion from `address`
            // to `TokenCreator` and assume that the type of
            // the calling contract is TokenCreator, there is
            // no real way to check that.
            creator = TokenCreator(msg.sender);
            name = _name;
        }

        function changeName(bytes32 newName) public {
            // Only the creator can alter the name --
            // the comparison is possible since contracts
            // are implicitly convertible to addresses.
            if (msg.sender == address(creator))
                name = newName;
        }

        function transfer(address newOwner) public {
            // Only the current owner can transfer the token.
            if (msg.sender != owner) return;
            // We also want to ask the creator if the transfer
            // is fine. Note that this calls a function of the
            // contract defined below. If the call fails (e.g.
            // due to out-of-gas), the execution here stops
            // immediately.
            if (creator.isTokenTransferOK(owner, newOwner))
                owner = newOwner;
        }
    }

    contract TokenCreator {
        function createToken(bytes32 name)
           public
           returns (OwnedToken tokenAddress)
        {
            // Create a new Token contract and return its address.
            // From the JavaScript side, the return type is simply
            // `address`, as this is the closest type available in
            // the ABI.
            return new OwnedToken(name);
        }

        function changeName(OwnedToken tokenAddress, bytes32 name)  public {
            // Again, the external type of `tokenAddress` is
            // simply `address`.
            tokenAddress.changeName(name);
        }

        function isTokenTransferOK(address currentOwner, address newOwner)
            public
            view
            returns (bool ok)
        {
            // Check some arbitrary condition.
            address tokenAddress = msg.sender;
            return (keccak256(newOwner) & 0xff) == (bytes20(tokenAddress) & 0xff);
        }
    }

.. index:: ! visibility, external, public, private, internal

.. _visibility-and-getters:

**********************
Visibility and Getters
**********************

Since Solidity knows two kinds of function calls (internal
ones that do not create an actual EVM call (also called
a "message call") and external
ones that do), there are four types of visibilities for
functions and state variables.

Functions can be specified as being ``external``,
``public``, ``internal`` or ``private``, where the default is
``public``. For state variables, ``external`` is not possible
and the default is ``internal``.

``external``:
    External functions are part of the contract
    interface, which means they can be called from other contracts and
    via transactions. An external function ``f`` cannot be called
    internally (i.e. ``f()`` does not work, but ``this.f()`` works).
    External functions are sometimes more efficient when
    they receive large arrays of data.

``public``:
    Public functions are part of the contract
    interface and can be either called internally or via
    messages. For public state variables, an automatic getter
    function (see below) is generated.

``internal``:
    Those functions and state variables can only be
    accessed internally (i.e. from within the current contract
    or contracts deriving from it), without using ``this``.

``private``:
    Private functions and state variables are only
    visible for the contract they are defined in and not in
    derived contracts.

.. note::
    Everything that is inside a contract is visible to
    all external observers. Making something ``private``
    only prevents other contracts from accessing and modifying
    the information, but it will still be visible to the
    whole world outside of the blockchain.

The visibility specifier is given after the type for
state variables and between parameter list and
return parameter list for functions.

::

    pragma solidity ^0.4.16;

    contract C {
        function f(uint a) private pure returns (uint b) { return a + 1; }
        function setData(uint a) internal { data = a; }
        uint public data;
    }

In the following example, ``D``, can call ``c.getData()`` to retrieve the value of
``data`` in state storage, but is not able to call ``f``. Contract ``E`` is derived from
``C`` and, thus, can call ``compute``.

::

    // This will not compile

    pragma solidity ^0.4.0;

    contract C {
        uint private data;

        function f(uint a) private returns(uint b) { return a + 1; }
        function setData(uint a) public { data = a; }
        function getData() public returns(uint) { return data; }
        function compute(uint a, uint b) internal returns (uint) { return a+b; }
    }

    contract D {
        function readData() public {
            C c = new C();
            uint local = c.f(7); // error: member `f` is not visible
            c.setData(3);
            local = c.getData();
            local = c.compute(3, 5); // error: member `compute` is not visible
        }
    }

    contract E is C {
        function g() public {
            C c = new C();
            uint val = compute(3, 5); // access to internal member (from derived to parent contract)
        }
    }

.. index:: ! getter;function, ! function;getter
.. _getter-functions:

Getter Functions
================

The compiler automatically creates getter functions for
all **public** state variables. For the contract given below, the compiler will
generate a function called ``data`` that does not take any
arguments and returns a ``uint``, the value of the state
variable ``data``. The initialization of state variables can
be done at declaration.

::

    pragma solidity ^0.4.0;

    contract C {
        uint public data = 42;
    }

    contract Caller {
        C c = new C();
        function f() public {
            uint local = c.data();
        }
    }

The getter functions have external visibility. If the
symbol is accessed internally (i.e. without ``this.``),
it is evaluated as a state variable.  If it is accessed externally
(i.e. with ``this.``), it is evaluated as a function.

::

    pragma solidity ^0.4.0;

    contract C {
        uint public data;
        function x() public {
            data = 3; // internal access
            uint val = this.data(); // external access
        }
    }

The next example is a bit more complex:

::

    pragma solidity ^0.4.0;

    contract Complex {
        struct Data {
            uint a;
            bytes3 b;
            mapping (uint => uint) map;
        }
        mapping (uint => mapping(bool => Data[])) public data;
    }

It will generate a function of the following form::

    function data(uint arg1, bool arg2, uint arg3) public returns (uint a, bytes3 b) {
        a = data[arg1][arg2][arg3].a;
        b = data[arg1][arg2][arg3].b;
    }

Note that the mapping in the struct is omitted because there
is no good way to provide the key for the mapping.

.. index:: ! function;modifier

.. _modifiers:

******************
Function Modifiers
******************

Modifiers can be used to easily change the behaviour of functions.  For example,
they can automatically check a condition prior to executing the function. Modifiers are
inheritable properties of contracts and may be overridden by derived contracts.

::

    pragma solidity ^0.4.22;

    contract owned {
        function owned() public { owner = msg.sender; }
        address owner;

        // This contract only defines a modifier but does not use
        // it: it will be used in derived contracts.
        // The function body is inserted where the special symbol
        // `_;` in the definition of a modifier appears.
        // This means that if the owner calls this function, the
        // function is executed and otherwise, an exception is
        // thrown.
        modifier onlyOwner {
            require(
                msg.sender == owner,
                "Only owner can call this function."
            );
            _;
        }
    }

    contract mortal is owned {
        // This contract inherits the `onlyOwner` modifier from
        // `owned` and applies it to the `close` function, which
        // causes that calls to `close` only have an effect if
        // they are made by the stored owner.
        function close() public onlyOwner {
            selfdestruct(owner);
        }
    }

    contract priced {
        // Modifiers can receive arguments:
        modifier costs(uint price) {
            if (msg.value >= price) {
                _;
            }
        }
    }

    contract Register is priced, owned {
        mapping (address => bool) registeredAddresses;
        uint price;

        function Register(uint initialPrice) public { price = initialPrice; }

        // It is important to also provide the
        // `payable` keyword here, otherwise the function will
        // automatically reject all Ether sent to it.
        function register() public payable costs(price) {
            registeredAddresses[msg.sender] = true;
        }

        function changePrice(uint _price) public onlyOwner {
            price = _price;
        }
    }

    contract Mutex {
        bool locked;
        modifier noReentrancy() {
            require(
                !locked,
                "Reentrant call."
            );
            locked = true;
            _;
            locked = false;
        }

        /// This function is protected by a mutex, which means that
        /// reentrant calls from within `msg.sender.call` cannot call `f` again.
        /// The `return 7` statement assigns 7 to the return value but still
        /// executes the statement `locked = false` in the modifier.
        function f() public noReentrancy returns (uint) {
            require(msg.sender.call());
            return 7;
        }
    }

Multiple modifiers are applied to a function by specifying them in a
whitespace-separated list and are evaluated in the order presented.

.. warning::
    In an earlier version of Solidity, ``return`` statements in functions
    having modifiers behaved differently.

Explicit returns from a modifier or function body only leave the current
modifier or function body. Return variables are assigned and
control flow continues after the "_" in the preceding modifier.

Arbitrary expressions are allowed for modifier arguments and in this context,
all symbols visible from the function are visible in the modifier. Symbols
introduced in the modifier are not visible in the function (as they might
change by overriding).

.. index:: ! constant

************************
Constant State Variables
************************

State variables can be declared as ``constant``. In this case, they have to be
assigned from an expression which is a constant at compile time. Any expression
that accesses storage, blockchain data (e.g. ``now``, ``this.balance`` or
``block.number``) or
execution data (``msg.value`` or ``gasleft()``) or make calls to external contracts are disallowed. Expressions
that might have a side-effect on memory allocation are allowed, but those that
might have a side-effect on other memory objects are not. The built-in functions
``keccak256``, ``sha256``, ``ripemd160``, ``ecrecover``, ``addmod`` and ``mulmod``
are allowed (even though they do call external contracts).

The reason behind allowing side-effects on the memory allocator is that it
should be possible to construct complex objects like e.g. lookup-tables.
This feature is not yet fully usable.

The compiler does not reserve a storage slot for these variables, and every occurrence is
replaced by the respective constant expression (which might be computed to a single value by the optimizer).

Not all types for constants are implemented at this time. The only supported types are
value types and strings.

::

    pragma solidity ^0.4.0;

    contract C {
        uint constant x = 32**22 + 8;
        string constant text = "abc";
        bytes32 constant myHash = keccak256("abc");
    }

.. index:: ! functions

.. _functions:

*********
Functions
*********

.. index:: ! view function, function;view

.. _view-functions:

View Functions
==============

Functions can be declared ``view`` in which case they promise not to modify the state.

The following statements are considered modifying the state:

#. Writing to state variables.
#. :ref:`Emitting events <events>`.
#. :ref:`Creating other contracts <creating-contracts>`.
#. Using ``selfdestruct``.
#. Sending Ether via calls.
#. Calling any function not marked ``view`` or ``pure``.
#. Using low-level calls.
#. Using inline assembly that contains certain opcodes.

::

    pragma solidity ^0.4.16;

    contract C {
        function f(uint a, uint b) public view returns (uint) {
            return a * (b + 42) + now;
        }
    }

.. note::
  ``constant`` on functions is an alias to ``view``, but this is deprecated and is planned to be dropped in version 0.5.0.

.. note::
  Getter methods are marked ``view``.

.. note::
  If invalid explicit type conversions are used, state modifications are possible
  even though a ``view`` function was called.
  You can switch the compiler to use ``STATICCALL`` when calling such functions and thus
  prevent modifications to the state on the level of the EVM by adding
  ``pragma experimental "v0.5.0";``

.. warning::
  The compiler does not enforce yet that a ``view`` method is not modifying state. It raises a warning though.

.. index:: ! pure function, function;pure

.. _pure-functions:

Pure Functions
==============

Functions can be declared ``pure`` in which case they promise not to read from or modify the state.

In addition to the list of state modifying statements explained above, the following are considered reading from the state:

#. Reading from state variables.
#. Accessing ``this.balance`` or ``<address>.balance``.
#. Accessing any of the members of ``block``, ``tx``, ``msg`` (with the exception of ``msg.sig`` and ``msg.data``).
#. Calling any function not marked ``pure``.
#. Using inline assembly that contains certain opcodes.

::

    pragma solidity ^0.4.16;

    contract C {
        function f(uint a, uint b) public pure returns (uint) {
            return a * (b + 42);
        }
    }

.. note::
  If invalid explicit type conversions are used, state modifications are possible
  even though a ``pure`` function was called.
  You can switch the compiler to use ``STATICCALL`` when calling such functions and thus
  prevent modifications to the state on the level of the EVM by adding
  ``pragma experimental "v0.5.0";``

.. warning::
  It is not possible to prevent functions from reading the state at the level
  of the EVM, it is only possible to prevent them from writing to the state
  (i.e. only ``view`` can be enforced at the EVM level, ``pure`` can not).

.. warning::
  Before version 0.4.17 the compiler didn't enforce that ``pure`` is not reading the state.

.. index:: ! fallback function, function;fallback

.. _fallback-function:

Fallback Function
=================

A contract can have exactly one unnamed function. This function cannot have
arguments and cannot return anything.
It is executed on a call to the contract if none of the other
functions match the given function identifier (or if no data was supplied at
all).

Furthermore, this function is executed whenever the contract receives plain
Ether (without data). Additionally, in order to receive Ether, the fallback function
must be marked ``payable``. If no such function exists, the contract cannot receive
Ether through regular transactions.

In the worst case, the fallback function can only rely on 2300 gas being available (for example when send or transfer is used), leaving not much room to perform other operations except basic logging. The following operations will consume more gas than the 2300 gas stipend:

- Writing to storage
- Creating a contract
- Calling an external function which consumes a large amount of gas
- Sending Ether

Like any function, the fallback function can execute complex operations as long as there is enough gas passed on to it.

.. note::
    Even though the fallback function cannot have arguments, one can still use ``msg.data`` to retrieve
    any payload supplied with the call.

.. warning::
    Contracts that receive Ether directly (without a function call, i.e. using ``send`` or ``transfer``)
    but do not define a fallback function
    throw an exception, sending back the Ether (this was different
    before Solidity v0.4.0). So if you want your contract to receive Ether,
    you have to implement a fallback function.

.. warning::
    A contract without a payable fallback function can receive Ether as a recipient of a `coinbase transaction` (aka `miner block reward`)
    or as a destination of a ``selfdestruct``.

    A contract cannot react to such Ether transfers and thus also cannot reject them. This is a design choice of the EVM and Solidity cannot work around it.

    It also means that ``this.balance`` can be higher than the sum of some manual accounting implemented in a contract (i.e. having a counter updated in the fallback function).

::

    pragma solidity ^0.4.0;

    contract Test {
        // This function is called for all messages sent to
        // this contract (there is no other function).
        // Sending Ether to this contract will cause an exception,
        // because the fallback function does not have the `payable`
        // modifier.
        function() public { x = 1; }
        uint x;
    }


    // This contract keeps all Ether sent to it with no way
    // to get it back.
    contract Sink {
        function() public payable { }
    }

    contract Caller {
        function callTest(Test test) public {
            test.call(0xabcdef01); // hash does not exist
            // results in test.x becoming == 1.

            // The following will not compile, but even
            // if someone sends ether to that contract,
            // the transaction will fail and reject the
            // Ether.
            //test.send(2 ether);
        }
    }

.. index:: ! overload

.. _overload-function:

Function Overloading
====================

A Contract can have multiple functions of the same name but with different arguments.
This also applies to inherited functions. The following example shows overloading of the
``f`` function in the scope of contract ``A``.

::

    pragma solidity ^0.4.16;

    contract A {
        function f(uint _in) public pure returns (uint out) {
            out = 1;
        }

        function f(uint _in, bytes32 _key) public pure returns (uint out) {
            out = 2;
        }
    }

Overloaded functions are also present in the external interface. It is an error if two
externally visible functions differ by their Solidity types but not by their external types.

::

    // This will not compile
    pragma solidity ^0.4.16;

    contract A {
        function f(B _in) public pure returns (B out) {
            out = _in;
        }

        function f(address _in) public pure returns (address out) {
            out = _in;
        }
    }

    contract B {
    }


Both ``f`` function overloads above end up accepting the address type for the ABI although
they are considered different inside Solidity.

Overload resolution and Argument matching
-----------------------------------------

Overloaded functions are selected by matching the function declarations in the current scope
to the arguments supplied in the function call. Functions are selected as overload candidates
if all arguments can be implicitly converted to the expected types. If there is not exactly one
candidate, resolution fails.

.. note::
    Return parameters are not taken into account for overload resolution.

::

    pragma solidity ^0.4.16;

    contract A {
        function f(uint8 _in) public pure returns (uint8 out) {
            out = _in;
        }

        function f(uint256 _in) public pure returns (uint256 out) {
            out = _in;
        }
    }

Calling ``f(50)`` would create a type error since ``50`` can be implicitly converted both to ``uint8``
and ``uint256`` types. On another hand ``f(256)`` would resolve to ``f(uint256)`` overload as ``256`` cannot be implicitly
converted to ``uint8``.

.. index:: ! event

.. _events:

******
Events
******

Events allow the convenient usage of the EVM logging facilities,
which in turn can be used to "call" JavaScript callbacks in the user interface
of a dapp, which listen for these events.

Events are
inheritable members of contracts. When they are called, they cause the
arguments to be stored in the transaction's log - a special data structure
in the blockchain. These logs are associated with the address of
the contract and will be incorporated into the blockchain
and stay there as long as a block is accessible (forever as of
Frontier and Homestead, but this might change with Serenity). Log and
event data is not accessible from within contracts (not even from
the contract that created them).

SPV proofs for logs are possible, so if an external entity supplies
a contract with such a proof, it can check that the log actually
exists inside the blockchain.  But be aware that block headers have to be supplied because
the contract can only see the last 256 block hashes.

Up to three parameters can
receive the attribute ``indexed`` which will cause the respective arguments
to be searched for: It is possible to filter for specific values of
indexed arguments in the user interface.

If arrays (including ``string`` and ``bytes``) are used as indexed arguments, the
Keccak-256 hash of it is stored as topic instead.

The hash of the signature of the event is one of the topics except if you
declared the event with ``anonymous`` specifier. This means that it is
not possible to filter for specific anonymous events by name.

All non-indexed arguments will be stored in the data part of the log.

.. note::
    Indexed arguments will not be stored themselves.  You can only
    search for the values, but it is impossible to retrieve the
    values themselves.

::

    pragma solidity ^0.4.0;

    contract ClientReceipt {
        event Deposit(
            address indexed _from,
            bytes32 indexed _id,
            uint _value
        );

        function deposit(bytes32 _id) public payable {
            // Events are emitted using `emit`, followed by
            // the name of the event and the arguments
            // (if any) in parentheses. Any such invocation
            // (even deeply nested) can be detected from
            // the JavaScript API by filtering for `Deposit`.
            emit Deposit(msg.sender, _id, msg.value);
        }
    }

The use in the JavaScript API would be as follows:

::

    var abi = /* abi as generated by the compiler */;
    var ClientReceipt = web3.eth.contract(abi);
    var clientReceipt = ClientReceipt.at("0x1234...ab67" /* address */);

    var event = clientReceipt.Deposit();

    // watch for changes
    event.watch(function(error, result){
        // result will contain various information
        // including the argumets given to the `Deposit`
        // call.
        if (!error)
            console.log(result);
    });

    // Or pass a callback to start watching immediately
    var event = clientReceipt.Deposit(function(error, result) {
        if (!error)
            console.log(result);
    });

.. index:: ! log

Low-Level Interface to Logs
===========================

It is also possible to access the low-level interface to the logging
mechanism via the functions ``log0``, ``log1``, ``log2``, ``log3`` and ``log4``.
``logi`` takes ``i + 1`` parameter of type ``bytes32``, where the first
argument will be used for the data part of the log and the others
as topics. The event call above can be performed in the same way as

::

    pragma solidity ^0.4.10;

    contract C {
        function f() public payable {
            bytes32 _id = 0x420042;
            log3(
                bytes32(msg.value),
                bytes32(0x50cb9fe53daa9737b786ab3646f04d0150dc50ef4e75f59509d83667ad5adb20),
                bytes32(msg.sender),
                _id
            );
        }
    }

where the long hexadecimal number is equal to
``keccak256("Deposit(address,bytes32,uint256)")``, the signature of the event.

Additional Resources for Understanding Events
==============================================

- `Javascript documentation <https://github.com/ethereum/wiki/wiki/JavaScript-API#contract-events>`_
- `Example usage of events <https://github.com/debris/smart-exchange/blob/master/lib/contracts/SmartExchange.sol>`_
- `How to access them in js <https://github.com/debris/smart-exchange/blob/master/lib/exchange_transactions.js>`_

.. index:: ! inheritance, ! base class, ! contract;base, ! deriving

***********
Inheritance
***********

Solidity supports multiple inheritance by copying code including polymorphism.

All function calls are virtual, which means that the most derived function
is called, except when the contract name is explicitly given.

When a contract inherits from multiple contracts, only a single
contract is created on the blockchain, and the code from all the base contracts
is copied into the created contract.

The general inheritance system is very similar to
`Python's <https://docs.python.org/3/tutorial/classes.html#inheritance>`_,
especially concerning multiple inheritance.

Details are given in the following example.

::

    pragma solidity ^0.4.22;

    contract owned {
        constructor() { owner = msg.sender; }
        address owner;
    }

    // Use `is` to derive from another contract. Derived
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
        function lookup(uint id) public returns (address adr);
    }

    contract NameReg {
        function register(bytes32 name) public;
        function unregister() public;
     }

    // Multiple inheritance is possible. Note that `owned` is
    // also a base class of `mortal`, yet there is only a single
    // instance of `owned` (as for virtual inheritance in C++).
    contract named is owned, mortal {
        constructor(bytes32 name) {
            Config config = Config(0xD5f9D8D94886E70b06E474c3fB14Fd43E2f23970);
            NameReg(config.lookup(1)).register(name);
        }

        // Functions can be overridden by another function with the same name and
        // the same number/types of inputs.  If the overriding function has different
        // types of output parameters, that causes an error.
        // Both local and message-based function calls take these overrides
        // into account.
        function kill() public {
            if (msg.sender == owner) {
                Config config = Config(0xD5f9D8D94886E70b06E474c3fB14Fd43E2f23970);
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
       function updateInfo(uint newInfo) public {
          if (msg.sender == owner) info = newInfo;
       }

       function get() public view returns(uint r) { return info; }

       uint info;
    }

Note that above, we call ``mortal.kill()`` to "forward" the
destruction request. The way this is done is problematic, as
seen in the following example::

    pragma solidity ^0.4.22;

    contract owned {
        constructor() public { owner = msg.sender; }
        address owner;
    }

    contract mortal is owned {
        function kill() public {
            if (msg.sender == owner) selfdestruct(owner);
        }
    }

    contract Base1 is mortal {
        function kill() public { /* do cleanup 1 */ mortal.kill(); }
    }

    contract Base2 is mortal {
        function kill() public { /* do cleanup 2 */ mortal.kill(); }
    }

    contract Final is Base1, Base2 {
    }

A call to ``Final.kill()`` will call ``Base2.kill`` as the most
derived override, but this function will bypass
``Base1.kill``, basically because it does not even know about
``Base1``.  The way around this is to use ``super``::

    pragma solidity ^0.4.22;

    contract owned {
        constructor() public { owner = msg.sender; }
        address owner;
    }

    contract mortal is owned {
        function kill() public {
            if (msg.sender == owner) selfdestruct(owner);
        }
    }

    contract Base1 is mortal {
        function kill() public { /* do cleanup 1 */ super.kill(); }
    }


    contract Base2 is mortal {
        function kill() public { /* do cleanup 2 */ super.kill(); }
    }

    contract Final is Base1, Base2 {
    }

If ``Base2`` calls a function of ``super``, it does not simply
call this function on one of its base contracts.  Rather, it
calls this function on the next base contract in the final
inheritance graph, so it will call ``Base1.kill()`` (note that
the final inheritance sequence is -- starting with the most
derived contract: Final, Base2, Base1, mortal, owned).
The actual function that is called when using super is
not known in the context of the class where it is used,
although its type is known. This is similar for ordinary
virtual method lookup.

.. index:: ! constructor

Constructors
============
A constructor is an optional function declared with the ``constructor`` keyword which is executed upon contract creation.
Constructor functions can be either ``public`` or ``internal``. If there is no constructor, the contract will assume the
default constructor: ``contructor() public {}``.


::

    pragma solidity ^0.4.22;

    contract A {
        uint public a;

        constructor(uint _a) internal {
            a = _a;
        }
    }

    contract B is A(1) {
        constructor() public {}
    }

A constructor set as ``internal`` causes the contract to be marked as :ref:`abstract <abstract-contract>`.

.. note ::
    Prior to version 0.4.22, constructors were defined as functions with the same name as the contract. This syntax is now deprecated.

::

    pragma solidity ^0.4.11;

    contract A {
        uint public a;

        function A(uint _a) internal {
            a = _a;
        }
    }

    contract B is A(1) {
        function B() public {}
    }


.. index:: ! base;constructor

Arguments for Base Constructors
===============================

Derived contracts need to provide all arguments needed for
the base constructors. This can be done in two ways::

    pragma solidity ^0.4.22;

    contract Base {
        uint x;
        constructor(uint _x) public { x = _x; }
    }

    contract Derived1 is Base(7) {
        constructor(uint _y) public {}
    }

    contract Derived2 is Base {
        constructor(uint _y) Base(_y * _y) public {}
    }

One way is directly in the inheritance list (``is Base(7)``).  The other is in
the way a modifier would be invoked as part of the header of
the derived constructor (``Base(_y * _y)``). The first way to
do it is more convenient if the constructor argument is a
constant and defines the behaviour of the contract or
describes it. The second way has to be used if the
constructor arguments of the base depend on those of the
derived contract. Arguments have to be given either in the
inheritance list or in modifier-style in the derived constuctor.
Specifying arguments in both places is an error.

If the constructor of a base contract has no arguments, it will be implicitly
executed upon contract creation.

.. index:: ! inheritance;multiple, ! linearization, ! C3 linearization

Multiple Inheritance and Linearization
======================================

Languages that allow multiple inheritance have to deal with
several problems.  One is the `Diamond Problem <https://en.wikipedia.org/wiki/Multiple_inheritance#The_diamond_problem>`_.
Solidity is similar to Python in that it uses "`C3 Linearization <https://en.wikipedia.org/wiki/C3_linearization>`_"
to force a specific order in the DAG of base classes. This
results in the desirable property of monotonicity but
disallows some inheritance graphs. Especially, the order in
which the base classes are given in the ``is`` directive is
important: You have to list the direct base contracts
in the order from "most base-like" to "most derived".
Note that this order is different from the one used in Python.
In the following code, Solidity will give the
error "Linearization of inheritance graph impossible".

::

    // This will not compile

    pragma solidity ^0.4.0;

    contract X {}
    contract A is X {}
    contract C is A, X {}

The reason for this is that ``C`` requests ``X`` to override ``A``
(by specifying ``A, X`` in this order), but ``A`` itself
requests to override ``X``, which is a contradiction that
cannot be resolved.

Inheriting Different Kinds of Members of the Same Name
======================================================

When the inheritance results in a contract with a function and a modifier of the same name, it is considered as an error.
This error is produced also by an event and a modifier of the same name, and a function and an event of the same name.
As an exception, a state variable getter can override a public function.

.. index:: ! contract;abstract, ! abstract contract

.. _abstract-contract:

******************
Abstract Contracts
******************

Contracts are marked as abstract when at least one of their functions lacks an implementation as in the following example (note that the function declaration header is terminated by ``;``)::

    pragma solidity ^0.4.0;

    contract Feline {
        function utterance() public returns (bytes32);
    }

Such contracts cannot be compiled (even if they contain implemented functions alongside non-implemented functions), but they can be used as base contracts::

    pragma solidity ^0.4.0;

    contract Feline {
        function utterance() public returns (bytes32);
    }

    contract Cat is Feline {
        function utterance() public returns (bytes32) { return "miaow"; }
    }

If a contract inherits from an abstract contract and does not implement all non-implemented functions by overriding, it will itself be abstract.

Note that a function without implementation is different from a :ref:`Function Type <function_types>` even though their syntax looks very similar.

Example of function without implementation (a function declaration)::

    function foo(address) external returns (address);

Example of a Function Type (a variable declaration, where the variable is of type ``function``)::

    function(address) external returns (address) foo;

Abstract contracts decouple the definition of a contract from its implementation providing better extensibility and self-documentation and
facilitating patterns like the `Template method <https://en.wikipedia.org/wiki/Template_method_pattern>`_ and removing code duplication.

.. index:: ! contract;interface, ! interface contract

**********
Interfaces
**********

Interfaces are similar to abstract contracts, but they cannot have any functions implemented. There are further restrictions:

#. Cannot inherit other contracts or interfaces.
#. Cannot define constructor.
#. Cannot define variables.
#. Cannot define structs.
#. Cannot define enums.

Some of these restrictions might be lifted in the future.

Interfaces are basically limited to what the Contract ABI can represent, and the conversion between the ABI and
an Interface should be possible without any information loss.

Interfaces are denoted by their own keyword:

::

    pragma solidity ^0.4.11;

    interface Token {
        function transfer(address recipient, uint amount) public;
    }

Contracts can inherit interfaces as they would inherit other contracts.

.. index:: ! library, callcode, delegatecall

.. _libraries:

************
Libraries
************

Libraries are similar to contracts, but their purpose is that they are deployed
only once at a specific address and their code is reused using the ``DELEGATECALL``
(``CALLCODE`` until Homestead)
feature of the EVM. This means that if library functions are called, their code
is executed in the context of the calling contract, i.e. ``this`` points to the
calling contract, and especially the storage from the calling contract can be
accessed. As a library is an isolated piece of source code, it can only access
state variables of the calling contract if they are explicitly supplied (it
would have no way to name them, otherwise). Library functions can only be
called directly (i.e. without the use of ``DELEGATECALL``) if they do not modify
the state (i.e. if they are ``view`` or ``pure`` functions),
because libraries are assumed to be stateless. In particular, it is
not possible to destroy a library unless Solidity's type system is circumvented.

Libraries can be seen as implicit base contracts of the contracts that use them.
They will not be explicitly visible in the inheritance hierarchy, but calls
to library functions look just like calls to functions of explicit base
contracts (``L.f()`` if ``L`` is the name of the library). Furthermore,
``internal`` functions of libraries are visible in all contracts, just as
if the library were a base contract. Of course, calls to internal functions
use the internal calling convention, which means that all internal types
can be passed and memory types will be passed by reference and not copied.
To realize this in the EVM, code of internal library functions
and all functions called from therein will at compile time be pulled into the calling
contract, and a regular ``JUMP`` call will be used instead of a ``DELEGATECALL``.

.. index:: using for, set

The following example illustrates how to use libraries (but
be sure to check out :ref:`using for <using-for>` for a
more advanced example to implement a set).

::

    pragma solidity ^0.4.22;

    library Set {
      // We define a new struct datatype that will be used to
      // hold its data in the calling contract.
      struct Data { mapping(uint => bool) flags; }

      // Note that the first parameter is of type "storage
      // reference" and thus only its storage address and not
      // its contents is passed as part of the call.  This is a
      // special feature of library functions.  It is idiomatic
      // to call the first parameter `self`, if the function can
      // be seen as a method of that object.
      function insert(Data storage self, uint value)
          public
          returns (bool)
      {
          if (self.flags[value])
              return false; // already there
          self.flags[value] = true;
          return true;
      }

      function remove(Data storage self, uint value)
          public
          returns (bool)
      {
          if (!self.flags[value])
              return false; // not there
          self.flags[value] = false;
          return true;
      }

      function contains(Data storage self, uint value)
          public
          view
          returns (bool)
      {
          return self.flags[value];
      }
    }

    contract C {
        Set.Data knownValues;

        function register(uint value) public {
            // The library functions can be called without a
            // specific instance of the library, since the
            // "instance" will be the current contract.
            require(Set.insert(knownValues, value));
        }
        // In this contract, we can also directly access knownValues.flags, if we want.
    }

Of course, you do not have to follow this way to use
libraries: they can also be used without defining struct
data types. Functions also work without any storage
reference parameters, and they can have multiple storage reference
parameters and in any position.

The calls to ``Set.contains``, ``Set.insert`` and ``Set.remove``
are all compiled as calls (``DELEGATECALL``) to an external
contract/library. If you use libraries, take care that an
actual external function call is performed.
``msg.sender``, ``msg.value`` and ``this`` will retain their values
in this call, though (prior to Homestead, because of the use of ``CALLCODE``, ``msg.sender`` and
``msg.value`` changed, though).

The following example shows how to use memory types and
internal functions in libraries in order to implement
custom types without the overhead of external function calls:

::

    pragma solidity ^0.4.16;

    library BigInt {
        struct bigint {
            uint[] limbs;
        }

        function fromUint(uint x) internal pure returns (bigint r) {
            r.limbs = new uint[](1);
            r.limbs[0] = x;
        }

        function add(bigint _a, bigint _b) internal pure returns (bigint r) {
            r.limbs = new uint[](max(_a.limbs.length, _b.limbs.length));
            uint carry = 0;
            for (uint i = 0; i < r.limbs.length; ++i) {
                uint a = limb(_a, i);
                uint b = limb(_b, i);
                r.limbs[i] = a + b + carry;
                if (a + b < a || (a + b == uint(-1) && carry > 0))
                    carry = 1;
                else
                    carry = 0;
            }
            if (carry > 0) {
                // too bad, we have to add a limb
                uint[] memory newLimbs = new uint[](r.limbs.length + 1);
                for (i = 0; i < r.limbs.length; ++i)
                    newLimbs[i] = r.limbs[i];
                newLimbs[i] = carry;
                r.limbs = newLimbs;
            }
        }

        function limb(bigint _a, uint _limb) internal pure returns (uint) {
            return _limb < _a.limbs.length ? _a.limbs[_limb] : 0;
        }

        function max(uint a, uint b) private pure returns (uint) {
            return a > b ? a : b;
        }
    }

    contract C {
        using BigInt for BigInt.bigint;

        function f() public pure {
            var x = BigInt.fromUint(7);
            var y = BigInt.fromUint(uint(-1));
            var z = x.add(y);
        }
    }

As the compiler cannot know where the library will be
deployed at, these addresses have to be filled into the
final bytecode by a linker
(see :ref:`commandline-compiler` for how to use the
commandline compiler for linking). If the addresses are not
given as arguments to the compiler, the compiled hex code
will contain placeholders of the form ``__Set______`` (where
``Set`` is the name of the library). The address can be filled
manually by replacing all those 40 symbols by the hex
encoding of the address of the library contract.

Restrictions for libraries in comparison to contracts:

- No state variables
- Cannot inherit nor be inherited
- Cannot receive Ether

(These might be lifted at a later point.)

Call Protection For Libraries
=============================

As mentioned in the introduction, if a library's code is executed
using a ``CALL`` instead of a ``DELEGATECALL`` or ``CALLCODE``,
it will revert unless a ``view`` or ``pure`` function is called.

The EVM does not provide a direct way for a contract to detect
whether it was called using ``CALL`` or not, but a contract
can use the ``ADDRESS`` opcode to find out "where" it is
currently running. The generated code compares this address
to the address used at construction time to determine the mode
of calling.

More specifically, the runtime code of a library always starts
with a push instruction, which is a zero of 20 bytes at
compilation time. When the deploy code runs, this constant
is replaced in memory by the current address and this
modified code is stored in the contract. At runtime,
this causes the deploy time address to be the first
constant to be pushed onto the stack and the dispatcher
code compares the current address against this constant
for any non-view and non-pure function.

.. index:: ! using for, library

.. _using-for:

*********
Using For
*********

The directive ``using A for B;`` can be used to attach library
functions (from the library ``A``) to any type (``B``).
These functions will receive the object they are called on
as their first parameter (like the ``self`` variable in
Python).

The effect of ``using A for *;`` is that the functions from
the library ``A`` are attached to any type.

In both situations, all functions, even those where the
type of the first parameter does not match the type of
the object, are attached. The type is checked at the
point the function is called and function overload
resolution is performed.

The ``using A for B;`` directive is active for the current
scope, which is limited to a contract for now but will
be lifted to the global scope later, so that by including
a module, its data types including library functions are
available without having to add further code.

Let us rewrite the set example from the
:ref:`libraries` in this way::

    pragma solidity ^0.4.16;

    // This is the same code as before, just without comments
    library Set {
      struct Data { mapping(uint => bool) flags; }

      function insert(Data storage self, uint value)
          public
          returns (bool)
      {
          if (self.flags[value])
            return false; // already there
          self.flags[value] = true;
          return true;
      }

      function remove(Data storage self, uint value)
          public
          returns (bool)
      {
          if (!self.flags[value])
              return false; // not there
          self.flags[value] = false;
          return true;
      }

      function contains(Data storage self, uint value)
          public
          view
          returns (bool)
      {
          return self.flags[value];
      }
    }

    contract C {
        using Set for Set.Data; // this is the crucial change
        Set.Data knownValues;

        function register(uint value) public {
            // Here, all variables of type Set.Data have
            // corresponding member functions.
            // The following function call is identical to
            // `Set.insert(knownValues, value)`
            require(knownValues.insert(value));
        }
    }

It is also possible to extend elementary types in that way::

    pragma solidity ^0.4.16;

    library Search {
        function indexOf(uint[] storage self, uint value)
            public
            view
            returns (uint)
        {
            for (uint i = 0; i < self.length; i++)
                if (self[i] == value) return i;
            return uint(-1);
        }
    }

    contract C {
        using Search for uint[];
        uint[] data;

        function append(uint value) public {
            data.push(value);
        }

        function replace(uint _old, uint _new) public {
            // This performs the library function call
            uint index = data.indexOf(_old);
            if (index == uint(-1))
                data.push(_new);
            else
                data[index] = _new;
        }
    }

Note that all library calls are actual EVM function calls. This means that
if you pass memory or value types, a copy will be performed, even of the
``self`` variable. The only situation where no copy will be performed
is when storage reference variables are used.
